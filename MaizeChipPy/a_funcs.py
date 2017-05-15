# "a" functions for MaizeChipPy
# com_number = number of virtual com port used
# MaizeChipPy 1.0 May 2017 JJM

import serial
import settings
import numpy as np
from math import floor
from math import ceil

# pre-defined parameters
startcode = 170;
endcode = 85;
bcmd = 5;
	
def a_fire(n):
	global ser
	data = np.array([startcode,bcmd,0,0,0,n,8,endcode,1,1,1,1,1,1,1,1],dtype='uint8')
	ser.write(data)
	return 
	
def a_halt(n):
	global ser
	data = np.array([startcode,bcmd,0,0,0,0,1,endcode,1,1,1,1,1,1,1,1],dtype='uint8')
	ser.write(data)
	return 
	
def a_loadincr_chipmem(n,m):
	global ser
	acmd = 11
	a = format(acmd,'04b')
	q_val = format(m,'016b')
	
	byte1 = int('{}{}{}{}{}'.format(n,a[0],a[1],a[2],a[3]),2)
	byte2 = int(q_val[8:16],2)
	byte3 = int(q_val[0:8],2)
	byte4 = 0
	data = np.array([startcode,bcmd,0,byte4,byte3,byte2,byte1,endcode,1,1,1,1,1,1,1,1],dtype='uint8')
	ser.write(data)
	return 
	
def a_set_amp(n):
	global ser
	data = np.array([startcode,bcmd,0,0,0,n,9,endcode,1,1,1,1,1,1,1,1],dtype='uint8')
	ser.write(data)
	return 
	
def a_set_LEDs(n):
	global ser
	acmd = 7
	data = np.array([startcode,bcmd,0,0,0,n,acmd,endcode,1,1,1,1,1,1,1,1],dtype='uint8')
	ser.write(data)
	return 

def a_set_phase(n):
	global ser
	data = np.array([startcode,bcmd,0,0,0,n,10,endcode,1,1,1,1,1,1,1,1],dtype='uint8')
	ser.write(data)
	return 
	
def a_set_trig(n):
	global ser
	acmd = 6
	data = np.array([startcode,bcmd,0,0,0,n,acmd,endcode,1,1,1,1,1,1,1,1],dtype='uint8')
	ser.write(data)
	return 
	
def a_start_loop(n,m):
	global ser
	acmd = 2
	a = format(acmd,'04b')
	q_val = format(m,'024b')
	nbin = format(n,'03b')
	
	byte1 = int('{}{}{}{}{}{}{}'.format(nbin[0],nbin[1],nbin[2],a[0],a[1],a[2],a[3]),2)
	byte2 = int(q_val[16:24],2)
	byte3 = int(q_val[8:16],2)
	byte4 = int(q_val[0:8],2)
	data = np.array([startcode,bcmd,0,byte4,byte3,byte2,byte1,endcode,1,1,1,1,1,1,1,1],dtype='uint8')
	
	ser.write(data)
	return 
	
def a_end_loop(n):
	global ser
	acmd = 3
	a = format(acmd,'04b')
	q_val = format(m,'016b')
	nbin = format(n,'03b')
	byte1 = int('{}{}{}{}{}{}{}'.format(nbin[0],nbin[1],nbin[2],a[0],a[1],a[2],a[3]),2)

	data = np.array([startcode,bcmd,0,0,0,0,byte1,endcode,1,1,1,1,1,1,1,1],dtype='uint8')
	ser.write(data)
	return 
	
def a_wait(n):
	global ser
	acmd = 4
	a = format(acmd,'04b')
	bits = format(n,'028b')
	byte1 = int('{}{}{}{}{}{}{}{}'.format(bits[24],bits[25],bits[26],bits[27],a[0],a[1],a[2],a[3]),2)
	byte2 = int(bits[16:24],2)
	byte3 = int(bits[8:16],2)
	byte4 = int(bits[0:8],2)
	data = np.array([startcode,bcmd,0,byte4,byte3,byte2,byte1,endcode,1,1,1,1,1,1,1,1],dtype='uint8')
	ser.write(data)
	return 
	
def a_waitsec(t):
	global ser
	
	n = round(t*100e6)-7
	if n < 0:
		n = 0
	
	acmd = 4
	bits = format(n,'028b')
	byte1 = int('{}{}{}{}{}{}{}{}'.format(bits[24],bits[25],bits[26],bits[27],a[0],a[1],a[2],a[3]),2)
	byte2 = int(bits[16:24],2)
	byte3 = int(bits[8:16],2)
	byte4 = int(bits[0:8],2)
	data = np.array([startcode,bcmd,0,byte4,byte3,byte2,byte1,endcode,1,1,1,1,1,1,1,1],dtype='uint8')
	ser.write(data)
	return 
	
def a_noop(n):
	
	return

