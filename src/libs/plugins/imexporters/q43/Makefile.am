SUBDIRS=profiles

INCLUDES = -I$(aqbanking_symlinkdir) -I$(top_srcdir)/src/libs/aqbanking $(all_includes)

AM_CFLAGS=@visibility_cflags@

EXTRA_DIST=

imexporterplugindir = $(aqbanking_plugindir)/imexporters
noinst_LTLIBRARIES=libabimexporters_q43.la
imexporterplugin_DATA=q43.xml

noinst_HEADERS=q43_p.h q43.h


libabimexporters_q43_la_SOURCES=q43.c

sources:
	for f in $(libabimexporters_q43_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done


cppcheck:
	for f in $(libabimexporters_q43_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done

