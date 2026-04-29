//	CGDrawStub.cpp
//
//	Stub implementations for CGDraw on macOS
//	Provides minimal implementations to allow linking

#include "PreComp.h"
#include "DXImage32.h"

void CGDraw::LineBroken(CG32bitImage&, int, int, int, int, int, CG32bitPixel) {}
void CGDraw::LineDotted(CG32bitImage&, int, int, int, int, CG32bitPixel) {}
void CGDraw::CircleImage(CG32bitImage&, int, int, int, unsigned char, CG32bitImage const&, CGDraw::EBlendModes, int, int, int, int) {}
void CGDraw::RectOutline(CG32bitImage&, int, int, int, int, CG32bitPixel) {}
void CGDraw::RingGlowing(CG32bitImage&, int, int, int, int, CG32bitPixel) {}
void CGDraw::RingGlowing(CG32bitImage&, int, int, int, int, Kernel::TArray<CG32bitPixel> const&, unsigned char) {}
void CGDraw::RoundedRect(CG32bitImage&, int, int, int, int, int, CG32bitPixel, CGDraw::EBlendModes) {}
void CGDraw::LineGradient(CG32bitImage&, int, int, int, int, int, CG32bitPixel, CG32bitPixel) {}
void CGDraw::RectGradient(CG32bitImage&, int, int, int, int, CG32bitPixel, CG32bitPixel, GradientDirections) {}
void CGDraw::CircleOutline(CG32bitImage&, int, int, int, int, CG32bitPixel) {}
void CGDraw::LineBresenham(CG32bitImage&, int, int, int, int, int, CG32bitPixel) {}
void CGDraw::CircleGradient(CG32bitImage&, int, int, int, CG32bitPixel, CGDraw::EBlendModes) {}
void CGDraw::Circle(CG32bitImage&, int, int, int, CG32bitPixel, CGDraw::EBlendModes) {}
void CGDraw::CircleFill(CG32bitImage&, int, int, int, CG32bitPixel, CGDraw::EBlendModes) {}
void CGDraw::Rect(CG32bitImage&, int, int, int, int, CG32bitPixel, CGDraw::EBlendModes) {}
void CGDraw::RectFill(CG32bitImage&, int, int, int, int, CG32bitPixel, CGDraw::EBlendModes) {}
void CGDraw::Arc(CG32bitImage&, const CVector&, Metric, Metric, Metric, Metric, CG32bitPixel, CGDraw::EBlendModes, int, DWORD) {}
void CGDraw::Line(CG32bitImage&, int, int, int, int, CG32bitPixel, CGDraw::EBlendModes) {}
void CGDraw::Pixel(CG32bitImage&, int, int, CG32bitPixel, CGDraw::EBlendModes) {}
void CGDraw::Region(CG32bitImage&, int, int, CGRegion const&, CG32bitPixel, CGDraw::EBlendModes) {}
