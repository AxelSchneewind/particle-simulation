import numpy as np
import FM as fm
import random

class SimulationInfo:
	def __init__(self):
		# simulation parameters
		self.timePeriod:float = 10000

		m:int = 7# particles
		self.N:int = m*m
		self.area:int = 40#m
		self.fps:float = 30#Hz
		self.dt:float = 1/60#s

		# initial values
		position = np.ndarray((self.N, 2), dtype=np.float64)
		velocity = np.ndarray((self.N, 2), dtype=np.float64)
		force = np.ndarray((self.N, 2), dtype=np.float64)
		mass = np.ndarray((self.N,1), dtype=np.float64)
		
		for i in range(self.N):
			position[i][0] = 0.9 * (2*self.area / m) * (i // m - m/2) + random.uniform(-0.5, 0.5)
			position[i][1] = 0.9 * (2*self.area / m) * (i % m - m/2) + random.uniform(-0.5, 0.5)
		
			velocity[i] = [0,0]
			force[i] = [0,0]
			mass[i] = 1

		# setup potentials
		self.potentials = [np.array([-8, 20], dtype=float), np.array([1,2], dtype=int)]
	
		# setup c data
		(self.position, self.velocity, self.force, self.mass) = fm.setup(self.N, self.area, self.dt, position, velocity, force, mass, self.potentials)

	def access(self):
		fm.access()

	def release(self):
		fm.release()

	def update(self):
		(self.position, self.velocity, self.force, self.mass) = fm.update()

	def potential(self, distance):
		return fm.potential(distance, self)

	def buffer(self):
		return fm.buffer()

