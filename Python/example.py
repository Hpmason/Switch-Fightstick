
import time

from serial.tools import list_ports

import seriallib
from JoystickEnums import Button, HAT



def get_port_from_user():
	port_list = list(list_ports.grep(""))
	if len(port_list) == 0:
		raise LookupError("Unable to detect Serial Device.")
	index_port_list_str = [f"Index: {index}, Port: {port.device}, Description: {port.description}"
						   for index, port in enumerate(port_list)]
	print(index_port_list_str)
	while True:
		ind = input("What port index should be used? ")
		if not str.isdigit(ind):
			print(f"Value given is not a digit")
		elif not (0 <= int(ind) < len(port_list)):
			print("Value given is not an index in the list")
		else:
			return port_list[int(ind)].device

def packet_cycle(ser_man, payload):
	ser_man.write(payload.as_byte_arr())
	while ser_man.in_waiting < 1:
		time.sleep(1/1000)
	ser_man.read()

if __name__ == "__main__":
	BAUD = 38400
	UPDATES_PER_SECOND = 60

	pack = seriallib.Payload()
	with seriallib.SerialManager(get_port_from_user(), BAUD) as ser:
		print("Flushing Serial Port")
		ser.flush()
		print("Starting Main Loop")
		pack.apply_buttons(Button.A)
		packet_cycle(ser, pack)
		while True:
			time.sleep(1)
			pack.set_left_stick(128, 255)
			packet_cycle(ser, pack)
			pack.reset_inputs()
			time.sleep(1)
			pack.set_hat(0,-1)
			packet_cycle(ser, pack)
			


				
