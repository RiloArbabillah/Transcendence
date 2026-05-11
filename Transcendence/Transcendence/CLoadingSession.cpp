//	CLoadingSession.cpp
//
//	CLoadingSession class

#include "PreComp.h"
#include "Transcendence.h"

#include <cstdio>

static void loading_log(const char *pszMsg)
	{
	fprintf(stderr, "%s\n", pszMsg);
	fflush(stderr);
	}

const int STARGATE_WIDTH =						128;
const int STARGATE_HEIGHT =						128;

const int Y_COPYRIGHT_TEXT =					392;

const CG32bitPixel RGB_IMAGE_BACKGROUND =		CG32bitPixel(0, 0, 0);

static void ApplyAlphaMask (CG32bitImage &Dest, const SBMPImageLoad &Mask)
	{
	int cxWidth = Min(Dest.GetWidth(), Mask.cxWidth);
	int cyHeight = Min(Dest.GetHeight(), Mask.cyHeight);

	for (int y = 0; y < cyHeight; y++)
		{
		CG32bitPixel *pDest = Dest.GetPixelPos(0, y);
		const CG32bitPixel *pMask = (const CG32bitPixel *)(Mask.Pixels.GetPointer() + (y * Mask.iPitch));

		for (int x = 0; x < cxWidth; x++)
			{
			BYTE byAlpha;
			if (Mask.iType == bitmapMonochrome)
				byAlpha = (pMask->GetGreen() ? 0xff : 0x00);
			else
				byAlpha = (pMask->GetGreen() >= 0x80 ? 0xff : 0x00);

			if (byAlpha == 0x00)
				*pDest = CG32bitPixel(0, 0, 0, 0);

			pDest->SetAlpha(byAlpha);
			pDest++;
			pMask++;
			}
		}

	Dest.SetAlphaType(CG32bitImage::alpha1);
	}

static void BltAlpha1Frame (CG32bitImage &Dest, int xDest, int yDest, const CG32bitImage &Source, int xSrc, int ySrc, int cxWidth, int cyHeight, const CG32bitImage &Background)
	{
	if (xDest < 0 || yDest < 0 || xSrc < 0 || ySrc < 0)
		return;

	if ((xDest + cxWidth) > Dest.GetWidth() || (yDest + cyHeight) > Dest.GetHeight())
		return;

	if ((xSrc + cxWidth) > Source.GetWidth() || (ySrc + cyHeight) > Source.GetHeight())
		return;

	if (Background.GetWidth() != cxWidth || Background.GetHeight() != cyHeight)
		return;

	for (int y = 0; y < cyHeight; y++)
		{
		CG32bitPixel *pDest = Dest.GetPixelPos(xDest, yDest + y);
		const CG32bitPixel *pSrc = Source.GetPixelPos(xSrc, ySrc + y);
		const CG32bitPixel *pBackground = Background.GetPixelPos(0, y);

		for (int x = 0; x < cxWidth; x++)
			{
			if (pSrc->GetAlpha() != 0x00)
				*pDest = *pSrc;
			else
				*pDest = *pBackground;

			pDest++;
			pSrc++;
			pBackground++;
			}
		}
	}

ALERROR CLoadingSession::OnInit (CString *retsError)

//	OnInit

	{
	ALERROR error;
	const CVisualPalette &VI = m_HI.GetVisuals();
	loading_log("CLoadingSession::OnInit start");

	RECT rcCenter;
	VI.GetWidescreenRect(&rcCenter);

	//	Load a JPEG of the background image

	SJPEGLoadInfo Image;
	CString sTitleFilespec;
	if (!CResourcePathResolver::FindJPEGResource(CONSTLIT("IDR_TITLE_IMAGE"), &sTitleFilespec))
		return ERR_FAIL;
	loading_log("CLoadingSession::OnInit found title resource");

	if (error = JPEGLoadToRGBAFromFile(sTitleFilespec, &Image))
		return error;
	loading_log("CLoadingSession::OnInit loaded title image");

	bool bSuccess = m_TitleImage.CreateFromRaw(Image.Pixels.GetPointer(), Image.cxWidth, Image.cyHeight, Image.iPitch, CG32bitImage::alphaNone);
	if (!bSuccess)
		return ERR_FAIL;

	//	Load stargate image

	CString sStargateFilespec;
	if (!CResourcePathResolver::FindJPEGResource(CONSTLIT("IDR_STARGATE_IMAGE"), &sStargateFilespec))
		return ERR_FAIL;
	loading_log("CLoadingSession::OnInit found stargate resource");

	if (error = JPEGLoadToRGBAFromFile(sStargateFilespec, &Image))
		return error;
	loading_log("CLoadingSession::OnInit loaded stargate image");

	bSuccess = m_StargateImage.CreateFromRaw(Image.Pixels.GetPointer(), Image.cxWidth, Image.cyHeight, Image.iPitch, CG32bitImage::alphaNone);
	if (!bSuccess)
		return ERR_FAIL;

	CString sMaskFilespec;
	if (!CResourcePathResolver::FindBitmapResource(CONSTLIT("IDR_STARGATE_MASK"), &sMaskFilespec))
		return ERR_FAIL;
	loading_log("CLoadingSession::OnInit found stargate mask");

	SBMPImageLoad Mask;
	if (error = dibLoadToBufferFromFile(sMaskFilespec, &Mask))
		return error;
	loading_log("CLoadingSession::OnInit loaded stargate mask");

	ApplyAlphaMask(m_StargateImage, Mask);
	if (m_StargateImage.IsEmpty())
		return ERR_FAIL;

	//	Figure out position of copyright text.

	m_cyCopyright = Y_COPYRIGHT_TEXT - (Max(0, m_TitleImage.GetHeight() - RectHeight(rcCenter)) / 2);

	//	Figure out the position of the stargate animation

	int cxScreen = g_pHI->GetScreenWidth();
	int cyScreen = g_pHI->GetScreenHeight();

	m_rcStargate.left = (cxScreen - STARGATE_WIDTH) / 2;
	m_rcStargate.right = m_rcStargate.left + STARGATE_WIDTH;

	if (RectHeight(rcCenter) >= 512)
		{
		m_rcStargate.top = rcCenter.bottom - (STARGATE_HEIGHT / 2);
		m_rcStargate.bottom = m_rcStargate.top + STARGATE_HEIGHT;
		}
	else
		{
		m_rcStargate.bottom = cyScreen;
		m_rcStargate.top = m_rcStargate.bottom - STARGATE_HEIGHT;
		}

	loading_log("CLoadingSession::OnInit done");

	return NOERROR;
	}

void CLoadingSession::OnPaint (CG32bitImage &Screen, const RECT &rcInvalid)

//	OnPaint

	{
	static bool bLogged = false;
	if (!bLogged)
		{
		loading_log("CLoadingSession::OnPaint first paint");
		bLogged = true;
		}

	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &MediumHeavyBoldFont = VI.GetFont(fontMediumHeavyBold);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

	RECT rcCenter;
	VI.DrawSessionBackground(Screen, m_TitleImage, RGB_IMAGE_BACKGROUND, 0, &rcCenter);

	//	Paint copyright text

	int cxWidth = MediumHeavyBoldFont.MeasureText(m_sCopyright);
	MediumHeavyBoldFont.DrawText(Screen,
			(Screen.GetWidth() - cxWidth) / 2,
			rcCenter.top + m_cyCopyright,
			VI.GetColor(colorTextHighlight),
			m_sCopyright);

	//	Paint the loading title

	CString sLoading = CONSTLIT("Loading");
	cxWidth = SubTitleFont.MeasureText(sLoading, NULL);
	SubTitleFont.DrawText(Screen,
			(Screen.GetWidth() - cxWidth) / 2,
			m_rcStargate.bottom,
			VI.GetColor(colorTextFade),
			sLoading);

	//	Paint the stargate

	CG32bitImage StargateBackground;
	StargateBackground.Create(STARGATE_WIDTH, STARGATE_HEIGHT, CG32bitImage::alphaNone);
	StargateBackground.Copy(0,
			0,
			STARGATE_WIDTH,
			STARGATE_HEIGHT,
			Screen,
			m_rcStargate.left,
			m_rcStargate.top);

	BltAlpha1Frame(Screen,
			m_rcStargate.left,
			m_rcStargate.top,
			m_StargateImage,
			STARGATE_WIDTH * (m_iTick % 48),
			0,
			STARGATE_WIDTH,
			STARGATE_HEIGHT,
			StargateBackground);
	}

void CLoadingSession::OnReportHardCrash (CString *retsMessage)

//	OnReportHardCrash
//
//	Describe current state

	{
	*retsMessage = CONSTLIT("session: CLoadingSession\r\n");
	}

void CLoadingSession::OnUpdate (bool bTopMost)

//	OnUpdate

	{
	m_iFrame++;
	if (bTopMost)
		{
		if (!m_b60fps || (m_iFrame % 2 == 0))
			m_iTick++;

		HIInvalidate(m_rcStargate);
		}
	}
