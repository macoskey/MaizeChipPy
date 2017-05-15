# Set of all treatment scripts including basic steering
# J. Macoskey - May 2017

import numpy as np
import scipy.io as sio
import math as m

# PRF = PRF in regular units
# nPulses = pulses-per-location
# locations = treatment locations coordinates in (fi,3) dimension... use patterns.py

class point_uploader():
	def steeringChargetimes(locations):
		
		locations = locations*1e-3 # convert to meters
		
		# Initialize calibrated chargetimes:
		chargetime = 500*np.ones(self.nMemLocs)
		chargetime[self.nEls:] = 0
		try:
			tmp = sio.loadmat('delays_112.mat')					# load calibrated delays
			calibrationDelays = np.zeros(self.nMemLocs)
			calibrationDelays[0:self.nEls] = np.round(tmp['delays_FPGA']) 	# the argument here will change depending on what you named the variable in your matfile
			chargetime = chargetime + calibrationDelays
		except:
			print '"delays_112.mat" not found. Using non-calibrated delays'
		
		# Alter chargetimes based on steering locations:
		nLocs = locations.shape[0] 
		
		phasecals = np.array([chargetime,]*nLocs)	# shape is (nLocs,112)
		delays = np.empty([nLocs,self.nMemLocs]) 			# shape is (nLocs,112)
		delays[:,:] = 0								
		tmp = sio.loadmat('trans_center.mat')		# import transducer element center locations
		transCenter = tmp['trans_center'] 			# shape is now (3,112)
		timeDelay = np.ones(self.nMemLocs)-1					# zeros array for making time delays
		
		for fi in range(0,nLocs):
			for ei in range(0,self.nMemLocs):
				u = transCenter[:,ei] - locations[fi,:]
				dist = np.sqrt(np.sum(u**2))
				timeDelay[ei] = 1e-4-float(dist)/(1500)
			delays[fi,:] = timeDelay+abs(np.min(timeDelay))
			phasecals[fi,:] = phasecals[fi,:] + np.round(delays[fi,:]*1e8)
		
		return phasecals
		
	def moboProgram(phasecals,nLocs):
		# Program chargetimes into Mobos:
		for fi in range(0,nLocs):
			for mother in range(0,self.nMobos):
				self.b.select_motherboard(mother)
				chanset = np.asarray(range(0,32))+mother*32 # we are programming chans 0 through 111
				self.b.set_chipmem_wloc(0+fi)
				self.b.write_array_pattern_16bit(phasecals[fi,chanset])
			
			print "Location %d initialized\n" %fi

	def loadpts(self,pts):
		# Calculate phase delays
		phasecals = steeringChargetimes(pts)
		
		# Program motherboards
		nLocs = pts.shape[0]
		moboProgram(phasecals,nLocs)
		
	def __init__(self,b1):
		self.nEls = 112
		self.nMobos = 4
		self.nMemLocs = self.nMobos*32
		
		self.b = b1
		#### SETUP ####
		self.b.stop_execution()
		
		
		
	
def basic112_steering(a,b,PRF,nPulses,locations):
	# Parameters:
	trig_len = 10e-6
	
	#### SETUP ####
	b.stop_execution()
	
	# Program motherboards
	nLocs = locations.shape[0]
		
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

	a.fire(0)
	a.set_trig(15)
	a.waitsec(trig_len)
	a.set_trig(0)

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
