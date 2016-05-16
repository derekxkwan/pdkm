#Makefile for dxkpure

lib.name = dxkpure

class.sources = src/dxkred.c \
				src/dxkernd.c \
				src/dxklrnd.c \
				src/dxkscldeg.c \
				src/dxkeuclid.c \
				src/dxkgrnrd.c \
				src/dxksawlim.c \
				src/dxksqlim.c \
				src/dxksnbnkd.c \
				src/dxksnbnkr.c \
				src/dxksnbnk.c \
				src/dxkrndlst.c \
				src/dxkwesley.c \
				src/dxkadsr.c \
				src/dxkmagikp.c \
				src/dxkstut.c \
				src/dxkmetrox.c 

datafiles =  help/dxkpure-meta.pd \
			 help/dxkred~-help.pd \
			 help/dxkernd-help.pd \
			 help/dxklrnd-help.pd \
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
			 README.txt \
			 LICENSE.txt

include Makefile.pdlibbuilder
