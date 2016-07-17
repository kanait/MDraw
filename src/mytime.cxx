//
// time.cxx
// time measurement functions
//
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved. 
//

#include "StdAfx.h"

#if defined(_DEBUG) && defined(_WINDOWS)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <stdio.h>
#include <sys/types.h>

#if defined(_WIN32) || defined(_WIN64)
#else
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <sys/times.h>
#endif

#include "smd.h"

#ifdef WIN32
struct tms {
  time_t tms_utime;
  time_t tms_stime;
  time_t tms_cutime;
  time_t tms_cstime;
};
#endif

/* long  times(); */
#define TICKS 60.

#if defined(_WIN32) || defined(_WIN64)
#else
static struct tms tbuf1;
static long real1;
#endif

// start timer
void time_start(void)
{				
#if defined(_WIN32) || defined(_WIN64)
#else
  real1 = times(&tbuf1);
#endif
}

// stop timer 
void time_stop( void )
{				
#if defined(_WIN32) || defined(_WIN64)
#else
  struct tms tbuf2;
  long  real2;
  double realtime, usertime, systime;
  double ct;

  ct = (double) CLK_TCK;

  real2 = times(&tbuf2);
  realtime = ((double) real2 - (double) real1) / ct;
  usertime = ((double) tbuf2.tms_utime - (double) tbuf1.tms_utime) / ct;
  systime  = ((double) tbuf2.tms_stime - (double) tbuf1.tms_stime) / ct;
#endif
}

// stop timer 
void time_stop_value( double *realtime, double *usertime, double *systime )
{				
#if defined(_WIN32) || defined(_WIN64)
#else
  struct tms tbuf2;
  long  real2;
  double ct;

  ct = (double) CLK_TCK;

  real2 = times(&tbuf2);
  *realtime = ((double) real2 - (double) real1) / ct;
  *usertime = ((double) tbuf2.tms_utime - (double) tbuf1.tms_utime) / ct;
  *systime  = ((double) tbuf2.tms_stime - (double) tbuf1.tms_stime) / ct;
//   display("processed time: \n");
//   display("\treal:\t%d (s)\n", real2 - real1);
//   display("\tuser:\t%d (s)\n", tbuf2.tms_utime - tbuf1.tms_utime);
//   display("\tsys: \t%d (s)\n", tbuf2.tms_stime - tbuf1.tms_stime);
//   display("\treal:\t%d (s)\n", real2 - real1);
//   display("\tuser:\t%d (s)\n", tbuf2.tms_utime - tbuf1.tms_utime);
//   display("\tsys: \t%d (s)\n", tbuf2.tms_stime - tbuf1.tms_stime);
//   display("\tCLK_TCK %d\n", CLK_TCK );
//   display("processed time: \n");
//   display("\treal:\t%g (s)\n", *realtime);
//   display("\tuser:\t%g (s)\n", *usertime);
//   display("\tsys: \t%g (s)\n", *systime);
#endif
}

