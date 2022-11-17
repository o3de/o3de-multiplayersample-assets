//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/vector.h>

#if defined(O3DE_USE_PK)

namespace PopcornFX
{
	class CResourceHandlerMesh;
	class CResourceHandlerImage;
	class CResourceHandlerRectangleList;
	class CResourceHandlerFontMetrics;
	class CResourceHandlerVectorField;
	class CResourceManager;
	class CCookery;
	class CMessageStream;
	namespace HBO {
		class CContext;
	}
	PK_FORWARD_DECLARE(OvenBakeConfig_HBO);
}

namespace PopcornFX {

class CWindManager;

//----------------------------------------------------------------------------

class CBakerManager
{
public:
	bool	Activate();
	void	Deactivate();

	AZStd::string	BakeSingleAsset(const AZStd::string &assetPath, const AZStd::string &outDir, const AZStd::string &platform, CWindManager &windManager);
	bool			GatherDependencies(const AZStd::string &assetPath, AZStd::vector<AZStd::string> &dependencies, CWindManager &windManager);

private:
	class		SBakeContext
	{
	public:
		SBakeContext();
		~SBakeContext();
		CResourceHandlerMesh			*m_BakeResourceMeshHandler;
		CResourceHandlerImage			*m_BakeResourceImageHandler;
		CResourceHandlerRectangleList	*m_BakeResourceRectangleListHandler;
		CResourceHandlerFontMetrics		*m_BakeResourceFontMetricsHandler;
		CResourceHandlerVectorField		*m_BakeResourceVectorFieldHandler;
		IFileSystem						*m_BakeFSController;
		CResourceManager				*m_BakeResourceManager;
		HBO::CContext					*m_BakeContext;

		bool	Initialize();
	};

	void	LogBakerMessages(const CMessageStream &messages);
	bool	_SetPackIFN(const AZStd::string &assetPath, CWindManager &windManager);
	void	_SetBuildVersion(const AZStd::string &platform);

	CCookery		*m_Cookery = null;
	SBakeContext	*m_BakeContext = null;
};

//----------------------------------------------------------------------------
}

#endif //O3DE_USE_PK
