//
// hidden.cpp
//
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved.
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#include <sys/stat.h>
#include "smd.h"
#include "screen.h"
#include "draw.h"
#include "file.h"

// Include the OpenGL headers
#include "gl\gl.h"
#include "gl\glu.h"
//#include "gl\glaux.h"

void write_vw_file( char *file )
{
  int       i;
  FILE      *fp;
  double    mmat[16], pmat[16];

  ScreenAtr *screen;
  screen = &(swin->screenatr);
//  clear_gl3d(screen);
//  view_init(screen);
	// Get Matrices
  ::glGetDoublev(GL_MODELVIEW_MATRIX,  (GLdouble *) mmat);
  ::glGetDoublev(GL_PROJECTION_MATRIX, (GLdouble *) pmat);
//  ::glPopMatrix();
  
  if ((fp = fopen(file, "w")) == NULL) {
    return;
  }
  
  fprintf(fp, "screensize\t%d %d\n", screen->width, screen->height);
  
  fprintf(fp, "eye_direction\t%g %g %g\n",
	  screen->pers.eye_dir.x,
	  screen->pers.eye_dir.y,
	  screen->pers.eye_dir.z);
  fprintf(fp, "reference_point\t%g %g %g\n",
	  screen->pers.ref.x,
	  screen->pers.ref.y,
	  screen->pers.ref.z);
  fprintf(fp, "focal_distance\t%g\n", screen->pers.fd);
  fprintf(fp, "view_angle\t%g\n", screen->pers.angle);
  fprintf(fp, "znear_ratio\t%g\n", screen->pers.znear_ratio);
  fprintf(fp, "zfar_ratio\t%g\n", screen->pers.zfar_ratio);
  fprintf(fp, "aspect\t%g\n", screen->pers.scraspct);
  fprintf(fp, "zoom\t%g\n", screen->zoom);
	fprintf(fp, "translate\t%g %g %g\n",
          screen->translate_vec.x,
          screen->translate_vec.y,
          screen->translate_vec.z);

  
  fprintf(fp, "rmat\n");
  for (i = 0; i < 4; ++i) {
    fprintf(fp, "%g %g %g %g\n",
	    screen->rmat[i],
	    screen->rmat[4  + i],
	    screen->rmat[8  + i],
	    screen->rmat[12 + i]);
  }
  fprintf(fp, "mmat\n");
  for (i = 0; i < 4; ++i) {
    fprintf(fp, "%g %g %g %g\n",
	    mmat[i],
	    mmat[4  + i],
	    mmat[8  + i],
	    mmat[12 + i]);
  }
  fprintf(fp, "pmat\n");
  for (i = 0; i < 4; ++i) {
    fprintf(fp, "%g %g %g %g\n",
	    pmat[i],
	    pmat[4  + i],
	    pmat[8  + i],
	    pmat[12 + i]);
  }
  
  fclose(fp);
}

void open_vw(char *vffile, double *mmat, double *pmat)
{
  FILE      *fp;
  int       i;
  ScreenAtr *screen;
  char      key[BUFSIZ], val[4][BUFSIZ], buf[BUFSIZ];
  
  if ((fp = fopen(vffile, "r")) == NULL) {
    return;
  }
  screen = &(swin->screenatr);
  set_default_view( screen );

  while (fgets(buf, BUFSIZ, fp)) {
    sscanf(buf, "%s", key);
    if (comment(buf[0])) continue;
    if (!strcmp(key, "zoom")) {
      sscanf(buf, "%s%s", key, val[0]);
      screen->zoom = (float) atof(val[0]);
    } else if (!strcmp(key, "screensize")) {
      sscanf(buf, "%s%s%s", key, val[0], val[1]);
      screen->width  = atoi(val[0]);
      screen->height = atoi(val[1]);
      screen->xcenter = (double) screen->width  / 2.0;
      screen->ycenter = (double) screen->height / 2.0;
    } else if (!strcmp(key, "eye_direction")) {
      sscanf(buf, "%s%s%s%s", key, val[0], val[1], val[2]);
      screen->pers.eye_dir.x = atof(val[0]);
      screen->pers.eye_dir.y = atof(val[1]);
      screen->pers.eye_dir.z = atof(val[2]);
    } else if (!strcmp(key, "reference_point")) {
      sscanf(buf, "%s%s%s%s", key, val[0], val[1], val[2]);
      screen->pers.ref.x = atof(val[0]);
      screen->pers.ref.y = atof(val[1]);
      screen->pers.ref.z = atof(val[2]);
    } else if (!strcmp(key, "focal_distance")) {
      sscanf(buf, "%s%s", key, val[0]);
      screen->pers.fd = atof(val[0]);
    } else if (!strcmp(key, "view_angle")) {
      sscanf(buf, "%s%s", key, val[0]);
      screen->pers.angle = atof(val[0]);
    } else if (!strcmp(key, "znear_ratio")) {
      sscanf(buf, "%s%s", key, val[0]);
      screen->pers.znear_ratio = atof(val[0]);
    } else if (!strcmp(key, "zfar_ratio")) {
      sscanf(buf, "%s%s", key, val[0]);
      screen->pers.zfar_ratio = atof(val[0]);
    } else if (!strcmp(key, "aspect")) {
      sscanf(buf, "%s%s", key, val[0]);
      screen->pers.scraspct = atof(val[0]);
		} else if (!strcmp(key, "translate")) {
      sscanf(buf, "%s%s%s%s", key, val[0], val[1], val[2]);
      screen->translate_vec.x = atof(val[0]);
      screen->translate_vec.y = atof(val[1]);
      screen->translate_vec.z = atof(val[2]);
    } else if (!strcmp(key, "rmat")) {
      for (i = 0; i < 4; ++i) {
	fgets(buf, BUFSIZ, fp);
	sscanf(buf, "%s%s%s%s", val[0], val[1], val[2], val[3]);
	screen->rmat[i] = atof(val[0]);
	screen->rmat[4 +i] = atof(val[1]);
	screen->rmat[8 +i] = atof(val[2]);
	screen->rmat[12+i] = atof(val[3]);
      }
      screen->rotate_xvec.x = screen->rmat[0];
      screen->rotate_yvec.x = screen->rmat[1];
      screen->rotate_zvec.x = screen->rmat[2];
      screen->rotate_xvec.y = screen->rmat[4];
      screen->rotate_yvec.y = screen->rmat[5];
      screen->rotate_zvec.y = screen->rmat[6];
      screen->rotate_xvec.z = screen->rmat[8];
      screen->rotate_yvec.z = screen->rmat[9];
      screen->rotate_zvec.z = screen->rmat[10];
      screen->rotate_x = 0.0;
      screen->rotate_y = 0.0;
      screen->rotate_z = 0.0;
    } else if (!strcmp(key, "mmat")) {
      for (i = 0; i < 4; ++i) {
	fgets(buf, BUFSIZ, fp);
	sscanf(buf, "%s%s%s%s", val[0], val[1], val[2], val[3]);
	mmat[i] = atof(val[0]);
	mmat[4 +i] = atof(val[1]);
	mmat[8 +i] = atof(val[2]);
	mmat[12+i] = atof(val[3]);
      }
    } else if (!strcmp(key, "pmat")) {
      for (i = 0; i < 4; ++i) {
	fgets(buf, BUFSIZ, fp);
	sscanf(buf, "%s%s%s%s", val[0], val[1], val[2], val[3]);
	pmat[i] = atof(val[0]);
	pmat[4 +i] = atof(val[1]);
	pmat[8 +i] = atof(val[2]);
	pmat[12+i] = atof(val[3]);
      }
    } 
  }
  fclose(fp);
}

