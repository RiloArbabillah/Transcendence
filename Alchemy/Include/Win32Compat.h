//	Win32Compat.h
//
//	Windows compatibility layer for macOS
//	Provides Windows types and functions using POSIX equivalents

#pragma once

#include <cstdint>
#include <sys/time.h>
#include <mach/mach_time.h>

#ifndef _WIN32

#ifndef __forceinline
#define __forceinline inline
#endif

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef RGB
#define RGB(r, g, b) ((DWORD)(((BYTE)(r) | ((WORD)((BYTE)(g)) << 8) | (((DWORD)(BYTE)(b)) << 16)))
#endif

#ifndef GetRValue
#define GetRValue(rgb) ((BYTE)(rgb))
#endif

#ifndef GetGValue
#define GetGValue(rgb) ((BYTE)(((WORD)(rgb)) >> 8))
#endif

#ifndef GetBValue
#define GetBValue(rgb) ((BYTE)((rgb) >> 16))
#endif

#ifndef GetTickCount
inline DWORD GetTickCount(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (DWORD)((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}
#endif

#ifndef MAKELONG
#define MAKELONG(a, b) ((DWORD)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#endif

#ifndef RectWidth
#define RectWidth(rc) ((rc).right - (rc).left)
#endif

#ifndef RectHeight
#define RectHeight(rc) ((rc).bottom - (rc).top)
#endif

#ifndef SetRect
inline BOOL SetRect(RECT *prc, int left, int top, int right, int bottom)
{
    prc->left = left;
    prc->top = top;
    prc->right = right;
    prc->bottom = bottom;
    return TRUE;
}
#endif

#ifndef SetRectEmpty
inline BOOL SetRectEmpty(RECT *prc)
{
    prc->left = prc->top = prc->right = prc->bottom = 0;
    return TRUE;
}
#endif

#ifndef CopyRect
inline BOOL CopyRect(RECT *prcDest, const RECT *prcSrc)
{
    prcDest->left = prcSrc->left;
    prcDest->top = prcSrc->top;
    prcDest->right = prcSrc->right;
    prcDest->bottom = prcSrc->bottom;
    return TRUE;
}
#endif

#ifndef InflateRect
inline BOOL InflateRect(RECT *prc, int dx, int dy)
{
    prc->left -= dx;
    prc->top -= dy;
    prc->right += dx;
    prc->bottom += dy;
    return TRUE;
}
#endif

#ifndef OffsetRect
inline BOOL OffsetRect(RECT *prc, int dx, int dy)
{
    prc->left += dx;
    prc->top += dy;
    prc->right += dx;
    prc->bottom += dy;
    return TRUE;
}
#endif

#ifndef UnionRect
inline BOOL UnionRect(RECT *prcDest, const RECT *prcSrc1, const RECT *prcSrc2)
{
    prcDest->left = min(prcSrc1->left, prcSrc2->left);
    prcDest->top = min(prcSrc1->top, prcSrc2->top);
    prcDest->right = max(prcSrc1->right, prcSrc2->right);
    prcDest->bottom = max(prcSrc1->bottom, prcSrc2->bottom);
    return TRUE;
}
#endif

#ifndef IntersectRect
inline BOOL IntersectRect(RECT *prcDest, const RECT *prcSrc1, const RECT *prcSrc2)
{
    prcDest->left = max(prcSrc1->left, prcSrc2->left);
    prcDest->top = max(prcSrc1->top, prcSrc2->top);
    prcDest->right = min(prcSrc1->right, prcSrc2->right);
    prcDest->bottom = min(prcSrc1->bottom, prcSrc2->bottom);
    return (prcDest->right >= prcDest->left && prcDest->bottom >= prcDest->top) ? TRUE : FALSE;
}
#endif

#ifndef IsRectEmpty
inline BOOL IsRectEmpty(const RECT *prc)
{
    return (prc->right <= prc->left || prc->bottom <= prc->top) ? TRUE : FALSE;
}
#endif

#ifndef PtInRect
inline BOOL PtInRect(const RECT *prc, POINT pt)
{
    return (pt.x >= prc->left && pt.x < prc->right && pt.y >= prc->top && pt.y < prc->bottom) ? TRUE : FALSE;
}
#endif

#ifndef GetClientRect
inline BOOL GetClientRect(HWND hWnd, RECT *prc)
{
    prc->left = prc->top = 0;
    prc->right = 800;
    prc->bottom = 600;
    return TRUE;
}
#endif

#ifndef MapWindowPoints
inline int MapWindowPoints(HWND hWndFrom, HWND hWndTo, POINT *lpPoints, UINT cPoints)
{
    return 0;
}
#endif

struct POINT
{
    LONG x;
    LONG y;
};

struct SIZE
{
    LONG cx;
    LONG cy;
};

typedef void* HDC;
typedef void* HWND;
typedef void* HBITMAP;
typedef void* HFONT;
typedef void* HPalette;
typedef void* HRGN;
typedef void* HINSTANCE;
typedef void* HMODULE;
typedef void* HANDLE;
typedef void* HGLOBAL;
typedef void* HRSRC;
typedef HANDLE HACCEL;
typedef HANDLE HGLOBAL;
typedef HANDLE HLOCAL;
typedef HANDLE HGDIOBJ;
typedef int HFILE;
typedef long LPARAM;
typedef unsigned int WPARAM;
typedef long LRESULT;
typedef int INT;
typedef unsigned int UINT;
typedef unsigned int UINT_PTR;
typedef long LONG;
typedef unsigned long ULONG;
typedef short SHORT;
typedef unsigned short USHORT;
typedef DWORDLONG ULONGLONG;
typedef ULONGLONG DWORDLONG;
typedef LONGLONG ULONGLONG;
typedef LONGLONG LONGLONG;
typedef LONG_PTR SPONG_PTR;
typedef LONG_PTR LPARAM;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LRESULT;

#ifndef D3DFMT_UNKNOWN
#define D3DFMT_UNKNOWN 0
#endif

#ifndef D3DFMT_R8G8B8
#define D3DFMT_R8G8B8 20
#endif

#ifndef D3DFMT_A8R8G8B8
#define D3DFMT_A8R8G8B8 21
#endif

#ifndef D3DFMT_X8R8G8B8
#define D3DFMT_X8R8G8B8 22
#endif

#ifndef D3DFMT_R5G6B5
#define D3DFMT_R5G6B5 23
#endif

#ifndef D3DFMT_X1R5G5B5
#define D3DFMT_X1R5G5B5 24
#endif

#ifndef D3DFMT_A1R5G5B5
#define D3DFMT_A1R5G5B5 25
#endif

#ifndef D3DFMT_A4R4G4B4
#define D3DFMT_A4R4G4B4 26
#endif

#ifndef D3DFMT_R3G3B2
#define D3DFMT_R3G3B2 27
#endif

#ifndef D3DFMT_A8
#define D3DFMT_A8 28
#endif

#ifndef D3DPOOL_DEFAULT
#define D3DPOOL_DEFAULT 0
#endif

#ifndef D3DPOOL_MANAGED
#define D3DPOOL_MANAGED 1
#endif

#ifndef D3DPOOL_SYSTEMMEM
#define D3DPOOL_SYSTEMMEM 2
#endif

#ifndef D3DPOOL_SCRATCH
#define D3DPOOL_SCRATCH 3
#endif

#ifndef D3DUSAGE_RENDERTARGET
#define D3DUSAGE_RENDERTARGET 0x00000001L
#endif

#ifndef D3DUSAGE_DEPTHSTENCIL
#define D3DUSAGE_DEPTHSTENCIL 0x00000002L
#endif

#ifndef D3DUSAGE_SOFTWAREPROCESSING
#define D3DUSAGE_SOFTWAREPROCESSING 0x00000010L
#endif

#ifndef D3DCLRDOC_WRITEOUT
#define D3DCLRDOC_WRITEOUT 0x00000001L
#endif

#ifndef D3DCS_ALL
#define D3DCS_ALL 0x0000007FL
#endif

#ifndef D3DCS_LEFT
#define D3DCS_LEFT 0x00000001L
#endif

#ifndef D3DCS_RIGHT
#define D3DCS_RIGHT 0x00000002L
#endif

#ifndef D3DCS_TOP
#define D3DCS_TOP 0x00000004L
#endif

#ifndef D3DCS_BOTTOM
#define D3DCS_BOTTOM 0x00000008L
#endif

#ifndef D3DCS_FRONT
#define D3DCS_FRONT 0x00000010L
#endif

#ifndef D3DCS_BACK
#define D3DCS_BACK 0x00000020L
#endif

#ifndef D3DCS_PLANE0
#define D3DCS_PLANE0 0x00000040L
#endif

typedef DWORD D3DFORMAT;
typedef DWORD D3DPOOL;
typedef DWORD D3DUSAGE;
typedef DWORD D3DCOLOR;
typedef DWORD D3DCLEAR;
typedef DWORD D3DZBUFFERTYPE;
typedef DWORD D3DBLEND;
typedef DWORD D3DBLENDOP;
typedef DWORD D3DTEXTUREFILTERTYPE;
typedef DWORD D3DSHADEMODE;
typedef DWORD D3DFILLMODE;
typedef DWORD D3DCULL;
typedef DWORD D3DPRIMITIVETYPE;
typedef DWORD D3DOPCODE;
typedef DWORD D3DSTATEBLOCKTYPE;
typedef DWORD D3DMATRIXSTATETYPE;
typedef DWORD D3DRENDERSTATETYPE;
typedef DWORD D3DTRANSFORMSTATETYPE;
typedef DWORD D3DVERTEXBLENDSTAGE;
typedef DWORD D3DTEXTURESTAGESTATETYPE;
typedef DWORD D3DSAMPLERSTATETYPE;

#define D3DCOLOR_ARGB(a,r,g,b) ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#define D3DCOLOR_RGBA(r,g,b,a) D3DCOLOR_ARGB(a,r,g,b)
#define D3DCOLOR_XRGB(r,g,b) D3DCOLOR_ARGB(255,r,g,b)

#define D3DTA_DIFFUSE 0
#define D3DTA_CURRENT 1
#define D3DTA_TEXTURE 2
#define D3DTA_TFACTOR 3
#define D3DTA_SPECULAR 4
#define D3DTA_COMPLEMENT 8
#define D3DTA_ALPHAREPLICATE 16

#define D3DTOP_DISABLE 1
#define D3DTOP_SELECTARG1 2
#define D3DTOP_SELECTARG2 3
#define D3DTOP_MODULATE 4
#define D3DTOP_MODULATE2 5
#define D3DTOP_ADD 6
#define D3DTOP_ADDSIGNED 7
#define D3DTOP_REVSUBTRACT 8
#define D3DTOP_SUBTRACT 9
#define D3DTOP_ADDSMOOTH 10
#define D3DTOP_BLENDDIFFUSEALPHA 11
#define D3DTOP_BLENDTEXTUREALPHA 12
#define D3DTOP_BLENDFACTORALPHA 13
#define D3DTOP_MODULATEALPHA_ADDCOLOR 14
#define D3DTOP_MODULATECOLOR_ADDALPHA 15
#define D3DTOP_MODULATEINVALPHA_ADDCOLOR 16
#define D3DTOP_MODULATEINVCOLOR_ADDALPHA 17
#define D3DTOP_MULTIPLYADD 18
#define D3DTOP_LERP 19

#define D3DTTFF_DISABLE 0
#define D3DTTFF_COUNT1 1
#define D3DTTFF_COUNT2 2
#define D3DTTFF_COUNT3 3
#define D3DTTFF_COUNT4 4
#define D3DTTFF_PROJECTED 256
#define D3DTTFF_SPECULAR 512
#define D3DTTFF_FORCE_DWORD 0x7fffffff

#define D3DTA_TFACTOR 3

#define D3DRENDERSTATE_WRONG -1

struct D3DRECT
{
    DWORD x1, y1, x2, y2;
};

struct D3DCLIPSTATUS
{
    DWORD dwClipUnion;
    DWORD dwClipIntersection;
};

#define D3DFVF_XYZ 0x002
#define D3DFVF_XYZRHW 0x004
#define D3DFVF_NORMAL 0x010
#define D3DFVF_DIFFUSE 0x040
#define D3DFVF_SPECULAR 0x080
#define D3DFVF_TEX1 0x100
#define D3DFVF_TEX2 0x200
#define D3DFVF_TEX3 0x400
#define D3DFVF_TEX4 0x800

struct D3DVERTEX
{
    float x, y, z;
    DWORD color;
};

struct D3DLVERTEX
{
    float x, y, z;
    DWORD color;
    DWORD specular;
};

struct D3DTLVERTEX
{
    float x, y, z, rhw;
    DWORD color;
    DWORD specular;
    float tu, tv;
};

#define D3DCLEAR_TARGET 0x00000001l
#define D3DCLEAR_ZBUFFER 0x00000002l
#define D3DCLEAR_STENCIL 0x00000004l

#define D3DZB_FALSE 0
#define D3DZB_TRUE 1
#define D3DZB_USEW 2

#define D3DBLEND_ZERO 1
#define D3DBLEND_ONE 2
#define D3DBLEND_SRCCOLOR 3
#define D3DBLEND_INVSRCCOLOR 4
#define D3DBLEND_SRCALPHA 5
#define D3DBLEND_INVSRCALPHA 6
#define D3DBLEND_DESTALPHA 7
#define D3DBLEND_INVDESTALPHA 8
#define D3DBLEND_DESTCOLOR 9
#define D3DBLEND_INVDESTCOLOR 10
#define D3DBLEND_SRCALPHASAT 11
#define D3DBLEND_BOTHSRCALPHA 12
#define D3DBLEND_BOTHINVSRCALPHA 13

#define D3DBLENDOP_ADD 1
#define D3DBLENDOP_SUBTRACT 2
#define D3DBLENDOP_REVSUBTRACT 3
#define D3DBLENDOP_MIN 4
#define D3DBLENDOP_MAX 5

#define D3DOP_POINT 1
#define D3DOP_LINE 2
#define D3DOP_TRIANGLE 3
#define D3DOP_TEXBLT 4
#define D3DOP_STATE 5
#define D3DOP_END 6

#define D3DPT_POINTLIST 1
#define D3DPT_LINELIST 2
#define D3DPT_LINESTRIP 3
#define D3DPT_TRIANGLELIST 4
#define D3DPT_TRIANGLESTRIP 5
#define D3DPT_TRIANGLEFAN 6

#define D3DRS_ZENABLE 7
#define D3DRS_ZFUNC 8
#define D3DRS_ZWRITEENABLE 14
#define D3DRS_ALPHABLENDENABLE 19
#define D3DRS_SRCBLEND 20
#define D3DRS_DESTBLEND 21
#define D3DRS_BLENDOP 27
#define D3DRS_SHADEMODE 9
#define D3DRS_FILLMODE 8
#define D3DRS_CULLMODE 22
#define D3DRS_CLIPPING 16
#define D3DRS_LIGHTING 137
#define D3DRS_COLORVERTEX 176
#define D3DRS_DITHERENABLE 174
#define D3DRS_SPECULARENABLE 183
#define D3DRS_STENCILENABLE 185
#define D3DRS_FOGSTART 186
#define D3DRS_FOGEND 187
#define D3DRS_FOGDENSITY 188
#define D3DRS_RANGEFOGENABLE 198
#define D3DRS_AMBIENT 139
#define D3DRS_FOGCOLOR 194
#define D3DRS_POINTSIZE 257
#define D3DRS_POINTSIZE_MIN 259
#define D3DRS_POINTSPRITEENABLE 260

#define D3DCULL_NONE 1
#define D3DCULL_CW 2
#define D3DCULL_CCW 3

#define D3DSHADE_FLAT 1
#define D3DSHADE_GOURAUD 2
#define D3DSHADE_PHONG 3

#define D3DFILL_POINT 1
#define D3DFILL_WIREFRAME 2
#define D3DFILL_SOLID 3

#define D3DTEXF_NONE 0
#define D3DTEXF_POINT 1
#define D3DTEXF_LINEAR 2
#define D3DTEXF_ANISOTROPIC 3
#define D3DTEXF_PYRAMIDALQUAD 6
#define D3DTEXF_GAUSSIANQUAD 7

#define D3DTSS_TCI_CAMERASPACENORMAL 0x00010000
#define D3DTSS_TCI_CAMERASPACEPOSITION 0x00020000
#define D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR 0x00030000
#define D3DTSS_TCI_SPHEREMAP 0x00040000

#define D3DTSS_COLOROP 1
#define D3DTSS_COLORARG1 2
#define D3DTSS_COLORARG2 3
#define D3DTSS_ALPHAOP 4
#define D3DTSS_ALPHAARG1 5
#define D3DTSS_ALPHAARG2 6
#define D3DTSS_BUMPENVMAT00 7
#define D3DTSS_BUMPENVMAT01 8
#define D3DTSS_BUMPENVMAT10 9
#define D3DTSS_BUMPENVMAT11 10
#define D3DTSS_TEXTURETRANSFORMFLAGS 13
#define D3DTSS_ADDRESS 14
#define D3DTSS_ADDRESSU 15
#define D3DTSS_ADDRESSV 16
#define D3DTSS_BORDERCOLOR 17
#define D3DTSS_MAGFILTER 18
#define D3DTSS_MINFILTER 19
#define D3DTSS_MIPFILTER 20
#define D3DTSS_MIPMAPLODBIAS 21
#define D3DTSS_MAXMIPLEVEL 22
#define D3DTSS_MAXANISOTROPY 23
#define D3DTSS_ELEMENTINDEX 26
#define D3DTSS_CONSTANT 32

#define D3DTSS_ADDRESSWRAP 7
#define D3DTSS_ADDRESSMIRROR 8

#define D3DPCMPCAPS_LESSEQUAL 0x00000004L
#define D3DPCMPCAPS_EQUAL 0x00000008L
#define D3DPCMPCAPS_GREATEREQUAL 0x00000010L

#define D3DCOLORWRITEENABLE_RED 1
#define D3DCOLORWRITEENABLE_GREEN 2
#define D3DCOLORWRITEENABLE_BLUE 4
#define D3DCOLORWRITEENABLE_ALPHA 8
#define D3DCOLORWRITEENABLE_ALL (D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_ALPHA)

#define D3DRS_COLORWRITEENABLE 168

#define D3DPMISCCAPS_MASKZ 0x00000002L
#define D3DPMISCCAPS_CULLNONE 0x00000010L
#define D3DPMISCCAPS_CULLCW 0x00000020L
#define D3DPMISCCAPS_CULLCCW 0x00000040L

#define D3DDEVCAPS_EXECUTESYSTEMMEMORY 0x00000010L
#define D3DDEVCAPS_EXECUTEVIDEOMEMORY 0x00000020L
#define D3DDEVCAPS_TLVERTEXSYSTEMMEMORY 0x00000040L
#define D3DDEVCAPS_TLVERTEXVIDEOMEMORY 0x00000080L
#define D3DDEVCAPS_TEXTURESYSTEMMEMORY 0x00000100L
#define D3DDEVCAPS_TEXTUREVIDEOMEMORY 0x00000200L
#define D3DDEVCAPS_DRAWPRIMTLVERTEX 0x00000400L
#define D3DDEVCAPS_DRAWPRIMTLVERTEX 0x00000800L

#define D3DD3DDEVCAPS2_NO_TEX2 0x00001000L
#define D3DDEVCAPS2_STREAMOFFSET 0x00004000L

struct D3DMATRIX
{
    union
    {
        struct
        {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
    };
};

struct D3DVIEWPORT9
{
    DWORD X;
    DWORD Y;
    DWORD Width;
    DWORD Height;
    float MinZ;
    float MaxZ;
};

#define D3DVIEWPORT_DEFAULT -1

struct D3DLINE
{
    float x1, y1, z1;
    float x2, y2, z2;
};

struct D3DRECT
{
    DWORD x1, y1, x2, y2;
};

struct tagPIXELFORMATDESCRIPTOR
{
    WORD  nSize;
    WORD  nVersion;
    DWORD dwFlags;
    BYTE  iPixelType;
    BYTE  cColorBits;
    BYTE  cRedBits;
    BYTE  cRedShift;
    BYTE  cGreenBits;
    BYTE  cGreenShift;
    BYTE  cBlueBits;
    BYTE  cBlueShift;
    BYTE  cAlphaBits;
    BYTE  cAlphaShift;
    BYTE  cAccumBits;
    BYTE  cAccumRedBits;
    BYTE  cAccumGreenBits;
    BYTE  cAccumBlueBits;
    BYTE  cAccumAlphaBits;
    BYTE  cDepthBits;
    BYTE  cStencilBits;
    BYTE  cAuxBuffers;
    BYTE  iLayerType;
    BYTE  bReserved;
    DWORD dwLayerMask;
    DWORD dwVisibleMask;
    DWORD dwDamageMask;
};

#define PFD_TYPE_RGBA 0
#define PFD_TYPE_COLORINDEX 1
#define PFD_MAIN_PLANE 0
#define PFD_OVERLAY_PLANE 1
#define PFD_UNDERLAY_PLANE (-1)
#define PFD_DOUBLEBUFFER 0x00000001
#define PFD_STEREO 0x00000002
#define PFD_DRAW_TO_WINDOW 0x00000004
#define PFD_DRAW_TO_BITMAP 0x00000008
#define PFD_SUPPORT_GDI 0x00000010
#define PFD_SUPPORT_OPENGL 0x00000020
#define PFD_GENERIC_FORMAT 0x00000040
#define PFD_NEED_PALETTE 0x00000080
#define PFD_NEED_SYSTEM_PALETTE 0x00000100
#define PFD_SWAP_EXCHANGE 0x00000200
#define PFD_SWAP_COPY 0x00000400
#define PFD_SWAP_LAYER_BUFFERS 0x00000800
#define PFD_GENERIC_ACCELERATED 0x00001000
#define PFD_DEPTH_DONTCARE 0x20000000
#define PFD_DOUBLEBUFFER_DONTCARE 0x40000000
#define PFD_STEREO_DONTCARE 0x80000000

typedef tagPIXELFORMATDESCRIPTOR PIXELFORMATDESCRIPTOR;
typedef PIXELFORMATDESCRIPTOR* PPIXELFORMATDESCRIPTOR;
typedef const PIXELFORMATDESCRIPTOR* PCPIXELFORMATDESCRIPTOR;

#ifndef D3DFMT_D16
#define D3DFMT_D16 80
#endif

#ifndef D3DFMT_D32
#define D3DFMT_D32 71
#endif

#ifndef D3DFMT_D24S8
#define D3DFMT_D24S8 75
#endif

#ifndef D3DFMT_L16
#define D3DFMT_L16 81
#endif

#define _3D_Z_ZEROVAL 0.0f
#define _3D_Z_MAXZVAL 1.0f

#endif