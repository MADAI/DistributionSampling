#!/usr/bin/python -u

#=========================================================================
#
#  Copyright (c) 2010-2012 The University of North Carolina at Chapel Hill
#  All rights reserved.
# 
#   Licensed under the MADAI Software License. You may obtain a copy of
#   this license at
# 
#          https://madai-public.cs.unc.edu/software/license/
# 
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
# 
# =========================================================================*/

# Emulator Emulator (used for testing emulator interface)
# Be sure to call with '-u' flag.


import sys
from math import *
def tok(readable):
	while True:
		line = readable.readline()
		if line == '':
			break
		for token in line.split():
			yield token
def f(x,y):
	scale = 0.2 * exp(-16 * ((x - 0.5)**2 + (y - 0.5)**2))
	return scale * (3.0 + sin(25 * x) + sin(25 * y))
o = sys.stdout
it = tok(sys.stdin)
o.write('2\nparam_0\nparam_1\n2\nmean_0\nvariance_0\n')
o.flush()
while True:
	try:
		x = float(it.next())
		y = float(it.next())
		o.write('%r\n%r\n' % (f(x,y), 0.0))
		o.flush()
	except (StopIteration,):
		break
