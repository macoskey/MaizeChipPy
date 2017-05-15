# Loads all functions including a commands, b commands, tests, and basic steering
# type "from varLoader import *" on startup of python using "sudo python"
#
# MaizeChipPy 1.0 - JJM - May 2017


from fpga_Class import *
from tests import *
from treatments import *
from patterns import *

x = FPGA()

def b_stop_execution():
	x.b_stop_execution()

def bstop():
	x.bstop()

		
print "master branch test"
