# Initializes USB communications with FPGA
# com_number = number of virtual com port used
# MaizeChipPy 1.0 May 2017 JJM

import time
import serial
import glob
import settings

def maize_init():
	global ser
	ser = settings.serVar # define the serial port as a global variable
	print 'Initializing communications...\n'
	
	ports = glob.glob('/dev/ttyUSB*')
	if len(ports) == 1:
		ser = serial.Serial(ports[0])
		print 'driving system communications initialized\n'
	else:
		print 'more than one USB device is connected. '
		try:
			serNum = input("Input correct com number: ")
			print '\n'
			ser = serial.Serial('/dev/ttyUSB%s' % serNum)
			ser.reset_output_buffer()
			print 'driving system communications initialized\n'
		except:
			print 'port number invalid'		
	return ser
		
	
	

