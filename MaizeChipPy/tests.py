# easy test scripts for debugging and simple running
# J. Macoskey - May 2017


import serial
import numpy as np
import scipy.io as sio
import time

from fpga_Class import *


# Test for communications: flashes each LED according to its number\
# wt = wait time in clock cycles. Normal number is 20e6
def LEDflashtest(x,wt):
	#x = FPGA()
	#~ maize_init()
	#~ global ser
	#~ wt = 20e6
	x.b_stop_execution()
	x.b_set_imem_wloc(0)
	
	# begin outermost loop
	x.a_start_loop(2,100)
	
	# flash LED1 one time
	x.a_start_loop(1,1)
	x.a_set_LEDs(1)
	x.a_wait(wt)
	x.a_set_LEDs(0)
	x.a_wait(wt)
	x.a_end_loop(1)

	# flash LED2 two times
	x.a_start_loop(1,2)
	x.a_set_LEDs(2)
	x.a_wait(wt)
	x.a_set_LEDs(0)
	x.a_wait(wt)
	x.a_end_loop(1)
	
	# flash LED3 three times
	x.a_start_loop(1,3)
	x.a_set_LEDs(4)
	x.a_wait(wt)
	x.a_set_LEDs(0)
	x.a_wait(wt)
	x.a_end_loop(1)
	
	# flash LED4 four times
	x.a_start_loop(1,4)
	x.a_set_LEDs(8)
	x.a_wait(wt)
	x.a_set_LEDs(0)
	x.a_wait(wt)
	x.a_end_loop(1)
	
	x.a_end_loop(2)
	# end outermost loop
	x.a_halt()
			
	x.b_execute_program(0)		
	#~ return

def basic112_center(x,PRF,nPulses):
	#### SETUP ####
	x.b_stop_execution()
	
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
		x.b_select_motherboard(mother)
		chanset = np.asarray(range(0,32))+mother*32 # we are programming chans 0 through 111
		x.b_set_chipmem_wloc(0)
		x.write_array_pattern_16bit(chargetime[chanset])
	
	x.b_select_motherboard(3)						# special programming of 4th mobo required due to non 2^n element count
	chanset = np.asarray(range(96,112))
	x.b_set_chipmem_wloc(0)
	mobo3chargetime = np.ones(32)-1 				# array of zeros
	mobo3chargetime[0:16] = chargetime[chanset] 	# change first 16 elements to correct chargetimes
	x.write_array_pattern_16bit(mobo3chargetime)
	
	#### TREATMENT PROGRAM ####
	x.b_stop_execution()
	x.b_set_imem_wloc(0)
	x.a_set_amp(0)
	x.a_set_phase(0)
	
	# begin outermost loop
	x.a_start_loop(1,nPulses)
	x.a_loadincr_chipmem(1,0)
	x.a_wait(0)
	x.a_set_amp(0)
	x.a_set_phase(0)
	
	x.a_fire(0)
	x.a_set_trig(15)
	x.a_waitsec(10e-6)
	x.a_set_trig(0)
	
	x.a_set_LEDs(15)
	x.a_waitsec(100e-6)
	x.a_set_LEDs(0)
	
	x.a_waitsec(float(1)/(PRF))

	x.a_end_loop(1)
	# end outermost loop
    
	x.a_halt()
	print "Array initialized. Waiting to fire at center...\n"
	
