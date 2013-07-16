import math
class ParabolicPotentialModel(object):
	"""
	Parabolic Potential Model
	Copyright 2012-2013, Michigan State University.
	The software was written in 2013 by Jeffrey Wyka
	while working for the MADAI project <http://madai.us/>
	This is inteded as an example of a parameterized model of a
	physical system with a parabolic potential
	"""
	Parameters = [
		('X0',   'UNIFORM',   (-2.0,    2.0)),
		('K',    'UNIFORM',    (0.5,    4.0)),
		('TEMP', 'UNIFORM',    (0.5,   10.0))]
	Outputs = [
		'MEAN_X',
		'MEAN_X_SQUARED',
		'MEAN_ENERGY']
	@staticmethod
	def Run(x):
		for i in xrange(1,3):
			if x[i] <= 0:
				raise Exception('Parameter %d cannot be non-positive' % (i+1))

		x0, k, T = x[:3]

		def GetGaussianIntegral( power, scale  ):
			n = int(power)
			alpha = float(scale)
			Integral = float(1.0)
			if n % 2 == 1:
				Integral = 0.0
			elif n % 2 == 0:
				for i in xrange(n/2, n):
					Integral *= float(i + 1)
				Integral *= (2.0 * math.sqrt(3.14159265))
				Integral /= ( 2.0 * math.sqrt(alpha) )**( n+1 )
			return Integral

		# Calculate the normalization
		# Integrate( e^( -k/T*(x-x0)^2 ) )
		normalization = float(GetGaussianIntegral(0, k/T))

		# Calculate Mean X
		# Integrate( x e^( -k/T*(x - x0)^2 ) ) / normalization
		MeanX = (float(GetGaussianIntegral(1, k/T))
				 + x0*float(GetGaussianIntegral(0, k/T)))
		MeanX /= normalization

		# Calculate Mean X^2
		# Integrate( x^2 e^( -k/T*(x - x0)^2 ) ) / normalization
		MeanX2 = float(GetGaussianIntegral(2, k/T))
		MeanX2 += 2.0*x0*float(GetGaussianIntegral(1, k/T))
		MeanX2 += x0*x0*float(GetGaussianIntegral(0, k/T))
		MeanX2 /=normalization

		# Calculate Mean Energy
		# Integrate( k (x - x0)^2 * e^( -k/T*(x - x0)^2 ) ) /
		#					 normalization + T/2
		MeanE = k*float(GetGaussianIntegral(2, k/T))/normalization + T/2.0

		# Calculate Mean X^4 for getting the model error
		# Integrate( x^4 e^( -k/T*(x-x0)^2 ) ) / normalization
		MeanX4 = float(GetGaussianIntegral(4, k/T))
		MeanX4 += 4.0*x0*float(GetGaussianIntegral(3, k/T))
		MeanX4 += 6.0*x0*x0*float(GetGaussianIntegral(2, k/T))
		MeanX4 += 4.0*(x0**3)*float(GetGaussianIntegral(1, k/T))
		MeanX4 += (x0**4)*float(GetGaussianIntegral(0, k/T))
		MeanX4 /= normalization

		# Calculate Mean Energy^2 for getting the model error
		# Integrate( ( k^2*(x - x0)^4 + k*T*(x - x0)^2 )
		#					  * e^( -k/T*(x-x0)^2 ) ) + 0.25*T^2
		MeanE2 = k*k*float(GetGaussianIntegral(4, k/T))/normalization
		MeanE2 += k*T*float(GetGaussianIntegral(2, k/T))/normalization
		MeanE2 += T*T/4.0

		# Calculate Errors
		ErrorX = (MeanX2 - MeanX**2)**(0.5)
		ErrorX2 = (MeanX4 - MeanX2**2)**(0.5)
		ErrorE = (MeanE2 - MeanE**2)**(0.5)

		return ( [ MeanX, MeanX2, MeanE], [ErrorX, ErrorX2, ErrorE ])
