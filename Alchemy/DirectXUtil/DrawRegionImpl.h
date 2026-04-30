//	DrawRegionImpl.h
//
//	Drawing implementation classes
//	Copyright (c) 2015 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

	template <class BLENDER> class TFillRegionSolid : public TRegionPainter32<TFillRegionSolid<BLENDER>, BLENDER>
	{
	public:
		TFillRegionSolid (CG32bitPixel rgbColor) :
				m_rgbColor(rgbColor)
			{ }

		CG32bitPixel GetPixelAt (int x, int y) const { return m_rgbColor; }

	private:
		CG32bitPixel m_rgbColor;
	};