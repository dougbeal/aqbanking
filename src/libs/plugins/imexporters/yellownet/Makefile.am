SUBDIRS=profiles

INCLUDES = -I$(aqbanking_symlinkdir) -I$(top_srcdir)/src/libs/aqbanking $(all_includes)

AM_CFLAGS=@visibility_cflags@

#EXTRA_DIST=

noinst_HEADERS=yellownet_p.h yellownet.h

imexporterplugindir = $(aqbanking_plugindir)/imexporters
noinst_LTLIBRARIES=libabimexporters_yellownet.la
imexporterplugin_DATA=yellownet.xml

libabimexporters_yellownet_la_SOURCES=yellownet.c


sources:
	for f in $(libabimexporters_yellownet_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done

cppcheck:
	for f in $(libabimexporters_yellownet_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done

