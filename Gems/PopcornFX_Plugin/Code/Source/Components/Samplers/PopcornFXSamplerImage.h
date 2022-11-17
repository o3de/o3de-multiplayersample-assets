//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>

#include "Integration/PopcornFXIntegrationBus.h"

#if defined(O3DE_USE_PK)
#include <pk_particles/include/ps_samplers_image.h>
#include <pk_imaging/include/im_samplers.h>
#include <AzCore/Asset/AssetTypeInfoBus.h>
#endif //O3DE_USE_PK

namespace PopcornFX {

	class PopcornFXSamplerImage
		: public PopcornFXSamplerComponentRequestBus::Handler
#if defined(O3DE_USE_PK)
		, public AZ::Data::AssetBus::Handler
#endif
	{
	public:
		AZ_TYPE_INFO(PopcornFXSamplerImage, "{969B9AC2-3218-4208-AA82-CFCD985CF1A3}")

		static void	Reflect(AZ::ReflectContext *context);

		void		CopyFrom(const PopcornFXSamplerImage &other);

		void		Activate();
		void		Deactivate();

		AZ::EntityId	m_AttachedToEntityId;

	protected:
		AZ::u32		_OnTextureChanged();

		AZ::Data::Asset<AZ::RPI::StreamingImageAsset>	m_Texture;

#if defined(O3DE_USE_PK)
	public:
		// AZ::Data::AssetBus::Handler
		void	OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
		void	OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset) override;

		// PopcornFXSamplerComponentRequestBus::Handler
		PopcornFX::CParticleSamplerDescriptor	*GetDescriptor() override { return m_SamplerDescriptor.Get(); }
		virtual AZ::u32							GetType() override { return CParticleSamplerDescriptor_Image::SamplerTypeID(); };

	protected:
		void	_Clean();
		void	_LoadTexture();
		bool	_BuildDescriptor(const AZ::RHI::ImageDescriptor &imgDesc);

		PParticleSamplerDescriptor	m_SamplerDescriptor = null;
		CImageSampler				*m_ImageSampler = null;
#endif //O3DE_USE_PK
};

}
