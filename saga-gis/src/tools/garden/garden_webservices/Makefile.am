if DEBUG
DBGFLAGS = -g -DDEBUG
endif

DEP_DEFS = -fPIC `wx-config --unicode=yes --static=no --cxxflags`
DEP_LFLG = -fPIC -shared `wx-config --unicode=yes --static=no --libs`

DEF_SAGA           = -D_SAGA_LINUX -D_TYPEDEF_BYTE -D_TYPEDEF_WORD -D_SAGA_UNICODE
CXX_INCS           = -I$(top_srcdir)/src/saga_core
AM_CXXFLAGS        = $(CXX_INCS) $(DEF_SAGA) $(DEP_DEFS) $(DBGFLAGS) $(GOMPFLAGS)
AM_LDFLAGS         = $(DEP_LFLG) -avoid-version
pkglib_LTLIBRARIES = libgarden_webservices.la
libgarden_webservices_la_SOURCES =\
MLB_Interface.cpp\
osm_import.cpp\
wms_import.cpp\
MLB_Interface.h\
osm_import.h\
wms_import.h

libgarden_webservices_la_LIBADD = $(top_srcdir)/src/saga_core/saga_api/libsaga_api.la
