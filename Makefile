# Project: Console-Secrets
# Copyright (C) 2013  Simone Pagan Griso
# Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
# see the LICENCE file or visit <http://www.gnu.org/licenses/>
# File: Makefile
# Description: Build csm program
# Last Modified: $Id: Makefile 54 2013-03-20 21:54:57Z gridge $

###################
### Settings
prefix=/usr/local
bindir=$(prefix)/bin

## Un-comment to enable debug symbols
#ENABLE_DEBUG=-g

## External programs
INSTALL=/usr/local/bin/install -c

## Define external libraries paths
EXTRALIBS_HEADERS=$(null-string)
EXTRALIBS_LINKER=$(null-string)
# GnuPG
EXTRALIBS_HEADERS+=$(shell gpgme-config --cflags) -D_FILE_OFFSET_BITS=64
EXTRALIBS_LINKER+=$(shell gpgme-config --libs)
# ncurses and derived
EXTRALIBS_LINKER+=-lform -lmenu -lncurses

## Define other compiler and linker option 
ARCH_OPTS=-m32
#ARCH_OPTS=-m64

### End of settings
###################

### CSM Makefile header printout
CSM="(CSM) "

### Define objects which needs to be linked into CSM
OBJECTS:= $(patsubst src/%.cc,obj/%.o,$(wildcard src/*.cc))
#OBJECTS:= $(patsubst %.cc,%.o,$(wildcard *.cc))

### Now steer everything together
CXX=g++
CC=g++
LD=g++
CXXFLAGS=-Wall $(ENABLE_DEBUG) $(ARCH_OPTS) $(EXTRALIBS_HEADERS)
LDLIBS=$(EXTRALIBS_LINKER)

#Targets
all: directories csm

csm: $(OBJECTS)
	@echo $(CSM)"Linking $@"
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

doc:
	@echo $(CSM)"Generating doxygen documentation"
	@doxygen csmDoxy.conf 2>&1 > csmDoxy.log

install: all
	@echo $(CSM)"Installing csm / console-secrets binary and documentation"
	@$(INSTALL) csm $(bindir)/csm
	@ln -s $(bindir)/csm $(bindir)/console-secrets

.PHONY: clean
clean:
	@echo $(CSM)"Cleaning objects, binary and log files"
	@rm -f $(OBJECTS)
	@rm -f *~
	@rm -f csm
	@rm -f csmDoxy.log

.PHONY: directories
directories: obj docs

obj:
	@echo $(CSM)"Creating tansient directory: "$@
	@mkdir -p $@

docs:
	@echo $(CSM)"Creating tansient directory: "$@
	@mkdir -p $@

obj/%.o: src/%.cc
	@echo $(CSM)"Compiling $<"
	$(CXX) $(CXXFLAGS) -o $@ -c $< 

test:
	@echo $(CSM)"Objects: ${OBJECTS}"
