# Set of aberration correctoin treatment scripts for testing
# J. Macoskey - June 2017

import serial
import numpy as np
import scipy.io as sio
import math as m
import time


#~ def twoLoc112_upload(a,b):
	#~ b.stop_execution()
	
	#~ loc1 = np.array([5,0,0])
	#~ loc2 = np.array([-5,0,0])
	
	#~ phasecal1 = v.singleLocChargetimes(loc1) # located in treatments_2.py
	#~ phasecal2 = v.singleLocChargetimes(loc2)
	
	#~ phasecalmix = np.empty(phasecal1.shape)
	#~ for li in range(0,len(phasecalmix)):
		#~ if li%2 == 0: phasecalmix[li] = phasecal1[li]
		#~ else: phasecalmix[li] = phasecal2[li]
	
	#~ moboProgram(a,b,phasecalmix,1)
	
	#~ return phasecal1, phasecal2, phasecalmix
	
