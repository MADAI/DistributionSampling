import math

class ParabolicPotentialModel(object):
	"""
	Parabolic Potential Model
	Copyright 2012-2013, Michigan State University.
	The software was written in 2013 by Scott Pratt
	while working for the MADAI project <http://madai.us/>
	This is inteded as an example of a parameterized model of a
	physical system with a parabolic potential
	"""
	Parameters = [
		('X0',   'UNIFORM',   (-2.0,    2.0)),
		('Kinv', 'UNIFORM',    (0.25,   4.0)),
		('TEMP', 'UNIFORM',    (0.25,   4.0))]
	Outputs = [
		'MEAN_X',
		'MEAN_X_SQUARED',
		'MEAN_ENERGY']
	@staticmethod
	def Run(x):
		def CalcObservables(X0, Kinv, T):
			root2 = math.sqrt(2.0)
			PI = math.pi
			I0 = math.sqrt(PI * Kinv * T / 2.0) * math.erfc(
				- X0 * math.sqrt(1.0 / (Kinv * T)) / root2)
			f = math.exp(-0.5 * (X0 ** 2) / (Kinv * T))
			I1 = (Kinv * T) * math.exp(-0.5 * (X0 ** 2) / (Kinv * T)) + X0 * I0
			I2 = (Kinv * T) * (I0 - (X0 * f)) + 2.0 * X0 * I1 - (X0 ** 2) * I0
			xbar = I1 / I0;
			x2bar = I2 / I0;
			ebar = (0.5 * T) + (0.5 / Kinv) * (x2bar + (X0 ** 2) - 2.0 * X0 * xbar)
			return (xbar,x2bar,ebar)

		def CalcObservablesAndError(X0, Kinv, T, uncertainty_scale):
			xbar, x2bar, ebar = CalcObservables(X0, Kinv, T)
			sigmax = math.sqrt(Kinv * T);
			xbar_error = uncertainty_scale * sigmax
			x2bar_error = uncertainty_scale * ((sigmax ** 2) + (2.0 * xbar * sigmax))
			ebar_error = uncertainty_scale * T + (0.5 / Kinv) * uncertainty_scale * (
				(sigmax ** 2) + (2.0 * xbar * sigmax))
			return ( [xbar, x2bar, ebar], [xbar_error, x2bar_error, ebar_error] )

		X0, Kinv, T = map(float, x[:3])
		uncertainty_scale = 0.05
		return CalcObservablesAndError(X0, Kinv, T, uncertainty_scale)
