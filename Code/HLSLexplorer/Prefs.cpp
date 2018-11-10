//////////////////////////////////////////////////////////////////////////////
// File:        contrib/samples/stc/prefs.cpp
// Purpose:     STC test Preferences initialization
// Maintainer:  Wyo
// Created:     2003-09-01
// Copyright:   (c) wxGuide
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// headers
//----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx/wx.h".

#include "PCH.h"

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all 'standard' wxWidgets headers)
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

//! wxWidgets headers

//! wxWidgets/contrib headers

//! application headers
//#include "defsext.h"     // Additional definitions
#include "prefs.h"       // Preferences


//============================================================================
// declarations
//============================================================================

//----------------------------------------------------------------------------
//! language types
const CommonInfo g_CommonPrefs = {
	// editor functionality prefs
	true,  // syntaxEnable
	true,  // foldEnable
	true,  // indentEnable
		   // display defaults prefs
		   false, // overTypeInitial
		   false, // readOnlyInitial
		   false,  // wrapModeInitial
		   false, // displayEOLEnable
		   false, // IndentGuideEnable
		   true,  // lineNumberEnable
		   false, // longLineOnEnable
		   false, // whiteSpaceEnable
};

//----------------------------------------------------------------------------
// keywordlists
// C++
const char* HLSLWordsList =
"AppendStructuredBuffer bool break Buffer ByteAddressBuffer case cbuffer centroid "
"column_major compile compile_fragment const ConsumeStructuredBuffer continue default discard do double "
"else false float float1 float1x2 float1x3 float1x4 float2 float2x1 float2x2 float2x3 float2x4 float3 float3x1 float3x2 "
"float3x3 float3x4 float4 float4x1 float4x2 float4x3 float4x4 for groupshared half if in inline inout InputPatch "
"int lineadj linear LineStream linke matrix min10float min12int min16float min16int min16uint nointerpolation noperspective out "
"OutputPatch packoffset point PointStream precise register return row_major RWBuffer RWByteAddressBuffer RWStructuredBuffer "
"RWTexture1D RWTexture1DArray RWTexture2D RWTexture2DArray RWTexture3D sampler SamplerComparisonState SamplerState shared snorm static "
"struct StructuredBuffertbuffer switch Texture1D Texture1DArray Texture2D Texture2DArray Texture2DMS Texture2DMSArray Texture3D TextureCube "
"TextureCubeArray triangle triangleadj TriangleStream true typedef uint uniform unorm unroll unsigned vertexfragment void volatile while";
const char* CppWordlist2 =
"file";
const char* CppWordlist3 =
"a addindex addtogroup anchor arg attention author b brief bug c "
"class code date def defgroup deprecated dontinclude e em endcode "
"endhtmlonly endif endlatexonly endlink endverbatim enum example "
"exception f$ f[ f] file fn hideinitializer htmlinclude "
"htmlonly if image include ingroup internal invariant interface "
"latexonly li line link mainpage name namespace nosubgrouping note "
"overload p page par param post pre ref relates remarks return "
"retval sa section see showinitializer since skip skipline struct "
"subsection test throw todo typedef union until var verbatim "
"verbinclude version warning weakgroup $ @ \"\" & < > # { }";


// ASM
const char* ASMWordList =
"mov mad add exp log ";



//----------------------------------------------------------------------------
//! languages
const LanguageInfo g_LanguagePrefs[] = {
	// HLSL
	{ "HLSL",
	"*.hlsl;*.fx",
	wxSTC_LEX_CPP,
	{ { mySTC_TYPE_DEFAULT, NULL },
	{ mySTC_TYPE_COMMENT, NULL },
	{ mySTC_TYPE_COMMENT_LINE, NULL },
	{ mySTC_TYPE_COMMENT_DOC, NULL },
	{ mySTC_TYPE_NUMBER, NULL },
	{ mySTC_TYPE_WORD1, HLSLWordsList }, // KEYWORDS
	{ mySTC_TYPE_STRING, NULL },
	{ mySTC_TYPE_CHARACTER, NULL },
	{ mySTC_TYPE_UUID, NULL },
	{ mySTC_TYPE_PREPROCESSOR, NULL },
	{ mySTC_TYPE_OPERATOR, NULL },
	{ mySTC_TYPE_IDENTIFIER, NULL },
	{ mySTC_TYPE_STRING_EOL, NULL },
	{ mySTC_TYPE_DEFAULT, NULL }, // VERBATIM
	{ mySTC_TYPE_REGEX, NULL },
	{ mySTC_TYPE_COMMENT_SPECIAL, NULL }, // DOXY
	{ mySTC_TYPE_WORD2, NULL }, // EXTRA WORDS
	{ mySTC_TYPE_WORD3, NULL }, // DOXY KEYWORDS
	{ mySTC_TYPE_ERROR, NULL }, // KEYWORDS ERROR
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL } },
	mySTC_FOLD_COMMENT | mySTC_FOLD_COMPACT | mySTC_FOLD_PREPROC },
	// ASM
	{ "HLSL",
	"*.hlsl;*.fx",
	wxSTC_LEX_CPP,
	{ { mySTC_TYPE_DEFAULT, NULL },
	{ mySTC_TYPE_COMMENT, NULL },
	{ mySTC_TYPE_COMMENT_LINE, NULL },
	{ mySTC_TYPE_COMMENT_DOC, NULL },
	{ mySTC_TYPE_NUMBER, NULL },
	{ mySTC_TYPE_WORD1, ASMWordList }, // KEYWORDS
	{ mySTC_TYPE_STRING, NULL },
	{ mySTC_TYPE_CHARACTER, NULL },
	{ mySTC_TYPE_UUID, NULL },
	{ mySTC_TYPE_PREPROCESSOR, NULL },
	{ mySTC_TYPE_OPERATOR, NULL },
	{ mySTC_TYPE_IDENTIFIER, NULL },
	{ mySTC_TYPE_STRING_EOL, NULL },
	{ mySTC_TYPE_DEFAULT, NULL }, // VERBATIM
	{ mySTC_TYPE_REGEX, NULL },
	{ mySTC_TYPE_COMMENT_SPECIAL, NULL }, // DOXY
	{ mySTC_TYPE_WORD2, NULL }, // EXTRA WORDS
	{ mySTC_TYPE_WORD3, NULL }, // DOXY KEYWORDS
	{ mySTC_TYPE_ERROR, NULL }, // KEYWORDS ERROR
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL } },
	mySTC_FOLD_COMMENT | mySTC_FOLD_COMPACT | mySTC_FOLD_PREPROC },
	// * (any)
	{ wxTRANSLATE("<default>"),
	"*.*",
	wxSTC_LEX_PROPERTIES,
	{ { mySTC_TYPE_DEFAULT, NULL },
	{ mySTC_TYPE_DEFAULT, NULL },
	{ mySTC_TYPE_DEFAULT, NULL },
	{ mySTC_TYPE_DEFAULT, NULL },
	{ mySTC_TYPE_DEFAULT, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL },
	{ -1, NULL } },
	0 },
};

const int g_LanguagePrefsSize = WXSIZEOF(g_LanguagePrefs);

//----------------------------------------------------------------------------
//! style types
const StyleInfo g_StylePrefs[] = {
	// mySTC_TYPE_DEFAULT
	{ wxT("Default"),
	wxT("BLACK"), wxT("WHITE"),
	wxT("CONSOLAS"), 10, 0, 0 },

	// mySTC_TYPE_WORD1
	{ wxT("Keyword1"),
	wxT("BLUE"), wxT("WHITE"),
	wxT(""), 10, mySTC_STYLE_BOLD, 0 },

	// mySTC_TYPE_WORD2
	{ wxT("Keyword2"),
	wxT("MIDNIGHT BLUE"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_WORD3
	{ wxT("Keyword3"),
	wxT("CORNFLOWER BLUE"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_WORD4
	{ wxT("Keyword4"),
	wxT("CYAN"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_WORD5
	{ wxT("Keyword5"),
	wxT("DARK GREY"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_WORD6
	{ wxT("Keyword6"),
	wxT("GREY"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_COMMENT
	{ wxT("Comment"),
	wxT("FOREST GREEN"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_COMMENT_DOC
	{ wxT("Comment (Doc)"),
	wxT("FOREST GREEN"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_COMMENT_LINE
	{ wxT("Comment line"),
	wxT("FOREST GREEN"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_COMMENT_SPECIAL
	{ wxT("Special comment"),
	wxT("FOREST GREEN"), wxT("WHITE"),
	wxT(""), 10, mySTC_STYLE_ITALIC, 0 },

	// mySTC_TYPE_CHARACTER
	{ wxT("Character"),
	wxT("KHAKI"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_CHARACTER_EOL
	{ wxT("Character (EOL)"),
	wxT("KHAKI"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_STRING
	{ wxT("String"),
	wxT("BROWN"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_STRING_EOL
	{ wxT("String (EOL)"),
	wxT("BROWN"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_DELIMITER
	{ wxT("Delimiter"),
	wxT("ORANGE"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_PUNCTUATION
	{ wxT("Punctuation"),
	wxT("ORANGE"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_OPERATOR
	{ wxT("Operator"),
	wxT("BLACK"), wxT("WHITE"),
	wxT(""), 10, mySTC_STYLE_BOLD, 0 },

	// mySTC_TYPE_BRACE
	{ wxT("Label"),
	wxT("VIOLET"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_COMMAND
	{ wxT("Command"),
	wxT("BLUE"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_IDENTIFIER
	{ wxT("Identifier"),
	wxT("BLACK"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_LABEL
	{ wxT("Label"),
	wxT("VIOLET"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_NUMBER
	{ wxT("Number"),
	wxT("SIENNA"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_PARAMETER
	{ wxT("Parameter"),
	wxT("VIOLET"), wxT("WHITE"),
	wxT(""), 10, mySTC_STYLE_ITALIC, 0 },

	// mySTC_TYPE_REGEX
	{ wxT("Regular expression"),
	wxT("ORCHID"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_UUID
	{ wxT("UUID"),
	wxT("ORCHID"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_VALUE
	{ wxT("Value"),
	wxT("ORCHID"), wxT("WHITE"),
	wxT(""), 10, mySTC_STYLE_ITALIC, 0 },

	// mySTC_TYPE_PREPROCESSOR
	{ wxT("Preprocessor"),
	wxT("GREY"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_SCRIPT
	{ wxT("Script"),
	wxT("DARK GREY"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_ERROR
	{ wxT("Error"),
	wxT("RED"), wxT("WHITE"),
	wxT(""), 10, 0, 0 },

	// mySTC_TYPE_UNDEFINED
	{ wxT("Undefined"),
	wxT("ORANGE"), wxT("WHITE"),
	wxT(""), 10, 0, 0 }

};

const int g_StylePrefsSize = WXSIZEOF(g_StylePrefs);
