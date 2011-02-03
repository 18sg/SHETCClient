#include "shet.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int recursive = 0;
int complete = 0;
int show_type = 0;

char *path = "/";

char *dir_part;
char *file_part;

int retval = EXIT_SUCCESS;


// Print the value to stderr and exit with EXIT_FAILURE.
void error_cb(shet_state *state, struct json_object *val, void *data)
{
	fprintf(stderr, "%s\n", json_object_to_json_string(val));
	retval = EXIT_FAILURE;
	shet_exit(state);
}

// Print a listing in a sensible way, and  exit.
void ls_cb(shet_state *state, struct json_object *val, void *data)
{
	// Print each key, possibly with the value.
	// XXX: Handle recursive listings.
	json_object_object_foreach(val, name, type) {
		if (show_type)
			printf("%s\t%s\n", json_object_get_string(type), name);
		else
			printf("%s\n", name);
	}
	shet_exit(state);
}

// Print tab completions and exit.
void complete_cb(shet_state *state, struct json_object *val, void *data)
{
	json_object_object_foreach(val, name, json_type) {
		// If it starts with the file name:
		if (strstr(name, file_part) == name) {
			// Extract the type.
			const char *type = json_object_get_string(json_type);
			
			// Print it with a slash after the name if it's a directory.
			if (strcmp(type, "dir") == 0)
				printf("%s/%s/\n", dir_part, name);
			else
				printf("%s/%s\n", dir_part, name);
		}
	}
	shet_exit(state);
}


int main(int argc, char *argv[])
{
	// Parse the arguments.
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (strcmp(argv[i], "-r") == 0)
				recursive = 1;
			else if (strcmp(argv[i], "-c") == 0)
				complete = 1;
			else if (strcmp(argv[i], "-l") == 0)
				show_type = 1;
		} else {
			path = argv[i];
			break;
		}
	}
	
	// Initialise and commect to shet.
	shet_state *state = shet_new_state();
	shet_set_error_callback(state, error_cb, NULL);
	shet_connect_default(state);
	
	// We only malloc this sometimes :/
	int dir_part_malloced = 0;
	
	if (complete) {
		// Find the last slash.
		char *last_slash = strrchr(path, '/');
		
		if (last_slash != NULL) {
			// Extract up to the slash.
			dir_part = malloc(((last_slash - path) + 1) * sizeof(char));
			dir_part_malloced = 1;
			memcpy(dir_part, path, last_slash - path);
			// Give it a proper ending.
			dir_part[(last_slash - path)] = '\0';
			
			// The part after the last slash.
			file_part = last_slash + 1;
		} else {
			// Otherwise, search in the root.
			dir_part = "";
			file_part = path;
		}
		
		// finally, call ls on this directory.
		struct json_object *args = json_object_new_array();
		json_object_array_add(args, json_object_new_string(dir_part));
		
		shet_call_action(state, "/meta/ls", args, complete_cb, NULL);
			
		json_object_put(args);
	} else {
		// Otherwise, call ls or ls-r.
		struct json_object *args = json_object_new_array();
		json_object_array_add(args, json_object_new_string(path));
		
		char *action = recursive ? "/meta/ls-r" : "/meta/ls";
		shet_call_action(state, action, args, ls_cb, NULL);
		
		json_object_put(args);
	}
	
	// Loop, free and exit.
	shet_loop(state);
	
	if (dir_part_malloced)
		free(dir_part);
	
	shet_free_state(state);
	return retval;
}
