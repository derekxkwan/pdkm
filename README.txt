==========================================
pdkm v0.1 - pure data external library
(formerly dxkpure)
by Derek Kwan, 2016

Released under GPL v3.0
==========================================

ABSTRACTIONS (written in pd vanilla):
(note, help files for abstractions coming soon, can just open subpatch for now)
	- dkbbangsync - bang syncer
        - dkbinlist - int to binary list
	- dkbpm2ms - bpm to ms
        - dkcospan~ - cosine panner
        - dkfbdel~ - simple delay with fb
        - dkfor - for loop
	- dkincwr~ - incremental tabwriter
        - dkint~ - "typecast" signals to integer
	- dkloopplay~ - synced looper based around tabplay~
	- dkloopwin~ - signal-driven loop windowing
	- dkmbang - bang multiplier
	- dkmidirt - abstraction to handle midi real-time messages
	- dkms2smp - ms to samples
        - dknobu~ - sample chopper abstraction
        - dknot - control-rate bitwise not
	- dknzgt - only let nonzero floats through
        - dkphsrd~ - phasor sound reader abstraction
        - dkpolyish1 - polyish abs with no note off
        - dkrunmax - running maximum
        - dkrunmin - running minimum
	- dkrndgt - let through anything x% of the time
	- dkseqgen1 - timed random sequence generator
	- dksmp2ms - samples to ms
	- dkswing - bang swinger
	- dksyncb - synchronized banger
	- dksyncf - synchronized floats
	- dkunit2ms - ms calculator
	- dkvanctr - vanilla counter (can go both up and down)
	- dkvancomp~ - vanilla compressor/limiter
	- dkvandrk - vanilla drunk random number generator 
	- dkvanphs~ - vanilla 3-delay line phaser/chorus effect 
	- dkvavg - vanilla float averager
	- dkvisclock - visual clock
	- dkvuzi - vanilla uzi/multiple banger
	- dkxor - control-rate bitwise xor

EXTERNALS (compiled c code):

	- dkadsr~ - triggered adsr envelope generator
	- dkatmp~ - nth-order allpass
	- dkcmb~ - nth-order comb filter
	- dkclik~ - triggered sample-length impulses
	- dkclnoiz~ - outputs -1 or 1 randomly (adaptation of SuperCollider's ClipNoise)
	- dkcofork - control-outlet forker/router
	- dkdntl~ - comb filter with seperate feedforward- and feedback- delays
	- dkeuclid - outputs euclidean rhythms based on the work of Godfried Toussaint
	- dkenv~ - triggered envelope generator ala SuperCollider Env
	- dkernd - random values with exponential distribution
	- dkgpitch~ - rotating tapehead pitch shifter with four grains
	- dkgrnrd~ - granular sound buffer player
	- dklrnd - list munger
	- dklrnd - random values with triangular (linear) distribution
	- dkipls~ - impulse oscillator
	- dkmagikp~ - karplus-strong guitar synthesizer
	- dkmkv - first-order markov generator
	- dkmkv2 - second-order markov generator
	- dkmtx - control-rate multiple metro
        - dknoiz~ - (un)ramped noise with frequency
	- dknumpat - number pattern list converter
        - dkpink~ - 1/f^alpha noise generator
	- dkpop~ - random impulses of adjustable density
        - dkrndl - generate list of random numbers within a given range
        - dkrpt - repeater with optional zero-indexed counter
	- dksawlim~ - bandlimited sawtooth wave generator
	- dksnbnk~ - sine oscillator bank
	- dksnbnkd~ - sine oscillator bank with separate envelopes with triggered output
	- dksnbnkr~ - sine oscillator bank with separate envelopes and signal amplitude control
	- dkstut~ - windowed stutter effect
	- dksqlim~ - bandlimited square wave generator
	- dkscldeg - maps scale degrees to midi notes, ported from SuperCollider
	- dktanh~ - fast approxmation of tanh with pregain
	- dkwesley~ - bitcrusher/downsampler
	- dkzd~ - delay by samples

HOW TO INSTALL:
	- make sure you have the pure-data source installed (can be found at github.com/pure-data/pure-data )
	- type "make"
	- move folder (pdkm) to externals folder
