#!/bin/python
import numpy as np
import time
from SimulationInfo import SimulationInfo
import sys
import shutil
import os
import matplotlib
from matplotlib import pyplot as plt
import imageio.v2 as imageio
import io

Vector = np.array

import matplotlib.style as mplstyle
matplotlib.use('agg')
mplstyle.use('fast')
mplstyle.use(['ggplot', 'fast'])

def display(position, velocity, area:float, ax, fig, frame:int, showVelocity = False, outputWriter=None):
	# show positions and velocities
	posT = np.transpose(position)
	ax.scatter(posT[0], posT[1], color=['red'] + ['black']*(len(position)-1), s=4, label=frame)

	if showVelocity:
		velT = np.transpose(velocity)
		ax.quiver(posT[0], posT[1], velT[0], velT[1])

	# set layout
	lim = [int(-area) - 1, int(area) + 1]
	ax.set_xlim(lim)
	ax.set_ylim(lim)
	ax.set_title(frame)

	if outputWriter != None:
		stream = io.BytesIO()
		fig.savefig(stream)
		outputWriter.append_data(imageio.imread(stream))

	ax.clear()


def plot_potential(simulation):
	xValues = np.arange(0.01, simulation.area, 0.1)
	pValues = np.array([simulation.potential(x) for x in xValues])

	plt.plot(xValues, pValues)
	plt.savefig('potential.png')


def sim(outputFile):
	# setup c data
	simulation = SimulationInfo()

	#plot_potential(simulation)
	plt.cla()

	frame = 0

	# initialize plot
	fig,ax = plt.subplots()
	writer = imageio.get_writer('/tmp/' + outputFile + '.gif', format='gif', fps=simulation.fps)
	writer.write = writer.append_data
	display(simulation.position, simulation.velocity, simulation.area + 2, ax, fig, frame, outputWriter=writer)

	# calculate time steps
	frameCount = int((simulation.timePeriod / simulation.dt))
	simTime:float = 0
	simTimeSinceRender:float = 0
	print('rendering',frameCount, 'frames')
	for i in range(1, frameCount):
		frame = i
		
		simulation.update()

		simTime += simulation.dt
		simTimeSinceRender += simulation.dt

		if simTimeSinceRender >= 1/simulation.fps:
			print('simTime', simTime, 's')

			t = time.clock_gettime_ns(0)
			simulation.access()
			display(simulation.position, simulation.velocity, simulation.area, ax, fig, frame, outputWriter=writer)
			simulation.release()
			t2 = time.clock_gettime_ns(0)

			simTimeSinceRender -= 1/simulation.fps

	writer.close()
	shutil.move('/tmp/' + outputFile + '.gif', './' + outputFile + '.gif')
	print()


if __name__ == '__main__':
	outputName = sys.argv[1] if len(sys.argv) > 1 else str(time.time())
	sim(outputName)
