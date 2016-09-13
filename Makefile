#Makefile for pdkm

lib.name = pdkm

# headers
dkrnd := src/dkrnd.c
dkwtab := src/dkwtab.c

dkred~.class.sources = src/dkred.c 
dkernd.class.sources = src/dkernd.c 
dkfm~.class.sources = src/dkfm.c 
dklrnd.class.sources = src/dklrnd.c 
dkscldeg.class.sources = src/dkscldeg.c 
dkeuclid.class.sources = src/dkeuclid.c 
dkgrnrd~.class.sources = src/dkgrnrd.c 
dksawlim~.class.sources = src/dksawlim.c  $(dkwtab)
dksqlim~.class.sources = src/dksqlim.c  $(dkwtab)
dksnbnkd~.class.sources = src/dksnbnkd.c $(dkwtab)
dksnbnkr~.class.sources = src/dksnbnkr.c $(dkwtab) 
dksnbnk~.class.sources = src/dksnbnk.c $(dkwtab)
dkrndl.class.sources = src/dkrndl.c 
dkwesley~.class.sources = src/dkwesley.c 
dkadsr~.class.sources = src/dkadsr.c 
dkmagikp~.class.sources = src/dkmagikp.c 
dkstut~.class.sources = src/dkstut.c 
dkmtx.class.sources = src/dkmtx.c 
dkpop~.class.sources = src/dkpop.c $(dkrnd)
dkmkv2.class.sources = src/dkmkv2.c $(dkrnd)
dkenv~.class.sources = src/dkenv.c $(dkwtab)
dkipls~.class.sources = src/dkipls.c $(dkrnd) 

datafiles =  help/pdkm-meta.pd \
			 help/dkred~-help.pd \
			 help/dkernd-help.pd \
			 help/dklrnd-help.pd \
			 help/dkfm~-help.pd \
			 help/dkscldeg-help.pd \
			 help/dkeuclid-help.pd \
			 help/dkgrnrd~-help.pd \
			 help/dksawlim~-help.pd \
			 help/dksqlim~-help.pd \
			 help/dksnbnkd~-help.pd \
			 help/dksnbnkr~-help.pd \
			 help/dksnbnk~-help.pd \
			 help/dkrndl-help.pd \
			 help/dkwesley~-help.pd \
			 help/dkadsr~-help.pd \
			 help/dkmagikp~-help.pd \
			 help/dkstut~-help.pd \
			 help/dkmtx-help.pd \
			 help/dkpop~-help.pd \
			 README.txt \
			 LICENSE.txt

suppress-wunused = yes

include Makefile.pdlibbuilder
