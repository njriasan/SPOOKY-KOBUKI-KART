#!/usr/bin/env python3
import inspect
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
parser.add_argument('controller_server_port', metavar='P', type=int, help='Port to connect to bluetooth endpoint')
parser.add_argument('manager_server_port', metavar='P', type=int, help='Port to connect to process_manager')
args = parser.parse_args()
addr = args.addr.lower()
controller_server_port = args.controller_server_port
manager_server_port = args.manager_server_port

SERVICE_UUID = "5607eda0-f65e-4d59-a9ff-84420d87a4ca"
CHAR_UUIDS = [
    "5607eda1-f65e-4d59-a9ff-84420d87a4ca",
    "5607eda2-f65e-4d59-a9ff-84420d87a4ca",
    "5607eda3-f65e-4d59-a9ff-84420d87a4ca",
    "5607eda4-f65e-4d59-a9ff-84420d87a4ca",
    ] 

class RobotDelegate(DefaultDelegate):
    def __init__(self, manager_sock):
        DefaultDelegate.__init__(self)
        self.manager_sock = manager_sock
    
    def handleNotification(self, cHandle, data):
        # Add support for switching on cHandle if multiple notifications are used.
        assert(len(data) == 12)
        self.manager_sock.send(data)

class RobotController():

    def __init__(self, address, controller_server_port, manager_server_port):

                        

        # Create a new Joycon
        self.controller = JoyCon()
        self.controller.display_all_pressed_buttons()


        # PUT SOCKET LISTENING CODE FOR LOCATION UPDATES
        self.manager_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.manager_sock.connect(('localhost', manager_server_port))

        # robot refers to buckler, our peripheral
        self.robot = Peripheral(addr)
        self.robot.withDelegate(RobotDelegate(self.manager_sock))

        print("Connected to the robot")


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
        # Code for subscribing to the
        handle = self.location_characteristic.valHandle
        self.robot.writeCharacteristic(handle + 1, b"\x01\x00")

        # PUT SOCKET LISTENING CODE HERE
        # will call on_pkt_receive
        self.controller_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.controller_sock.connect(('localhost', controller_server_port))
        self.controller_sock.setblocking(0)

    
        self.receive_buttons()

    def receive_buttons(self):
        while True:
            try:
                pkt = self.controller_sock.recv(12)
                self.on_pkt_receive(pkt)
            except socket.error:
                # If we send a packet we may receive notifications as
                # an interrupt. If we do not receive a packet then
                # waitForNotifications will enable us to poll on our reads
                # from the socket, although if it recieves a packet it will
                # return sooner, so it is imprecise
                self.robot.waitForNotifications(0.1)

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

    # Function to send hazard
    def send_hazard(self, hazard_byte):
        self.hazard_characteristic.write(hazard_byte)


    def __enter__(self):
        return self
    def __exit__(self, exc_type, exc_value, traceback):
        self.robot.disconnect()

with RobotController(addr, controller_server_port, manager_server_port) as robot:
    getpass('Use arrow keys to control robot')
