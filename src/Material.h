//
// Material.h
//
// Copyright (c) 1998-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Include the OpenGL headers
#include "gl\gl.h"

// Material class

class CMaterial {

private:

	float ambient[4];
	float diffuse[4];
	float emission[4];
	float specular[4];
	float shininess[1];
	
public:

	CMaterial();
	~CMaterial();
	void Binding();
};

