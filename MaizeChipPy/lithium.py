# Loads all functions including a commands, b commands, tests, and basic steering
# type "from varLoader import *" on startup of python using "sudo python"
#
# MaizeChipPy 1.0 - JJM - May 2017


from fpgaClass import *
from tests import *
from treatments import *
from patterns import *

x = FPGA()
a = a_funcs(x)
b = b_funcs(x)


		

