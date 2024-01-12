include Makefile.arch

#------------------------------------------------------------------------------

FOURVECROOTO       = src/fluka4vectorsToRoot.$(ObjSuf)
FOURVECROOTS       = src/fluka4vectorsToRoot.$(SrcSuf)
FOURVECROOT        = bin/fluka4vectorsToRoot$(ExeSuf)

CNGSTARGETO       = src/CngsTarget.$(ObjSuf)
CNGSTARGETS       = src/CngsTarget.$(SrcSuf)
CNGSTARGET        = bin/CngsTarget$(ExeSuf)

PARTICLEO      = src/Particle.$(ObjSuf)
PARTICLES      = src/Particle.$(SrcSuf)
PARTICLESO     = libs/Particle.$(DllSuf)

HORNO      = src/Horn.$(ObjSuf)
HORNS      = src/Horn.$(SrcSuf)
HORNSO     = libs/Horn.$(DllSuf)

FOCUSO      = src/FocusingGeometry.$(ObjSuf)
FOCUSS      = src/FocusingGeometry.$(SrcSuf)
FOCUSSO     = libs/FocusingGeometry.$(DllSuf)

HELPERSO      = src/Helpers.$(ObjSuf)
HELPERSS      = src/Helpers.$(SrcSuf)
HELPERSSO     = libs/Helpers.$(DllSuf)

CONFIGO      = src/ConfigFile.$(ObjSuf)
CONFIGS      = src/ConfigFile.$(SrcSuf)
CONFIGSO     = libs/ConfigFile.$(DllSuf)

OBJS          = $(FOURVECROOTO) $(CNGSTARGETO) $(HELPERSO) $(PARTICLEO) $(HORNO) $(FOCUSO) $(CONFIGO) mydict.o

LIBS          += HELAS/libdhelas31.a libs/Helpers.so libs/Particle.so libs/Horn.so libs/ConfigFile.so libs/FocusingGeometry.so

PROGRAMS      = $(HELPERSSO) mydict.C $(PARTICLESO) $(HORNSO) $(FOCUSSO) $(CONFIGSO)  $(FOURVECROOT) $(CNGSTARGET)

#------------------------------------------------------------------------------

.SUFFIXES: .$(SrcSuf) .$(ObjSuf) .$(DllSuf)
.PHONY:   

all:            $(PROGRAMS)

helpers:       $(HELPERSSO) $(CONFIGSO)
$(HELPERSSO):  $(HELPERSO)
		$(LD) $(SOFLAGS) $(LDFLAGS) $^ $(EXPLLINKLIBS) $(OutPutOpt)$@
		$(MT_DLL)

particle:       $(PARTICLESO)
$(PARTICLESO):  $(PARTICLEO) 
		$(LD) $(SOFLAGS) $(LDFLAGS) $^ $(EXPLLINKLIBS) $(OutPutOpt)$@
		$(MT_DLL)

horn:       $(HORNSO)
$(HORNSO):  $(HORNO) 
		$(LD) $(SOFLAGS) $(LDFLAGS) $^ $(EXPLLINKLIBS) $(OutPutOpt)$@
		$(MT_DLL)

focus:       $(FOCUSSO)
$(FOCUSSO):  $(FOCUSO) 
		$(LD) $(SOFLAGS) $(LDFLAGS) $^ $(EXPLLINKLIBS) $(OutPutOpt)$@
		$(MT_DLL)

$(CONFIGSO):  $(CONFIGO)
		$(LD) $(SOFLAGS) $(LDFLAGS) $^ $(EXPLLINKLIBS) $(OutPutOpt)$@
		$(MT_DLL)

$(FOURVECROOT):      $(FOURVECROOTO) mydict.o
		$(LD) $(LDFLAGS) $^ $(LIBS) $(OutPutOpt)$@
		$(MT_EXE)
		@echo " ===> $@ compiled"

$(CNGSTARGET):      $(CNGSTARGETO) mydict.o
		$(LD) $(LDFLAGS) $^ $(LIBS) $(OutPutOpt)$@
		$(MT_EXE)
		@echo " ===> $@ compiled"

		rm -f $(OBJS) mydict.?

clean:
		@rm -f $(OBJS) mydict.?

distclean:      clean
		@rm -f $(PROGRAMS) $(EVENTSO) $(EVENTLIB) *Dict.* *.def \
		*.exp *.ps *.so *.lib *.dll *.d *.log *~ .def so_locations mydict.* \
		*LinkDef.h libs/*

.SUFFIXES: .$(SrcSuf)

.$(SrcSuf).$(ObjSuf):
	$(CXX)  $(CXXFLAGS) -c $<

###
# Particle.o: Particle.h

# Horn.o: Horn.h

# FocusingGeometry.o: FocusingGeometry.h

mydict.C: include/Particle.h include/FocusingGeometry.h include/Horn.h
	@echo "Generating dictionary ..."
	@rootcint mydict.C -c -p include/Particle.h include/Horn.h include/FocusingGeometry.h
#---------------------------------------------------
