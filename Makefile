#
# THIS FILE HAS BEEN GENERATED AUTOMATICALLY.
#
# If you edit it, you will lose your changes, should it be regenerated.
#
GEODE           = pcity
SOURCE          = handler.c handler.h drawing.c drawing.h globals.c globals.h build.c build.h simulation.c simulation.h
NOKIA           = nokia.goc
UI_TO_RDFS      =
OBJS            = handler.obj drawing.obj globals.obj build.obj simulation.obj nokia.obj
COMMON          =
MODULES         =
CMODULES        = source nokia
SRCS            = $(SOURCE) $(NOKIA) $(COMMON)
LOBJS           =

SYSMAKEFILE     = geode.mk

#include <geos.mk>
#include <gpath.mk>

#if exists(local.mk)
#include "local.mk"
#else
#include <$(SYSMAKEFILE)>
#endif

#if exists($(DEPFILE))
#include "$(DEPFILE)"
#endif
