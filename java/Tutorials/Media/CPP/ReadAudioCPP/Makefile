include ../../../Common/Build/config.mk

### target ###

TARGET_NAME   = ReadAudioCPP

### Objects, sources ###

# CPP source files

CXXSRCS = \
	ReadAudioCPP.cpp

ifeq ($(N_MAC_OSX_FRAMEWORKS),yes)
  LDLIBS ?= -framework NMedia -framework NCore -framework NLicensing
else
  LDLIBS ?= -lNMedia -lNCore -lNLicensing
endif

include ../../../Common/Build/rules.mk

