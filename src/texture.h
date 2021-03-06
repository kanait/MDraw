//
// texture.h
//
// Copyright (c) 1999-2000 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

/*
 * read_texture() - read in an image file in SGI 'libimage' format
 * 	currently its very simple minded and converts all images
 *      to RGBA8 regardless of the input format and returns the
 *	original number of components in the appropriate parameter.
 *    
 *     
 *	the components are converted as follows
 *		L    -> LLL 1.0
 *		LA   -> LLL A
 *		RGB  -> RGB 1.0
 *		RGBA -> RGB A
 *
 */

#ifndef _TEXTURE_H
#define _TEXTURE_H

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned *read_texture( char *, int *, int *, int * );

#ifdef __cplusplus
}
#endif

#endif // _TEXTURE_H

