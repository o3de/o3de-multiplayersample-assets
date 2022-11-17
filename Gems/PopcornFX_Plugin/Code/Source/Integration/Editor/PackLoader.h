//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include <AzCore/std/string/string.h>

#if defined(O3DE_USE_PK)

namespace PopcornFX {
	void	GetThumbnailPathForAsset(const AZStd::string &projectFilePath, AZStd::string &outRootDir, AZStd::string &outThumbnailsPath);
	bool	ChangePackIFN(	const AZStd::string &assetPath, IFileSystem *fileSystem,
							AZStd::string &rootPath, AZStd::string &outLibraryPath, bool assetProcessor = true);
}

#endif //O3DE_USE_PK
