# Class for controlling histotripsy FPGAs
# use varLoader.py to initialize class
#
# MaizeChip 1.0 Feb 2014 TLH
# MaizeChipPy 1.0 May 2017 JJM


import time
import serial
import settings
import numpy as np
import array
from math import floor
from math import ceil
import glob

class FPGA(object):
	
	def __init__(self):
		
		#~ self.ser = settings.serVar # define the serial port as a global variable
		print 'Initializing communications...\n'
		
		ports = glob.glob('/dev/ttyUSB*')
		if len(ports) == 1:
			self.ser = serial.Serial(ports[0])
			print 'driving system communications initialized\n'
		else:
			print 'more than one USB device is connected. '
			try:
				serNum = input("Input correct com number: ")
				print '\n'
				self.ser = serial.Serial('/dev/ttyUSB%s' % serNum)
				self.ser.reset_output_buffer()
				print 'driving system communications initialized\n'
			except:
				print 'port number invalid'		

		

class a_funcs():
	def fire(self, n):
		data = np.array([self.startcode,self.bcmd5,0,0,0,n,8,self.endcode,1,1,1,1,1,1,1,1],dtype='uint8')
		self.ser.write(data)
				
	def halt(self):

		data = np.array([self.startcode,self.bcmd5,0,0,0,0,1,self.endcode,1,1,1,1,1,1,1,1],dtype='uint8')
		self.ser.write(data)
		 		
	def loadincr_chipmem(self, n,m):

		acmd = 11
		a = format(acmd,'04b')
		q_val = format(m,'016b')
		
		byte1 = int('{}{}{}{}{}'.format(n,a[0],a[1],a[2],a[3]),2)
		byte2 = int(q_val[8:16],2)
		byte3 = int(q_val[0:8],2)
		byte4 = 0
		data = np.array([self.startcode,self.bcmd5,0,byte4,byte3,byte2,byte1,self.endcode,1,1,1,1,1,1,1,1],dtype='uint8')
		self.ser.write(data)
		
		
	def set_amp(self, n):

		data = np.array([self.startcode,self.bcmd5,0,0,0,n,9,self.endcode,1,1,1,1,1,1,1,1],dtype='uint8')
		self.ser.write(data)
		
		
	def set_LEDs(self, n):

		acmd = 7
		data = np.array([self.startcode,self.bcmd5,0,0,0,n,acmd,self.endcode,1,1,1,1,1,1,1,1],dtype='uint8')
		self.ser.write(data)
		

	def set_phase(self, n):

		data = np.array([self.startcode,self.bcmd5,0,0,0,n,10,self.endcode,1,1,1,1,1,1,1,1],dtype='uint8')
		self.ser.write(data)
		
		
	def set_trig(self, n):

		acmd = 6
		data = np.array([self.startcode,self.bcmd5,0,0,0,n,acmd,self.endcode,1,1,1,1,1,1,1,1],dtype='uint8')
		self.ser.write(data)
		
		
	def start_loop(self, n,m):

		acmd = 2
		a = format(acmd,'04b')
		q_val = format(m,'024b')
		nbin = format(n,'03b')
		
		byte1 = int('{}{}{}{}{}{}{}'.format(nbin[0],nbin[1],nbin[2],a[0],a[1],a[2],a[3]),2)
		byte2 = int(q_val[16:24],2)
		byte3 = int(q_val[8:16],2)
		byte4 = int(q_val[0:8],2)
		data = np.array([self.startcode,self.bcmd5,0,byte4,byte3,byte2,byte1,self.endcode,1,1,1,1,1,1,1,1],dtype='uint8')
		
		self.ser.write(data)
		
		
	def end_loop(self, n):

		acmd = 3
		a = format(acmd,'04b')
		nbin = format(n,'03b')
		byte1 = int('{}{}{}{}{}{}{}'.format(nbin[0],nbin[1],nbin[2],a[0],a[1],a[2],a[3]),2)

		data = np.array([self.startcode,self.bcmd5,0,0,0,0,byte1,self.endcode,1,1,1,1,1,1,1,1],dtype='uint8')
		self.ser.write(data)
		
		
	def wait(self, n):

		acmd = 4
		a = format(acmd,'04b')
		
		bits = format(int(n),'028b')
		byte1 = int('{}{}{}{}{}{}{}{}'.format(bits[24],bits[25],bits[26],bits[27],a[0],a[1],a[2],a[3]),2)
		byte2 = int(bits[16:24],2)
		byte3 = int(bits[8:16],2)
		byte4 = int(bits[0:8],2)
		data = np.array([self.startcode,self.bcmd5,0,byte4,byte3,byte2,byte1,self.endcode,1,1,1,1,1,1,1,1],dtype='uint8')
		self.ser.write(data)
		
		
	def waitsec(self, t):
		
		n = round(t*100e6)-7
		if n < 0:
			n = 0
		
		acmd = 4
		a = format(acmd,'04b')
		
		bits = format(int(n),'028b')
		byte1 = int('{}{}{}{}{}{}{}{}'.format(bits[24],bits[25],bits[26],bits[27],a[0],a[1],a[2],a[3]),2)
		byte2 = int(bits[16:24],2)
		byte3 = int(bits[8:16],2)
		byte4 = int(bits[0:8],2)
		data = np.array([self.startcode,self.bcmd5,0,byte4,byte3,byte2,byte1,self.endcode,1,1,1,1,1,1,1,1],dtype='uint8')
		self.ser.write(data)
		
		
	def noop(self, n):
		
		return


	
			
	def __init__(self,x):
		self.ser = x.ser
		self.startcode = 170
		self.endcode = 85
		self.bcmd5 = 5
		
class b_funcs():
	def stop_execution(self):
		#~ self.ser = settings.serVar
		
		bdaddr = 0
		bcmd = 0
		data = np.array([self.startcode,bcmd,0,0,0,0,0,self.endcode,1,1,1,1,1,1,1,1],dtype='uint8')
		self.ser.write(data)
		
	def mask_off(self):
		
		bcmd = 13
		data = np.array([self.startcode,bcmd,0,255,255,255,255,self.endcode,1,1,1,1,1,1,1,1],dtype='uint8')
		self.ser.write(data)
		
	def select_motherboard(self,n):
		
		bcmd = 10
		data = np.array([self.startcode,bcmd,0,0,0,0,n % 256,self.endcode,1,1,1,1,1,1,1,1],dtype='uint8')
		self.ser.write(data)
		
	def set_chipmem_wloc(self,n):
		
		bcmd = 6
		data = np.array([self.startcode,bcmd,0,0,0,floor(n/256),n % 256,self.endcode,1,1,1,1,1,1,1,1],dtype='uint8')
		self.ser.write(data)
		
	def set_imem_wloc(self,n):

		bcmd = 4
		data = np.array([self.startcode,bcmd,0,0,0,floor(n/256),n % 256,self.endcode,1,1,1,1,1,1,1,1],dtype='uint8')
		self.ser.write(data)

		
				
	# THESE NEED WORK
	def set_mask(self,n):
		
		return
	def single_channel_mask(self,n):
		
		return
	#################


	def execute_program(self,n):
		bcmd = 1
		data = np.array([self.startcode,bcmd,0,0,0,floor(n/256),n % 256,self.endcode,1,1,1,1,1,1,1,1],dtype='uint8')
		self.ser.write(data)
		
	def write_chipmem(self,n):
		bcmd = 7
		data = np.array([self.startcode,bcmd,0,n[3],n[2],n[1],n[0],self.endcode,1,1,1,1,1,1,1,1],dtype='uint8')
		self.ser.write(data)
		
	def write_array_pattern_16bit(self, data):
		# Sends appropriate commands to write a full set of pattern information
		# to maizechip FPGA program.
		#
		# data is a 32 element vector of 16 bit numbers (0...65535) 
		# representing either phase or amplitude information for a pattern
		#
		# For example, data(5) = 46
		# This is 46 clock tics of phase delay or charge time for channel 5
		
		
		for i in xrange(0,len(data),2):
			bindata_tmp1 = format(int(data[i]),'016b')
			bindata_tmp2 = format(int(data[i+1]),'016b')
			a1 = int(bindata_tmp1[0:8],2)
			b1 = int(bindata_tmp1[8:16],2)
			a2 = int(bindata_tmp2[0:8],2)
			b2 = int(bindata_tmp2[8:16],2)
			data16bit_tmp = np.array([b1,a1,b2,a2])
			self.write_chipmem(data16bit_tmp)
		
	def go(self):
		self.execute_program(0)

	def stop(self):
		self.stop_execution()
	
	def __init__(self,x):
		self.ser = x.ser
		self.startcode = 170
		self.endcode = 85
		self.bcmd5 = 5
	
		
