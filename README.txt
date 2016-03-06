==========================================
dxkpure v0.1 - pure data external library
by Derek Kwan, 2016
==========================================

OBJECTS:

	- euclid - outputs euclidean rhythms based on the work of Godfried Toussaint
	- exprand - random values with exponential distribution
	- grainyrd~ - granular sound buffer player
	- linrand - random values with triangular (linear) distribution
	- randlist - generate list of random numbers within a given range
	- red~ - red noise generator with variable level of "redness"
	- resobank~ - frequency resonator bank with separate envelopes
	- sawlim~ - bandlimited sawtooth wave generator
	- sinbank~ - sine oscillator bank
	- sqlim~ - bandlimited square wave generator
	- scaledeg - maps scale degrees to midi notes, ported from SuperCollider

HOW TO INSTALL:
	- type "make"
	- create a folder within your externals directory called "dxkpure" and move all
	.dll if windows/.pd_darwin if osx/.pd_linux files as well as all -help.pd files
	into it (or easier, just move this whole folder in there if you don't mind the
	clutter)

