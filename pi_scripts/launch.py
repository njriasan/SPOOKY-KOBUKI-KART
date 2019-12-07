# Script giving the commands that should be run on startup for the raspberry pi
# This script should be added a to bashrc to ensure the code can be run in a
# headless mode on startup.

# NOTE this script must be run with sudo permissions

import os, subprocess

subprocess.Popen(["python3", "connect_joycon.py"])
os.chdir("../joycon")
subprocess.call(["make"])
subprocess.Popen(["./process_manager"])
