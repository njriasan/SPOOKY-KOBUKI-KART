# Script giving the commands that should be run on startup for the raspberry pi
# This script should be added a to bashrc to ensure the code can be run in a
# headless mode on startup.

# NOTE this script must be run with sudo permissions

import os, subprocess

connect_log = open("connection_log.txt", "w")
subprocess.Popen(["python3", "connect_joycon.py"], stdout=connect_log, stdout=connect_log)
os.chdir("../joycon")
subprocess.call(["make clean"])
subprocess.call(["make"])
manager_log = open("manager_log.txt", "w")
subprocess.call(["./process_manager"], stdout=manager_log, stderr=manager_log)
