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
	- sawlim~ - bandlimited sawtooth wave generator
	- sinbank~ - sine oscillator bank
	- sinbankd~ - sine oscillator bank with separate envelopes with triggered output
	- sinbankr~ - sine oscillator bank with separate envelopes and signal amplitude control
	- sqlim~ - bandlimited square wave generator
	- scaledeg - maps scale degrees to midi notes, ported from SuperCollider
	- wesley~ - bitcrusher/downsampler

HOW TO INSTALL:
	- type "make"
	- create a folder within your externals directory (check out the pure data website 
	puredata.info if you don't know where it is) and move the following into it:
		- all .dll for windows/.pd_darwin for osx/.pd_linux files for linux
		- dxkpure-meta.pd 
		- all -help.pd files

		(or you can just move this entire folder into your externals directory
		if you don't mind the clutter)
