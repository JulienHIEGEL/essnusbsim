# ----------------------------------------------------------------------------
#
# Makefile for DHELAS 3.0 library
# Feb. 28, 2001
#
# ----------------------------------------------------------------------------
#
# Use a TAB to precede shell commands (e.g., f90, ar, etc).
# Note: this Makefile uses features that *may not*
# be supported by make utilities other than GNU make.
#
# ----------------------------------------------------------------------------
# coupsm.F and coupsm.inc are not to be included if HELAS is used with 
# the 2003 version of MadEvent
# ----------------------------------------------------------------------------

FC            = g77

FFLAGS        = -O -I.

LD	      = ld

DEST	      = ./

EXTHDRS	      =

HDRS	      =

INSTALL	      = /etc/install

LIBRARY	      = ./libdhelas31.a

MAKEFILE      = Makefile

OBJS 	      =	boostx.o \
		coupsm.o \
		eaixxx.o \
		eaoxxx.o \
		ficxxx.o \
		focxxx.o \
		fsicxx.o \
		fsixxx.o \
		fsocxx.o \
		fsoxxx.o \
		ftixxx.o \
		ftoxxx.o \
		fvicxx.o \
		fvixxx.o \
		fvoxxx.o \
		fvtixx.o \
		fvtoxx.o \
		ggggtx.o \
		ggggxx.o \
		gggxxx.o \
		hiocxx.o \
		hioxxx.o \
		hsssxx.o \
		hssxxx.o \
		hstxxx.o \
		hvsxxx.o \
		hvvsxx.o \
		hvvxxx.o \
		ioscxx.o \
		iosxxx.o \
		iotxxx.o \
		iovcxx.o \
		iovtxx.o \
		iovxxx.o \
		ixxxxx.o \
		j3xxxx.o \
		jeexxx.o \
		jgggtx.o \
		jgggxx.o \
		jggxxx.o \
		jiocxx.o \
		jiotxx.o \
		jioxxx.o \
		jssxxx.o \
		jvssxx.o \
		jvsxxx.o \
		jvtxxx.o \
		jvvtxx.o \
		jvvxxx.o \
		jw3wxx.o \
		jwwwxx.o \
		mom2cx.o \
		momntx.o \
		oxxxxx.o \
		rotxxx.o \
		ssssxx.o \
		sssxxx.o \
		sstxxx.o \
		sxxxxx.o \
		txxxxx.o \
		uggggx.o \
		uiovxx.o \
		uioxxx.o \
		ussxxx.o \
		uvvvxx.o \
		uvvxxx.o \
		vssxxx.o \
		vvssxx.o \
		vvsxxx.o \
		vvtxxx.o \
		vvvtxx.o \
		vvvxxx.o \
		vxxxxx.o \
		w3w3xx.o \
		wwwwxx.o

SRCS 	      =	boostx.F \
		coupsm.F \
		eaixxx.F \
		eaoxxx.F \
		ficxxx.F \
		focxxx.F \
		fsicxx.F \
		fsixxx.F \
		fsocxx.F \
		fsoxxx.F \
		ftixxx.F \
		ftoxxx.F \
		fvicxx.F \
		fvixxx.F \
		fvoxxx.F \
		fvtixx.F \
		fvtoxx.F \
		ggggtx.F \
		ggggxx.F \
		gggxxx.F \
		hiocxx.F \
		hioxxx.F \
		hsssxx.F \
		hssxxx.F \
		hstxxx.F \
		hvsxxx.F \
		hvvsxx.F \
		hvvxxx.F \
		ioscxx.F \
		iosxxx.F \
		iotxxx.F \
		iovcxx.F \
		iovtxx.F \
		iovxxx.F \
		ixxxxx.F \
		j3xxxx.F \
		jeexxx.F \
		jgggtx.F \
		jgggxx.F \
		jggxxx.F \
		jiocxx.F \
		jiotxx.F \
		jioxxx.F \
		jssxxx.F \
		jvssxx.F \
		jvsxxx.F \
		jvtxxx.F \
		jvvtxx.F \
		jvvxxx.F \
		jw3wxx.F \
		jwwwxx.F \
		mom2cx.F \
		momntx.F \
		oxxxxx.F \
		rotxxx.F \
		ssssxx.F \
		sssxxx.F \
		sstxxx.F \
		sxxxxx.F \
		txxxxx.F \
		uggggx.F \
		uiovxx.F \
		uioxxx.F \
		ussxxx.F \
		uvvvxx.F \
		uvvxxx.F \
		vssxxx.F \
		vvssxx.F \
		vvsxxx.F \
		vvtxxx.F \
		vvvtxx.F \
		vvvxxx.F \
		vxxxxx.F \
		w3w3xx.F \
		wwwwxx.F

PRINT	      = pr

SHELL	      = /bin/sh

SYSHDRS	      =

MFLAGS        = -e

#.F.o:
%.o : %.F
	$(FC) $(FFLAGS) -c $<

%.o : %.f
	$(FC) $(FFLAGS) -c $<

all:		$(LIBRARY)

helas:
		-rm -f *.o
		export FFLAGS='-O +cpp' && \
		export LIBRARY=libdhelas3.a && \
		$(MAKE) $(MFLAGS)

helas_check:
		-rm -f *.o
		export FFLAGS='-O +cpp -DHELAS_CHECK' && \
		export LIBRARY=libdhelas3_check.a && \
		$(MAKE) $(MFLAGS)

install-helas:
		export LIBRARY=libdhelas3.a && \
		$(MAKE) $(MFLAGS) install	

install-helas_check:
		export LIBRARY=libdhelas3.a && \
		$(MAKE) $(MFLAGS) install	

$(LIBRARY):	$(OBJS)
		@echo  "Loading $(LIBRARY) ... "
		@ar cru $(LIBRARY) $(OBJS)
		@echo "done"

clean:;		@rm -f $(OBJS) core

clobber:;	@rm -f $(OBJS) $(LIBRARY) core tags

install:        $(LIBRARY)
	@echo Installing $(LIBRARY) in $(DEST)
	@if [ $(DEST) != . ]; then \
	(rm -f $(DEST)/$(LIBRARY); $(INSTALL) -f $(DEST) $(LIBRARY)); fi


