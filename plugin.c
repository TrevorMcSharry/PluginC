// Trevor McSharry
// 10/31/2023
// Plugin Lab
// A program that can load and unload plugins (DSO's) during run time. 
// It also implements a simple to use interface.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "vector.h"
// Define a Plugin structure
struct Plugin {
 char name[65]; // Name of the plugin
 int (*init)(void); // Initialization function pointer
 void (*fini)(void); // Finalization function pointer
 int (*cmd)(char *str); // Command function pointer
};
// Define a Record structure to hold plugin information
typedef struct {
 void *handle; // Handle for the dynamic library
 struct Plugin *plugin; // Pointer to the plugin 
} Record;
Vector *vec; // Vector to hold all loaded plugins
// Function to load a plugin
void load(char* plugin_path) {
 // Open the dynamic shared object (DSO)
 void* handle = dlopen(plugin_path, RTLD_NOW);
 if (!handle) {
 fprintf(stderr, "Error: %s\n", dlerror());
 return;
 }
 // Get the init function from the DSO
 struct Plugin *plugPtr = 0;
 plugPtr = (struct Plugin *)dlsym(handle, "export");
 if (!plugPtr){
 fprintf(stderr, "Error: %s\n", dlerror());
 dlclose(handle);
 return;
 }
 // Initialize the plugin
 if (plugPtr->init() != 0) {
 fprintf(stderr, "Error: Plugin initialization failed\n");
 dlclose(handle);
 return;
 }
 // Create a new record for the plugin and add it to the vector
 Record *record = (Record *)malloc(sizeof(Record));
 record->handle = handle;
 record->plugin = plugPtr;
 vector_push(vec, (int64_t)record);
}
// Function to unload a plugin
void unload(char* plugin_name) {
 int i = 0;
 
 // Iterate over all loaded plugins
 for (; i < vector_size(vec); i++) {
 Record *record;
 vector_get(vec, i, (int64_t *)&record);
 
 // If the plugin is found, finalize it and unload it from memory
 if (strcmp(record->plugin->name, plugin_name) == 0) {
 record->plugin->fini();
 dlclose(record->handle);
 free(record);
 break;
 }
 }
 
 // Remove the unloaded plugin from the vector
 vector_remove(vec, i);
}
// Function to list all loaded plugins
void list_plugins() {
 int i;
 
 // If no plugins are loaded, print a message and return
 if(vector_size(vec) == 0){
 printf("0 plugins loaded\n");
 return;
 }
 // Iterate over all loaded plugins and print their names
 for (i = 0; i < vector_size(vec); i++) {
 Record *record;
 vector_get(vec, i, (int64_t *)&record);
 printf("%s\n", record->plugin->name);
 }
 printf("%d plugins loaded.\n", i);
}
// Function to quit the program
void quit(){
 
 // Finalize and unload all plugins before exiting 
 for (int i = 0; i < vector_size(vec); i++) {
 Record *record;
 vector_get(vec, i, (int64_t *)&record);
 record->plugin->fini();
 dlclose(record->handle);
 free(record);
 }
 
 vector_free(vec);
 exit(0);
}
// Function to pass a command to a plugin
void com(char command[512]){
 int i;
 Record *record;
 if(vector_size(vec) == 0){
 return;
 }
 for (i = 0; i < vector_size(vec); i++) {
 int result = vector_get(vec, i, (int64_t *)&record);
 if (result != 0) {
 printf("%s got command %s\n", record->plugin->name, command);
 record->plugin->cmd(command);
 break;
 }
 }
}
// Main function of the program
int main() {
 char command[512];
 vec = vector_new();
 
 while (1) {
 printf("> ");
 fgets(command, sizeof(command), stdin);
 command[strcspn(command, "\n")] = '\0'; // Remove newline
 if (strcmp(command, "quit") == 0) {
 quit();
 } else if (strncmp(command, "load ", 5) == 0) {
 char* plugin_path = command + 5;
 load(plugin_path);
 } else if (strncmp(command, "unload ", 7) == 0) {
 char* plugin_name = command + 7;
 unload(plugin_name);
 } else if (strcmp(command, "list") == 0 || strcmp(command, "plugins") == 0) 
{
 list_plugins();
 } else {
 com(command);
 }
 }
 return 0;
}