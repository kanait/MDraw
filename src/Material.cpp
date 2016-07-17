// Material.cpp 
// created by T.Kanai 1998.08.02

#include "stdafx.h"
// #include "OpenGL.h"
#include "Material.h"

#include "smd.h"

#include "mtl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CMaterial::CMaterial( void )
{
  ambient[0] = (float) 0.2;
  ambient[1] = (float) 0.2;
  ambient[2] = (float) 0.2;
  ambient[3] = (float) 1.0;
  diffuse[0] = (float) 0.8;
  diffuse[1] = (float) 0.8;
  diffuse[2] = (float) 0.8;
  diffuse[3] = (float) 1.0;
  emission[0] = (float) 0.0;
  emission[1] = (float) 0.0;
  emission[2] = (float) 0.0;
  emission[3] = (float) 1.0;
  specular[0] = (float) 0.0;
  specular[1] = (float) 0.0;
  specular[2] = (float) 0.0;
  specular[3] = (float) 1.0;
  shininess[0] = (float) 0.0;	
}

CMaterial::~CMaterial( void )
{
  return;
}

void CMaterial::Binding( void )
{
  ::glMaterialfv( GL_FRONT, GL_AMBIENT, (GLfloat *) ambient );
  ::glMaterialfv( GL_FRONT, GL_DIFFUSE, (GLfloat *) diffuse );
  ::glMaterialfv( GL_FRONT, GL_SPECULAR, (GLfloat *) specular );
  ::glMaterialfv( GL_FRONT, GL_EMISSION, (GLfloat *) emission );
  ::glMaterialfv( GL_FRONT, GL_SHININESS, (GLfloat *) shininess );
}

void material_binding( Material *material )
{
  glMaterialfv(GL_FRONT, GL_AMBIENT, material->ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, material->diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, material->specular);
  glMaterialfv(GL_FRONT, GL_EMISSION, material->emission);
  glMaterialfv(GL_FRONT, GL_SHININESS, material->shininess);
}

// for material all
void copy_material( Material *mtl, int no, float *mtlall )
{
  //display("amb %g\n", mtl->ambient[0] );
  int id = no * NUM_MTL_ITEMS;
  mtl->ambient[0] = mtlall[id+0];
  mtl->ambient[1] = mtlall[id+1];
  mtl->ambient[2] = mtlall[id+2];
  mtl->ambient[3] = mtlall[id+3];
  mtl->diffuse[0] = mtlall[id+4];
  mtl->diffuse[1] = mtlall[id+5];
  mtl->diffuse[2] = mtlall[id+6];
  mtl->diffuse[3] = mtlall[id+7];
  mtl->emission[0] = mtlall[id+8];
  mtl->emission[1] = mtlall[id+9];
  mtl->emission[2] = mtlall[id+10];
  mtl->emission[3] = mtlall[id+11];
  mtl->specular[0] = mtlall[id+12];
  mtl->specular[1] = mtlall[id+13];
  mtl->specular[2] = mtlall[id+14];
  mtl->specular[3] = mtlall[id+15];
  mtl->shininess[0] = mtlall[id+16];
}

