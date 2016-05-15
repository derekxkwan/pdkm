#Makefile for dxkpure

lib.name = dxkpure

class.sources = src/red~.c \
				src/exprand.c \
				src/linrand.c \
				src/scaledeg.c \
				src/euclid.c \
				src/grainyrd~.c \
				src/sawlim~.c \
				src/sqlim~.c \
				src/sinbankd~.c \
				src/sinbankr~.c \
				src/sinbank~.c \
				src/randlist.c \
				src/wesley~.c \
				src/dxkadsr~.c \
				src/magikp~.c \
				src/dxkstut~.c \
				src/dxkmetrox.c 

datafiles =  help/dxkpure-meta.pd \
			 help/red~-help.pd \
			 help/exprand-help.pd \
			 help/linrand-help.pd \
			 help/scaledeg-help.pd \
			 help/euclid-help.pd \
			 help/grainyrd~-help.pd \
			 help/sawlim~-help.pd \
			 help/sqlim~-help.pd \
			 help/sinbankd~-help.pd \
			 help/sinbankr~-help.pd \
			 help/sinbank~-help.pd \
			 help/randlist-help.pd \
			 help/wesley~-help.pd \
			 help/dxkadsr~-help.pd \
			 help/magikp~-help.pd \
			 help/dxkstut~-help.pd \
			 help/dxkmetrox-help.pd \
			 README.txt \
			 LICENSE.txt

include Makefile.pdlibbuilder
