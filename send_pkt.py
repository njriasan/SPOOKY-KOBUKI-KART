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
        if pkt.direction_byte == 0x00:
            # up
            pass
        elif pkt.direction_byte == 0x01:
            # up-right
            self.characteristics["up-right"].write(b'\x01')
        elif pkt.direction_byte == 0x02:
            # right
            self.characteristics["right"].write(b'\x01')
        elif pkt.direction_byte == 0x03:
            # down-right
            self.characteristics["down-right"].write(b'\x01')
        elif pkt.direction_byte == 0x04:
            # down
            pass
        elif pkt.direction_byte == 0x05:
            # down-left
            self.characteristics["down-left"].write(b'\x01')
        elif pkt.direction_byte == 0x06:
            # left
            self.characteristics["left"].write(b'\x01')
        elif pkt.direction_byte == 0x07:
            # up-left
            self.characteristics["up-left"].write(b'\x01')
        else:
            pass

        if pkt.throttle_byte == 0x02:
            self.characteristics["accelerate"].write(b'\x01')
        elif pkt.throttle_byte == 0x04:
            self.characteristics["decelerate"].write(b'\x01')
        else:
            pass



    def on_key_event(self, event):
        # print key name
        print(event.name)
        # if a key unrelated to direction keys is pressed, ignore
        if event.name not in self.pressed: return
        # if a key is pressed down
        if event.event_type == keyboard.KEY_DOWN:
            # if that key is already pressed down, ignore
            if self.pressed[event.name]: return
            # set state of key to pressed
            self.pressed[event.name] = True
            # TODO write to characteristic to change direction
            self.characteristics[event.name].write(b'\x01')
        else:
            # set state of key to released
            self.pressed[event.name] = False
            self.characteristics[event.name].write(b'\x00')
            # TODO write to characteristic to stop moving in this direction

    def __enter__(self):
        return self
    def __exit__(self, exc_type, exc_value, traceback):
        self.robot.disconnect()

with RobotController(addr) as robot:
    getpass('Use arrow keys to control robot')
