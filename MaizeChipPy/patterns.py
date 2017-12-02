# Set of sample steering patterns
# J. Macoskey - May 2017

import numpy as np
import math as m

def makeCenter():
	pattern = np.array([[0,0,0],[0,0,0]])
	return pattern

def make2Points(p1,p2):
	pattern = np.array([[0,p1,0],[0,p2,0]])
	return pattern

def make4Points():
	pattern = np.array([[5,0,0],[0,5,0],[-5,0,0],[0,-5,0]])
	return pattern

def makeallthePoints():
	pattern = np.zeros((11**3,3))
	pattern[:,:] = 0
	g=0
	for i in range(-5,6):
		for j in range(-5,6):
			for k in range(-5,6):
				pattern[g,0],pattern[g,1],pattern[g,2] = i,j,k
				g+=1
	
	return pattern
	
def makeCube():
	
	return
	
def makeSphere():
	
	return
	
def makeCircle():
	
	return
	
def makeSpinner():
	
	return
