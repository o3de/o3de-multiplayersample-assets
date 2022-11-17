//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#include "PopcornFX_precompiled.h"

#include "PackLoader.h"

#if defined(O3DE_USE_PK)

#include <AzCore/StringFunc/StringFunc.h>
#include <AzFramework/IO/LocalFileIO.h>
#include <AzFramework/FileFunc/FileFunc.h>
#include <AzCore/std/function/function_template.h>

#include <pk_kernel/include/kr_file.h>
#include <pk_kernel/include/kr_resources.h>
#include <pk_base_object/include/hbo_context.h>
#include <pk_particles/include/ps_project_settings.h>

namespace PopcornFX
{
	bool	GetProjectSettings(const AZStd::string &projectFilePath, AZStd::string &outRootPath, AZStd::string &outLibraryPath)
	{
		// Create a local context for loading the .pkproj
		IFileSystem			*localFileSystem = File::NewInternalFileSystem();
		if (!PK_VERIFY(localFileSystem != null))
			return false;
		CResourceManager	*localResourceManager = PK_NEW(CResourceManager(localFileSystem)); // No resource handler bound
		if (!PK_VERIFY(localResourceManager != null))
		{
			PK_DELETE(localFileSystem);
			return false;
		}
		PopcornFX::HBO::CContext	*localBoContext = PK_NEW(PopcornFX::HBO::CContext(localResourceManager));
		if (PK_VERIFY(localBoContext != null))
		{
			CString	pkprojFile = projectFilePath.c_str();
			CFilePath::Purify(pkprojFile);

			const CString					projectSettingsContent = localFileSystem->BufferizeToString(pkprojFile, true);
			PopcornFX::CConstMemoryStream	stream(projectSettingsContent.Data(), projectSettingsContent.Length());
			PopcornFX::PProjectSettings		projectSettings = PopcornFX::CProjectSettings::LoadFromStream(stream, localBoContext);
			CString							folderPath = CFilePath::StripFilename(pkprojFile);

			if (projectSettings != null)
			{
				//Apply ApplyGlobalSettings
				projectSettings->ApplyGlobalSettings(); // Applies constants

				// CProjectSettings::ApplyGlobalSettings also applies the .pkproj's scene settings (distance units and axis system)
				// We don't need the .pkproj's axis system informations: override it (allows effects coming from incorrectly setup packs to be imported without issues)
				CCoordinateFrame::SetGlobalFrame(Frame_RightHand_Z_Up);

				if (!projectSettings->General()->RootDir().Empty()) // We need RootDir
				{
					folderPath /= projectSettings->General()->RootDir();
					CFilePath::Purify(folderPath);
				}
			}

			outLibraryPath = projectSettings->General()->LibraryDir().Data();

			localBoContext->UnloadAllFiles();
			PK_DELETE(localBoContext);
			PK_DELETE(localResourceManager);
			PK_DELETE(localFileSystem);
			AZStd::string	ret = folderPath.Data();
			AZ::StringFunc::Path::Normalize(ret);
			outRootPath = ret;
			return true;
		}
		PK_DELETE(localResourceManager);
		PK_DELETE(localFileSystem);
		return false;
	}

	AZStd::string	FindMatchingPkproj(const AZStd::string &assetPath)
	{
		AZ::IO::FixedMaxPath projectPath = AZ::Utils::GetProjectPath();

		AZStd::string	folderPath = assetPath;
		AZ::StringFunc::Path::StripFullName(folderPath);
		AZ::StringFunc::Path::Normalize(folderPath);

		AZ::Outcome<AZStd::list<AZStd::string>, AZStd::string> searchResult = AzFramework::FileFunc::FindFilesInPath(projectPath.c_str(), "*", true);

		if (!searchResult.IsSuccess())
			return "";

		const AZStd::list<AZStd::string> &pkprojList = searchResult.GetValue();

		AZStd::string pkProjPath;
		for (const auto& path : pkprojList)
		{
			if (path.ends_with(".pkproj"))
			{
				AZStd::string	rootPath;
				AZStd::string	dummy;
				if (!GetProjectSettings(path, rootPath, dummy))
					return "";

				const AZ::IO::PathView pathView(folderPath);
				if (pathView.IsRelativeTo(AZ::IO::PathView(rootPath)))
					return path;
			}
		}
		return "";
	}
	
	bool	GetProjectSettingsThumbnailsPath(const AZStd::string &projectFilePath, AZStd::string &outRootDir, AZStd::string &outThumbnailsPath)
	{
		// Create a local context for loading the .pkproj
		IFileSystem			*localFileSystem = File::NewInternalFileSystem();
		if (!PK_VERIFY(localFileSystem != null))
			return false;
		CResourceManager	*localResourceManager = PK_NEW(CResourceManager(localFileSystem)); // No resource handler bound
		if (!PK_VERIFY(localResourceManager != null))
		{
			PK_DELETE(localFileSystem);
			return false;
		}
		PopcornFX::HBO::CContext	*localBoContext = PK_NEW(PopcornFX::HBO::CContext(localResourceManager));
		if (PK_VERIFY(localBoContext != null))
		{
			CString	pkprojFile = projectFilePath.c_str();
			CFilePath::Purify(pkprojFile);
	
			const CString					projectSettingsContent = localFileSystem->BufferizeToString(pkprojFile, true);
			PopcornFX::CConstMemoryStream	stream(projectSettingsContent.Data(), projectSettingsContent.Length());
			PopcornFX::PProjectSettings		projectSettings = PopcornFX::CProjectSettings::LoadFromStream(stream, localBoContext);
	
			bool	found = false;
			if (projectSettings != null &&
				projectSettings->General() != null)
			{
				if (!projectSettings->General()->RootDir().Empty())
					outRootDir = projectSettings->General()->RootDir().Data();
				outThumbnailsPath = projectSettings->General()->ThumbnailsDir().Data();
				found = true;
			}
	
			localBoContext->UnloadAllFiles();
			PK_DELETE(localBoContext);
			PK_DELETE(localResourceManager);
			PK_DELETE(localFileSystem);
			return found;
		}
		PK_DELETE(localResourceManager);
		PK_DELETE(localFileSystem);
		return false;
	}
	
	void	GetThumbnailPathForAsset(const AZStd::string &assetPath, AZStd::string &outThumbnailPath, AZStd::string &inOutPkProjPath)
	{
		const bool pkProjCacheEmpty = inOutPkProjPath.empty();
		if (pkProjCacheEmpty)
		{
			inOutPkProjPath = FindMatchingPkproj(assetPath);
		}

		if (!inOutPkProjPath.empty())
		{
			AZStd::string	rootDir;
			if (GetProjectSettingsThumbnailsPath(inOutPkProjPath, rootDir, outThumbnailPath))
			{
				const CString	pkProjFolder = CFilePath::StripFilename(inOutPkProjPath.c_str());
				CString			filePath = assetPath.c_str();
				CString			projectPath = CString::Format("%s/%s/", pkProjFolder.Data(), rootDir.c_str());
	
				CFilePath::Purify(filePath);
				CFilePath::Purify(projectPath);
	
				if (filePath.StartsWith(projectPath + "/"))
				{
					const CString	virtualFilePath = filePath.Extract(projectPath.Length() + 1, filePath.Length());
					CString			thumbnailPath = CString::Format("%s/%s/%s.png", projectPath.Data(), outThumbnailPath.c_str(), virtualFilePath.Data());
	
					CFilePath::Purify(thumbnailPath);
					outThumbnailPath = thumbnailPath.Data();
				}
				else if (!pkProjCacheEmpty)
				{
					inOutPkProjPath.clear();
					GetThumbnailPathForAsset(assetPath, outThumbnailPath, inOutPkProjPath);
				}
			}
		}
	}
	
	bool	ChangePackWithPathIFN(	const AZStd::string &pkProjPath, IFileSystem *fileSystem,
									AZStd::string &outRootPath, AZStd::string &outLibraryPath,
									bool assetProcessor)
	{
		AZ::IO::FixedMaxPath projectPath = AZ::Utils::GetProjectPath();
	
		AZStd::string	rootPath;
		if (!GetProjectSettings(pkProjPath, rootPath, outLibraryPath))
			return false;
	
		//Check if the pack is already mounted
		TMemoryView<const PFilePack>	packs = fileSystem->PrioritizedPacks();
	
		if (packs.Count() > 0)
		{
			AZStd::string	packPath;
	
			packPath = packs[0]->Path().Data();
	
			if (!assetProcessor)
				AZ::StringFunc::Path::Join(projectPath.c_str(), packPath.c_str(), packPath);
	
			AZ::StringFunc::Path::Normalize(packPath);
	
			if (!packPath.empty() && rootPath == packPath)
			{
				//AZ_TracePrintf(AssetBuilderSDK::InfoWindow, "[PopcornFX] Pack %s already mounted", packPath.c_str());
				if (!assetProcessor)
					packPath = CFilePath::Relativize(projectPath.c_str(), rootPath.c_str()).Data();
				outRootPath = packPath;
				return false;
			}
			else
			{
				//AZ_TracePrintf(AssetBuilderSDK::InfoWindow, "[PopcornFX] Asset not in pack %s, unmounting it.", packPath.c_str());
				fileSystem->UnmountAllPacks();
			}
		}
	
		//mount the pack
		if (!rootPath.empty())
		{
			AZStd::string	packPath = rootPath;
			PFilePack		filePack = null;
			//AZ_TracePrintf(AssetBuilderSDK::InfoWindow, "[PopcornFX] Mounting RootDir %s", folderPath.Data());
	
			if (!assetProcessor)
				packPath = CFilePath::Relativize(projectPath.c_str(), rootPath.c_str()).Data();
	
			filePack = fileSystem->MountPack(packPath.c_str());
			if (filePack != null)
			{
				outRootPath = packPath.c_str();
				return true;
			}
		}
		return false;
	}

	bool	ChangePackIFN(	const AZStd::string &assetPath, IFileSystem *fileSystem,
							AZStd::string &rootPath, AZStd::string &outLibraryPath, bool assetProcessor /*= true*/)
	{
		if (!assetProcessor && !rootPath.empty())
		{
			AZ::IO::FixedMaxPath projectPath = AZ::Utils::GetProjectPath();

			AZStd::string	folderPath = assetPath;
			AZ::StringFunc::Path::StripFullName(folderPath);
			AZ::StringFunc::Path::Normalize(folderPath);
			folderPath = CFilePath::Relativize(projectPath.c_str(), folderPath.c_str()).Data();

			const AZ::IO::PathView pathView(folderPath);
			if (pathView.IsRelativeTo(AZ::IO::PathView(rootPath)))
				return false;
		}

		const AZStd::string pkProjPath = FindMatchingPkproj(assetPath);

		if (pkProjPath.empty())
			return false;
		return ChangePackWithPathIFN(pkProjPath, fileSystem, rootPath, outLibraryPath, assetProcessor);
	}

}
//----------------------------------------------------------------------------

#endif //O3DE_USE_PK

