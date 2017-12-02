# Set of all treatment scripts including basic steering
# J. Macoskey - May 2017

import serial
import numpy as np
import scipy.io as sio
import math as m
import time

from fpga_Class import *

# x = FPGA class
# PRF = PRF in regular units
# nPulses = pulses-per-location
# locations = treatment locations coordinates in (fi,3) dimension... use patterns.py

def steeringChargetimes(locations):
	locations = locations*1e-3 # convert to meters
	
	# Initialize calibrated chargetimes:
	chargetime = 500*np.ones(112)
	try:
		tmp = sio.loadmat('delays_112.mat')					# load calibrated delays
		calibrationDelays = np.round(tmp['delays_FPGA']) 	# the argument here will change depending on what you named the variable in your matfile
		calibrationDelays.shape = (112) 					# reshape to match chargetime
		chargetime = chargetime + calibrationDelays
	except:
		print '"delays_112.mat" not found. Using non-calibrated delays'
	
	# Alter chargetimes based on steering locations:
	nLocs = len(locations) 
	nEls = 112
	phasecals = np.array([chargetime,]*nLocs)	# shape is (nLocs,112)
	delays = np.empty([nLocs,nEls]) 			# shape is (nLocs,112)
	delays[:,:] = 0								
	tmp = sio.loadmat('trans_center.mat')		# import transducer element center locations
	transCenter = tmp['trans_center'] 			# shape is now (3,112)
	timeDelay = np.ones(nEls)-1					# zeros array for making time delays
	
	for fi in range(0,nLocs):
		for ei in range(0,nEls):
			u = transCenter[:,ei] - locations[fi,:]
			dist = np.sqrt(np.sum(u**2))
			timeDelay[ei] = 1e-4-float(dist)/(1500)
		delays[fi,:] = timeDelay+abs(np.min(timeDelay))
		phasecals[fi,:] = phasecals[fi,:] + np.round(delays[fi,:]*1e8)
	
	return phasecals
	
def moboProgram(x,phasecals,nLocs):
	# Program chargetimes into Mobos:
	for fi in range(0,nLocs):
		for mother in range(0,3):
			x.b_select_motherboard(mother)
			chanset = np.asarray(range(0,32))+mother*32 # we are programming chans 0 through 111
			x.b_set_chipmem_wloc(0+fi)
			x.write_array_pattern_16bit(phasecals[fi,chanset])
		
		x.b_select_motherboard(3)						# special programming of 4th mobo required due to non 2^n element count
		chanset = np.asarray(range(96,112))
		x.b_set_chipmem_wloc(0+fi)
		mobo3chargetime = np.ones(32)-1 				# array of zeros
		mobo3chargetime[0:16] = phasecals[fi,chanset] 	# change first 16 elements to correct chargetimes
		x.write_array_pattern_16bit(mobo3chargetime)
		print "Location %d initialized\n" %fi

def basic112_upload(x,locations):
	#### SETUP ####
	x.b_stop_execution()
	
	# Calculate phase delays
	phasecals = steeringChargetimes(locations)
	
	# Program motherboards
	nLocs = locations.shape[0]
	moboProgram(x,phasecals,nLocs)
		
	
def basic112_steering(x,PRF,nPulses,locations):
	# Parameters:
	trig_len = 10e-6
	
	#### SETUP ####
	x.b_stop_execution()
	
	# Program motherboards
	nLocs = locations.shape[0]
		
	#### TREATMENT PROGRAM ####
	x.b_stop_execution()
	x.b_set_imem_wloc(0)
	x.a_set_amp(0)
	x.a_set_phase(0)
	
	#* begin outermost loop
	x.a_start_loop(1,nPulses)
	x.a_loadincr_chipmem(1,0)
	x.a_wait(1)
	x.a_set_amp(0)
	x.a_set_phase(0)
	
	#** begin inner loop
	x.a_start_loop(2,nLocs)

	x.a_fire(0)
	
	x.a_set_trig(15)
	x.a_waitsec(trig_len)
	x.a_set_trig(0)

	x.a_waitsec(float(1)/(PRF)-trig_len)
	
	x.a_loadincr_chipmem(0,1)
	x.a_wait(1)
	x.a_set_phase(0)
	x.a_set_amp(0)
	
	x.a_end_loop(2)
	#** end inner loop

	x.a_end_loop(1)
	#* end outermost loop
    
	x.a_halt()
	
	print "Array initialized. Waiting to fire at %d locations...\n" %nLocs
