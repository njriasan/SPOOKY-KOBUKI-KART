#ifndef READ_JOYCON_INPUT_H
#define READ_JOYCON_INPUT_H

/*
 *  function that spawns a server and then places the port number
 *  inside the pointer passed in.
 */
int spawn_server(int* port_ptr);

/*
 * Highest level function for a process that handles all the outputs from the joycon.
 */
int handle_joycon(int write_pipe_fd, char* device_path);
#endif

