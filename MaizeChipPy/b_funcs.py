# "b" functions for MaizeChipPy
# com_number = number of virtual com port used
# MaizeChipPy 1.0 May 2017 JJM

import serial
import settings
import numpy as np
from math import floor
from math import ceil

startcode = 170
endcode = 85

def b_stop_execution():
	ser = settings.serVar
	
	bdaddr = 0
	bcmd = 0
	data = np.array([startcode,bcmd,0,0,0,0,0,endcode,1,1,1,1,1,1,1,1],dtype='uint8')
	ser.write(data)
	return 
	
def b_mask_off():
	global ser
	
	bcmd = 13
	data = np.array([startcode,bcmd,0,255,255,255,255,endcode,1,1,1,1,1,1,1,1],dtype='uint8')
	ser.write(data)
	return 
	
def b_select_motherboard(n):
	global ser
	
	bcmd = 10
	data = np.array([startcode,bcmd,0,0,0,0,n % 256,endcode,1,1,1,1,1,1,1,1],dtype='uint8')
	ser.write(data)
	return 
	
def b_set_chipmem_wloc(n):
	global ser
	
	bcmd = 6
	data = np.array([startcode,bcmd,0,0,0,floor(n/256),n % 256,endcode,1,1,1,1,1,1,1,1],dtype='uint8')
	ser.write(data)
	return 
	
def b_set_imem_wloc(n):
	global ser
	
	bcmd = 4
	data = np.array([startcode,bcmd,0,0,0,floor(n/256),n % 256,endcode,1,1,1,1,1,1,1,1],dtype='uint8')
	ser.write(data)
	return 
	
	
	
# THESE NEED WORK
def b_set_mask(n):
	global ser
	
	return 0
def b_single_channel_mask(n):
	global ser
	
	return 
#################


def b_execute_program(n):
	global ser
	
	bcmd = 1
	data = np.array([startcode,bcmd,0,0,0,floor(n/256),n % 256,endcode,1,1,1,1,1,1,1,1],dtype='uint8')
	ser.write(data)
	return 
	
def b_write_chipmem(n):
	global ser
	
	bcmd = 7
	data = np.array([startcode,bcmd,0,n[3],n[2],n[1],n[0],endcode,1,1,1,1,1,1,1,1],dtype='uint8')
	ser.write(data)
	return 
	
def bgo():
	b_execute_program
	return 
	
def bstop():
	b_stop_execution
	return 
	



































