#!/usr/bin/env python3
import sys
import struct
import threading
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
    "5607eda4-f65e-4d59-a9ff-84420d87a4ca",
    ] 

class RobotDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)
    
    def handleNotification(self, cHandle, data):
        print("Called")
        x_coor = int.from_bytes(data[0:4], "little", signed=True) / 1000.0
        y_coor = int.from_bytes(data[4:8], "little", signed=True) / 1000.0
        z_coor = int.from_bytes(data[8:12], "little", signed=True) / 1000.0
        print ("Coordinates are: {} {} {}".format(x_coor, y_coor, z_coor))

class RobotController():

    def __init__(self, address, server_port):

        print("connected")
                        

        # Create a new Joycon
        self.controller = JoyCon()
        self.controller.display_all_pressed_buttons()

        # robot refers to buckler, our peripheral
        self.robot = Peripheral(addr)
        self.robot.withDelegate(RobotDelegate())

        # get service from robot
        self.sv = self.robot.getServiceByUUID(SERVICE_UUID)

        # get controller characteristic from robot
        self.controller_characteristic = self.sv.getCharacteristics(CHAR_UUIDS[0])[0]
	
	# get powerup characteristic from robot
        self.powerup_characteristic = self.sv.getCharacteristics(CHAR_UUIDS[1])[0]

	# get hazard characteristic from robot
        self.hazard_characteristic = self.sv.getCharacteristics(CHAR_UUIDS[2])[0]

        # get location updates from robot
        self.location_characteristic = self.sv.getCharacteristics(CHAR_UUIDS[3])[0]

        # location_thread = threading.Thread(target=self.receive_robot_notifications)
        # location_thread.start()

        # PUT SOCKET LISTENING CODE HERE
        # will call on_pkt_receive
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect(('localhost', server_port))

        while True:
            pkt = self.sock.recv(12)
            self.on_pkt_receive(pkt)
            self.receive_robot_notifications()

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

    def receive_robot_notifications(self):
        #while True:
            self.robot.waitForNotifications(1.0)

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
