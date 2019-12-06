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

SERVICE_UUID = "5607eda0-f65e-4d59-a9ff-84420d87a4ca"
CHAR_UUIDS = [
    "5607eda1-f65e-4d59-a9ff-84420d87a4ca",
    "5607eda2-f65e-4d59-a9ff-84420d87a4ca",
    "5607eda3-f65e-4d59-a9ff-84420d87a4ca",
    ] 

class RobotController():

    def __init__(self, address, server_port):

        print("connected")
                        

        # Create a new Joycon
        self.controller = JoyCon()
        self.controller.display_all_pressed_buttons()

        # robot refers to buckler, our peripheral
        self.robot = Peripheral(addr)

        # get service from robot
        self.sv = self.robot.getServiceByUUID(SERVICE_UUID)

        # get controller characteristic from robot
        self.controller_characteristic = self.sv.getCharacteristics(CHAR_UUIDS[0])[0]
	
	# get powerup characteristic from robot
        self.powerup_characteristic = self.sv.getCharacteristics(CHAR_UUIDS[1])[0]

	# get hazard characteristic from robot
        self.hazard_characteristic = self.sv.getCharacteristics(CHAR_UUIDS[2])[0]

        # PUT SOCKET LISTENING CODE HERE
        # will call on_pkt_receive
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect(('localhost', server_port))

        while True:
            pkt = self.sock.recv(12)
            self.on_pkt_receive(pkt)

    def on_pkt_receive(self, pkt):
        if (len(pkt) == 0):
            sys.exit (1)
        self.controller.parse_next_state(pkt)
        self.controller.display_all_pressed_buttons()
        for byte in self.controller.get_output_message():
            print ("{} ".format(hex(byte)), end="")
        print()
        
        self.controller_characteristic.write(self.controller.get_output_message())
#        self.send_powerup(bytearray([1]))
#        self.send_hazard(bytearray([1]))


    # Function to send powerup
    def send_powerup(self, powerup_byte):
        self.powerup_characteristic.write(powerup_byte)

    def send_hazard(self, hazard_byte):
        self.hazard_characteristic.write(hazard_byte)

    def __enter__(self):
        return self
    def __exit__(self, exc_type, exc_value, traceback):
        self.robot.disconnect()

with RobotController(addr, server_port) as robot:
    getpass('Use arrow keys to control robot')
