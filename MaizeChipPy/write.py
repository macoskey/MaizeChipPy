# Sends appropriate commands to write a full set of pattern information
# to maizechip FPGA program.
#
# data is a 32 element vector of 16 bit numbers (0...65535) 
# representing either phase or amplitude information for a pattern
#
# For example, data(5) = 46
# This is 46 clock tics of phase delay or charge time for channel 5
#
# MaizeChip 1.0 Feb 2014 TLH
# MaizeChipPy 1.0 May 2017 JJM

import time
import serial
import numpy as np

def write_array_pattern_16bit(data):
	bindata = format(data,'016b')
	binsw = [bindata[:,8:16],bindata[:,0:8]]
	
	
	return
