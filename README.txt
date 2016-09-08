==========================================
dxkpure v0.1 - pure data external library
by Derek Kwan, 2016

Released under GPL v3.0
==========================================

ABSTRACTIONS (written in pd vanilla):
(note, help files for abstractions coming soon, can just open subpatch for now)
	- dxkbd1~ - bass drum synth
        - dxkcospan~ - cosine panner
	- dxkhh1~ - hihat synth
	- dxksd1~ - snare drum synth
	- dxkmbang - bang multiplier
	- dxkmidirt - abstraction to handle midi real-time messages
        - dxknobu~ - sample chopper abstraction
	- dxknzgt - only let nonzero floats through
        - dxkphsrd~ - phasor sound reader abstraction
        - dxkrunmax - running maximum
        - dxkrunmin - running minimum
	- dxkrndgt - let through anything x% of the time
	- dxksyncb - synchronized banger
	- dxksyncf - synchronized floats
	- dxkvanctr - vanilla counter (can go both up and down)
	- dxkvandrk - vanilla drunk random number generator 
	- dxkvanphs~ - vanilla 3-delay line phaser/chorus effect 
	- dxkvuzi - vanilla uzi/multiple banger

EXTERNALS (compiled c code):

	- dxkadsr~ - triggered adsr envelope generator
	- dxkmetrox - control-rate multiple metro
	- dxkstut~ - windowed stutter effect
	- dxkeuclid - outputs euclidean rhythms based on the work of Godfried Toussaint
	- dxkernd - random values with exponential distribution
	- dxkgrnrd~ - granular sound buffer player
	- dxklrnd - random values with triangular (linear) distribution
	- dxkmagikp~ - karplus-strong guitar synthesizer
	- dxkmkv2 - second-order markov generator
	- dxkrndlst - generate list of random numbers within a given range
	- dxkred~ - red noise generator with variable level of "redness"
	- dxksawlim~ - bandlimited sawtooth wave generator
	- dxksnbnk~ - sine oscillator bank
	- dxksnbnkd~ - sine oscillator bank with separate envelopes with triggered output
	- dxksnbnkr~ - sine oscillator bank with separate envelopes and signal amplitude control
	- dxksqlim~ - bandlimited square wave generator
	- dxkscldeg - maps scale degrees to midi notes, ported from SuperCollider
	- dxkwesley~ - bitcrusher/downsampler

HOW TO INSTALL:
	- make sure you have the pure-data source installed (can be found at github.com/pure-data/pure-data )
	- type "make"
	- move folder (dxkpure) to externals folder
