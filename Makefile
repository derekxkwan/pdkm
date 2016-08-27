#Makefile for dxkpure

lib.name = dxkpure

# headers
dxkrand := src/dxkrand.c


dxkred~.class.sources = src/dxkred.c 
dxkernd.class.sources = src/dxkernd.c 
dxkfm~.class.sources = src/dxkfm.c 
dxklrnd.class.sources = src/dxklrnd.c 
dxkscldeg.class.sources = src/dxkscldeg.c 
dxkeuclid.class.sources = src/dxkeuclid.c 
dxkgrnrd~.class.sources = src/dxkgrnrd.c 
dxksawlim~.class.sources = src/dxksawlim.c 
dxksqlim~.class.sources = src/dxksqlim.c 
dxksnbnkd~.class.sources = src/dxksnbnkd.c 
dxksnbnkr~.class.sources = src/dxksnbnkr.c 
dxksnbnk~.class.sources = src/dxksnbnk.c 
dxkrndlst.class.sources = src/dxkrndlst.c 
dxkwesley~.class.sources = src/dxkwesley.c 
dxkadsr~.class.sources = src/dxkadsr.c 
dxkmagikp~.class.sources = src/dxkmagikp.c 
dxkstut~.class.sources = src/dxkstut.c 
dxkmetrox.class.sources = src/dxkmetrox.c 
dxkpop~.class.sources = src/dxkpop.c $(dxkrand)

datafiles =  help/dxkpure-meta.pd \
			 help/dxkred~-help.pd \
			 help/dxkernd-help.pd \
			 help/dxklrnd-help.pd \
			 help/dxkfm~-help.pd \
			 help/dxkscldeg-help.pd \
			 help/dxkeuclid-help.pd \
			 help/dxkgrnrd~-help.pd \
			 help/dxksawlim~-help.pd \
			 help/dxksqlim~-help.pd \
			 help/dxksnbnkd~-help.pd \
			 help/dxksnbnkr~-help.pd \
			 help/dxksnbnk~-help.pd \
			 help/dxkrndlst-help.pd \
			 help/dxkwesley~-help.pd \
			 help/dxkadsr~-help.pd \
			 help/dxkmagikp~-help.pd \
			 help/dxkstut~-help.pd \
			 help/dxkmetrox-help.pd \
			 help/dxkpop~-help.pd \
			 README.txt \
			 LICENSE.txt

suppress-wunused = yes

include Makefile.pdlibbuilder
