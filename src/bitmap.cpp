//
// Bitmap.cpp
//  
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved.
//

#include "stdafx.h"

//#include "MainFrm.h"
#include "gl\gl.h"

#include "smd.h"
#include "screen.h"
#include "bitmap.h"
#include "draw.h"

BOOL SetupPixelFormat(HDC hDC, CPalette **ppCPalette, DWORD dwFlags, int bpp)
{
  PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR),	// size of pfd structure
    1,                  // version number
    dwFlags,		 
    PFD_TYPE_RGBA,			// use RGBA
    bpp,				// depth of color buffer : 24-bit
    0, 0, 0, 0, 0, 0,		// specify color bits (ignore)
    0,				// no alpha plane
    0,				// specify alpha bits (ignore)
    0,				// no accumulation buffer
    0, 0, 0, 0,			// specify accumulation bits (ignore)

    16,				// z-buffer depth
    0,				// no stencil buffer
    0,				// no aux buffer
    PFD_MAIN_PLANE,			// main plane only
    0,				// reserved
    0, 0, 0				// layer masks ignored
  };
  int	pixelformat;

  if ( (pixelformat =	ChoosePixelFormat(hDC, &pfd)) == 0 ) {
    TRACE("ChoosePixelFormat failed : %d\n", pfd.cColorBits);
    return FALSE;
  }

  DescribePixelFormat(hDC, pixelformat, 
		      sizeof(PIXELFORMATDESCRIPTOR), &pfd);

  if (!SetPixelFormat(hDC, pixelformat, &pfd)) {
    TRACE("SetPixelFormat failed: %d , %d, %d\n", 
	  pixelformat, pfd.cColorBits, bpp);
    return FALSE;
  }

  if (pfd.dwFlags & PFD_NEED_PALETTE) {
    TRACE("Yes, Palette Required\n"); 
    if (!CreateRGBPalette(hDC, ppCPalette)) {
      *ppCPalette = NULL;
      TRACE("CreateRGBPalette Error\n"); 
    }
  } else {	
    *ppCPalette = NULL;
    TRACE("No Palette Required\n"); 
  }

  return TRUE;
}

///////////////////////////////////////////////////
// BMP File Utility Function

void setupBmpHeader(BITMAPINFOHEADER *pbmih, int sx, int sy, int bpp)
{
  pbmih->biSize = sizeof(BITMAPINFOHEADER);
  pbmih->biWidth = sx;
  pbmih->biHeight = sy;
  pbmih->biPlanes = 1;
  pbmih->biBitCount = bpp;
  pbmih->biCompression = BI_RGB;
  pbmih->biSizeImage = sx * sy * (bpp/8);
  pbmih->biXPelsPerMeter = 2925;
  pbmih->biYPelsPerMeter = 2925;
  pbmih->biClrUsed = 0;
  pbmih->biClrImportant = 0;
}

///////////////////////////////////////////////////
// Palette Utility Functions

static unsigned char threeto8[8] = {
  0, 0111>>1, 0222>>1, 0333>>1, 0444>>1, 0555>>1, 0666>>1, 0377
};
static unsigned char twoto8[4] = {
  0, 0x55, 0xaa, 0xff
};
static unsigned char oneto8[2] = {
  0, 255
};
static int defaultOverride[13] = {
  0, 3, 24, 27, 64, 67, 88, 173, 181, 236, 247, 164, 91
};

static PALETTEENTRY defaultPalEntry[20] = {
  { 0,   0,   0,    0 },
  { 0x80,0,   0,    0 },
  { 0,   0x80,0,    0 },
  { 0x80,0x80,0,    0 },
  { 0,   0,   0x80, 0 },
  { 0x80,0,   0x80, 0 },
  { 0,   0x80,0x80, 0 },
  { 0xC0,0xC0,0xC0, 0 },

  { 192, 220, 192,  0 },
  { 166, 202, 240,  0 },
  { 255, 251, 240,  0 },
  { 160, 160, 164,  0 },

  { 0x80,0x80,0x80, 0 },
  { 0xFF,0,   0,    0 },
  { 0,   0xFF,0,    0 },
  { 0xFF,0xFF,0,    0 },
  { 0,   0,   0xFF, 0 },
  { 0xFF,0,   0xFF, 0 },
  { 0,   0xFF,0xFF, 0 },
  { 0xFF,0xFF,0xFF, 0 }
};

static unsigned char
ComponentFromIndex(int i, UINT nbits, UINT shift)
{
  unsigned char val;

  val = (unsigned char) (i >> shift);
  switch (nbits) {

  case 1:
    val &= 0x1;
    return oneto8[val];

  case 2:
    val &= 0x3;
    return twoto8[val];

  case 3:
    val &= 0x7;
    return threeto8[val];

  default:
    return 0;
  }
}

BOOL CreateRGBPalette(HDC hDC, CPalette **ppCPalette)
{
  //	HPALETTE ghPalette;

  PIXELFORMATDESCRIPTOR pfd;
  LOGPALETTE *pPal;
  int n, i;

  n = GetPixelFormat(hDC);
  DescribePixelFormat(hDC, n, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

  if (!(pfd.dwFlags & PFD_NEED_PALETTE)) return FALSE;
 
  n = 1 << pfd.cColorBits;
  pPal = (PLOGPALETTE)LocalAlloc(LMEM_FIXED, sizeof(LOGPALETTE) +
				 n * sizeof(PALETTEENTRY));
  pPal->palVersion = 0x300;
  pPal->palNumEntries = n;
  for (i=0; i<n; i++) {
    pPal->palPalEntry[i].peRed =
      ComponentFromIndex(i, pfd.cRedBits, pfd.cRedShift);
    pPal->palPalEntry[i].peGreen =
      ComponentFromIndex(i, pfd.cGreenBits, pfd.cGreenShift);
    pPal->palPalEntry[i].peBlue =
      ComponentFromIndex(i, pfd.cBlueBits, pfd.cBlueShift);
    pPal->palPalEntry[i].peFlags = 0;
  }

  /* fix up the palette to include the default GDI palette */
  if ((pfd.cColorBits == 8)                           &&
      (pfd.cRedBits   == 3) && (pfd.cRedShift   == 0) &&
      (pfd.cGreenBits == 3) && (pfd.cGreenShift == 3) &&
      (pfd.cBlueBits  == 2) && (pfd.cBlueShift  == 6)
      ) {
    for (i = 1 ; i <= 12 ; i++)
      pPal->palPalEntry[defaultOverride[i]] = defaultPalEntry[i];
  }

  if (*ppCPalette) delete *ppCPalette;
  *ppCPalette = new CPalette;
  BOOL result = (*ppCPalette)->CreatePalette(pPal);

  LocalFree(pPal);
  return result;
}

void BmpSave( char *bmpFileName, ScreenAtr *screen ) 
{
  unsigned char *oglImage;
  CFile *pbmpFile;
  CFileException fileException;

  pbmpFile = new CFile;
  if (!pbmpFile->Open(bmpFileName , 
		      CFile::modeCreate | CFile::modeWrite ) ) {
    TRACE( "Unable to open file : %s\n, error = %u\n",
	   bmpFileName ,fileException.m_cause );
  }

  //	TRACE("cx, cy = %d, %d\n", screen->width, screen->height);
  int dwSizeImage = screen->width * screen->height * 4;
  oglImage = (unsigned char *)malloc(dwSizeImage);

  // read RGB data from OpenGL RGB plane
  oglGetImage(screen->width, screen->height, oglImage);

  // set up BMP header
  BITMAPFILEHEADER bmfh;
  BITMAPINFOHEADER bmih;
  bmfh.bfType = 0x4d42;  // 'BM'
  int nSize =  sizeof(BITMAPINFOHEADER) +  dwSizeImage;
  bmfh.bfSize = nSize + sizeof(BITMAPFILEHEADER);
    
  ::setupBmpHeader(&bmih, screen->width, screen->height, 32);

  bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
  bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	
  // Writing the RGBA image
  TRY 
    {
      pbmpFile->Write((LPVOID) &bmfh, sizeof(BITMAPFILEHEADER));
      pbmpFile->Write((LPVOID) &bmih, sizeof(BITMAPINFOHEADER));
      pbmpFile->Write((LPVOID) oglImage, dwSizeImage);
    }
  CATCH (CException, e) 
    {
      AfxMessageBox("write error");
      return;
    }
  END_CATCH

    pbmpFile->Close();
  delete pbmpFile;
  free(oglImage);

}

void oglGetImage(int sx, int sy, unsigned char *pixels)
{
  ::glFinish();				/* Finish all OpenGL commands */
  ::glPixelStorei(GL_PACK_ALIGNMENT, 4);	/* Force 4-byte alignment */
  ::glPixelStorei(GL_PACK_ROW_LENGTH, 0);
  ::glPixelStorei(GL_PACK_SKIP_ROWS, 0);
  ::glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

#ifdef GL_VERSION_1_1
  ::glReadPixels(0, 0, sx, sy, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels);
#else
  ::glReadPixels(0, 0, sx, sy, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  unsigned char *buf = pixels;
  unsigned char *bufmax = pixels + sx*sy*4;
  unsigned char r;
  for (; buf < bufmax; buf += 4) {
    r = buf[0];
    buf[0]   = buf[2];
    buf[2] = r;
  }

#endif
} 

void DibSave( char *bmpFileName, ScreenAtr *screen )
{
  LPVOID lpBits;

  CDC *pDC = screen->wnd->GetDC();
  ASSERT(pDC->m_hDC);

  // Creating Compatible Memory Device Context
  CDC *pMemDC = new CDC;
  if (!pMemDC->CreateCompatibleDC(pDC)) {
    AfxMessageBox("CompatibleDC Error");
    return;
  }

  // Preparing bitmap header for DIB section
  BITMAPINFO bi;
  int bpp, sx, sy;
  ZeroMemory(&bi, sizeof(BITMAPINFO));
  // Making a DIB image which is half size of GL window and full color(24 bpp)
  //   sx = m_size.cx/2;
  //   sy = m_size.cy/2;
  sx = screen->width;
  sy = screen->height;
  bpp = 24;
  int dwSizeImage = sx * sy * bpp / 8; // 8 means 8 bit/byte
  setupBmpHeader(&(bi.bmiHeader), sx, sy, bpp);

  // Creating a DIB surface
  HBITMAP hBm, hBmOld;
  hBm = CreateDIBSection(pDC->GetSafeHdc(), &bi, DIB_RGB_COLORS, 
			 (void **)&lpBits, NULL, (DWORD)0);
  if (!hBm) {
    AfxMessageBox("CreateDIBSection Error");
    return;
  }

  // Selecting the DIB Surface
  hBmOld = (HBITMAP)::SelectObject(pMemDC->GetSafeHdc(), hBm);
  if (!hBmOld) {
    AfxMessageBox("Select Object Error");
    return;
  }

  // Setting up a Pixel format for the DIB surface and Making a palette
  CPalette* pMemCPal;	//Palette
  if (!SetupPixelFormat(pMemDC->m_hDC, &pMemCPal,
			PFD_DRAW_TO_BITMAP |
			PFD_SUPPORT_OPENGL |
			PFD_SUPPORT_GDI,
			bpp) ) {
    AfxMessageBox("SetPixelFormatError");
  }

  // Realize the palette, if required
  if (pMemCPal) {
    CPalette* pOldPal = pMemDC->SelectPalette(pMemCPal, FALSE);
    UINT u = pMemDC->RealizePalette();
    //if (u != 0) InvalidateRect(NULL, TRUE);
    if (u != 0) {
      screen->wnd->RedrawWindow();
    }
  }

  // Creating a OpenGL context
  HGLRC hmemrc = wglCreateContext(pMemDC->GetSafeHdc());
  if (!hmemrc) {
    AfxMessageBox("OpenGL Context Error");
  }

  // Setting up the current OpenGL context
  if (!wglMakeCurrent(pMemDC->GetSafeHdc(), hmemrc)) {
    AfxMessageBox("Error");
    TRACE("wglMakeCurrent Failed %d\r\n", GetLastError());;
    return;
  }

  // OpenGL Rendering
  init_gl3d();
  clear_gl3d( screen );
  ::glPushMatrix();
  view_init( screen );

  //::glPushMatrix();
  draws3d( screen );
  ::glPopMatrix();

  ::glFinish();

  if ( ::SwapBuffers( pDC->GetSafeHdc() ) == FALSE ) {
    //  SetError(7);
  }

  // Preparing BMP file header information
  BITMAPFILEHEADER bmfh;
  bmfh.bfType = 0x4d42;  // 'BM'
  int nSize =  sizeof(BITMAPINFOHEADER) +  dwSizeImage;
  bmfh.bfSize = nSize + sizeof(BITMAPFILEHEADER);
  bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
  bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);	

  //char *bmpFileName = "test2.bmp"; // sorry for hard coding 
  CFile *pbmpFile;
  CFileException fileException;

  pbmpFile = new CFile;
  if (!pbmpFile->Open( bmpFileName,
		       CFile::modeCreate | CFile::modeWrite ) ) {
    TRACE( "Unable to open file: %s\n, error = %u\n",
	   bmpFileName ,fileException.m_cause);
  }

  // Wrinting the DIB image
  TRY {
    pbmpFile->Write( (LPVOID) &bmfh, sizeof(BITMAPFILEHEADER) );
    pbmpFile->Write( (LPVOID) &(bi.bmiHeader), sizeof(BITMAPINFOHEADER) );
    pbmpFile->Write( (LPVOID) lpBits, dwSizeImage );
  }
  CATCH (CException, e) {
    AfxMessageBox("write error");
    return;
  }
  END_CATCH

    // Cleaning
    pbmpFile->Close();
  delete pbmpFile;
  
  wglMakeCurrent(NULL, NULL);
  wglDeleteContext( hmemrc );

  hBm = (HBITMAP)::SelectObject(pMemDC->GetSafeHdc(), hBmOld);
  DeleteObject(hBm);

  if (pMemCPal != 0) delete pMemCPal;
  delete pMemDC;
  screen->wnd->ReleaseDC( pDC );
}

