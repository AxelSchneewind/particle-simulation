#!/bin/python
import numpy as np
from matplotlib import pyplot as plt
from SimulationInfo import SimulationInfo

def plot_potential(simulation):
	xValues = np.arange(1, simulation.area, 0.01)
	pValues = np.array([simulation.potential(x) for x in xValues])

	plt.plot(xValues, pValues)
	plt.show()
	plt.savefig('potential.png')


if __name__ == "__main__":
	# setup c data
	simulation = SimulationInfo()

	plot_potential(simulation)
	plt.cla()