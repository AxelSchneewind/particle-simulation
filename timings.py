#!/bin/python
import math
import pandas as pd
import sys
import os

from matplotlib import pyplot as plt




def describe(file):
	df = pd.read_csv(file, usecols= ['tree', 'expansions', 'grid', 'forces', 'positions', 'total']) 
	df = df.iloc[5:]

	N = file.replace("timings-", "").replace(".csv","")

	print(N)
	print(df.describe())
	
	with open('results.csv', 'a') as out:
		out.write('\n')
		#out.write('#### NEW ####\n')
		out.write(N)
		for val in df.mean().values:
			out.write(',')
			out.write(str(val))


def plot():
	df = pd.read_csv('results.csv', usecols= ['N', 'tree', 'expansions', 'grid', 'forces', 'positions', 'total']) 

	df = df.sort_values('N')
	N = df['N'].to_numpy()

	sqr = [50 * n**2 for n in N]
	nlogn = [100 * n * math.log(n) for n in N]

	plt.plot(N, sqr)
	plt.plot(N, nlogn)

	for c in df.columns:
		if not c == 'total':
			col = df[c].to_numpy()
			plt.plot(N, col, label=c)


	#plt.yscale("log")
	#plt.xscale("log")

	plt.legend()
	plt.show()


if __name__ == '__main__':
	if len(sys.argv) > 1:
		plot()
	else:
		for file in os.listdir():
			if file.startswith('timings') and file.endswith('.csv'):
				describe(file)