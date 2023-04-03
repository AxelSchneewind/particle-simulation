#!/bin/python
import ctypes
from ctypes import *
import numpy as np

debug = False
if debug:
	c_lib = CDLL("./lib/libFastMultipole_dbg.so")
else:
	c_lib = CDLL("./lib/libFastMultipole.so")
	
c_setup = c_lib._Z5setupidd
c_setup_values = c_lib._Z12setup_valuesPdS_S_S_
c_setup_potentials = c_lib._Z16setup_potentialsiPdPi
c_update = c_lib._Z6updatev
c_position = c_lib._Z8positionv
c_position.restype = ctypes.c_void_p
c_velocity = c_lib._Z8velocityv
c_velocity.restype = ctypes.c_void_p
c_force = c_lib._Z5forcev
c_force.restype = ctypes.c_void_p
c_mass = c_lib._Z4massv
c_mass.restype = ctypes.c_void_p

c_potential = c_lib._Z9potential6VectorILi2EER13PotentialInfo
c_potential.restype = ctypes.c_double

c_access = c_lib._Z6accessv
c_release = c_lib._Z7releasev


class Vector(Structure):
    _fields_ = [("x", c_double),
                ("y", c_double)]
class PotentialInfo(Structure):
    _fields_ = [("potentialTerms", c_int),
                ("potentialCoefficients", ctypes.POINTER(c_double)),
                ("potentialExponents", ctypes.POINTER(c_int))]

N = 0


def access():
	c_access()

def release():
	c_release()

def array_from_address(address, dtype, shape):
	pointer = ctypes.cast(address, ctypes.POINTER(dtype))
	return np.ctypeslib.as_array(pointer, shape=shape)

def buffer():
	position  = array_from_address(c_position(), ctypes.c_double, (N, 2))
	velocity  = array_from_address(c_velocity(), ctypes.c_double, (N, 2))
	force  = array_from_address(c_force(), ctypes.c_double, (N, 2))
	mass  = array_from_address(c_mass(), ctypes.c_double, (N, 1))

	return (position, velocity, force, mass)

def pointer_from_array(array, dtype):
	p = array.ctypes.data_as(ctypes.POINTER(dtype))
	return p

def pointers(position, velocity, force, mass):
	p = pointer_from_array(position, ctypes.c_double)
	v = pointer_from_array(velocity, ctypes.c_double)
	f = pointer_from_array(force, ctypes.c_double)
	m = pointer_from_array(mass, ctypes.c_double)

	return (p,v,f,m)

# setup parameters
def setup(n, size, dt, pos, vel, f, m, potentials):
	global N
	N = n
	cN = ctypes.c_int(n)
	cSize = ctypes.c_double(size)
	cdt = ctypes.c_double(dt)

	potTerms = len(potentials[0])
	potentialCoefficients = np.ndarray((potTerms,), dtype=np.float64)
	potentialExponents = np.ndarray((potTerms,), dtype=np.int32)
	
	for i in range(potTerms):
		potentialCoefficients[i] = potentials[0][i]
		potentialExponents[i] = potentials[1][i]

	print('setting up simulation...')
	c_setup(cN, cSize, cdt)
	print('setting up potentials...')
	c_setup_potentials(potTerms, pointer_from_array(potentialCoefficients, ctypes.c_double), pointer_from_array(potentialExponents, ctypes.c_int32))

	print('setting up values...')
	(cPos, cVel, cForce, cMass) = pointers(pos, vel, f, m)
	c_setup_values(cPos, cVel, cForce, cMass)
	print('done')

	return buffer()


def potential(distance, simulation):
	return c_potential(Vector(distance, 0), PotentialInfo(simulation.potentials[0].shape[0], pointer_from_array(simulation.potentials[0], c_double), pointer_from_array(simulation.potentials[1], c_int)))

# update values
def update():
	c_update()
	return buffer()