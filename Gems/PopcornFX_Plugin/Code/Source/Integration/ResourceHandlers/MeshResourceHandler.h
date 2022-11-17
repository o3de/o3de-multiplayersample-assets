//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once

#include "Integration/PopcornFXIntegrationBus.h"

#include <pk_kernel/include/kr_resources.h>
#include <pk_geometrics/include/ge_mesh_resource.h>
#include <Atom/RHI.Reflect/Format.h>

namespace PopcornFX {

class CMeshResourceHandler : public IResourceHandler
{
public:
	CMeshResourceHandler();
	virtual ~CMeshResourceHandler();

	virtual void	*Load(	const CResourceManager	*resourceManager,
							u32						resourceTypeID,
							const CString			&resourcePath,
							bool					pathNotVirtual,
							const SResourceLoadCtl	&loadCtl,
							CMessageStream			&loadReport,
							SAsyncLoadStatus		*asyncLoadStatus) override;

	virtual void	*Load(	const CResourceManager	*resourceManager,
							u32						resourceTypeID,
							const CFilePackPath		&resourcePath,
							const SResourceLoadCtl	&loadCtl,
							CMessageStream			&loadReport,
							SAsyncLoadStatus		*asyncLoadStatus) override;

	virtual void	Unload( const CResourceManager	*resourceManager,
							u32						 resourceTypeID,		// used to check we are called with the correct type
							void					*resource) override;

	virtual void	AppendDependencies(	const CResourceManager		*resourceManager,
										u32							resourceTypeID,
										void						*resource,
										PopcornFX::TArray<CString>	&outResourcePaths) const override;

	virtual void	AppendDependencies(	const CResourceManager		*resourceManager,
										u32							resourceTypeID,
										const CString				&resourcePath,
										bool						pathNotVirtual,
										PopcornFX::TArray<CString>	&outResourcePaths) const override;

	virtual void	AppendDependencies(	const CResourceManager		*resourceManager,
										u32							resourceTypeID,
										const CFilePackPath			&resourcePath,
										PopcornFX::TArray<CString>	&outResourcePaths) const override;

	virtual void	BroadcastResourceChanged(const CResourceManager *resourceManager, const CFilePackPath &resourcePath) override;

private:
	Threads::CCriticalSection	m_Lock;

	struct	SMeshResource
	{
		PResourceMesh	m_Mesh;
		u32				m_RefCount;
	};

	THashMap<SMeshResource, AZ::Data::AssetId> m_AssetIdToMesh;
	THashMap<AZ::Data::AssetId, CResourceMesh*> m_MeshToAssetId;
};

}
