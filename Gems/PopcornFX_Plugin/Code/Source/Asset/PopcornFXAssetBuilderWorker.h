//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include <AssetBuilderSDK/AssetBuilderSDK.h>

namespace PopcornFX {

class PopcornFXBuilderWorker
	: public AssetBuilderSDK::AssetBuilderCommandBus::Handler
{
public:
	AZ_RTTI(PopcornFXBuilderWorker, "{F42BB484-7EC1-483E-A117-B45603929B6F}");

	PopcornFXBuilderWorker() = default;
	~PopcornFXBuilderWorker() = default;

	void							CreateJobs(const AssetBuilderSDK::CreateJobsRequest &request, AssetBuilderSDK::CreateJobsResponse &response);
	void							ProcessJob(const AssetBuilderSDK::ProcessJobRequest &request, AssetBuilderSDK::ProcessJobResponse &response);
	void							ShutDown() override;
private:
	bool							m_IsShuttingDown = false;
};

}
