# Script giving the commands that should be run on startup for the raspberry pi
# This script should be added a to bashrc to ensure the code can be run in a
# headless mode on startup.

# NOTE this script must be run with sudo permissions

import os, subprocess

fd_1 = os.open("connection_log.txt", os.O_WRONLY | os.O_CREAT, 0o666)
connect_log = open(fd_1, "w")
subprocess.Popen(["python3", "connect_joycon.py"], stdout=connect_log, stderr=connect_log)
os.chdir("../joycon")
subprocess.call(["pwd"])
subprocess.call(["make", "clean"])
subprocess.call(["make"])
fd_2 = os.open("../pi_scripts/manager_log.txt", os.O_WRONLY | os.O_CREAT, 0o666)
manager_log = open(fd_2, "w")
subprocess.call(["./process_manager"], stdout=manager_log, stderr=manager_log)
