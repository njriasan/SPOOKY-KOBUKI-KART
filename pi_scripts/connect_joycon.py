from time import sleep
import subprocess
import tempfile

joycon_macs = ["7C:BB:8A:9E:3E:8D", "98:B6:E9:71:62:01", "B8:78:26:1E:35:06", "B8:78:26:20:DC:1E"]



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
    print("connect {0}".format(addr), file=f)
    f.seek (0, 0)
    subprocess.call (["bluetoothctl"], stdin=f)

def main():
  while True:
    scan_joycons ()
    sleep (1)

if __name__ == "__main__":
  main()
