DIST_SUBDIRS=aqbanking-cli

if ENABLE_CLI
SUBDIRS=aqbanking-cli
endif


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;


cppcheck:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done

types:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d types; \
	done

typedefs:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typedefs; \
	done

