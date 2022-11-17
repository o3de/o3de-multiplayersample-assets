//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#if defined(O3DE_USE_PK)

#include <AzCore/IO/FileIO.h>
#include <pk_kernel/include/kr_file_controller.h>

namespace PopcornFX {

class FileSystemController_O3DE;

class	CFileStreamFS_O3DE : public CFileStream
{
private:
	CFileStreamFS_O3DE(
		FileSystemController_O3DE	*controller,
		PFilePack					pack,
		const CString				&path,
		IFileSystem::EAccessPolicy	mode,
		AZ::IO::HandleType			file);

public:
	virtual ~CFileStreamFS_O3DE();

	static CFileStreamFS_O3DE	*Open(FileSystemController_O3DE *controller, PFilePack pack, const CString &path, IFileSystem::EAccessPolicy mode);

	virtual u64		Read(void *targetBuffer, u64 byteCount) override;
	virtual u64		Write(const void *sourceBuffer, u64 byteCount) override;
	virtual bool	Seek(s64 offset, ESeekMode whence) override;
	virtual u64		Tell() const override;
	virtual bool	Eof() const override;
	virtual bool	Flush() override;
	virtual void	Close() override;
	virtual u64		SizeInBytes() const override;
	virtual void	Timestamps(SFileTimes &timestamps) override;

private:
	AZ::IO::HandleType			m_File;
};
PK_DECLARE_REFPTRCLASS(FileStreamFS_O3DE);

class	FileSystemController_O3DE : public PopcornFX::CFileSystemBase
{
public:
	FileSystemController_O3DE() {}
	~FileSystemController_O3DE() {}

	virtual PopcornFX::PFileStream	OpenStream(const PopcornFX::CString &path, PopcornFX::IFileSystem::EAccessPolicy mode, bool pathNotVirtual) override;
	virtual bool					Exists(const PopcornFX::CString &path, bool pathNotVirtual = false) override;
	virtual bool					FileDelete(const PopcornFX::CString &path, bool pathNotVirtual = false) override;
	virtual bool					FileCopy(const PopcornFX::CString &sourcePath, const PopcornFX::CString &targetPath, bool pathsNotVirtual = false) override;
	virtual void					GetDirectoryContents(char *dpath, char *virtualPath, u32 pathLength, PopcornFX::CFileDirectoryWalker *walker, const PopcornFX::CFilePack *pack) override;
	virtual bool					CreateDirectoryChainIFN(const PopcornFX::CString &directoryPath, bool pathNotVirtual = false) override;
	virtual bool					DirectoryDelete(const PopcornFX::CString &path, bool pathNotVirtual = false);
};

}

#endif //O3DE_USE_PK
