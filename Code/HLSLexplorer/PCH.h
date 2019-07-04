#ifndef __PCH_H__
#define __PCH_H__

#pragma once

#ifdef _DEBUG
# define _CRTDBG_MAP_ALLOC
#endif

// wxWidgets
#include <wx/wxprec.h>

#ifdef GetHwnd
# undef GetHwnd
#endif

enum class ETextureType
{
	ETexType_Invalid,

	ETexType_1D,
	ETexType_1DArray,
	ETexType_2D,
	ETexType_2DArray,
	ETexType_3D,
	ETexType_Cube,
	ETexType_CubeArray
};

#ifndef SAFE_RELEASE
# define SAFE_RELEASE(p)	{ if(p) { (p)->Release(); (p)=nullptr; } }
#endif

#endif