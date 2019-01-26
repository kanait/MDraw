//
// glppd.cpp
//
// Copyright (c) 1997-1999 by Takashi Kanai
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

#include <math.h>
#include "gl\gl.h"
#include "gl\glu.h"

#include "smd.h"
#include "screen.h"
#include "glppd.h"
#include "draw.h"
#include "sgraph.h"

#ifndef _COLOR_H
#include "color.h"
#endif

#ifndef _MTL_H
#include "mtl.h"
#endif

// for displaying sgraph edge
#define VOFFSET 0.001

//#define screen->rad_sph  0.1
//#define screen->rad_sph  0.01
//#define SMALL_SPHERE_RADIUS  0.008
//#define SMALL_SPHERE_RADIUS  0.05
//#define screen->rad_cyl    0.05
//#define screen->rad_cyl    0.005
#define NUM_GROUP_EDGE 5

static Material line_mat[] = {
  0.187004f, 0.138930f, 0.138930f, 1.0f,
  0.748016f, 0.75721f,  0.155721f, 1.0f,
  0.0f, 0.0f, 0.0f, 1.0f,
  0.836364f, 0.836364f, 0.836364f, 1.0f,
  77.575806f
};

static Material pointred_mat[] = {
  0.187004f, 0.0f, 0.0f, 1.0f,
  0.748016f, 0.0f, 0.0f, 1.0f,
  0.0f, 0.0f, 0.0f, 1.0f,
  0.672727f, 0.672727f, 0.672727f, 1.0f,
  29.478785f
};

static Material point_mat[] = {
  0.005183f, 0.084911f, 0.084911f, 1.0f,
  0.020732f, 0.339645f, 0.339645f, 1.0f,
  0.0f, 0.0f, 0.0f, 1.0f,
  0.903030f, 0.903030f, 0.903030f, 1.0f,
  118.690941f
};

static Material vertex_mat[] = {
  0.0f, 0.187004f, 0.187004f, 1.0f, 
  0.0f, 0.748016f, 0.748016f, 1.0f, 
  0.0f, 0.0f, 0.0f, 1.0f, 
  0.981818f, 0.981818f, 0.981818f, 1.0f, 
  92.315140f
};

static Material edge_mat[] = {
  0.038095f, 0.036285f, 0.036285f, 1.0f, 
  0.152381f, 0.145141f, 0.145141f, 1.0f, 
  0.0f, 0.0f, 0.0f, 1.0f, 
  0.745455f, 0.745455f, 0.745455f, 1.0f, 
  122.569725f
};

void drawppd_vertex( Sppd *ppd, ScreenAtr *screen )
{
  if ( ppd == NULL ) return;
	
  glPointSize( 2.5 );
  for ( Spvt *v = ppd->spvt; v != (Spvt *) NULL; v = v->nxt ) {

    if ( v->col == PNTGREEN ) {
      glColor3dv( blackvec );
    } else if ( v->col == PNTRED ) {
      glColor3dv( redvec );
    } else if ( v->col == PNTBLUE ) {
      glColor3dv( bluevec );
    }
    
    // temporary
    glBegin( GL_POINTS );
    Vec *vec = &(v->vec);
    glVertex3d( (GLdouble) vec->x, (GLdouble) vec->y, (GLdouble) vec->z );
    glEnd();

  }

//    glEnable(GL_LIGHTING);
//    material_binding( pointred_mat );
//    for ( v = ppd->spvt; v != (Spvt *) NULL; v = v->nxt ) {
//      if ( v->isConst == SMD_ON ) glu_sphere( &(v->vec), screen->rad_sph );
//    }
//    glDisable(GL_LIGHTING);
}

void drawppd_vertex_enhanced( Sppd *ppd, ScreenAtr *screen )
{
  if ( ppd == NULL ) return;
	
  glEnable(GL_LIGHTING);
	
  Spvt *v;
  for ( v = ppd->spvt; v != (Spvt *) NULL; v = v->nxt ) {
		
    if ( v->type == VERTEX_ORIGINAL ) {
			
      switch ( v->sp_type ) {
      case SP_VERTEX_TVERTEX:
	material_binding( point_mat );
	//material_binding( tropical20 );
	break;
      default:
	material_binding( tropical20 );
	break;
      }
      if ( v->col == PNTRED ) material_binding( pointred_mat );
			
      glu_sphere( &(v->vec), screen->rad_sph );
			
    }
		
  }
	
  glDisable(GL_LIGHTING);
}

void drawppd_vertexid( Sppd *ppd, ScreenAtr *screen )
{
  glColor3dv( bluevec );
  for ( Spvt* v = ppd->spvt; v != NULL; v = v->nxt )
    {
      if ( (v->no == 10323) )
	{
	  glRasterPos3d( v->vec.x, v->vec.y, v->vec.z );
	  char item[BUFSIZ];
	  sprintf( item, "v%d", v->no );
	  (void) draw_string( screen, item );
	}
    }
}

//
// wireframe
//
void drawppd_wireframe( Sppd *ppd, ScreenAtr *screen )
{
  if ( ppd == NULL ) return;
	
  glColor3dv( blackvec );
  //glColor3dv( gray50vec );
  glLineWidth( (GLfloat) 1.0 );

  for ( Spfc *fc = ppd->spfc; fc != NULL; fc = fc->nxt ) {
    Sphe *he = fc->sphe;
    glBegin( GL_LINE_LOOP );
    do {
      Vec *vec = &( he->vt->vec );
      glVertex3d( (GLdouble) vec->x, (GLdouble) vec->y, (GLdouble) vec->z );
    } while ( (he = he->nxt) != fc->sphe );
    glEnd();

  }
}

void drawppd_edge( Sppd *ppd, ScreenAtr *screen, double *color )
{
  Sped *e;
  Vec  *svec, *evec;
	
  if ( ppd == NULL ) return;
	
  glLineWidth((GLfloat) 1.0 );
	
  for ( e = ppd->sped; e != (Sped *) NULL; e = e->nxt ) {
		
    //if ( e->type != EDGE_ORIGINAL )  continue;

    glLineWidth((GLfloat) 1.0 );
    glColor3dv( (GLdouble *) color );
    
    if ( (e->type == EDGE_ONFACE) && (e->sp_fc == NULL) ) {
      glLineWidth( 5.0 );
      glColor3dv( redvec );
    }
    
    svec = &(e->sv->vec);
    evec = &(e->ev->vec);
    glBegin(GL_LINE_STRIP);
    glVertex3d( (GLdouble) svec->x, (GLdouble) svec->y, (GLdouble) svec->z );
    glVertex3d( (GLdouble) evec->x, (GLdouble) evec->y, (GLdouble) evec->z );
    glEnd();
		
  }
}

void drawppd_edgeid( Sppd *ppd, ScreenAtr *screen )
{
  glColor3dv( redvec );
  for ( Sped* e = ppd->sped; e != (Sped *) NULL; e = e->nxt )
    {
      if ( e->no == 37391 )
	{
	  glRasterPos3d( ( e->sv->vec.x + e->ev->vec.x ) / 2.0, 
			 ( e->sv->vec.y + e->ev->vec.y ) / 2.0, 
			 ( e->sv->vec.z + e->ev->vec.z ) / 2.0 );
	  char item[BUFSIZ];
	  sprintf( item, "e%d", e->no );
	  (void) draw_string( screen, item );
	}
    }
}

void drawppd_hiddenline( Sppd *ppd, ScreenAtr *screen )
{
  Spfc  *f;
  Sphe  *he;
  Vec   *vec, *nrm;
	
  glLineWidth( 1.0 );
	
  glEnable(GL_STENCIL_TEST);
  glClear(GL_STENCIL_BUFFER_BIT);
  glStencilFunc(GL_ALWAYS, 0, 1);
  glStencilOp( GL_INVERT, GL_INVERT, GL_INVERT );
	
  for ( f = ppd->spfc; f != NULL; f = f->nxt ) {
    //
    // edge drawing (1st stencil)
    //
    glColor3dv( blackvec );
    he = f->sphe;
    glBegin(GL_LINE_LOOP);
    do {
      vec = &(he->vt->vec);
      glVertex3d( vec->x, vec->y, vec->z );
    } while ((he = he->nxt) != f->sphe);
    glEnd();
    //
    // polygon rendering
    //
    //glColor3dv( whitevec );
    glColor3d( screen->bgrgb[0], screen->bgrgb[1], screen->bgrgb[2]  );
    glStencilFunc(GL_EQUAL, 0, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glBegin( GL_POLYGON );
    he = f->sphe;
    nrm = &(f->nrm);
    do {
      glNormal3d( nrm->x, nrm->y, nrm->z );
      vec = &(he->vt->vec);
      glVertex3d( vec->x, vec->y, vec->z );
    } while ((he = he->nxt) != f->sphe);
    glEnd();
    //
    // edge drawing (2nd stencil)
    //
    glColor3dv( blackvec );
    glStencilFunc(GL_ALWAYS, 0, 1);
    glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);
    he = f->sphe;
    glBegin(GL_LINE_LOOP);
    do {
      vec = &(he->vt->vec);
      glVertex3d( vec->x, vec->y, vec->z );
    } while ((he = he->nxt) != f->sphe);
    glEnd();
    //
    //
  }

}

// sgraph edges
void draw_sged( SGraph *sg,
		ScreenAtr *screen,
		GLdouble *color,
		GLfloat width
		)
{
 if ( sg == NULL ) return;
	
 SGed *e;
 Vec  *svec, *evec;
 glLineWidth((GLfloat) width );
 glColor3dv( color );
	
 // edge
 for ( e = sg->sged; e != (SGed *) NULL; e = e->nxt ) {

   // test.
   //if ( e->sped == NULL ) {
   //if ( e->sped->sp_type != EDGE_ORIGINAL ) {
   svec = &(e->sv->vec);
   evec = &(e->ev->vec);
    
   glBegin( GL_LINE_STRIP );
   glVertex3d( (GLdouble) svec->x, (GLdouble) svec->y, (GLdouble) svec->z );
   glVertex3d( (GLdouble) evec->x, (GLdouble) evec->y, (GLdouble) evec->z );
   glEnd();
   //}
    
 }
}

// –@ü‚Ì³‚Ì•ûŒü‚É•‚‚©‚·
//  void draw_sged( SGraph *sg,
//  		ScreenAtr *screen,
//  		GLdouble *color,
//  		GLfloat width
//  		)
//  {
//    if ( sg == NULL ) return;
	
//    SGed *e;
//    SGvt *sv, *ev;
//    Vec  svec, evec, svnrm, evnrm;
  
//    glLineWidth((GLfloat) width );
//    glColor3dv( color );

//    //double VOFFSET = 0.0005 * ppd->scale;
//    //double VOFFSET = 0.0005;
//    double voffset = VOFFSET * sg->ppd->scale;
//    // edge
//    for ( e = sg->sged; e != (SGed *) NULL; e = e->nxt ) {

//      if ( e->type != EDGE_ONFACE ) continue;
//      sv = e->sv;
//      SGvtnrm( sv, &svnrm );
//      svec.x = sv->vec.x + VOFFSET * svnrm.x;
//      svec.y = sv->vec.y + VOFFSET * svnrm.y;
//      svec.z = sv->vec.z + VOFFSET * svnrm.z;
    
//      ev = e->ev;
//      SGvtnrm( ev, &evnrm );
//      evec.x = ev->vec.x + VOFFSET * evnrm.x;
//      evec.y = ev->vec.y + VOFFSET * evnrm.y;
//      evec.z = ev->vec.z + VOFFSET * evnrm.z;
    
//      glBegin( GL_LINE_STRIP );
//      glVertex3d( (GLdouble) svec.x, (GLdouble) svec.y, (GLdouble) svec.z );
//      glVertex3d( (GLdouble) evec.x, (GLdouble) evec.y, (GLdouble) evec.z );
//      glEnd();
//      //}
    
//    }
//  }

void drawppd_edge_enhanced( Sppd *ppd, ScreenAtr *screen )
{
  if ( ppd == NULL ) return;
	
  Sped *e;
	
  glEnable(GL_LIGHTING);
  material_binding( edge_mat );
	
  // edge
  for (e = ppd->sped; e != (Sped *) NULL; e = e->nxt) {
		
    if ( e->type == EDGE_ORIGINAL ) {
      glu_cylinder( &(e->sv->vec), &(e->ev->vec) , screen->rad_cyl );
    }
		
  }
	
  glDisable(GL_LIGHTING);
	
}

void drawppd_shading( Sppd *ppd, ScreenAtr *screen )
{
  Spfc  *f;
  Sphe  *he;
  Vec   *vec;
	
  if ( ppd == NULL ) return;
	
  //material_binding( swin->material );
  
  //display("fn %d\n", ppd->fn );
  for ( f = ppd->spfc; f != NULL; f = f->nxt ) {
		
    if ( (swin->dis3d.group_mesh == TRUE) ||
	 (f->tile_id == SMDNULL) ) {
      
      if ( f->col == FACEBLUE )
	{
//        if ( (f->col == FACEBLUE) && (f->no != 7328) ) {
	  ::glEnable(GL_LIGHTING);
	}

      if ( swin->dis3d.texture == TRUE )
	{
	  ::glEnable(GL_TEXTURE_2D);
	}
      
      //display("aa\n");
      if ( f->tile_id == SMDNULL ) material_binding( &(ppd->matl) );
      else material_binding( tropical3 );

      glBegin( GL_POLYGON );
      he = f->sphe;
      do {
	if ( (he->nm != NULL) && (swin->dis3d.smooth == TRUE) ) {
	  vec = &(he->nm->vec);
	  glNormal3d( vec->x, vec->y, vec->z );
	} else {
	  glNormal3d( f->nrm.x, f->nrm.y, f->nrm.z );
	}

	if ( (ppd->texcoord == TRUE) && (swin->dis3d.texture == TRUE) ) {
	  // texture coord
	  Vec2d* uvw = &(he->vt->uvw);
	  glTexCoord2d( uvw->x, uvw->y );
	}

	// coordinates
	vec = &(he->vt->vec);
	glVertex3d( vec->x, vec->y, vec->z );
				
      } while ((he = he->nxt) != f->sphe);
      glEnd();

      if ( swin->dis3d.texture == TRUE )
	{
	  glDisable(GL_TEXTURE_2D);
	}
      glDisable(GL_LIGHTING);
      
//        glDisable(GL_TEXTURE_2D);
//        glDisable(GL_LIGHTING);
			
      //} else if ( f->tile_id != SMDNULL ) {
    } else if ( f->col == FACERED ) {
			
      glColor3dv( redvec );
      glBegin(GL_POLYGON);
      he = f->sphe;
      do {
	vec = &(he->vt->vec);
	glVertex3d( (GLdouble) vec->x, (GLdouble) vec->y, (GLdouble) vec->z );
      } while ((he = he->nxt) != f->sphe);
      glEnd();
			
    } else if ( f->no == 7328 ) {
//      } else if ( f->col == FACEGREEN ) {
			
      glColor3dv(greenvec);
      glBegin(GL_POLYGON);
      he = f->sphe;
      do {
	vec = &(he->vt->vec);
	glVertex3d( (GLdouble) vec->x, (GLdouble) vec->y, (GLdouble) vec->z );
      } while ((he = he->nxt) != f->sphe);
      glEnd();
			
    }

    // temp
//      if ( (f->no == 7328) ) {
//        Vec a;
//        a.x = 0; a.y = 0; a.z = 0;
//        he = f->sphe;
//        do {
//  	a.x += he->vt->vec.x;
//  	a.y += he->vt->vec.y;
//  	a.z += he->vt->vec.z;
//        } while ( (he = he->nxt) != f->sphe );
//        a.x /= (double) f->hen;
//        a.y /= (double) f->hen;
//        a.z /= (double) f->hen;
//        char item[BUFSIZ];
//        sprintf( item, "f%d", f->no );
//        glColor3dv( blackvec );
//        glRasterPos3d((GLdouble) a.x, (GLdouble) a.y, (GLdouble) a.z);
//        (void) draw_string( screen, item );
//      }
    
  }
}


void drawppd_loop( Sppd *ppd, ScreenAtr *screen )
{
  Splp *lp;
	
  for ( lp = ppd->splp; lp != (Splp *) NULL; lp = lp->nxt ) {
		
    draw_loop( lp );
		
  }
}

void draw_loop( Splp *lp )
{
  Splv *lv;
  Spvt *vt;
  Vec  *vec;
	
  if ( lp == NULL ) return;
  /* loops */
	
  glLineWidth((GLfloat) 2.0);
	
  switch ( lp->type ) {
  case SHORTESTPATH:
    glColor3dv( red3vec );
    break;
  default:
    glColor3dv( bluevec );
  }

  // selected
  if ( lp->col == LOOPRED ) glColor3dv( redvec );
	
  switch ( lp->type ) {
  case CLOSEDLOOP:
    glBegin( GL_LINE_LOOP );
    break;
  default:
    glBegin( GL_LINE_STRIP );
    break;
  }
	
  for ( lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt ) {
    vec = &(lv->vt->vec);
    glVertex3d( (GLdouble) vec->x, (GLdouble) vec->y, (GLdouble) vec->z );
  }
  glEnd();
	
  glPointSize(7.0);
	
  for ( lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt ) {
		
    vt = lv->vt;
		
    if ( vt->col != PNTRED ) glColor3dv( orangevec );
    else glColor3dv( redvec );
		
//      if ( (vt->sp_type != SP_VERTEX_TVERTEX) && (vt->spn > 1 ) ) {
//        glColor3dv( bluevec );
//      }
		
    glBegin(GL_POINTS);
    glVertex3d( (GLdouble) vt->vec.x, (GLdouble) vt->vec.y, (GLdouble) vt->vec.z );
    glEnd();
  }
	
}

//
// display of splp
// 
void draw_splp( Splp *lp, double *color, float width )
{
  if ( lp == NULL ) return;

  // edge width
  glLineWidth( (GLfloat) width );

  // color
  if ( lp->col == LOOPBLUE ) {
    glColor3dv( (GLdouble *) color );
  } else if ( lp->col == PNTRED ) {
    glColor3dv( redvec );
  }

  switch ( lp->type ) {
  case CLOSEDLOOP:
    glBegin( GL_LINE_LOOP );
    break;
  default:
    glBegin( GL_LINE_STRIP );
    break;
  }

  for ( Splv *lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt ) {
    Vec *vec = &( lv->vt->vec );
    glVertex3d( (GLdouble) vec->x, (GLdouble) vec->y, (GLdouble) vec->z );
  }
  glEnd();

  if ( lp->type == CLOSEDLOOP ) return;
  
  // vertices
  glPointSize( 6.0 );
  glBegin( GL_POINTS );
  for ( Splv *lv = lp->splv; lv != NULL; lv = lv->nxt ) {
    
    if ( lv->vt->sp_type == SP_VERTEX_TVERTEX ) {
      
      glColor3dv( orangevec );
      
    } else if ( lv->vt->sp_type == SP_VERTEX_TEDGE_BP ) {
      
      glColor3dv( lightgreenvec );
      
    } else {

      glColor3dv( bluevec );
      
//        continue;
      
    }

    if ( lv->vt->col == PNTRED ) glColor3dv( redvec );

    Vec *vec = &( lv->vt->vec );
    glVertex3d( (GLdouble) vec->x, (GLdouble) vec->y, (GLdouble) vec->z );
  }
  glEnd();
}

//
// display of splp
// 
void draw_splp_enhanced( Splp *lp, double *color, float width,
			 ScreenAtr *screen )
{
  if ( lp == NULL ) return;

  // edge width
  glLineWidth( (GLfloat) width );

  // color
  if ( lp->col == LOOPBLUE ) {
    glColor3dv( (GLdouble *) color );
  } else if ( lp->col == PNTRED ) {
    glColor3dv( redvec );
  }

  switch ( lp->type ) {
  case CLOSEDLOOP:
    glBegin( GL_LINE_LOOP );
    break;
  default:
    glBegin( GL_LINE_STRIP );
    break;
  }

  for ( Splv *lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt ) {
    Vec *vec = &( lv->vt->vec );
    glVertex3d( (GLdouble) vec->x, (GLdouble) vec->y, (GLdouble) vec->z );
  }
  glEnd();

  if ( lp->type == CLOSEDLOOP ) return;

//    // loop vertices
//    material_binding( point_mat );
//    for ( lv = lp->splv; lv != NULL; lv = lv->nxt ) {
//      //		material_binding( pointred_mat );
//      glu_sphere( &(lv->vt->vec), screen->rad_sph );

//    }
//    glDisable( GL_LIGHTING );
//  }

  
  // vertices
  //glPointSize( 6.0 );
  glEnable( GL_LIGHTING );
  //glBegin( GL_POINTS );
  for ( Splv *lv = lp->splv; lv != NULL; lv = lv->nxt ) {
    
    if ( lv->vt->sp_type == SP_VERTEX_TVERTEX ) {

      material_binding( pointred_mat );
      //material_binding( point_mat );
      //glColor3dv( orangevec );
      
    } else if ( lv->vt->sp_type == SP_VERTEX_TEDGE_BP ) {
      
      material_binding( pointred_mat );
      //material_binding( point_mat );
      //glColor3dv( lightgreenvec );
      
    } else {
      
      continue;
      
    }

    if ( lv->vt->col == PNTRED ) material_binding( pointred_mat );

    Vec *vec = &( lv->vt->vec );
    glu_sphere( vec, screen->rad_sph );
    
  }

  glDisable( GL_LIGHTING );
  //glEnd();
}

//
// enhanced display of splp
//
//  void draw_splp_enhanced( Splp *lp, ScreenAtr *screen )
//  {
//    if ( lp == NULL ) return;

//    glEnable( GL_LIGHTING );
//    // loop edges
//    material_binding( line_mat );
//    for ( Splv *lv = lp->splv; lv != NULL; lv = lv->nxt ) {
//      if ( lv->nxt != NULL ) {
//        glu_cylinder( &(lv->vt->vec), &(lv->nxt->vt->vec) , screen->rad_cyl );
//      }

//    }

//    // loop vertices
//    material_binding( point_mat );
//    for ( lv = lp->splv; lv != NULL; lv = lv->nxt ) {
//      //		material_binding( pointred_mat );
//      glu_sphere( &(lv->vt->vec), screen->rad_sph );

//    }
//    glDisable( GL_LIGHTING );
//  }

//
// display of sglp
//
void draw_sglp( SGlp *lp )
{
  SGlpvt *lv;
  Vec  *vec;
  Vec nrm, svec;
	
  if ( lp == NULL ) return;
  /* loops */
	
  glLineWidth( (GLfloat) 5.0 );
  //glColor3dv( darkvioletvec );
  glColor3dv( bluevec );
  
  glBegin( GL_LINE_STRIP );
  for ( lv = lp->slpvt; lv != (SGlpvt *) NULL; lv = lv->nxt ) {
    SGvtnrm( lv->vt, &nrm );
    vec = &( lv->vt->vec );
    svec.x = vec->x + VOFFSET * nrm.x;
    svec.y = vec->y + VOFFSET * nrm.y;
    svec.z = vec->z + VOFFSET * nrm.z;
    glVertex3d( (GLdouble) svec.x, (GLdouble) svec.y, (GLdouble) svec.z );
  }
  glEnd();
	
  glPointSize( 5.0 );
  glColor3dv( orangevec );
	
  glBegin( GL_POINTS );
  for ( lv = lp->slpvt; lv != (SGlpvt *) NULL; lv = lv->nxt ) {
    vec = &( lv->vt->vec );
    glVertex3d( (GLdouble) vec->x, (GLdouble) vec->y, (GLdouble) vec->z );
  }
  glEnd();
}

//
// enhanced display of sglp
//
void draw_sglp_enhanced( SGlp *lp, ScreenAtr *screen )
{
  SGlpvt *lv;
	
  if ( lp == NULL ) return;

  glEnable( GL_LIGHTING );
  // loop edges
  material_binding( line_mat );
  for ( lv = lp->slpvt; lv != (SGlpvt *) NULL; lv = lv->nxt ) {
    if ( lv->nxt != NULL ) {
      glu_cylinder( &(lv->vt->vec), &(lv->nxt->vt->vec) , screen->rad_cyl );
    }

  }

  // loop vertices
  material_binding( point_mat );
  for ( lv = lp->slpvt; lv != (SGlpvt *) NULL; lv = lv->nxt ) {
    //		material_binding( pointred_mat );
    glu_sphere( &(lv->vt->vec), screen->rad_sph );

  }
  glDisable( GL_LIGHTING );
}

void drawppd_loop_enhanced( Sppd *ppd, ScreenAtr *screen )
{
  Splp *lp;
	
  for ( lp = ppd->splp; lp != (Splp *) NULL; lp = lp->nxt ) {
		
    draw_loop_enhanced( lp, screen );
		
  }
}

void draw_loop_enhanced( Splp *lp, ScreenAtr *screen )
{
  Splv *lv;
  Spvt *vt;
	
  if ( lp == NULL ) return;
  /* loops */
	
  glLineWidth((GLfloat) 2.0);
	
  switch ( lp->type ) {
  case CLOSEDLOOP:
    glBegin( GL_LINE_LOOP );
    break;
  default:
    glBegin( GL_LINE_STRIP );
    break;
  }
	
  for (lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt) {
    glVertex3d( (GLdouble) lv->vt->vec.x,
		(GLdouble) lv->vt->vec.y,
		(GLdouble) lv->vt->vec.z );
  }
  glEnd();
	
  glEnable( GL_LIGHTING );
  for ( lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt ) {
    vt = lv->vt;
    switch ( vt->sp_type ) {
    case SP_VERTEX_TVERTEX:
      if ( vt->col != PNTRED ) material_binding( point_mat );
      else material_binding( pointred_mat );
      glu_sphere( &(vt->vec), screen->rad_sph );
      break;
    default:
      if ( vt->col != PNTRED ) material_binding( vertex_mat );
      else material_binding( pointred_mat );
      glu_sphere( &(vt->vec), screen->rad_sph );
      break;
    }
  }
  glDisable( GL_LIGHTING );
}

//
// draw primitives 
//
void glu_sphere( Vec *pos, double radius )
{
  GLUquadricObj   *qobj;
  GLint           slices = 20,staks = 10;
	
  if ((qobj = gluNewQuadric()) != NULL) {
    glPushMatrix();
    glTranslated( (GLdouble) pos->x, (GLdouble) pos->y, (GLdouble) pos->z );
    gluSphere(qobj, (GLdouble) radius, slices, staks);
    glPopMatrix();
    gluDeleteQuadric(qobj);
  }
}

void glu_cylinder( Vec *sv, Vec *ev, double rad )
{
  GLUquadricObj   *cyl;
  GLint           slices = 20,staks = 10;
  Vec sub;
  double height;
  double rx, ry;
  double V3Length( Vec * );
	
  if ((cyl = gluNewQuadric()) != NULL) {
    glPushMatrix();
    sub.x  = ev->x - sv->x;
    sub.y  = ev->y - sv->y;
    sub.z  = ev->z - sv->z;
    height = V3Length( &sub );
    sub.x /= height; sub.y /= height; sub.z /= height;
		
    ry = asin( sub.x );
    rx = acos( sub.z / cos(ry) );
		
    if ( fabs ( sub.y + sin(rx) * cos(ry) > SMDZEROEPS) ) {
      rx = 2 * SMDPI - rx;
    }
		
    glTranslated ( sv->x, sv->y, sv->z );
    glRotated( rx * 180.0 / SMDPI, 1.0, 0.0, 0.0 );
    glRotated( ry * 180.0 / SMDPI, 0.0, 1.0, 0.0 );
    gluCylinder(cyl, rad, rad, height, slices, staks);
    glPopMatrix();
    gluDeleteQuadric(cyl);
  }
}
