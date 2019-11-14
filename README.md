# Overview

This repo contains all the Kobuki Racing code for the ee149 final project.

While we will attempt in the future to keep our overall vision of our design available on this repo, this readme will only describe implemented portions.

## Process Manager for the Raspberry Pi

Our first challenge is to integrate a JoyCon from the Nintendo switch with our Kobukis. This is problematic because the Buckler only supports BLE, whereas JoyCon only support regular bluetooth. As a result we have opted to run a series of endpoints through a process manager on a Raspberry Pi. Because online documentation suggests that a device can hold a max of 8 bluetooth connections we expect to be able to support at most 4 players in a race easily.

The process manager code relies on the `hidapi` c library, which can be found online [here](https://github.com/libusb/hidapi) and must be added as a shared library from source. This code requires root permission but allows reading/writing to connections to devices connected using the HID bluetooth driver. Our process manager then works by keeping track of a series of MAC addresses for possible switch controllers (perhaps in the future we can explore trying to infer that a device is a switch controller but we do not have support for this yet) and then spawns a separate process (the bluetooth endpoint) for each joycon connected and use `hid_read` to read 12 byte incoming packets whenever a button is pressed.

This process manager should then launch a BLE endpoint python, which will connect with 1 bluetooth endpoint and transfer the packet over TCP sockets. This process will then establish a connection with a buckler and communicate button state over ble characteristics. Currently we do not have support for automatically spawning the BLE endpoint and have implemented just a simple TCP client to transfer the packets over.

### Running the process manager

First make sure you have followed the installation instructions for `hidapi`. 

Next you need to also install `python3` and to run the python script you need to `pip3 install` all necessary dependencies (for example `pybluez`). Note because this runs in `sudo` you may need to `sudo pip3 install` to get the installations on the sudo version.

Then the following series of commands will execute the process manager (which we assume is perminantly running on the Raspberry Pi).

```
$ cd joycon
$ make
$ sudo ./process_manager
```

Note that the last step must be run as a root user or else we cannot access information on hid devices. At this point nothing will occur until a joycon is connected. Once you have connected to a joycon over bluetooth you will see an output that a process has spawned and probably that a python script has launched. You should now be able to view output when a button is pressed.

We do not yet connect to the Kobukis so the mac address can be any string. This should then output connected. At this point is you press buttons on your joycon you should a 12 byte stream print in the terminal in which your client is located.

## BLE Characteristic

For connecting to the Kobuki we will have a single characteristic representing the state of all buttons on the Joy Con. All the buttons on a joycon only occupy 15 bits of a switch message, so we will specify everything in a 2 byte message.

This is the breakdown of the 2 Bytes (calling the MSB of the first byte bit 0 and the LSB of the second byte bit 15).
  *  Bit 0: UNUSED (always 0)
  *  Bit 1: HOME/CAPTURE
  *  Bit 2: LEFT/RIGHT STICK CLICK
  *  Bit 3: +/-
  *  Bits 4-7: STICK PUSH 
  *  Bit 8: SL
  *  Bit 9: SR
  *  Bit 10: R
  *  Bit 11: RZ
  *  Bit 12: X
  *  Bit 13: Y
  *  Bit 14: A
  *  Bit 15: B

All buttons except the stick pushes are toggleable buttons (pressed or not pressed). For all of those buttons a bit value of 1 means pressed and 0 means not pressed.

For the stick push we have following mapping (treating towards buttons as being right) of direction to output values:

  * UP: 0
  * RIGHT-UP: 1
  * RIGHT: 2
  * RIGHT-DOWN: 3
  * DOWN: 4
  * LEFT-DOWN: 5
  * LEFT: 6
  * LEFT-UP: 7
  * NOT PRESSED: 8
  * UNUSED: 9-15

To illustrate this, if the controller currently has the stick pressed in the left direction, and the R, X, and Home buttons are pressed, then the following message in hex will be delivered:

0x4628
