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
		  ppdpart.h ppdsolid.h ppdvertex.h ps.h quadtree.h linbcg.h \
		  sigproc.h smd.h smdsrc.h smf.h tile.h hestack.h veclib.h \
		  dspr.h hidden.h intsec.h params.h
BSSRCS		= file.cxx dspr.cxx mytime.cxx oppd.cxx \
		  linbcg.cxx matlib.cxx \
		  ppd.cxx ppdedge.cxx ppdface.cxx ppdlist.cxx ppdloop.cxx \
		  ppdnormal.cxx ppdpart.cxx ppdsolid.cxx \
		  ppdvertex.cxx ps.cxx quadtree.cxx tile.cxx hestack.cxx \
		  sigproc.cxx smf.cxx veclib.cxx intsec.cxx params.cxx
BSOBJS		= $(BSSRCS:cxx=o)
BSLIBS		= 
BSTARGET	= libBS.a

SPHEADERS	= asp.h esp.h esplist.h esptree.h sgprique.h sgraph.h
SPSRCS		= asp.cxx esp.cxx esptree.cxx sp.cxx sgprique.cxx sgraph.cxx 
SPOBJS		= $(SPSRCS:cxx=o)
SPLIBS		= 

SPMHEADERS	= spm.h remesh.h spedprique.h ffwarp.h umbrella.h uvwsigproc.h \
		  floater.h cylspm.h tfcremesh.h
SPMSRCS		= spm.cxx remesh.cxx spedprique.cxx ffwarp.cxx umbrella.cxx \
		  uvwsigproc.cxx floater.cxx cylspm.cxx tfcremesh.cxx
SPMOBJS		= $(SPMSRCS:cxx=o)
SPMLIBS		= 

TOPSHEADERS	= 
TOPSSRCS	= tops.cxx
TOPSOBJS	= $(TOPSSRCS:cxx=o)
TOPSLIBS	= 

ALLSRCS		= $(BSSRCS) $(SPSRCS) $(SPMSRCS) $(TOPSSRCS)
ALLHEADERS	= $(BSHEADERS) $(SPHEADERS) $(SPMHEADERS) $(TOPSHEADERS)

###################

LIBS		= -L. -lBS $(LDLIBS)
SAMPLE		= ppdsp ppdspm ppdtops

all:
	@echo "	Linux:		make linux"
	@echo "	Linux debug:	make linux-debug"
	@echo "	Cygwin:		make cygwin"
	@echo "	Cygwin debug:	make cygwin-debug"

linux:
	$(MAKE) $(TARGET) \
	"LDLIBS = -lm" \
	"CXX = g++" \
	"CXXFLAGS = -O2 -Wall" \
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

ppdspm: $(BSTARGET) $(SPMOBJS)
	$(CXX) $(CXXFLAGS) -o ppdspm $(SPMOBJS) $(LIBS)

ppdtops: $(BSTARGET) $(TOPSOBJS)
	$(CXX) $(CXXFLAGS) -o ppdtops $(TOPSOBJS) $(LIBS)

clean:
	$(RM) $(BSOBJS) $(BSTARGET)
	$(RM) $(SPOBJS)
	$(RM) $(SPMOBJS)
	$(RM) $(TOPSOBJS)
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

