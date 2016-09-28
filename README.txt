==========================================
pdkm v0.1 - pure data external library
(formerly dxkpure)
by Derek Kwan, 2016

Released under GPL v3.0
==========================================

ABSTRACTIONS (written in pd vanilla):
(note, help files for abstractions coming soon, can just open subpatch for now)
	- dkbd1~ - bass drum synth
	- dkbd2~ - 808ish bass drum synth (requires dkclik~)
        - dkcospan~ - cosine panner
	- dkhh1~ - hihat synth
	- dksd1~ - snare drum synth
	- dkmbang - bang multiplier
	- dkmidirt - abstraction to handle midi real-time messages
        - dknobu~ - sample chopper abstraction
	- dknzgt - only let nonzero floats through
        - dkphsrd~ - phasor sound reader abstraction
        - dkrunmax - running maximum
        - dkrunmin - running minimum
	- dkrndgt - let through anything x% of the time
	- dksyncb - synchronized banger
	- dksyncf - synchronized floats
	- dkvanctr - vanilla counter (can go both up and down)
	- dkvandrk - vanilla drunk random number generator 
	- dkvanphs~ - vanilla 3-delay line phaser/chorus effect 
	- dkvuzi - vanilla uzi/multiple banger

EXTERNALS (compiled c code):

	- dkadsr~ - triggered adsr envelope generator
	- dkclk~ - triggered single sample impulse
	- dkmtx - control-rate multiple metro
	- dkstut~ - windowed stutter effect
	- dkeuclid - outputs euclidean rhythms based on the work of Godfried Toussaint
	- dkenv~ - triggered envelope generator ala SuperCollider Env
	- dkernd - random values with exponential distribution
	- dkgrnrd~ - granular sound buffer player
	- dklrnd - random values with triangular (linear) distribution
	- dkipls~ - impulse oscillator
	- dkmagikp~ - karplus-strong guitar synthesizer
	- dkmkv2 - second-order markov generator
	- dknumpat - number pattern list converter
        - dkpink~ - 1/f^alpha noise generator
	- dkpop~ - random impulses of adjustable density
        - dkrndl - generate list of random numbers within a given range
	- dksawlim~ - bandlimited sawtooth wave generator
	- dksnbnk~ - sine oscillator bank
	- dksnbnkd~ - sine oscillator bank with separate envelopes with triggered output
	- dksnbnkr~ - sine oscillator bank with separate envelopes and signal amplitude control
	- dksqlim~ - bandlimited square wave generator
	- dkscldeg - maps scale degrees to midi notes, ported from SuperCollider
	- dkwesley~ - bitcrusher/downsampler

HOW TO INSTALL:
	- make sure you have the pure-data source installed (can be found at github.com/pure-data/pure-data )
	- type "make"
	- move folder (pdkm) to externals folder
