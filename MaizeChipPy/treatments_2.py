# Set of all treatment scripts including basic steering
# J. Macoskey - May 2017

import serial
import numpy as np
import scipy.io as sio
import math as m
import time

#~ from fpga_Class import *

# x = FPGA class
# PRF = PRF in regular units
# nPulses = pulses-per-location
# locations = treatment locations coordinates in (fi,3) dimension... use patterns.py

def loadCalibDelays():
	nEls = 112
	# Initialize calibrated chargetimes:
	chargetime = 500*np.ones(112)
	try:
		tmp = sio.loadmat('delays_112.mat')					# load calibrated delays
		calibrationDelays = np.round(tmp['delays_FPGA']) 	# the argument here will change depending on what you named the variable in your matfile
		calibrationDelays.shape = (112) 					# reshape to match chargetime
		chargetime = chargetime + calibrationDelays
	except:
		print '"delays_112.mat" not found. Using non-calibrated delays'
	return chargetime

def steeringChargetimes(locations):
	nEls = 112
	locations = locations*1e-3 # convert to meters
	chargetime = loadCalibDelays()
	
	# Alter chargetimes based on steering locations:
	nLocs = len(locations)
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

def singleLocChargetimes(location):
	nEls = 112
	location = location*1e-3 # convert to meters
	chargetime = loadCalibDelays()
	
	phasecals = chargetime
	delays = np.ones(nEls)-1
	tmp = sio.loadmat('trans_center.mat')
	transCenter = tmp['trans_center']
	timeDelay = np.ones(nEls)-1
	
	for ei in range(0,112):
		u = transCenter[:,ei] - location
		dist = np.sqrt(np.sum(u**2))
		timeDelay[ei] = 1e-4-float(dist)/(1500)
	delays[:] = timeDelay+abs(np.min(timeDelay))
	phasecals[:] = phasecals[:] + np.round(delays[:]*1e8)
	
	return phasecals

	
def moboProgram(a,b,phasecals,nLocs):
	# Program chargetimes into Mobos:
	for fi in range(0,nLocs):
		for mother in range(0,3):
			b.select_motherboard(mother)
			chanset = np.asarray(range(0,32))+mother*32 # we are programming chans 0 through 111
			b.set_chipmem_wloc(0+fi)
			b.write_array_pattern_16bit(phasecals[fi,chanset])
		
		b.select_motherboard(3)						# special programming of 4th mobo required due to non 2^n element count
		chanset = np.asarray(range(96,112))
		b.set_chipmem_wloc(0+fi)
		mobo3chargetime = np.ones(32)-1 				# array of zeros
		mobo3chargetime[0:16] = phasecals[fi,chanset] 	# change first 16 elements to correct chargetimes
		b.write_array_pattern_16bit(mobo3chargetime)
		print "Location %d initialized\n" %fi

def basic112_upload(a,b,locations):
	#### SETUP ####
	b.stop_execution()
	
	# Calculate phase delays
	phasecals = steeringChargetimes(locations)
	
	# Program motherboards
	nLocs = locations.shape[0]
	moboProgram(a,b,phasecals,nLocs)
	
	
	
	
	
def twoFoci_initPhasecals(a,b):
	b.stop_execution()
	
	loc1 = np.array([0,10,0]) # in mm
	loc2 = np.array([0,-10,0]) # in mm
	
	phasecal1 = singleLocChargetimes(loc1) # located in treatments_2.py
	phasecal2 = singleLocChargetimes(loc2)
	
	return phasecal1, phasecal2
	
def singleFocusMoboUpload(a,b,phasecalmix):
	# Program chargetimes into Mobo:
	for mother in range(0,3):
		b.select_motherboard(mother)
		chanset = np.asarray(range(0,32))+mother*32 # we are programming chans 0 through 111
		b.set_chipmem_wloc(0)
		b.write_array_pattern_16bit(phasecalmix[chanset])
	
	b.select_motherboard(3)						# special programming of 4th mobo required due to non 2^n element count
	chanset = np.asarray(range(96,112))
	b.set_chipmem_wloc(0)
	mobo3chargetime = np.ones(32)-1 				# array of zeros
	mobo3chargetime[0:16] = phasecalmix[chanset] 	# change first 16 elements to correct chargetimes
	b.write_array_pattern_16bit(mobo3chargetime)
	print "mobos uploaded"
	
	
def twoFociOnePerPulse(a,b):
	b.stop_execution()
	
	pts = np.array([[0,10,0],[0,-10,0]])
	basic112_upload(a,b,pts)
	basic112_steering(a,b,2,10000,pts)
	b.go()
	
def twoFociHalfSame(a,b):
	# uses same halves to excite the locations
	phasecal1, phasecal2 = twoFoci_initPhasecals(a,b)
	
	########### Choose halves ############
	phasecalmix = np.empty(phasecal1.shape)
	for li in range(0,len(phasecalmix)):
		if li <= 56: phasecalmix[li] = phasecal1[li] # the first 56 elements are positive y
		else: phasecalmix[li] = phasecal2[li] 		 # the second 56 elements are negative y
	######################################
	
	singleFocusMoboUpload(a,b,phasecalmix)
	basic112_steering(a,b,2,10000,1)
	return phasecalmix
	
def twoFociHalfOpp(a,b):
	# uses oppositve halves to excite the two locations
	phasecal1, phasecal2 = twoFoci_initPhasecals(a,b)
	
	########### Choose halves ############
	phasecalmix = np.empty(phasecal1.shape)
	for li in range(0,len(phasecalmix)):
		if li <= 56: phasecalmix[li] = phasecal2[li] # this time, make the pos-y elements fire at the neg-y location
		else: phasecalmix[li] = phasecal1[li] 		 # and vice versa
	######################################
	
	singleFocusMoboUpload(a,b,phasecalmix)
	basic112_steering(a,b,2,10000,1)
	return phasecalmix

def twoFociAlternating(a,b):
	# uses alternating elements to excite the halves
	phasecal1, phasecal2 = twoFoci_initPhasecals(a,b)
	
	######### Choose alternating els #############
	phasecalmix = np.empty(phasecal1.shape)
	for li in range(0,len(phasecalmix)):
		if li%2 == 0: phasecalmix[li] = phasecal1[li]
		else: phasecalmix[li] = phasecal2[li]
	##############################################
	
	singleFocusMoboUpload(a,b,phasecalmix)
	basic112_steering(a,b,2,10000,1)
	return phasecalmix
	
def twoFociRandom(a,b):
	# uses random elements to excite the two locations
	phasecal1, phasecal2 = twoFoci_initPhasecals(a,b)
	
	############ Choose random els #############
	check = 0
	while check == 0:
		rvec = np.random.rand(112) > 0.5
		if sum(rvec) == 56:
			check = 1
	# assign phasecals based on random assignment
	phasecalmix = np.empty(phasecal1.shape)
	for li in range(0,len(phasecalmix)):
		if rvec[li] == True: phasecalmix[li] = phasecal1[li]
		else: phasecalmix[li] = phasecal2[li]		
	############################################
	
	singleFocusMoboUpload(a,b,phasecalmix)
	basic112_steering(a,b,2,10000,1)
	return phasecalmix, rvec
	
def basic112_steering(a,b,PRF,nPulses,locations):
	# Parameters:
	trig_len = 20e-6
	
	#### SETUP ####
	b.stop_execution()
	
	# Program motherboards
	try:
		nLocs = locations.shape[0]
	except:
		nLocs = 1
		
	#### TREATMENT PROGRAM ####
	b.stop_execution()
	b.set_imem_wloc(0)
	a.set_amp(0)
	a.set_phase(0)
	
	#* begin outermost loop
	a.start_loop(1,nPulses)
	a.loadincr_chipmem(1,0)
	a.wait(1)
	a.set_amp(0)
	a.set_phase(0)
	
	#** begin inner loop
	a.start_loop(2,nLocs)
	
	a.set_trig(0)
	a.waitsec(trig_len)
	a.set_trig(15)
	
	a.fire(0)
	
	a.waitsec(float(1)/(PRF)-trig_len)
	
	a.loadincr_chipmem(0,1)
	a.wait(1)
	a.set_phase(0)
	a.set_amp(0)
	
	a.end_loop(2)
	#** end inner loop

	a.end_loop(1)
	#* end outermost loop
    
	a.halt()
	
	print "Array initialized. Waiting to fire at %d locations...\n" %nLocs


