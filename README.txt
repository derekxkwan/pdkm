==========================================
pdkm v0.1 - pure data external library
(formerly dxkpure)
by Derek Kwan, 2016

Released under GPL v3.0
==========================================

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
	- dklmunge - list munger
	- dklrnd - random values with triangular (linear) distribution
	- dkipls~ - impulse oscillator
	- dkmagikp~ - karplus-strong guitar synthesizer
	- dkmtx - control-rate multiple metro
        - dknoiz~ - (un)ramped noise with frequency
	- dknumpat - number pattern list converter
        - dkpink~ - 1/f^alpha noise generator
	- dkpop~ - random impulses of adjustable density
        - dkrndl - generate list of random numbers within a given range
        - dkrpt - repeater with optional zero-indexed counter
	- dksawlim~ - bandlimited sawtooth wave generator
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
        
NOTE FOR WINDOWS USERS:
        - I don't have an access to a windows machine so these are untested on Windows, I would try the directions up to "Building Pd-Vanilla" on the following link and then follow the regular directions:

        https://puredata.info/docs/developer/WindowsMinGW
