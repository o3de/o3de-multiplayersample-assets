//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include <AzCore/std/string/string.h>
#include <AzCore/Asset/AssetCommon.h>

#if defined(O3DE_USE_PK)
#include <pk_particles/include/ps_effect.h>
#else
namespace PopcornFX
{
	class CParticleEffect;
	typedef CParticleEffect*	PParticleEffect;
	class CBaseObjectFile;
	typedef CBaseObjectFile*	PBaseObjectFile;

	namespace HBO
	{
		class CContext;
	}
}
#endif

namespace PopcornFX {

	class PopcornFXAsset
		: public AZ::Data::AssetData
	{
	public:

		AZ_RTTI(PopcornFXAsset, "{45047C35-64F7-43BA-B463-000081B587C3}", AZ::Data::AssetData);
		AZ_CLASS_ALLOCATOR(PopcornFXAsset, AZ::SystemAllocator, 0);

		PParticleEffect	m_Effect = null;
		PBaseObjectFile	m_File = null;
		HBO::CContext	*m_Context = null;
		AZStd::string	m_Path;
	};

	class PopcornFXAssetPKMM
		: public AZ::Data::AssetData
	{
	public:

		AZ_RTTI(PopcornFXAssetPKMM, "{4AD0B88D-4138-4A42-986B-D021995B2250}", AZ::Data::AssetData);
		AZ_CLASS_ALLOCATOR(PopcornFXAssetPKMM, AZ::SystemAllocator, 0);
	};

	class PopcornFXAssetPKAN
		: public AZ::Data::AssetData
	{
	public:

		AZ_RTTI(PopcornFXAssetPKAN, "{9AA19C85-B18C-4245-831D-E589582E4B9D}", AZ::Data::AssetData);
		AZ_CLASS_ALLOCATOR(PopcornFXAssetPKAN, AZ::SystemAllocator, 0);
	};

	class PopcornFXAssetPKVF
		: public AZ::Data::AssetData
	{
	public:

		AZ_RTTI(PopcornFXAssetPKVF, "{0F8F0BF4-C87E-430D-AD75-EC5A9A631F06}", AZ::Data::AssetData);
		AZ_CLASS_ALLOCATOR(PopcornFXAssetPKVF, AZ::SystemAllocator, 0);
	};

	/*class PopcornFXAssetPKFM
		: public AZ::Data::AssetData
	{
	public:

		AZ_RTTI(PopcornFXAssetPKFM, "{}", AZ::Data::AssetData);
		AZ_CLASS_ALLOCATOR(PopcornFXAssetPKFM, AZ::SystemAllocator, 0);
	};*/

}
