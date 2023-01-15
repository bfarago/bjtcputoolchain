#these targets available in all subdirs
TOPTARGETS := all clean check

#these subdirs have a makefile
SUBDIRS := libbjtcpu asmb simb 
#emub/raspberry
#SUBDIRS := $(wildcard */.)

$(TOPTARGETS): $(SUBDIRS)
all: $(SUBDIRS)

build:
	mkdir -p build
	$(MAKE) all
	cp asmb/asmb build/
	cp simb/simb build/
#	cp emub/raspberry/playb build/

$(SUBDIRS):
	@echo "--- MAKE SUBFOLDER:" $@
	$(MAKE) -C $@ $(MAKECMDGOALS)

#in case of new machine is used, install the pre required librarys
installprereq:
	$(MAKE) -C emub/raspberry $@

.PHONY: all build $(SUBDIRS)
