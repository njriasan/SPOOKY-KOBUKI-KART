#!/usr/bin/env python3
import inspect
import sys
import struct
import time
from enum import IntEnum
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
    "5607eda5-f65e-4d59-a9ff-84420d87a4ca",
    ] 

class Powerups(IntEnum):
    mushroom = 1
    redshell = 2
    blueshell = 3

class Hazards(IntEnum):
    banana = 1
    redshell = 2
    blueshell = 3

class ShellValues(IntEnum):
    redshell = 1
    blueshell = 2

class RobotDelegate(DefaultDelegate):
    def __init__(self, manager_sock, location_handle, shell_handle):
        DefaultDelegate.__init__(self)
        self.manager_sock = manager_sock
        self.location_handle = location_handle
        self.shell_handle = shell_handle
    
    def handleNotification(self, cHandle, data):
        # Add support for switching on cHandle if multiple notifications are used.
        if cHandle == self.location_handle:
            assert(len(data) == 12)
            # Compose a new 13 byte message with a leading 0 and then the location
            # 12 bytes after
            total_data = bytearray([0]) + data
            assert(len(total_data) == 13)
            self.manager_sock.send(total_data)
        elif cHandle == self.shell_handle:
            print("Receives a shell message")
            assert(len(data) == 1)
            # Compose a new 13 byte message with a leading shell value and then
            # 12 0s. Only send a message if it is a valid shell value
            shell_value = int.from_bytes(data, byteorder='little')
            if shell_value == ShellValues.redshell.value or shell_value == ShellValues.blueshell.value:
                zeros = [0 for i in range(12)]
                total_data = data + bytearray(zeros)
                assert(len(total_data) == 13)
                self.manager_sock.send(total_data)

            
            

class RobotController():

    def __init__(self, address, controller_server_port, manager_server_port):

                        

        # Create a new Joycon
        self.controller = JoyCon()
        self.controller.display_all_pressed_buttons()


        # PUT SOCKET LISTENING CODE FOR LOCATION UPDATES
        self.manager_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        waiting_to_connect = True
        while waiting_to_connect:
            try:
                self.manager_sock.connect(('localhost', manager_server_port))
                waiting_to_connect = False
            except socket.error:
                continue

        # robot refers to buckler, our peripheral
        self.robot = Peripheral(addr)

        print("Connected to the robot")


        # get service from robot
        self.sv = self.robot.getServiceByUUID(SERVICE_UUID)

        # get controller characteristic from the robot
        self.controller_characteristic = self.sv.getCharacteristics(CHAR_UUIDS[0])[0]
	
	# get powerup characteristic from the robot
        self.powerup_characteristic = self.sv.getCharacteristics(CHAR_UUIDS[1])[0]

	# get hazard characteristic from the robot
        self.hazard_characteristic = self.sv.getCharacteristics(CHAR_UUIDS[2])[0]

        # get location updates from the robot
        self.location_characteristic = self.sv.getCharacteristics(CHAR_UUIDS[3])[0]

        # get shell requests from the robot
        self.shell_characteristic = self.sv.getCharacteristics(CHAR_UUIDS[4])[0]

        # Code for subscribing to the notifications
        location_handle = self.location_characteristic.valHandle
        self.robot.writeCharacteristic(location_handle + 1, b"\x01\x00")

        shell_handle = self.shell_characteristic.valHandle
        self.robot.writeCharacteristic(shell_handle + 1, b"\x01\x00")

        self.robot.withDelegate(RobotDelegate(self.manager_sock, location_handle, shell_handle))

        # PUT SOCKET LISTENING CODE HERE
        # will call on_pkt_receive
        self.controller_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.controller_sock.connect(('localhost', controller_server_port))
        self.controller_sock.setblocking(0)

    
        self.receive_buttons()

    def receive_buttons(self):
        while True:
            try:
                pkt = self.manager_sock.recv(1, socket.MSG_DONTWAIT)
                self.process_manager_msg(pkt)
            except socket.error:
                pass
                # if we have no message from the manager just ignore it
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

    def process_manager_msg(self, pkt):
        if (len(pkt) == 0):
            sys.exit (1)
        assert(len(pkt) == 1)
        pkt_value = int.from_bytes(pkt, byteorder='little')
        # We have received a powerup value
        if pkt_value >= 1 and pkt_value <= 3:
            # Powerups map to the same value map to the same value
            self.send_powerup(pkt)
        # We have received a hazard value
        elif pkt_value >= 4 and pkt_value <= 6:
            # Hazard values are 3 greater than than their final
            # value
            hazard_value = pkt_value - 3
            hazard_msg = bytearray([hazard_value])
            self.send_hazard(hazard_msg)


    def on_pkt_receive(self, pkt):
        if (len(pkt) == 0):
            sys.exit (1)
        assert(len(pkt) == 12)
        self.controller.parse_next_state(pkt)
        #self.controller.display_all_pressed_buttons()
        
        self.controller_characteristic.write(self.controller.get_output_message())
        for button in self.controller.buttons:
            if (button.name == "HOME" or button.name == "CAPTURE") and not button.is_not_pressed():
                self.send_hazard(bytearray([Hazards.banana.value]))
            elif (button.name == "-" or button.name == "+") and not button.is_not_pressed():
                self.send_powerup(bytearray([Powerups.mushroom.value]))
            elif (button.name == "Y") and not button.is_not_pressed():
                self.send_powerup(bytearray([Powerups.redshell.value]))



    # Function to send powerup
    def send_powerup(self, powerup_byte):
        print("Powerup Sent")
        self.powerup_characteristic.write(powerup_byte)

    # Function to send hazard
    def send_hazard(self, hazard_byte):
        print("Hazard Sent")
        self.hazard_characteristic.write(hazard_byte)


    def __enter__(self):
        return self
    def __exit__(self, exc_type, exc_value, traceback):
        self.robot.disconnect()

with RobotController(addr, controller_server_port, manager_server_port) as robot:
    getpass('Use arrow keys to control robot')
