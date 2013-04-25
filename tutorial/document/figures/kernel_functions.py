#!/usr/bin/env python
# -*- coding: utf8 -*-
import matplotlib.pyplot
import numpy
x = numpy.arange(-4,4,0.01)
SE = numpy.exp(-0.5 * (x ** 2))
M52 = (1 + numpy.sqrt(5) * numpy.abs(x) + 5/3.0 *((numpy.abs(x))**2) ) * numpy.exp(- numpy.sqrt(5) * numpy.abs(x))
M32 = (1 + numpy.sqrt(3) * numpy.abs(x) ) * numpy.exp(- numpy.sqrt(3) * numpy.abs(x))
matplotlib.pyplot.plot(x, SE,  '-',  label=u'Squared\nExponential')
matplotlib.pyplot.plot(x, M32, '--', label=u'Matérn 3/2')
matplotlib.pyplot.plot(x, M52, '-.', label=u'Matérn 5/2')
matplotlib.pyplot.legend()
matplotlib.pyplot.savefig('kernel_functions.pdf')
