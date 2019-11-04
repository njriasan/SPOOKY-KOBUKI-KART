#!/usr/bin/env python3
import struct
import time
import keyboard
from getpass import getpass
from bluepy.btle import Peripheral, DefaultDelegate
import argparse

parser = argparse.ArgumentParser(description='Print advertisement data from a BLE device')
parser.add_argument('addr', metavar='A', type=str, help='Address of the form XX:XX:XX:XX:XX:XX')
args = parser.parse_args()
addr = args.addr.lower()
if len(addr) != 17:
    raise ValueError("Invalid address supplied")

SERVICE_UUID = "4607eda0-f65e-4d59-a9ff-84420d87a4ca"
CHAR_UUIDS = [
    "4607eda1-f65e-4d59-a9ff-84420d87a4ca",
    "4607eda2-f65e-4d59-a9ff-84420d87a4ca",
    "4607eda3-f65e-4d59-a9ff-84420d87a4ca",
    "4607eda4-f65e-4d59-a9ff-84420d87a4ca"
    ] 

class RobotController():

    def __init__(self, address):

        self.robot = Peripheral(addr)
        print("connected")

        self.buttons_on = {0x01: "up-right", 0x02: "right", 0x03: "down-right", 0x05: "down-left",
                        0x06: "left", 0x07: "up-left", 0x08: "accelerate", 0x09: "decelerate"}
        self.buttons_off = {0x010: "up-right", 0x020: "right", 0x030: "down-right", 0x050: "down-left",
                        0x060: "left", 0x070: "up-left", 0x080: "accelerate", 0x090: "decelerate"}
                        
        # keep state for keypresses
        # these are in order of "byte" that gets updated from 0 -> 8
        # controller is oriented sideways (we used left contoller for testing)
        self.pressed = {"right": False, "down-right": False, "decelerate": False, "down-left": False,
                        "left": False, "up-left": False, "accelerate": False, "up-right": False, 
                        "stationary": False}
        # TODO get service from robot
        self.sv = self.robot.getServiceByUUID(SERVICE_UUID)

        self.characteristics = {}
        self.characteristics["right"] = self.sv.getCharacteristics(CHAR_UUIDS[0])[0]
        self.characteristics["down-right"] = self.sv.getCharacteristics(CHAR_UUIDS[0])[0]
        self.characteristics["up-right"] = self.sv.getCharacteristics(CHAR_UUIDS[0])[0]

        self.characteristics["left"] = self.sv.getCharacteristics(CHAR_UUIDS[1])[0]
        self.characteristics["down-left"] = self.sv.getCharacteristics(CHAR_UUIDS[1])[0]
        self.characteristics["up-left"] = self.sv.getCharacteristics(CHAR_UUIDS[1])[0]

        # not including up or down since they shouldn't mean anything wrt direction

        self.characteristics["accelerate"] = self.sv.getCharacteristics(CHAR_UUIDS[2])[0]

        self.characteristics["decelerate"] = self.sv.getCharacteristics(CHAR_UUIDS[3])[0]

        # TODO get characteristic handles from service/robot

        # keyboard.hook(self.on_key_event)

        # PUT SOCKET LISTENING CODE HERE
        # will call on_pkt_receive


    def on_pkt_receive(self, pkt):

        # A13F 0000 0000 8000 8000 8000 80 -> up
        # A13F 0000 0100 8000 8000 8000 80 -> up-right
        # A13F 0000 0200 8000 8000 8000 80 -> right 
        # A13F 0000 0300 8000 8000 8000 80 -> down-right 
        # A13F 0000 0400 8000 8000 8000 80 -> down 
        # A13F 0000 0500 8000 8000 8000 80 -> down-left 
        # A13F 0000 0600 8000 8000 8000 80 -> left 
        # A13F 0000 0700 8000 8000 8000 80 -> up-left 

        # A13F 0000 0800 8000 8000 8000 80 -> stationary 

        # A13F 0100 0800 8000 8000 8000 80 -> A
        # A13F 0200 0800 8000 8000 8000 80 -> X
        # A13F 0400 0800 8000 8000 8000 80 -> B
        # A13F 0800 0800 8000 8000 8000 80 -> Y

        # extract appropriate bytes in packet
        if pkt is in self.buttons_on[pkt]:
            button = self.buttons_on[pkt]
            self.pressed[button] = True
            self.characteristics[button].write(bytes([self.pressed[button]]))
        elif pkt is in self.buttons_off[pkt]:
            button = self.buttons_off[pkt]
            self.pressed[button] = False
            self.characteristics[button].write(bytes([self.pressed[button]]))
        else:
            return

    def __enter__(self):
        return self
    def __exit__(self, exc_type, exc_value, traceback):
        self.robot.disconnect()

with RobotController(addr) as robot:
    getpass('Use arrow keys to control robot')
