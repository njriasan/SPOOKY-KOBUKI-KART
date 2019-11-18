import subprocess
import tempfile

joycon_macs = ["04:03:D6:7B:59:CA"]



def scan_joycons():
  # Open a temporary file
  with tempfile.TemporaryFile("w+") as f:
    subprocess.call (["hcitool", "scan"], stdout=f)
    f.seek (0, 0)
    for line in f:
      for addr in joycon_macs:
        if addr.lower() in line.lower():
          connect_joycon(addr)

def connect_joycon(addr):
  print (addr)
  with tempfile.TemporaryFile("w+") as f:
    print("connect {}".format(addr), file=f)
    f.seek (0, 0)
    subprocess.call (["bluetoothctl"], stdin=f)

scan_joycons ()
