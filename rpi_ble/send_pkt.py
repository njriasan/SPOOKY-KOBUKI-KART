#!/usr/bin/env python3
import sys
import struct
import time
from getpass import getpass
from bluepy.btle import Peripheral, DefaultDelegate
import argparse
import socket

from buttons import JoyCon

parser = argparse.ArgumentParser(description='Print advertisement data from a BLE device')
parser.add_argument('addr', metavar='A', type=str, help='Address of the form XX:XX:XX:XX:XX:XX')
parser.add_argument('server_port', metavar='P', type=int, help='Port to connect to bluetooth endpoint')
args = parser.parse_args()
addr = args.addr.lower()
server_port = args.server_port

SERVICE_UUID = "4607eda0-f65e-4d59-a9ff-84420d87a4ca"
CHAR_UUIDS = [
    "4607eda1-f65e-4d59-a9ff-84420d87a4ca"
    # "4607eda2-f65e-4d59-a9ff-84420d87a4ca",
    # "4607eda3-f65e-4d59-a9ff-84420d87a4ca",
    # "4607eda4-f65e-4d59-a9ff-84420d87a4ca"
    ] 

class RobotController():

    def __init__(self, address, server_port):

        print("connected")
                        
        # get service from robot
        self.sv = self.robot.getServiceByUUID(SERVICE_UUID)

        # get characteristic from robot
        self.controller_characteristic = self.sv.getCharacteristics(CHAR_UUIDS[0])[0]

        # PUT SOCKET LISTENING CODE HERE
        # will call on_pkt_receive
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect(('localhost', server_port))

        # Create a new Joycon
        controller = JoyCon()
        controller.display_all_pressed_buttons()

        # robot refers to buckler, our peripheral
        self.robot = Peripheral(addr)

        while True:
            pkt = self.sock.recv(12)
            self.on_pkt_receive(pkt)

    def on_pkt_receive(self, pkt):
        if (len(pkt) == 0):
            sys.exit (1)
        controller.parse_next_state(pkt)
        controller.display_all_pressed_buttons()
        #for byte in controller.get_output_message():
        #    print ("{} ".format(hex(byte)), end="")
        #print()
        
        self.controller_characteristic.write(controller.get_output_message())

    def __enter__(self):
        return self
    def __exit__(self, exc_type, exc_value, traceback):
        self.robot.disconnect()

with RobotController(addr, server_port) as robot:
    getpass('Use arrow keys to control robot')
