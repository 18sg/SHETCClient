#include "shet.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#define DPRINTF(...) fprintf(stderr, __VA_ARGS__)
#else
#define DPRINTF(...)
#endif

// The application state to be passed between callbacks.
typedef struct {
	char *path;
	struct json_object *args;
	shet_state *state;
	int return_val;
} app_state;

// Parse the command line args into an array.
// Try each arg first as a JSON value, then a string.
struct json_object *parse_args(int argc, char **argv)
{
	json_object *arg_list = json_object_new_array();
	
	for (int i = 0; i < argc; i++) {
		json_object *parsed = json_tokener_parse(argv[i]);
		if (is_error(parsed))
			parsed = json_object_new_string(argv[i]);
		json_object_array_add(arg_list, parsed);
	}
	
	return arg_list;
}

// Print the value and exit successfully.
void print_exit_cb(shet_state *state, struct json_object *val, void *my_state)
{
	printf("%s\n", json_object_to_json_string(val));
	shet_exit(state);
}

// Print the value.
void print_cb(shet_state *state, struct json_object *val, void *my_state)
{
	printf("%s\n", json_object_to_json_string(val));
}

// Print the value to stderr and exit with EXIT_FAILURE.
void error_cb(shet_state *state, struct json_object *val, void *my_state)
{
	fprintf(stderr, "%s\n", json_object_to_json_string(val));
	((app_state *)my_state)->return_val = EXIT_FAILURE;
	shet_exit(state);
}

// Call an action.
void do_action(app_state *my_state)
{
	shet_call_action(my_state->state, my_state->path, my_state->args, print_exit_cb, (void *)my_state);
}

// Get or set a property.
void do_prop(app_state *my_state)
{
	// If no additional args were supplied, perform a get.
	if (json_object_array_length(my_state->args) == 0)
		shet_get_prop(my_state->state, my_state->path, print_exit_cb, (void *)my_state);
	else {
		// Otherwise set to the first arg.
		if (json_object_array_length(my_state->args) > 1)
			fprintf(stderr, "Ignoring extra args for set.\n");
		
		shet_set_prop(my_state->state, my_state->path, 
		              json_object_array_get_idx(my_state->args, 0),
		              print_exit_cb, (void *)my_state);
	}
}

// Watch an event.
void do_event(app_state *my_state)
{
	// Can't pass args to an event.
	if (json_object_array_length(my_state->args) > 0)
		fprintf(stderr, "Ignoring extra args for event.\n");
	
	// Ignore the created/deleted callbacks for now.
	shet_watch_event(my_state->state, my_state->path, print_cb, NULL, NULL, (void *)my_state);
}

// Called with the type.
// Dispatch above do_* methods as appropriate.
void type_cb(shet_state *state, struct json_object *val, void *my_state)
{
	const char *type_str = json_object_get_string(val);
	
	DPRINTF("type: %s\n", type_str);
	
	if (strcmp(type_str, "action") == 0)
		do_action((app_state *)my_state);
	else if (strcmp(type_str, "prop") == 0)
		do_prop((app_state *)my_state);
	else if (strcmp(type_str, "event") == 0)
		do_event((app_state *)my_state);
	
}

int main(int argc, char *argv[])
{
	// We need at least a path.
	if (argc < 2)
		die("Not enough args.\n");
	
	// The application state to be passed between callbacks.
	app_state *my_state = malloc(sizeof(*my_state));
	
	// Exit successfully by default.
	my_state->return_val = EXIT_SUCCESS;
	
	// Don't accept any args yet.
	my_state->path = argv[1];
	int next_arg = 2;
	
	// Parse the remaining arguments.
	my_state->args = parse_args(argc - next_arg, argv + next_arg);
	
	// Start the SHET client.
	my_state->state = shet_new_state();
	// Call error_cb if there are any errors.
	shet_set_error_callback(my_state->state, error_cb, (void *)my_state);
	shet_connect_default(my_state->state);
	
	// Make the type call to start the whole process.
	// Just pass the path as a single argument.
	struct json_object *type_args = json_object_new_array();
	json_object_array_add(type_args, json_object_new_string(my_state->path));
	
	shet_call_action(my_state->state, "/meta/type", type_args, type_cb, (void *)my_state);
	
	json_object_put(type_args);
	
	// Loop until shet_exit is called.
	shet_loop(my_state->state);
	// Free the shet state.
	shet_free_state(my_state->state);
	
	// Get the return value, free the state, and return.
	int retval = my_state->return_val;
	json_object_put(my_state->args);
	free(my_state);
	return retval;
}
