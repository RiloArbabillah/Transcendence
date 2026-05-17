//	CResourcePathResolver.cpp
//
//	Resolves milestone-1 UI resource names to on-disk files.

#include "PreComp.h"
#include "Transcendence.h"

struct SResourceEntry
	{
	const char *pszName;
	const char *pszFilename;
	};

static SResourceEntry FONT_RESOURCES[] =
	{
		{ "DXFN_HEADER", "Header.dxfn" },
		{ "DXFN_HEADER_BOLD", "HeaderBold.dxfn" },
		{ "DXFN_SUBTITLE", "SubTitle.dxfn" },
		{ "DXFN_SUBTITLE_BOLD", "SubTitleBold.dxfn" },
		{ "DXFN_SUBTITLE_HEAVY_BOLD", "SubTitleHeavyBold.dxfn" },
		{ "DXFN_TITLE", "Title.dxfn" },
		{ "DXFN_LOGO_TITLE", "LogoTitle.dxfn" },
	};

static SResourceEntry JPEG_RESOURCES[] =
	{
		{ "IDR_TITLE_IMAGE", "Title.JPG" },
		{ "IDR_STARGATE_IMAGE", "Stargate.JPG" },
		{ "IDR_GAME_BUTTONS_IMAGE", "GameButtonIcons.jpg" },
		{ "IDR_GENERIC_EXTENSION_SMALL", "GenericExtensionSmall.jpg" },
		{ "IDR_ICON_DISPLAY_IMAGE", "IconDisplay.JPG" },
		{ "IDR_HELP_BACKGROUND", "Help Screen.jpg" },
		{ "IDR_SELECT_SHIP_IMAGE", "SelectShipIcons.jpg" },
		{ "IDR_GAME_STATS_SCREEN", "Game Stats Screen.jpg" },
		{ "JPEG_DAMAGE_TYPE_ICONS", "DamageTypes.jpg" },
		{ "JPEG_UI_ICONS", "UIIcons.jpg" },
	};

static SResourceEntry BITMAP_RESOURCES[] =
	{
		{ "IDR_STARGATE_MASK", "StargateMask.BMP" },
		{ "BMP_DAMAGE_TYPE_ICONS_MASK", "DamageTypesMask.bmp" },
		{ "BMP_UI_ICONS_MASK", "UIIconsMask.bmp" },
	};

static bool FindResourceEntry (const SResourceEntry *pTable, int iCount, const CString &sName, CString *retsFilespec)
	{
	for (int i = 0; i < iCount; i++)
		if (strEquals(sName, CString(pTable[i].pszName)))
			{
			if (retsFilespec)
				*retsFilespec = pathAddComponent(CResourcePathResolver::GetResourcesRoot(), CString(pTable[i].pszFilename));

			return true;
			}

	return false;
	}

static CString FindResourcesRootCandidate (void)
	{
	TArray<CString> Candidates;

	CString sExecutablePath = pathGetExecutablePath(NULL);
	if (!sExecutablePath.IsBlank())
		{
		Candidates.Insert(pathAddComponent(sExecutablePath, CONSTLIT("../Resources")));
		Candidates.Insert(pathAddComponent(sExecutablePath, CONSTLIT("../../Resources")));
		Candidates.Insert(pathAddComponent(sExecutablePath, CONSTLIT("../../../Transcendence/Transcendence/Resources")));
		Candidates.Insert(pathAddComponent(sExecutablePath, CONSTLIT("../../../Transcendence/TransCore/Resources")));
		Candidates.Insert(pathAddComponent(sExecutablePath, CONSTLIT("../../../../Transcendence/Transcendence/Resources")));
		Candidates.Insert(pathAddComponent(sExecutablePath, CONSTLIT("../../../../Transcendence/TransCore/Resources")));
		}

	static const char *CANDIDATES[] =
		{
		"Transcendence/Transcendence/Resources",
		"../../Transcendence/Transcendence/Resources",
		"../Transcendence/Transcendence/Resources",
		"Transcendence/TransCore/Resources",
		"../../Transcendence/TransCore/Resources",
		"../Transcendence/TransCore/Resources",
		"Contents/Resources",
		};

	for (int i = 0; i < (sizeof(CANDIDATES) / sizeof(CANDIDATES[0])); i++)
		Candidates.Insert(CString(CANDIDATES[i]));

	for (int i = 0; i < Candidates.GetCount(); i++)
		if (pathExists(Candidates[i]))
			return Candidates[i];

	return CONSTLIT("Transcendence/Transcendence/Resources");
	}

CString CResourcePathResolver::GetResourcesRoot (void)

	{
	return FindResourcesRootCandidate();
	}

bool CResourcePathResolver::FindBitmapResource (const CString &sName, CString *retsFilespec)

	{
	return FindResourceEntry(BITMAP_RESOURCES, (sizeof(BITMAP_RESOURCES) / sizeof(BITMAP_RESOURCES[0])), sName, retsFilespec);
	}

bool CResourcePathResolver::FindFontResource (const CString &sName, CString *retsFilespec)

	{
	return FindResourceEntry(FONT_RESOURCES, (sizeof(FONT_RESOURCES) / sizeof(FONT_RESOURCES[0])), sName, retsFilespec);
	}

bool CResourcePathResolver::FindJPEGResource (const CString &sName, CString *retsFilespec)

	{
	return FindResourceEntry(JPEG_RESOURCES, (sizeof(JPEG_RESOURCES) / sizeof(JPEG_RESOURCES[0])), sName, retsFilespec);
	}

ALERROR LoadBMPResourceAsDIB (const CString &sName, HBITMAP *rethBitmap, EBitmapTypes *retiType)

	{
	CString sFilespec;
	if (!CResourcePathResolver::FindBitmapResource(sName, &sFilespec))
		return ERR_FAIL;

	return dibLoadFromFile(sFilespec, rethBitmap, retiType);
	}

ALERROR LoadJPEGResourceAsDIB (const CString &sName, HBITMAP *rethBitmap)

	{
	CString sFilespec;
	if (!CResourcePathResolver::FindJPEGResource(sName, &sFilespec))
		return ERR_FAIL;

	return JPEGLoadFromFile(sFilespec, JPEG_LFR_DIB, NULL, rethBitmap);
	}
