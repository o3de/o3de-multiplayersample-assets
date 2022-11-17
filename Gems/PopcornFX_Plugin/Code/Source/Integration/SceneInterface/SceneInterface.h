//----------------------------------------------------------------------------
// Copyright Persistant Studios, SARL. All Rights Reserved.
// https://www.popcornfx.com/terms-and-conditions/
//----------------------------------------------------------------------------

#pragma once
#if defined(O3DE_USE_PK)

#include <pk_particles/include/ps_scene.h>

#define RESOLVE_MATERIAL_PROPERTIES 0
#define RESOLVE_CONTACT_OBJECT 0

namespace PopcornFX {

	class	CSceneInterface : public IParticleScene
	{
	public:
		virtual void	RayTracePacket(	const Colliders::STraceFilter	&traceFilter,
										const Colliders::SRayPacket		&packet,
										const Colliders::STracePacket	&results) override;

#if RESOLVE_MATERIAL_PROPERTIES
		virtual void	ResolveContactMaterials(const TMemoryView<void * const>					&contactObjects,
												const TMemoryView<void * const>					&contactSurfaces,
												const TMemoryView<Colliders::SSurfaceProperties>&outSurfaceProperties) const override;
#endif

		void	SetInGameMode(bool inGameMode) { m_InGameMode = inGameMode; }

	private:
		bool	m_InGameMode = false;
	};

}

#endif //O3DE_USE_PK
