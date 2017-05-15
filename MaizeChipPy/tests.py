# easy test scripts for debugging and simple running
# J. Macoskey - May 2017


import serial
import numpy as np
import scipy.io as sio
import time

#~ from fpga_Class_2 import *

# Test for communications: flashes each LED according to its number\
# wt = wait time in clock cycles. Normal number is 20e6
def LEDflashtest(a,b,wt):
	#x = FPGA()
	#~ maize_init()
	#~ global ser
	#~ wt = 20e6
	b.stop_execution()
	b.set_imem_wloc(0)
	
	# begin outermost loop
	a.start_loop(2,100)
	
	# flash LED1 one time
	a.start_loop(1,1)
	a.set_LEDs(1)
	a.wait(wt)
	a.set_LEDs(0)
	a.wait(wt)
	a.end_loop(1)

	# flash LED2 two times
	a.start_loop(1,2)
	a.set_LEDs(2)
	a.wait(wt)
	a.set_LEDs(0)
	a.wait(wt)
	a.end_loop(1)
	
	# flash LED3 three times
	a.start_loop(1,3)
	a.set_LEDs(4)
	a.wait(wt)
	a.set_LEDs(0)
	a.wait(wt)
	a.end_loop(1)
	
	# flash LED4 four times
	a.start_loop(1,4)
	a.set_LEDs(8)
	a.wait(wt)
	a.set_LEDs(0)
	a.wait(wt)
	a.end_loop(1)
	
	a.end_loop(2)
	# end outermost loop
	a.halt()
			
	b.execute_program(0)		
	#~ return

def basic112_center(a,b,PRF,nPulses):
	#### SETUP ####
	b.stop_execution()
	
	# Initialize chargetimes:
	chargetime = 500*np.ones(112)
	try:
		tmp = sio.loadmat('delays_112.mat')
		calibrationDelays = np.round(tmp['delays_FPGA']) 	# the argument here will change depending on what you named the variable in your matfile
		calibrationDelays.shape = (112) 					# reshape to match chargetime
		chargetime = chargetime + calibrationDelays
	except:
		print '"delays_112.mat" not found. Using non-calibrated delays'
	
	# Program chargetimes into Mobo:
	for mother in range(0,3):
		b.select_motherboard(mother)
		chanset = np.asarray(range(0,32))+mother*32 # we are programming chans 0 through 111
		b.set_chipmem_wloc(0)
		b.write_array_pattern_16bit(chargetime[chanset])
	
	b.select_motherboard(3)						# special programming of 4th mobo required due to non 2^n element count
	chanset = np.asarray(range(96,112))
	b.set_chipmem_wloc(0)
	mobo3chargetime = np.ones(32)-1 				# array of zeros
	mobo3chargetime[0:16] = chargetime[chanset] 	# change first 16 elements to correct chargetimes
	b.write_array_pattern_16bit(mobo3chargetime)
	
	#### TREATMENT PROGRAM ####
	b.stop_execution()
	b.set_imem_wloc(0)
	a.set_amp(0)
	a.set_phase(0)
	
	# begin outermost loop
	a.start_loop(1,nPulses)
	a.loadincr_chipmem(1,0)
	a.wait(0)
	a.set_amp(0)
	a.set_phase(0)
	
	a.fire(0)
	a.set_trig(15)
	a.waitsec(10e-6)
	a.set_trig(0)
	
	a.set_LEDs(15)
	a.waitsec(100e-6)
	a.set_LEDs(0)
	
	a.waitsec(float(1)/(PRF))

	a.end_loop(1)
	# end outermost loop
    
	a.halt()
	print "Array initialized. Waiting to fire at center...\n"
	
