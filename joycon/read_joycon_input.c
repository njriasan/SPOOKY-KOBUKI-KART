#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "hidapi/hidapi.h"


// Experimentation showed a msg size of 12 bytes per joycon input
#define MSG_SIZE 12

/*
 * TODO:
 *   1. Add code to spawn a server socket
 *   2. Add code to fork/exec the client process to connect via BLE
 *   3. Send the data over the socket connection as it arrives
 */


int main(int argc, char* argv[])
{
    if (argc != 2) {
        perror ("Usage: ./read_joycon_input <JOYCON_mac_addr>");
        exit (1);
    }
    const char *mac_addr = argv[1];
	int res;

	// Initialize the hidapi library
	res = hid_init();

    // Find all paired devices
    struct hid_device_info *devices = hid_enumerate (0, 0);
    
    struct hid_device_info *device = devices;

    // Convert to wide string type
    wchar_t *wideMac = malloc (strlen (mac_addr) + 1);
    mbsrtowcs (wideMac, &mac_addr, strlen(mac_addr) + 1, NULL);

    // Iterate through all the devices looking for our mac addr
    while (device != NULL) {
        // Check the mac addr
        if (!wcscmp (device->serial_number, wideMac)) {
            // Open the device if it matches
            hid_device *actual_dev = hid_open_path (device->path);
            assert (actual_dev);
            // Load in each message
            unsigned char response[MSG_SIZE];
            int data_len;
            while((data_len = hid_read (actual_dev, response, MSG_SIZE)) > 0) {
                for (int i = 0; i < data_len; i++) {
                    printf ("%x ", response[i]);
                }
                printf ("\n");
            }
            break;
        } else {
            device = device->next;
        }
    }
    printf ("Device not found\n");
    if (devices != NULL) {
        // Free the malloced data
        hid_free_enumeration (devices);
    }

	// Finalize the hidapi library
	res = hid_exit();

	return 1;
}
