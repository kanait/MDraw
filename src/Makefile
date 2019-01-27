#
# makefile
#
SHELL	= /bin/sh

.SUFFIXES: .o .cxx
.SUFFIXES: .o .cpp
.SUFFIXES: .o .cc
.SUFFIXES: .o .c

RM		= rm -f
TOUCH		= touch
AR		= ar
AROPS		= cru
LODER		= $(AR) $(AROPS)
MAKE		= make
CORE		= core
TARGET		= target

ROOTDIR		= ..
SRCDIR		= $(ROOTDIR)/src
DATADIR		= $(ROOTDIR)/data

BSHEADERS	= file.h matlib.h mytime.h oppd.h \
                  ppd.h ppdedge.h ppdface.h ppdlist.h ppdloop.h ppdnormal.h \
		  ppdpart.h ppdsolid.h ppdvertex.h ps.h quadtree.h \
		  smd.h smdsrc.h smf.h hestack.h veclib.h \
		  hidden.h intsec.h params.h
BSSRCS		= file.cxx mytime.cxx oppd.cxx matlib.cxx \
		  ppd.cxx ppdedge.cxx ppdface.cxx ppdlist.cxx ppdloop.cxx \
		  ppdnormal.cxx ppdpart.cxx ppdsolid.cxx \
		  ppdvertex.cxx ps.cxx quadtree.cxx hestack.cxx \
		  smf.cxx veclib.cxx intsec.cxx params.cxx
BSOBJS		= $(BSSRCS:cxx=o)
BSLIBS		= 
BSTARGET	= libBS.a

SPHEADERS	= asp.h esp.h esplist.h esptree.h sgprique.h sgraph.h
SPSRCS		= asp.cxx esp.cxx esptree.cxx sp.cxx sgprique.cxx sgraph.cxx 
SPOBJS		= $(SPSRCS:cxx=o)
SPLIBS		= 

ALLSRCS		= $(BSSRCS) $(SPSRCS)
ALLHEADERS	= $(BSHEADERS) $(SPHEADERS)

###################

LIBS		= -L. -lBS $(LDLIBS)
SAMPLE		= ppdsp

all:
	@echo "	Linux:		make linux"
	@echo "	Linux debug:	make linux-debug"
	@echo "	Cygwin:		make cygwin"
	@echo "	Cygwin debug:	make cygwin-debug"

linux:
	$(MAKE) $(TARGET) \
	"LDLIBS = -lm" \
	"CXX = g++" \
	"CXXFLAGS = -O3 -Wall" \
	"RANLIB = ranlib"

linux-debug:
	$(MAKE) $(TARGET) \
	"LDLIBS = -lm" \
	"CXX = g++" \
	"CXXFLAGS = -O2 -g3 -Wall -D_DEBUG" \
	"RANLIB = ranlib"

cygwin:
	$(MAKE) $(TARGET) \
	"LDLIBS = -lm" \
	"CXX = g++" \
	"AR = ar" \
	"CXXFLAGS = -O3 -D_WIN32 -Wall" \
	"RANLIB = ranlib"

cygwin-debug:
	$(MAKE) $(TARGET) \
	"LDLIBS = -lm" \
	"CXX = g++" \
	"AR = ar" \
	"CXXFLAGS = -O3 -D_WIN32 -g3 -Wall -D_DEBUG" \
	"RANLIB = ranlib"

target : $(BSTARGET) $(TKTARGET) $(SAMPLE)

$(BSTARGET): $(BSOBJS)
	$(LODER) $(BSTARGET) $(BSOBJS)
	$(RANLIB) $(BSTARGET)

ppdsp: $(BSTARGET) $(SPOBJS)
	$(CXX) $(CXXFLAGS) -o ppdsp $(SPOBJS) $(LIBS)

clean:
	$(RM) $(BSOBJS) $(BSTARGET)
	$(RM) $(SPOBJS)
	$(RM) -rf Release Debug
	$(RM) $(CORE)

depend:: $(ALLSRCS) $(ADDHEADERS)
	makedepend -s "# DO NOT DELETE!!!" \
		-- $(CXXFLAGS) -- $(ALLSRCS)

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $<

.cc.o:
	$(CXX) -c $(CXXFLAGS) $<

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $<
.c.o:
	$(CC) -c $(CFLAGS) $<


# DO NOT DELETE!!!

