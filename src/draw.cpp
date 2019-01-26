//
// draw.cpp
//
// Copyright (c) 1997-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#include "cinc.h"
#include <math.h>
#include <assert.h>

#include "gl\gl.h"
#include "gl\glu.h"
//#include "gl\glaux.h"

#include "smd.h"
#include "screen.h"
#include "veclib.h"
#include "matlib.h"

#include "texture.h"
#include "draw.h"
#include "glppd.h"
#include "gltile.h"

#include "color.h"
#include "light.h"

void init_gl3d( void )
{
  // specify black as clear color
  ::glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
  // specify the back of the buffer as clear depth
  ::glClearDepth( 1.0f );

  light_init();

  //::glPolygonMode( GL_BACK, GL_FILL );
//    glCullFace( GL_BACK );
//    glEnable( GL_CULL_FACE );
  
  
  // enable depth testing
  ::glEnable( GL_DEPTH_TEST );
  ::glDepthFunc( GL_LESS );

  ::glEnable( GL_POLYGON_OFFSET_FILL );
  ::glPolygonOffset( (float) 1.0, (float) 1e-6 );
  
  //
  // texture map (for Floor)
  //
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

#if 0
  unsigned *floortex = NULL;
  int texcomps, texwid, texht;
//    floortex = read_texture("check512.sgi", &texwid, &texht, &texcomps);
  if ( (floortex = read_texture("kanai1.sgi", &texwid, &texht, &texcomps))
       != NULL ) {
    assert( floortex != NULL );
    ::gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGB, texwid, texht, GL_RGBA,
			 GL_UNSIGNED_BYTE, floortex );
    free( floortex );
  }
#endif
  
  // anti-aliasing
  // ::glEnable( GL_LINE_SMOOTH );
  // ::glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
}

// define lights
void light_init( void )
{
  ::glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE );

  // LIGHT No.0
  ::glLightfv( GL_LIGHT0, GL_POSITION, light0->position );
  ::glLightfv( GL_LIGHT0, GL_AMBIENT, light0->ambient );
  ::glLightfv( GL_LIGHT0, GL_DIFFUSE, light0->diffuse );
  ::glLightfv( GL_LIGHT0, GL_SPECULAR, light0->specular );
//    ::glLightf( GL_LIGHT0, GL_CONSTANT_ATTENUATION, light0->const_attenuation );
//    ::glLightf( GL_LIGHT0, GL_LINEAR_ATTENUATION, light0->linear_attenuation );
  ::glEnable( GL_LIGHT0 );

  // LIGHT No.1
  ::glLightfv( GL_LIGHT1, GL_POSITION, light1->position );
  ::glLightfv( GL_LIGHT1, GL_AMBIENT, light1->ambient );
  ::glLightfv( GL_LIGHT1, GL_DIFFUSE, light1->diffuse );
  ::glLightfv( GL_LIGHT1, GL_SPECULAR, light1->specular );
//    ::glLightf( GL_LIGHT1, GL_CONSTANT_ATTENUATION, light1->const_attenuation );
//    ::glLightf( GL_LIGHT1, GL_LINEAR_ATTENUATION, light1->linear_attenuation );
  ::glEnable( GL_LIGHT1 );

  // LIGHT No.2
  ::glLightfv( GL_LIGHT2, GL_POSITION, light2->position );
  ::glLightfv( GL_LIGHT2, GL_AMBIENT, light2->ambient );
  ::glLightfv( GL_LIGHT2, GL_DIFFUSE, light2->diffuse );
  ::glLightfv( GL_LIGHT2, GL_SPECULAR, light2->specular );
//    ::glLightf( GL_LIGHT2, GL_CONSTANT_ATTENUATION, light2->const_attenuation );
//    ::glLightf( GL_LIGHT2, GL_LINEAR_ATTENUATION, light2->linear_attenuation );
  ::glEnable( GL_LIGHT2 );

  // LIGHT No.3
  ::glLightfv( GL_LIGHT3, GL_POSITION, light3->position );
  ::glLightfv( GL_LIGHT3, GL_AMBIENT, light3->ambient );
  ::glLightfv( GL_LIGHT3, GL_DIFFUSE, light3->diffuse );
  ::glLightfv( GL_LIGHT3, GL_SPECULAR, light3->specular );
//    ::glLightf( GL_LIGHT3, GL_CONSTANT_ATTENUATION, light3->const_attenuation );
//    ::glLightf( GL_LIGHT3, GL_LINEAR_ATTENUATION, light3->linear_attenuation );
  ::glEnable( GL_LIGHT3 );
  
}

//  int light_init( Light *light )
//  {
//    ::glEnable(GL_LIGHT0);
//    ::glLightfv(GL_LIGHT0, GL_AMBIENT, light->ambient);
//    ::glLightfv(GL_LIGHT0, GL_DIFFUSE, light->diffuse);
//    ::glLightfv(GL_LIGHT0, GL_SPECULAR, light->specular);
//    ::glLightfv(GL_LIGHT0, GL_POSITION, light->position);
//    ::glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light->lmodel_ambient);
//    ::glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, light->local_view);
	
//    return 0;
//  }

void clear_gl3d( ScreenAtr *screen )
{
  ::glClearColor((GLfloat) screen->bgrgb[0], (GLfloat) screen->bgrgb[1],
  		 (GLfloat) screen->bgrgb[2], (GLfloat) 0.0);
  ::glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  if ( swin->dis3d.gradient == TRUE ) 
    draw_gradient_background();
}

//
// default view setting
//
void set_default_view( ScreenAtr *screen )
{
  double    rad;

  // perspective
  screen->pers.ref.x = 0.0;
  screen->pers.ref.y = 0.0;
  screen->pers.ref.z = 0.0;
  screen->pers.up.x  = 0.0;
  screen->pers.up.y  = 1.0;
  screen->pers.up.z  = 0.0;
  screen->pers.angle = 45.0;
  screen->pers.znear_ratio = 0.1;
  screen->pers.zfar_ratio  = 10.0;
  screen->pers.eye_dir.x = 0.0;
  screen->pers.eye_dir.y = 0.0;
  screen->pers.eye_dir.z = 1.0 / SQRT3;
  V3Normalize(&(screen->pers.eye_dir));
  /*  screen->pers.eye_dir.x = 1.0 / SQRT3;
      screen->pers.eye_dir.y = 1.0 / SQRT3;*/

  // translate
  screen->translate_vec.x = 0.0;
  screen->translate_vec.y = 0.0;
  screen->translate_vec.z = 0.0;

  /* default focal distance */
  rad = 1.0;
  screen->pers.fd = rad / tan(screen->pers.angle * SMDPI / 360.0);

  // zoom
  screen->zoom = 1.0;

  // rotate
  screen->rotate_x = 0.0;
  screen->rotate_y = 0.0;
  screen->rotate_z = 0.0;
  screen->zrot_rad = 0.0;
	
  calc_rotate_matrix( screen );
}

void calc_rotate_matrix(ScreenAtr *screen)
{
  Vec xvec, yvec, zvec;
	
  // calc rotate_axis vector
  zvec.x = screen->pers.eye_dir.x;
  zvec.y = screen->pers.eye_dir.y;
  zvec.z = screen->pers.eye_dir.z;
  yvec.x = screen->pers.up.x;
  yvec.y = screen->pers.up.y;
  yvec.z = screen->pers.up.z;
  V3Normalize(&(yvec));
  V3Normalize(&(zvec));
  V3Cross(&(yvec), &(zvec), &(xvec));
  V3Cross(&(zvec), &(xvec), &(yvec));
  V3Normalize(&(xvec));
  V3Normalize(&(yvec));
	
  screen->rotate_xvec.x = xvec.x;
  screen->rotate_xvec.y = xvec.y;
  screen->rotate_xvec.z = xvec.z;
  screen->rotate_yvec.x = yvec.x;
  screen->rotate_yvec.y = yvec.y;
  screen->rotate_yvec.z = yvec.z;
  screen->rotate_zvec.x = zvec.x;
  screen->rotate_zvec.y = zvec.y;
  screen->rotate_zvec.z = zvec.z;
	
  screen->rmat[0]  = screen->rotate_xvec.x;
  screen->rmat[1]  = screen->rotate_yvec.x;
  screen->rmat[2]  = screen->rotate_zvec.x;
  screen->rmat[3]  = 0.0;
  screen->rmat[4]  = screen->rotate_xvec.y;
  screen->rmat[5]  = screen->rotate_yvec.y;
  screen->rmat[6]  = screen->rotate_zvec.y;
  screen->rmat[7]  = 0.0;
  screen->rmat[8]  = screen->rotate_xvec.z;
  screen->rmat[9]  = screen->rotate_yvec.z;
  screen->rmat[10] = screen->rotate_zvec.z;
  screen->rmat[11] = 0.0;
  screen->rmat[12] = 0.0;
  screen->rmat[13] = 0.0;
  screen->rmat[14] = 0.0;
  screen->rmat[15] = 1.0;
}

void view_init( ScreenAtr *screen )
{
  GLdouble rotx[16], roty[16], rotz[16];
  GLdouble rinvx[16], rinvy[16], rinvz[16];
  GLdouble m1[16];
  Vec      eye;
	
//    ::glViewport((GLint) 0, (GLint) 0,
//  	       (GLsizei) screen->width, (GLsizei) screen->height);
	
  // make rotate matrix
	
  ::glMatrixMode(GL_PROJECTION);
  ::glLoadIdentity();
  screen->pers.scraspct = (double) screen->width / (double) screen->height;
  ::gluPerspective((GLdouble) screen->pers.angle,
		   (GLdouble) screen->pers.scraspct,
		   (GLdouble) screen->pers.fd
		   * screen->pers.znear_ratio / screen->zoom,
		   (GLdouble) screen->pers.fd
		   * screen->pers.zfar_ratio / screen->zoom);
	
  ::glMatrixMode(GL_MODELVIEW);
  ::glLoadIdentity();
	
  if (screen->rotate_type == ROTATE_XY) {
		
    // Rotate X
    iMakeRotVec(screen->rotate_y, &(screen->rotate_xvec), (double *) rinvx);
    iMakeRotInvVec(screen->rotate_y, &(screen->rotate_xvec), (double *) rotx);
    MultMatrix4((double *) screen->rmat, (double *) rotx, (double *) m1);
    iMultMatrixVec((double *) rinvx,  &(screen->pers.eye_dir));
    iMultMatrixVec((double *) rinvx,  &(screen->rotate_yvec));
    iMultMatrixVec((double *) rinvx,  &(screen->rotate_zvec));
    // Rotate Y
    iMakeRotVec(screen->rotate_x, &(screen->rotate_yvec), (double *) rinvy);
    iMakeRotInvVec(screen->rotate_x, &(screen->rotate_yvec), (double *) roty);
    MultMatrix4((double *) m1, (double *) roty, (double *) screen->rmat);
    iMultMatrixVec((double *) rinvy,  &(screen->pers.eye_dir));
    iMultMatrixVec((double *) rinvy,  &(screen->rotate_zvec));
    iMultMatrixVec((double *) rinvy,  &(screen->rotate_xvec));
    screen->rotate_x = 0.0;
    screen->rotate_y = 0.0;
    ::glMultMatrixd((GLdouble *) screen->rmat);
		
  } else if (screen->rotate_type == ROTATE_Z) {
		
    iMakeRotVec(screen->rotate_z, &(screen->rotate_zvec), (double *) rinvz);
    iMakeRotInvVec(screen->rotate_z, &(screen->rotate_zvec), (double *) rotz);
    MultMatrix4((double *) screen->rmat, (double *) rotz, (double *) m1);
    EqualMatrix4((double *) m1, (double *) screen->rmat);
    iMultMatrixVec((double *) rinvz, &(screen->pers.eye_dir));
    iMultMatrixVec((double *) rinvz, &(screen->rotate_xvec));
    iMultMatrixVec((double *) rinvz, &(screen->rotate_yvec));
    screen->rotate_z = 0.0;
    ::glMultMatrixd((GLdouble *) screen->rmat);
		
  }

  // current eye position
	
  eye.x = screen->pers.eye_dir.x * screen->pers.fd / screen->zoom;
  eye.y = screen->pers.eye_dir.y * screen->pers.fd / screen->zoom;
  eye.z = screen->pers.eye_dir.z * screen->pers.fd / screen->zoom;
  eye.x += (screen->pers.ref.x + screen->translate_vec.x);
  eye.y += (screen->pers.ref.y + screen->translate_vec.y);
  eye.z += (screen->pers.ref.z + screen->translate_vec.z);
	
  ::glTranslated((GLdouble) -eye.x, (GLdouble) -eye.y, (GLdouble) -eye.z);
  
}

void get_GL_attributes(ScreenAtr *screen)
{
  ::glPushMatrix();
	
  view_init( screen );
  ::glGetDoublev(GL_MODELVIEW_MATRIX,  (GLdouble *) screen->mmat);
  ::glGetDoublev(GL_PROJECTION_MATRIX, (GLdouble *) screen->pmat);
  ::glGetIntegerv(GL_VIEWPORT, (GLint *) screen->viewport);
	
  ::glPopMatrix();
}

static void transform_point( double out[4], double m[16], double in[4] )
{
#define M(row,col)  m[col*4+row]
  out[0] = M(0,0) * in[0] + M(0,1) * in[1] + M(0,2) * in[2] + M(0,3) * in[3];
  out[1] = M(1,0) * in[0] + M(1,1) * in[1] + M(1,2) * in[2] + M(1,3) * in[3];
  out[2] = M(2,0) * in[0] + M(2,1) * in[1] + M(2,2) * in[2] + M(2,3) * in[3];
  out[3] = M(3,0) * in[0] + M(3,1) * in[1] + M(3,2) * in[2] + M(3,3) * in[3];
#undef M
}

void world_to_win(Vec *obj, Vec2d *win, double model[16], double proj[16],
                  int viewport[4])
{
  /* matrice de transformation */
  double in[4],out[4];
	
	/* initilise la matrice et le vecteur a transformer */
  in[0] = obj->x;
  in[1] = obj->y;
  in[2] = obj->z;
  in[3] = 1.0;
  transform_point(out,model,in);
  transform_point(in,proj,out);
	
	/* d'ou le resultat normalise entre -1 et 1*/
  in[0]/=in[3];
  in[1]/=in[3];
  in[2]/=in[3];
	
  /* en coordonnees ecran */
  win->x = viewport[0]+(1+in[0])*viewport[2]/2;
  win->y = viewport[1]+(1+in[1])*viewport[3]/2;
  /* entre 0 et 1 suivant z */
  /*   win->z = (1+in[2])/2; */
}

/********************
* drawing functions
********************/

/* draw 3D coordinate axis */

static void draw3dcoaxis( ScreenAtr *screen )
{
  double x, y, z, w;
	
  ::glLineWidth((GLfloat) 1.0);
  ::glColor3dv(magentavec);
	
  x = screen->pers.ref.x;
  y = screen->pers.ref.y;
  z = screen->pers.ref.z;
  w = 0.2 * screen->pers.fd / (screen->zoom * SQRT3);
	
  ::glBegin(GL_LINE_STRIP);
  ::glVertex3d((GLdouble) x, (GLdouble) y, (GLdouble) z);
  ::glVertex3d((GLdouble) x + w, (GLdouble) y, (GLdouble) z);
  ::glEnd();
	
  ::glRasterPos3d((GLdouble) x + w, (GLdouble) y, (GLdouble) z);
  (void) draw_string(screen, "x");
	
  ::glBegin(GL_LINE_STRIP);
  ::glVertex3d((GLdouble) x, (GLdouble) y, (GLdouble) z);
  ::glVertex3d((GLdouble) x, (GLdouble) y + w, (GLdouble) z);
  ::glEnd();
  ::glRasterPos3d((GLdouble) x, (GLdouble) y + w, (GLdouble) z);
  (void) draw_string(screen, "y");
	
  ::glBegin(GL_LINE_STRIP);
  ::glVertex3d((GLdouble) x, (GLdouble) y, (GLdouble) z);
  ::glVertex3d((GLdouble) x, (GLdouble) y, (GLdouble) z + w);
  ::glEnd();
  ::glRasterPos3d((GLdouble) x, (GLdouble) y, (GLdouble) z + w);
  (void) draw_string(screen, "z");
}

/* draw 3D rotate axis */

static void draw3drotateaxis(ScreenAtr *screen)
{
  double x, y, z, w;
  Vec    vec;
	
  ::glLineWidth((GLfloat) 1.0);
  ::glColor3dv(yellowvec);
	
  x = screen->pers.ref.x;
  y = screen->pers.ref.y;
  z = screen->pers.ref.z;
  w = 0.2 * screen->pers.fd / (screen->zoom * SQRT3);
	
  vec.x = w * screen->rotate_xvec.x;
  vec.y = w * screen->rotate_xvec.y;
  vec.z = w * screen->rotate_xvec.z;
	
  ::glBegin(GL_LINE_STRIP);
  ::glVertex3d((GLdouble) x, (GLdouble) y, (GLdouble) z);
  ::glVertex3d((GLdouble) (x + vec.x),
	       (GLdouble) (y + vec.y),
	       (GLdouble) (z + vec.z));
  ::glEnd();
  //  ::glRasterPos3d((GLdouble) (x + vec.x),
  //		(GLdouble) (y + vec.y),
  //		(GLdouble) (z + vec.z));
  //  (void) printstring(screen, "Rx");
	
  vec.x = w * screen->rotate_yvec.x;
  vec.y = w * screen->rotate_yvec.y;
  vec.z = w * screen->rotate_yvec.z;
	
  ::glBegin(GL_LINE_STRIP);
  ::glVertex3d((GLdouble) x, (GLdouble) y, (GLdouble) z);
  ::glVertex3d((GLdouble) (x + vec.x),
	       (GLdouble) (y + vec.y),
	       (GLdouble) (z + vec.z));
  ::glEnd();
  //  glRasterPos3d((GLdouble) (x + vec.x),
  //		(GLdouble) (y + vec.y),
  //		(GLdouble) (z + vec.z));
  //  (void) printstring(screen, "Ry");
	
  vec.x = w * screen->rotate_zvec.x;
  vec.y = w * screen->rotate_zvec.y;
  vec.z = w * screen->rotate_zvec.z;
	
  ::glBegin(GL_LINE_STRIP);
  ::glVertex3d((GLdouble) x, (GLdouble) y, (GLdouble) z);
  ::glVertex3d((GLdouble) (x + vec.x),
	       (GLdouble) (y + vec.y),
	       (GLdouble) (z + vec.z));
  ::glEnd();
  //  glRasterPos3d((GLdouble) (x + vec.x),
  //		(GLdouble) (y + vec.y),
  //		(GLdouble) (z + vec.z));
  //  (void) printstring(screen, "Rz");
}

static void drawselectarea(ScreenAtr *screen)
{
  ::glPushMatrix();
  ::glMatrixMode(GL_PROJECTION);
  ::glLoadIdentity();
  ::gluOrtho2D((GLdouble) 0.0, (GLdouble) screen->width,
	       (GLdouble) 0.0, (GLdouble) screen->height);
  ::glMatrixMode(GL_MODELVIEW);
  ::glLoadIdentity();
	
  ::glLineWidth((GLfloat) 1.0);
  ::glColor3dv(redvec);
  ::glBegin(GL_LINE_STRIP);
  ::glVertex2d((GLdouble) screen->areaorg.x, (GLdouble) screen->areaorg.y);
  ::glVertex2d((GLdouble) screen->areaorg.x, (GLdouble) screen->areaatv.y);
  ::glEnd();
  ::glBegin(GL_LINE_STRIP);
  ::glVertex2d((GLdouble) screen->areaorg.x, (GLdouble) screen->areaorg.y);
  ::glVertex2d((GLdouble) screen->areaatv.x, (GLdouble) screen->areaorg.y);
  ::glEnd();
  ::glBegin(GL_LINE_STRIP);
  ::glVertex2d((GLdouble) screen->areaorg.x, (GLdouble) screen->areaatv.y);
  ::glVertex2d((GLdouble) screen->areaatv.x, (GLdouble) screen->areaatv.y);
  ::glEnd();
  ::glBegin(GL_LINE_STRIP);
  ::glVertex2d((GLdouble) screen->areaatv.x, (GLdouble) screen->areaorg.y);
  ::glVertex2d((GLdouble) screen->areaatv.x, (GLdouble) screen->areaatv.y);
  ::glEnd();
	
  ::glPopMatrix();
}

static void drawtile( Tile *tile, ScreenAtr *screen )
{
  // tile edge
  //draw_tvertex_enhanced( tile, screen );
  
  if ( swin->dis3d.remesh )
    {
      draw_tile_tface_remeshppd( tile, screen );
    }

  if ( swin->dis3d.remeshb )
    {
      draw_tile_tedge( tile, screen );
    }

  //
  // 選択用
  //
  draw_tile_tface_boundary( tile, screen );
}
  
static void drawppd( Sppd *ppd, ScreenAtr *screen )
{
  // vertex
  if ( swin->dis3d.vertex ) {
    drawppd_vertex( ppd, screen );
  }

  // vertex id
  if ( swin->dis3d.vertexid ) {
    drawppd_vertexid( ppd, screen );
  }

  // wireframe, shading, hiddenline
  if ( swin->dis3d.hidden )
    {
      drawppd_hiddenline( ppd, screen );
    }
  else if ( (swin->dis3d.wire) && (swin->dis3d.shading) )
    {
      drawppd_shading( ppd, screen );
      drawppd_edge( ppd, screen, blackvec );
    }
  else if ( !(swin->dis3d.wire) && (swin->dis3d.shading) )
    {
      drawppd_shading( ppd, screen );
    }
  else if ( (swin->dis3d.wire) && !(swin->dis3d.shading) )
    {
      drawppd_edge( ppd, screen, blackvec );
    }

  // edge id
  if ( swin->dis3d.edgeid )
    {
      drawppd_edgeid( ppd, screen );
    }

  // loop
  if ( swin->dis3d.loop ) {
    if ( swin->dis3d.enhanced == SMD_OFF ) {
      drawppd_loop( ppd, screen );
    } else {
      drawppd_loop_enhanced( ppd, screen );
    }
  }
		
  // subgraph
  if ( swin->dis3d.subgraph == SMD_ON ) {
    if ( ppd->sg != NULL ) {
      //draw_sged( ppd->sg, screen, (GLdouble *) bluevec, 1.0 );
      draw_sged( ppd->sg, screen, (GLdouble *) redvec, 1.0 );
    }
  }
  
}

void draws3d( ScreenAtr *screen )
{
  if ( swin->dis3d.coaxis ) {
    draw3dcoaxis( screen );
  }

  // display file
  if ( screen->current_tile != (Tile *) NULL ) {
    drawtile( screen->current_tile, screen );
  }
  
  // display ppd
  if ( screen->view_ppd != (Sppd *) NULL ) {
    drawppd( screen->view_ppd, screen );
  }

  // display sgraph
  if ( swin->dis3d.spath_anim == SMD_ON ) {
    draw_sged( screen->view_sg, screen, (GLdouble *) redvec, 1.0 );
    draw_sglp( screen->view_sglp );
  }

  // display approximate shortest path
  if ( screen->alp != (Splp *) NULL ) {

    if ( swin->dis3d.enhanced == SMD_OFF ) {
      draw_splp( screen->alp, redvec, 5.0 );
    } else {
      //draw_splp_enhanced( screen->alp, screen );
    }
  }

  // display exact shortset path
  if ( screen->elp != (Splp *) NULL ) {

    if ( swin->dis3d.enhanced == SMD_OFF ) {
      draw_splp( screen->elp, blackvec, 7.0 );
    } else {
      //draw_splp_enhanced( screen->elp, screen );
    }
  }

}

//
// display characters
//
void draw_string( ScreenAtr *screen, char *s )
{
  ::glPushAttrib( GL_LIST_BIT );
  ::glListBase(( GLuint) screen->fontOffset );
  ::glCallLists( strlen(s), GL_UNSIGNED_BYTE, (GLubyte *) s );
  ::glPopAttrib();
}

void draw_gradient_background()
{
  ::glDisable(GL_DEPTH_TEST);
  ::glPushMatrix();
  ::glLoadIdentity();
  ::glMatrixMode(GL_PROJECTION);
  ::glPushMatrix();
  ::glLoadIdentity();

  ::glBegin(GL_QUADS);
  ::glColor3f(0.2F, 0.0F, 1.0F); ::glVertex2f(-1.0F, -1.0F);
  ::glColor3f(0.2F, 0.0F, 1.0F); ::glVertex2f( 1.0F, -1.0F);
  ::glColor3f(0.0F, 0.0F, 0.1F); ::glVertex2f( 1.0F,  1.0F);
  ::glColor3f(0.0F, 0.0F, 0.1F); ::glVertex2f(-1.0F,  1.0F);
  ::glEnd();

  ::glEnable(GL_DEPTH_TEST);
  ::glPopMatrix();
  ::glMatrixMode(GL_MODELVIEW);
  ::glPopMatrix();
}

void clear_light( void )
{
  if ( swin->dis3d.light0 ) ::glEnable( GL_LIGHT0 ); else ::glDisable( GL_LIGHT0 );
  if ( swin->dis3d.light1 ) ::glEnable( GL_LIGHT1 ); else ::glDisable( GL_LIGHT1 );
  if ( swin->dis3d.light2 ) ::glEnable( GL_LIGHT2 ); else ::glDisable( GL_LIGHT2 );
  if ( swin->dis3d.light3 ) ::glEnable( GL_LIGHT3 ); else ::glDisable( GL_LIGHT3 );
}
  
void redraw( void )
{
  ScreenAtr *screen;
  screen = &( swin->screenatr );

  //display("width %d height %d\n", screen->width, screen->height );
  
  ::glViewport((GLint) 0, (GLint) 0,
	       (GLsizei) screen->width, (GLsizei) screen->height);
	
  clear_gl3d( screen );
  
  ::glPushMatrix();
  view_init( screen );
  clear_light();
  draws3d( screen );
  ::glPopMatrix();

  ::glFinish();
}
