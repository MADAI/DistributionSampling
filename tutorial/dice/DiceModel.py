#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Dice Model
Copyright 2012-2013, The University of North Carolina at Chapel Hill.

This software was written in 2012 by Hal Canary <cs.unc.edu/~hal/>
while working for the MADAI project <http://madai.us/>.

See <https://madai-public.cs.unc.edu/visualization/software-license/> for license
information.

This is intended as an example of:

-   A parametraized model of a physical process

"""
import random

def dice_model(x,N=1000):
	"""
	This function takes in a point in parameter space (x) and models N
	roles of a weighted dice.
	returns fraction of the first 5 sides.
	"""
	P = [0.0 for i in xrange(6)]
	for i in xrange(4):
		P[i] = 0.3 * x[i]
	P[4] = (x[4] + x[5]) * 0.3
	P[5] = 1.0 - sum(P[:5])
	cumlative_prob = [ sum(P[:(i+1)]) for i in xrange(len(P)) ]
	count = [0 for i in xrange(6)]
	for i in xrange(N):
		r = random.random()
		if r < cumlative_prob[0]:
			count[0] += 1
		elif r < cumlative_prob[1]:
			count[1] += 1
		elif r < cumlative_prob[2]:
			count[2] += 1
		elif r < cumlative_prob[3]:
			count[3] += 1
		elif r < cumlative_prob[4]:
			count[4] += 1
		else:
			count[5] += 1
	assert sum(count) == N
	return [x / float(N) for x in count[:5]]
