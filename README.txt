==========================================
dxkpure v0.1 - pure data external library
by Derek Kwan, 2016

Released under GPL v3.0
==========================================

OBJECTS:

	- dxkadsr~ - triggered adsr envelope generator
	- dxkmetrox - control-rate multiple metro
	- dxkstut~ - windowed stutter effect
	- dxkeuclid - outputs euclidean rhythms based on the work of Godfried Toussaint
	- dxkernd - random values with exponential distribution
	- dxkgrnrd~ - granular sound buffer player
	- dxklrnd - random values with triangular (linear) distribution
	- dxkmagikp~ - karplus-strong guitar synthesizer
	- dxkrndlst - generate list of random numbers within a given range
	- dxkred~ - red noise generator with variable level of "redness"
	- dxksawlim~ - bandlimited sawtooth wave generator
	- dxksnbnk~ - sine oscillator bank
	- dxksnbnkd~ - sine oscillator bank with separate envelopes with triggered output
	- dxksnbnkr~ - sine oscillator bank with separate envelopes and signal amplitude control
	- dxksqlim~ - bandlimited square wave generator
	- dxkscldeg - maps scale degrees to midi notes, ported from SuperCollider
	- dxkwesley~ - bitcrusher/downsampler

HOW TO INSTALL (pdlibbuilder version coming soon):
	- type "make"
	- create a folder within your externals directory (check out the pure data website 
	puredata.info if you don't know where it is) and move the following into it:
		- all .dll for windows/.pd_darwin for osx/.pd_linux files for linux
		- dxkpure-meta.pd 
		- all -help.pd files

		(or you can just move this entire folder into your externals directory
		if you don't mind the clutter)
