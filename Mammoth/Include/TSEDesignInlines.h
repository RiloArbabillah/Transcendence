//	TSEDesignInlines.h
//
//	Transcendence Space Engine
//	Inline implementations for design type references
//	NOTE: This file must be included AFTER TSEUniverse.h (when CUniverse is complete)

#pragma once

//	CDesignTypeRef::BindType implementation
//	These must be defined after CUniverse is complete (for FindDesignTypeUnbound)

template <class CLASS>
ALERROR CDesignTypeRef<CLASS>::BindType (SDesignLoadCtx &Ctx, DWORD dwUNID, CLASS *&pType)
	{
	CDesignType *pBaseType = Ctx.GetUniverse().FindDesignTypeUnbound(dwUNID);
	if (pBaseType)
		{
		if (!pBaseType->IsBound())
			{
			if (ALERROR error = pBaseType->BindDesign(Ctx))
				return error;
			}
		}
	else
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Unknown design type: %x"), dwUNID);
		return ERR_FAIL;
		}

	pType = CLASS::AsType(pBaseType);
	if (pType == NULL)
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Specified type is invalid: %x"), dwUNID);
		return ERR_FAIL;
		}

	return NOERROR;
	}