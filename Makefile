#Makefile for pdkm

lib.name = pdkm

# headers
dkrnd := src/dkrnd.c
dkwtab := src/dkwtab.c
dktime := src/dktime.c
dkbuf := src/dkbuf.c
dkmem:= src/dkmem.c

dkcmb~.class.sources = src/dkcmb.c 
dkcofork.class.sources = src/dkcofork.c
dkctr.class.sources = src/dkctr.c 
dkdutysaw~.class.sources = src/dkdutysaw.c 
dkrpt.class.sources = src/dkrpt.c 
dkatmp~.class.sources = src/dkatmp.c 
dkdntl~.class.sources = src/dkdntl.c 
dkernd.class.sources = src/dkernd.c 
dkfm~.class.sources = src/dkfm.c $(dkwtab) 
dklmunge.class.sources = src/dklmunge.c $(dkmem) $(dkrnd)
dklrnd.class.sources = src/dklrnd.c 
dkscldeg.class.sources = src/dkscldeg.c 
dkeuclid.class.sources = src/dkeuclid.c 
dkgrnrd~.class.sources = src/dkgrnrd.c $(dkrnd) $(dkbuf) 
dkmbang.class.sources = src/dkmbang.c $(dktime)
dksawlim~.class.sources = src/dksawlim.c  $(dkwtab)
dksqlim~.class.sources = src/dksqlim.c  $(dkwtab)
dksnbnkd~.class.sources = src/dksnbnkd.c $(dkwtab)
dksnbnkr~.class.sources = src/dksnbnkr.c $(dkwtab) 
dksnbnk~.class.sources = src/dksnbnk.c $(dkwtab)
dkrndl.class.sources = src/dkrndl.c 
dktanh~.class.sources = src/dktanh.c 
dkwesley~.class.sources = src/dkwesley.c 
dkadsr~.class.sources = src/dkadsr.c 
dkmagikp~.class.sources = src/dkmagikp.c 
dkstut~.class.sources = src/dkstut.c 
dkmtx.class.sources = src/dkmtx.c $(dktime)
dkpop~.class.sources = src/dkpop.c $(dkrnd)
dkmkv.class.sources = src/dkmkv.c $(dkrnd)
dkmkv2.class.sources = src/dkmkv2.c $(dkrnd)
dknoiz~.class.sources = src/dknoiz.c $(dkrnd)
dkenv~.class.sources = src/dkenv.c $(dkwtab)
dkgpitch~.class.sources = src/dkgpitch.c $(dkwtab) $(dkrnd)
dkipls~.class.sources = src/dkipls.c $(dkrnd) 
dkpink~.class.sources = src/dkpink.c $(dkrnd) 
dkclnoiz~.class.sources = src/dkclnoiz.c $(dkrnd) 
dkclik~.class.sources = src/dkclik.c 
dknumpat.class.sources = src/dknumpat.c 
dkzd~.class.sources = src/dkzd.c $(dkwtab) 

datafiles =  pdkm-meta.pd \
			 dkred~-help.pd \
			 dkernd-help.pd \
			 dklrnd-help.pd \
			 dkfm~-help.pd \
			 dkscldeg-help.pd \
			 dkeuclid-help.pd \
			 dkgrnrd~-help.pd \
			 dksawlim~-help.pd \
			 dksqlim~-help.pd \
			 dksnbnkd~-help.pd \
			 dksnbnkr~-help.pd \
			 dksnbnk~-help.pd \
			 dkrndl-help.pd \
			 dkwesley~-help.pd \
			 dkadsr~-help.pd \
			 dkmagikp~-help.pd \
			 dkstut~-help.pd \
			 dkenv~-help.pd \
			 dkipls~-help.pd \
			 dkmtx-help.pd \
			 dkpop~-help.pd \
			 dkcmb~-help.pd \
			 dkatmp~-help.pd \
			 dkdntl~-help.pd \
			 dkctr-help.pd \
			 dkclnoiz~-help.pd \
			 dkipls~-help.pd \
			 dkclik~-help.pd \
			 README.txt \
			 LICENSE.txt

suppress-wunused = yes

include Makefile.pdlibbuilder
