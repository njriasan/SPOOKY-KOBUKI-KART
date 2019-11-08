#!/usr/bin/env python3
import sys
import struct
import time
from getpass import getpass
from bluepy.btle import Peripheral, DefaultDelegate
import argparse
import socket

parser = argparse.ArgumentParser(description='Print advertisement data from a BLE device')
parser.add_argument('addr', metavar='A', type=str, help='Address of the form XX:XX:XX:XX:XX:XX')
parser.add_argument('server_port', metavar='P', type=int, help='Port to connect to bluetooth endpoint')
args = parser.parse_args()
addr = args.addr.lower()
server_port = args.server_port
#if len(addr) != 17:
#    raise ValueError("Invalid address supplied")

SERVICE_UUID = "4607eda0-f65e-4d59-a9ff-84420d87a4ca"
CHAR_UUIDS = [
    "4607eda1-f65e-4d59-a9ff-84420d87a4ca",
    "4607eda2-f65e-4d59-a9ff-84420d87a4ca",
    "4607eda3-f65e-4d59-a9ff-84420d87a4ca",
    "4607eda4-f65e-4d59-a9ff-84420d87a4ca"
    ] 

class RobotController():

    def __init__(self, address, server_port):

        # robot refers to buckler, our peripheral
        #self.robot = Peripheral(addr)
        print("connected")
                        
        # keep state for keypresses
        # these are in order of "byte" that gets updated from 0 -> 8
        # controller is oriented sideways (we used left contoller for testing)
        #self.pressed = {"right": False, "down-right": False, "decelerate": False, "down-left": False,
        #                "left": False, "up-left": False, "accelerate": False, "up-right": False, 
        #                "stationary": False}

        # get service from robot
        #self.sv = self.robot.getServiceByUUID(SERVICE_UUID)

        #self.characteristics = {}
        #self.characteristics["right"] = self.sv.getCharacteristics(CHAR_UUIDS[2])[0]
        #self.characteristics["down-right"] = self.sv.getCharacteristics(CHAR_UUIDS[2])[0]
        #self.characteristics["up-right"] = self.sv.getCharacteristics(CHAR_UUIDS[2])[0]

        #self.characteristics["left"] = self.sv.getCharacteristics(CHAR_UUIDS[3])[0]
        #self.characteristics["down-left"] = self.sv.getCharacteristics(CHAR_UUIDS[3])[0]
        #self.characteristics["up-left"] = self.sv.getCharacteristics(CHAR_UUIDS[3])[0]

        # not including up or down since they shouldn't mean anything wrt direction

        # get characteristic handles from service/robot
        #self.characteristics["accelerate"] = self.sv.getCharacteristics(CHAR_UUIDS[0])[0]

        #self.characteristics["decelerate"] = self.sv.getCharacteristics(CHAR_UUIDS[1])[0]

        # PUT SOCKET LISTENING CODE HERE
        # will call on_pkt_receive
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect(('localhost', server_port))

        while True:
            pkt = self.sock.recv(12)
            if (len(pkt) == 0):
                sys.exit (1)
            #self.on_pkt_receive(pkt)
            print (pkt)

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

        # keep a copy of previous button press states
        prev_pressed = self.pressed.copy()

        # set each button to its corresponding pkt bit
        #self.pressed["up-right"] = (bool) pkt & (0x1 << 1)
        #self.pressed["right"] = (bool) pkt & (0x1 << 1)
        #self.pressed["down-right"] = (bool) pkt & (0x1 << 1)
        #self.pressed["down-left"] = (bool) pkt & (0x1 << 1)
        #self.pressed["left"] = (bool) pkt & (0x1 << 1)
        #self.pressed["up-left"] = (bool) pkt & (0x1 << 1)
        #self.pressed["accelerate"] = (bool) pkt & (0x1 << 1)
        #self.pressed["decelerate"] = (bool) pkt & (0x1 << 1)

        # write to characteristic if a button is pressed (True value) 
        # or released (False value)
        for button, button_val in self.pressed.items():
            if button_val != prev_pressed[button]:
                self.characteristics[button].write(bytes([button_val]))

    def __enter__(self):
        return self
    def __exit__(self, exc_type, exc_value, traceback):
        self.robot.disconnect()

with RobotController(addr, server_port) as robot:
    getpass('Use arrow keys to control robot')
