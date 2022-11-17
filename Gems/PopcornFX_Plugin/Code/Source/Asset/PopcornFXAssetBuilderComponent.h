//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include <AzCore/RTTI/RTTI.h>
#include <AzCore/Component/Component.h>
#include <AssetBuilderSDK/AssetBuilderSDK.h>

#include "PopcornFXAssetBuilderWorker.h"


namespace PopcornFX {

class PopcornFXBuilderComponent
	: public AZ::Component
{
public:
	AZ_COMPONENT(PopcornFXBuilderComponent, "{473AFF50-A839-40A8-9EF7-7FD6C2D15337}");

	PopcornFXBuilderComponent();
	~PopcornFXBuilderComponent() override;

	//AZ::Component Impl
	void							Init() override;
	void							Activate() override;
	void							Deactivate() override;

	static void						Reflect(AZ::ReflectContext *context);
	static void						GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType &provided);
	static void						GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType &incompatible);
	static void						GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required);
	static void						GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType &dependent);

	PopcornFXBuilderWorker			m_PKBuilder;
};

}
