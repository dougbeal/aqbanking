
AM_CFLAGS=-DBUILDING_AQHBCI @visibility_cflags@

INCLUDES = -I$(aqbanking_symlinkdir) $(gwenhywfar_includes) \
  -I$(srcdir)/../msglayer -I$(srcdir)/../joblayer \
  -I$(srcdir)/../applayer -I$(srcdir)/.. \
  -I$(srcdir)/../ajobs -I$(srcdir)/../admjobs \
  -I$(srcdir)/../dialogs  -I$(srcdir)/../control

localedir = $(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\" \
  -DAQHBCI_PLUGINS=\"$(aqhbci_plugindir)\"

noinst_LTLIBRARIES=libhbcibanking.la

noinst_HEADERS=\
 provider_l.h \
 provider_p.h \
 provider.h


#iheaderdir=@aqbanking_headerdir_am@/aqhbci
#iheader_HEADERS=

libhbcibanking_la_SOURCES=\
 provider.c



EXTRA_DIST=\
  provider_accspec.c \
  provider_dialogs.c \
  provider_dtazv.c \
  provider_iniletter.c \
  provider_keys.c \
  provider_online.c \
  provider_sendcmd.c \
  provider_update.c



sources:
	for f in $(libhbcibanking_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


cppcheck:
	for f in $(libhbcibanking_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


