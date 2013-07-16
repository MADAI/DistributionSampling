#!/usr/bin/env python
# -*- coding: utf-8 -*-
##=========================================================================
##
##  Copyright 2011-2013 The University of North Carolina at Chapel Hill
##  and Michigan State University.  All rights reserved.
##
##  Licensed under the MADAI Software License. You may obtain a copy of
##  this license at
##
##    https://madai-public.cs.unc.edu/visualization/software-license/
##
##  Unless required by applicable law or agreed to in writing, software
##  distributed under the License is distributed on an "AS IS" BASIS,
##  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
##  See the License for the specific language governing permissions and
##  limitations under the License.
##
##=========================================================================
import os, glob, math, sys
from ParabolicPotentialModel import ParabolicPotentialModel
def Interact(model):
	def tok(readable):
		while True:
			line = readable.readline()
			if line == '':
				break
			for token in line.split():
				if token.lower() == "stop":
					return
				yield token
	o = sys.stdout
	it = tok(sys.stdin)
	parameters = model.Parameters
	outputs = model.Outputs
	o.write('VERSION 1\nPARAMETERS %d\n' % len(parameters))
	for pname, ptype, arguments in parameters:
		o.write('%s\t%s\t%s\n' %(pname, ptype, '\t'.join(map(repr, arguments))))
	o.write('OUTPUTS %d\n' % len(outputs))
	for output in outputs:
		o.write('%s\n' % output)
	o.write('VARIANCE %d\nEND_OF_HEADER\n' % len(outputs))
	o.flush()
	nparams = len(parameters)
	while True:
		try:
			v,e = model.Run([it.next() for i in xrange(nparams)])
			assert len(v) == len(e) == len(outputs)
			o.write(' '.join(map(repr, v)))
			o.write('\n')
			o.write(' '.join(map(repr, e)))
			o.write('\n')
			o.flush()
		except (StopIteration,KeyboardInterrupt,):
			break
if __name__ == '__main__':
	Interact(ParabolicPotentialModel())
