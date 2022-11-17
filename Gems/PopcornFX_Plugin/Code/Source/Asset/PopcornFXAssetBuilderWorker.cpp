//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PopcornFXAssetBuilderWorker.h"

#include <AzFramework/IO/LocalFileIO.h>
#include <AzCore/StringFunc/StringFunc.h>

#include "PopcornFXAsset.h"
#include "Integration/PopcornFXIntegrationBus.h"


namespace PopcornFX {

//----------------------------------------------------------------------------------------------------------//
//																											//
//											PopcornFXBuilderWorker											//
//																											//
//----------------------------------------------------------------------------------------------------------//

// Note - Shutdown will be called on a different thread than your process job thread
void	PopcornFXBuilderWorker::ShutDown()
{
	m_IsShuttingDown = true;
}

// CreateJobs will be called early on in the file scanning pass from the Asset Processor.
// You should create the same jobs, and avoid checking whether the job is up to date or not. The Asset Processor will manage this for you
void	PopcornFXBuilderWorker::CreateJobs(const AssetBuilderSDK::CreateJobsRequest &request, AssetBuilderSDK::CreateJobsResponse &response)
{
	AZ_TracePrintf(AssetBuilderSDK::InfoWindow, "[PopcornFX] Begin jobs creation for '%s'", request.m_sourceFile.c_str());
	if (m_IsShuttingDown)
	{
		response.m_result = AssetBuilderSDK::CreateJobsResultCode::ShuttingDown;
		return;
	}
	AZStd::string	ext;
	AZStd::string	relPath = request.m_sourceFile;

	response.m_result = AssetBuilderSDK::CreateJobsResultCode::Failed;

	// TODO: Fix that, if the user has some Cache/ folder somewhere and it's legit, this won't work.
	// We need to grab the project's editor cache folder from PopcornProject.pkproj
	if (AZ::StringFunc::Find(relPath.c_str(), "Cache/") != AZStd::string::npos)
	{
		response.m_result = AssetBuilderSDK::CreateJobsResultCode::Success; // We do not want to process files from project's cache folder. This is not an error
		return;
	}

	AZ::StringFunc::Path::GetExtension(request.m_sourceFile.c_str(), ext, false);

	if (AZ::StringFunc::Equal(ext.c_str(), "pkfx"))
	{
		AZStd::string	fullPath;
		AZ::StringFunc::Path::ConstructFull(request.m_watchFolder.c_str(), request.m_sourceFile.c_str(), fullPath, false);

		bool							ok = false;
		AZStd::vector<AZStd::string>	dependencies;
		PopcornFX::PopcornFXIntegrationBus::BroadcastResult(ok, &PopcornFX::PopcornFXIntegrationBus::Handler::GatherDependencies, fullPath, dependencies);

		if (!ok)
		{
			AZ_TracePrintf(AssetBuilderSDK::ErrorWindow, "[PK] - _GatherDependencies failed on '%s'.", fullPath.c_str());
			return;
		}
		for (int i = 0; i < dependencies.size(); ++i)
		{
			AssetBuilderSDK::SourceFileDependency	dependencyInfo;
			dependencyInfo.m_sourceFileDependencyPath = dependencies[i];
			response.m_sourceFileDependencyList.push_back(dependencyInfo);

			AZ_TracePrintf(AssetBuilderSDK::InfoWindow, "[PopcornFX] '%s': New dependency added '%s'", relPath.data(), dependencies[i].c_str());
		}
		for (const AssetBuilderSDK::PlatformInfo &platformInfo : request.m_enabledPlatforms)
		{
			AssetBuilderSDK::JobDescriptor	descriptor;
			descriptor.m_jobKey = "PK_MainAsset";
			descriptor.SetPlatformIdentifier(platformInfo.m_identifier.c_str());

			response.m_createJobOutputs.push_back(descriptor);
		}
		response.m_result = AssetBuilderSDK::CreateJobsResultCode::Success;
		return;
	}
	else if (AZ::StringFunc::Equal(ext.c_str(), "pkfbx") ||
		//AZ::StringFunc::Equal(ext.c_str(), "ttf") ||
		//AZ::StringFunc::Equal(ext.c_str(), "otf") ||
		AZ::StringFunc::Equal(ext.c_str(), "fga"))
	{
		for (const AssetBuilderSDK::PlatformInfo &platformInfo : request.m_enabledPlatforms)
		{
			AssetBuilderSDK::JobDescriptor	descriptor;
			descriptor.m_jobKey = "PK_Dependency";
			descriptor.SetPlatformIdentifier(platformInfo.m_identifier.c_str());
			response.m_createJobOutputs.push_back(descriptor);
		}
		response.m_result = AssetBuilderSDK::CreateJobsResultCode::Success;
		return;
	}
	AZ_TracePrintf(AssetBuilderSDK::ErrorWindow, "[PK] - Unhandled extension type in PopcornFXBuilderWorker for '%s'", request.m_sourceFile.c_str());
}


void	PopcornFXBuilderWorker::ProcessJob(const AssetBuilderSDK::ProcessJobRequest &request, AssetBuilderSDK::ProcessJobResponse &response)
{
	response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Cancelled;

	AssetBuilderSDK::JobCancelListener	jobCancelListener(request.m_jobId);
	AZStd::string						ext;

	AZ::StringFunc::Path::GetExtension(request.m_sourceFile.c_str(), ext, false);

	const bool	isDepend =	//AZ::StringFunc::Equal(ext.c_str(), "ttf") ||
							//AZ::StringFunc::Equal(ext.c_str(), "otf") ||
							AZ::StringFunc::Equal(ext.c_str(), "fga");

	if (!AZ::StringFunc::Equal(ext.c_str(), "pkfx") &&
		!isDepend)
	{
		AZ_TracePrintf(AssetBuilderSDK::WarningWindow, "[PopcornFX] Wrong file type for job '%s', ignoring.\n", request.m_fullPath.c_str());
		return;
	}
	if (!(AZ::StringFunc::Equal(request.m_jobDescription.m_jobKey.c_str(), "PK_MainAsset")
		|| AZ::StringFunc::Equal(request.m_jobDescription.m_jobKey.c_str(), "PK_Dependency")))
	{
		AZ_TracePrintf(AssetBuilderSDK::WarningWindow, "[PopcornFX] Wrong job description for job '%s', ignoring.\n", request.m_fullPath.c_str());
		return;
	}
	if (jobCancelListener.IsCancelled() || m_IsShuttingDown)
	{
		AZ_TracePrintf(AssetBuilderSDK::WarningWindow, "[PopcornFX] Cancel was requested for job '%s'.\n", request.m_fullPath.c_str());
		return;
	}
	response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;

	AZStd::string	fullSrcPath = request.m_fullPath;

	AZ_TracePrintf(AssetBuilderSDK::InfoWindow, "[PopcornFX] Processing asset '%s'", fullSrcPath.c_str());
	AZ_TracePrintf(AssetBuilderSDK::InfoWindow, "[PopcornFX] Asset temp bake directory: '%s'", request.m_tempDirPath.c_str());

	AZStd::string	virtualPath;
	PopcornFX::PopcornFXIntegrationBus::BroadcastResult(virtualPath, &PopcornFX::PopcornFXIntegrationBus::Handler::BakeSingleAsset, fullSrcPath, request.m_tempDirPath, request.m_platformInfo.m_identifier);

	if (virtualPath.empty())
		return;

	// Source -> Baked extension fixup
	// The asset baker processes some source files (.ttf, .fbx, .fga, ..) and procudes baked files for those (.pkfm, .pkmm, .pkvf, ..)
	// We need to trick the asset processor by fixing m_outputProducts, otherwise it'll fail copying the processed file from temp folder to final cache folder.

	AssetBuilderSDK::JobProduct		jobProduct{ virtualPath };
	if (isDepend)
	{
		AZ::IO::LocalFileIO	fileIO;
		AZStd::string		buildPath = request.m_tempDirPath + "/" + AZStd::string(request.m_sourceFile);

		AZ::StringFunc::Path::ReplaceExtension(buildPath, "pkmm");
		if (fileIO.Exists(buildPath.c_str()))
		{
			AZ::StringFunc::Path::ReplaceExtension(jobProduct.m_productFileName, "pkmm");
			jobProduct.m_productAssetType = AZ::AzTypeInfo<PopcornFXAssetPKMM>::Uuid();
		}
		AZ::StringFunc::Path::ReplaceExtension(buildPath, "pkan");
		if (fileIO.Exists(buildPath.c_str()))
		{
			AZ::StringFunc::Path::ReplaceExtension(jobProduct.m_productFileName, "pkan");
			jobProduct.m_productAssetType = AZ::AzTypeInfo<PopcornFXAssetPKAN>::Uuid();
		}
		AZ::StringFunc::Path::ReplaceExtension(buildPath, "pkvf");
		if (fileIO.Exists(buildPath.c_str()))
		{
			AZ::StringFunc::Path::ReplaceExtension(jobProduct.m_productFileName, "pkvf");
			jobProduct.m_productAssetType = AZ::AzTypeInfo<PopcornFXAssetPKVF>::Uuid();
		}
	}
	else
	{
		jobProduct.m_productAssetType = AZ::AzTypeInfo<PopcornFXAsset>::Uuid();
		jobProduct.m_dependenciesHandled = true; // We've output the dependencies immediately above so it's OK to tell the AP we've handled dependencies
	}
	response.m_outputProducts.push_back(jobProduct);
	response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;
}

}
