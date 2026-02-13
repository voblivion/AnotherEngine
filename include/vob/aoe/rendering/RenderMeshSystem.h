#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/CameraComponent.h>
#include <vob/aoe/rendering/CameraDirectorContext.h>
#include <vob/aoe/rendering/LightComponent.h>
#include <vob/aoe/rendering/MeshRenderingContext.h>
#include <vob/aoe/rendering/StaticMeshComponent.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>
#include <vob/aoe/spacetime/Transform.h>
#include <vob/aoe/window/WindowContext.h>


namespace vob::aoegl
{
	struct CulledStaticMesh
	{
		GraphicId materialProgram;
		GraphicId instanceUbo;
		GraphicId meshVao;
		int32_t indexCount;
		glm::mat4 transform;
	};

	struct alignas(16) CulledLight
	{
		glm::vec4 positionAndRadius;
		glm::vec4 colorAndIntensity;
		glm::vec4 directionAndType;
		glm::vec4 spotCosAngles;
	};

	class VOB_AOE_API RenderMeshSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);
		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<aoewi::WindowContext> m_windowContext;
		aoeng::EcsWorldContextRef<CameraDirectorContext> m_cameraDirectorContext;
		aoeng::EcsWorldContextRef<MeshRenderingContext> m_meshRenderingContext;

		aoeng::EcsWorldViewRef<aoest::Position const, aoest::Rotation const, CameraComponent const> m_cameraEntities;
		aoeng::EcsWorldViewRef<aoest::Position const, aoest::Rotation const, LightComponent const> m_lightEntities;
		aoeng::EcsWorldViewRef<aoest::Position const, aoest::Rotation const, StaticMeshComponent const> m_staticMeshEntities;

		// TODO: doesn't belong here
		GraphicId m_depthPassProgram = k_invalidId;
		GraphicInt m_uViewProjectionLoc = k_invalidUniformLocation;
		GraphicInt m_uModelLoc = k_invalidUniformLocation;
		GraphicId m_globalMeshRenderingDataUbo = k_invalidId;
		GraphicId m_modelMeshRenderingDataUbo = k_invalidId;
		mutable std::vector<CulledStaticMesh> m_staticMeshes;
		mutable std::vector<CulledLight> m_lights;
	};
}
