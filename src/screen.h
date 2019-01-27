//
// screen.h
//
// Copyright (c) 1997-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

//
// window attributes structure
//

#ifndef _SCREEN_H
#define _SCREEN_H

// "Pers" structure
typedef struct _pers {
  double scraspct;	// aspect ratio value
  double angle;		// the angle of the field of view
  double znear_ratio;	// znear plane ratio value
  double zfar_ratio;	// zfar plane ratio value
  double fd;		// focal distance
  Vec    eye_dir;	// eye direction
  Vec    ref;		// reference point vector
  Vec    up;            // up vector
} Pers;

// edit constants
// number of constants should be linked to the "edit_tb" arrays
#define EDIT_CREATE_SPATH           0
#define EDIT_MAKE_LINES             1
#define EDIT_DEL_LINES              2
#define EDIT_DEL_TILES              3
#define EDIT_MAKE_TILE_FACES        4
#define EDIT_MAKE_TILE_CYL_FACES    5
#define EDIT_REMESH                 6
#define EDIT_SEL_CVERTEX            7
#define EDIT_NONE		    999

// select constants
#define SELECTNONE      0
#define SELECTED        1
#define NONSELECTED     0
#define SELMAX          1000

#define SEL_VERTEX      1
#define SEL_EDGE        2
#define SEL_FACE        3
#define SEL_CLEAR       0

// file
#define SMDOPEN         0
#define SMDSAVE         1
#define SMDFILEPPD1     0
#define SMDFILEPPD2     1
#define SMDFILEGMH      2
#define SMDFILEGPPD     3

// rotate
#define ROTATE_NONE     0
#define ROTATE_XY       1
#define ROTATE_Z        2

// for calculating shortest path
// path type
#define PATH_APPROX	0
#define PATH_EXACT	1
#define PATH_BOTH	2

#define TWO_VERTICES  2

#ifndef _TILE_H
#include "tile.h"
#endif

typedef struct _sellist SelList;

struct _sellist {
  
  int     pid;
  Spvt    *vt;
  Spfc    *fc;
  Splp    *lp;
  TEdge   *ted;
  TFace   *tf;
  
  SelList *nxt, *prv;
};

#define SCREEN_SRC         0
#define SCREEN_DIST        1

typedef struct _screenatr {
  // screen id
  int           no;
  // screen type (2D or 3D)
  int           type;
  // resize flag
  BOOL          isResized;

  // for GLX
#ifdef GLX
  Widget        glw;
  XVisualInfo   *vi;
  GLXContext    xc;
  Widget        fr3d;
	// font
  unsigned int	fontOffset;
#endif

// for MFC
#ifdef _WINDOWS
  CWnd	        *wnd;
#endif

  // screen width, screen height, screen center
  int		width;
  int		height;
  double	xcenter;
  double	ycenter;

  //background color
  float         bgrgb[3];

  // zoom parameter
  BOOL          isZoom; // zoom_active
  float         zoom;

  // rotate parameter
  BOOL		isRotate; // rotate_active
  int		rotate_type;
  double	rotate_x, rotate_y, rotate_z;
  double	zrot_rad;
  double	rmat[16];
  Vec		rotate_xvec, rotate_yvec, rotate_zvec;
  Vec		translate_vec;

  // font
  unsigned int fontOffset;

  // select structure
  BOOL		isSelected; // sellist_flag
  int           n_sellist;
  SelList       *sel_first;
  SelList       *sel_last;
  
  // delete area parameter
  int           isAreaSelected; // areaflag
  Vec2d         areaorg;
  Vec2d         areaatv;

  // lighting parameter
  Light         light;

  // perspective parameter
  Pers          pers;

  // for getting GL perspectives
  double        mmat[16], pmat[16];
  int           viewport[4];

  // current_ppd
  Sppd          *current_ppd;
  Sppd          *view_ppd;

#if 0
  // current_tile
  Tile          *current_tile;
#endif

  // radius of sphere and cylinder for displaying
  double	rad_sph, rad_cyl;

  // for shortest path
  // approximate
  Splp          *alp;
  // for animation
  SGraph	*view_sg;
  SGlp		*view_sglp;
  // exact
  Splp          *elp;
  // gamma
  double sp_gamma;

  // for testing
  Splp          **elpary;
  Splp          **alpary;

} ScreenAtr;

//
// display structure
//
typedef struct _display3d {

  // common
  BOOL	vertex;
  BOOL	wire;
  BOOL	shading;
  BOOL	hidden;
  BOOL	loop;
  BOOL	remesh;
  BOOL	remeshb;
  BOOL  group_mesh;
  BOOL	coaxis;
  
  BOOL	vertexid;
  BOOL	edgeid;
  
  BOOL	spath;
  BOOL	subgraph;
  
  // texture
  BOOL texture;
  
  BOOL	area;

  // vertices as spheres, edges as tubes
  BOOL	enhanced;

  // for smooth shading
  BOOL	smooth;

  // for gradient background
  BOOL	gradient;

  // for animation of computing shortest paths
  BOOL	spath_anim;

  // light
  BOOL  light0;
  BOOL  light1;
  BOOL  light2;
  BOOL  light3;

} Disp3D;

#define VIEW_ORGMESH   0
#define VIEW_INTPMESH  1

// 
// The data structure of GUI (includes window dependant) parameters
// 
typedef struct _swin {

  // window wize
  int width, height;

  // edit type
  int edit_type;
  unsigned short select_type;
  
  // shortest path type
  int path_type;

  // parameter type (used in "Make Face")
  int param_type;

  // re-calc parameterization (used in "Remesh")
  BOOL recalc_param;

  // parameter correction type
  int paramcorr_type;
  
  // window dependent parameters
  ScreenAtr screenatr;

  // display structure 
  Disp3D    dis3d;

  // default materials
  Material  *material;

  // menu mode parameters
  // file flag (decide *.<***>) 
  unsigned short filed;
  // file open flag (0 : open 1 : save) 
  unsigned short opend;

  // Matrix for creating postscript file
  double	mmat[16];
  double	pmat[16];
  
  // standard output or not
  BOOL		isPrint; // prn

  // for making subgraph
  double	sublength;

  // save harmonic map to postscript file
  BOOL		isSaveHMtoPS; // savehmtops
	
  // save SGI image files.
  BOOL		isSaveSGIImage; // saveimg

  // save BMP images
  BOOL		isSaveBMPImage; // savebmp
  int		savebmpid;

} Swin;

#ifdef _WINDOWS
extern Swin *swin;
#endif

// functions
extern void start_screen3d_rotate(ScreenAtr *);
extern void update_screen3d_rotate(ScreenAtr *, int, int);
extern void update_screen3d_rotate_z(ScreenAtr *, double);
extern void finish_screen3d_rotate(ScreenAtr *);
extern void start_screen3d_zoom(ScreenAtr *);
extern void update_screen3d_zoom(ScreenAtr *, int, int);
extern void finish_screen3d_zoom(ScreenAtr *);
extern void update_screen3d_translate(ScreenAtr *, int, int);
extern void finish_screen3d_translate(ScreenAtr *);

#endif // _SCREEN_H
