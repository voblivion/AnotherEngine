#include <vob/aoe/rendering/RenderMeshSystem.h>

#include <vob/aoe/rendering/CameraUtils.h>
#include <vob/aoe/rendering/UniformUtils.h>
#include <vob/aoe/rendering/ProgramUtils.h>

#include <glm/gtx/quaternion.hpp>

namespace
{
	const char* depthPassVertexShaderSource = R"(
		#version 450
		layout(location = 0) in vec3 aPosition;
	
		uniform mat4 uViewProjection;
		uniform mat4 uModel;

		void main()
		{
			gl_Position = uViewProjection * uModel * vec4(aPosition, 1.0);
		}
	)";

	const char* depthPassFragmentShaderSource = R"(
		#version 450
	
		void main() {}
	)";
}

namespace vob::aoegl
{
	struct alignas(16) ModelMeshRenderingData
	{
		alignas(16) glm::mat4 model;
	};

	void RenderMeshSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_windowContext.init(a_wdar);
		m_cameraDirectorContext.init(a_wdar);
		m_meshRenderingContext.init(a_wdar);
		m_cameraEntities.init(a_wdar);
		m_lightEntities.init(a_wdar);
		m_staticMeshEntities.init(a_wdar);

		m_depthPassProgram = createProgram(depthPassVertexShaderSource, depthPassFragmentShaderSource);
		m_uViewProjectionLoc = glGetUniformLocation(m_depthPassProgram, "uViewProjection");
		m_uModelLoc = glGetUniformLocation(m_depthPassProgram, "uModel");

		glCreateBuffers(1, &m_globalMeshRenderingDataUbo);
		glBindBuffer(GL_UNIFORM_BUFFER, m_globalMeshRenderingDataUbo);
		glNamedBufferStorage(
			m_globalMeshRenderingDataUbo,
			sizeof(GlobalMeshRenderingData),
			nullptr,
			GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

		glCreateBuffers(1, &m_modelMeshRenderingDataUbo);
		glBindBuffer(GL_UNIFORM_BUFFER, m_modelMeshRenderingDataUbo);
		glNamedBufferStorage(
			m_modelMeshRenderingDataUbo,
			sizeof(ModelMeshRenderingData),
			nullptr,
			GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
	}

	void RenderMeshSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto cameraEntities = m_cameraEntities.get(a_wdap);
		auto& cameraDirectorCtx = m_cameraDirectorContext.get(a_wdap);
		if (!cameraEntities.contains(cameraDirectorCtx.activeCameraEntity))
		{
			return;
		}

		// 0. Collect camera properties
		auto const windowSize = m_windowContext.get(a_wdap).window.get().getSize();
		auto const aspectRatio = static_cast<float>(windowSize.x) / windowSize.y;
		auto [cameraPos, cameraRot, cameraCmp] = cameraEntities.get(cameraDirectorCtx.activeCameraEntity);
		auto const cameraFrustumPlanes = computeCameraFrustumPlanes(cameraPos, cameraRot, cameraCmp, aspectRatio);
		auto const viewProjection =
			glm::perspective(cameraCmp.fov, aspectRatio, cameraCmp.nearClip, cameraCmp.farClip)
			* glm::inverse(aoest::combine(cameraPos, cameraRot));
		auto const globalMeshRenderingData = GlobalMeshRenderingData{
			.viewProjection = viewProjection,
			.cameraPosition = cameraPos,
			.clusterSizes = glm::ivec2{ 16, 16 },
			.resolution = windowSize,
			.near = cameraCmp.nearClip,
			.far = cameraCmp.farClip,
			.clusterCountZ = 24,
			.maxClusterLightCount = 64
		};
		glNamedBufferSubData(
			m_globalMeshRenderingDataUbo, 0, sizeof(globalMeshRenderingData), &globalMeshRenderingData);

		// 1. Prepare culled draw calls
		auto staticMeshEntities = m_staticMeshEntities.get(a_wdap);
		for (auto const [entity, position, rotation, staticMeshCmp] : staticMeshEntities.each())
		{
			if (!testSphereCameraFrustumPlanesIntersection(cameraFrustumPlanes, position, staticMeshCmp.boundingRadius))
			{
				continue;
			}

			for (auto const& part : staticMeshCmp.parts)
			{
				m_staticMeshes.emplace_back(
					part.materialProgram,
					part.instanceUbo,
					part.meshVao,
					part.indexCount,
					aoest::combine(position, rotation));
			}
		}

		// 2. Sort draw calls
		std::sort(
			m_staticMeshes.begin(),
			m_staticMeshes.end(),
			[](auto const& a_lhs, auto const& a_rhs) {
				return a_lhs.materialProgram < a_rhs.materialProgram
					|| (a_lhs.materialProgram == a_rhs.materialProgram && a_lhs.instanceUbo < a_rhs.instanceUbo);
			});

		// 3. Depth pre-pass
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glUseProgram(m_depthPassProgram);
		glUniformMatrix4fv(m_uViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(viewProjection));
		for (auto const& staticMesh : m_staticMeshes)
		{
			glUniformMatrix4fv(m_uModelLoc, 1, GL_FALSE, glm::value_ptr(staticMesh.transform));
			glBindVertexArray(staticMesh.meshVao);
			glDrawElements(GL_TRIANGLES, staticMesh.indexCount, GL_UNSIGNED_INT, nullptr);
		}

		// 4. TODO: Build light clusters
		auto lightEntities = m_lightEntities.get(a_wdap);
		for (auto const [entity, position, rotation, lightCmp] : lightEntities.each())
		{
			if (!testSphereCameraFrustumPlanesIntersection(cameraFrustumPlanes, position, lightCmp.radius))
			{
				continue;
			}

			auto const type = lightCmp.type == LightComponent::Type::Point ? 0.0f : 1.0f;
			m_lights.emplace_back(
				glm::vec4{ position, lightCmp.radius },
				glm::vec4{ lightCmp.color, lightCmp.intensity },
				glm::vec4{ glm::rotate(rotation, glm::vec3{0.0f, 0.0f, -1.0f}), type },
				glm::vec4{ std::cos(lightCmp.outerAngle), std::cos(lightCmp.innerAngle), 0.0f, 0.0f }
			);
		}
		auto& meshRenderingCtx = m_meshRenderingContext.get(a_wdap);
		glNamedBufferSubData(
			meshRenderingCtx.lightsSsbo,
			0,
			m_lights.size() * sizeof(aoegl::CulledLight),
			m_lights.data());

		auto const globalLightClusteringData = GlobalLightClusteringData{
			.view = glm::inverse(aoest::combine(cameraPos, cameraRot)),
			.projection = glm::inverse(glm::perspective(cameraCmp.fov, aspectRatio, cameraCmp.nearClip, cameraCmp.farClip)),
			.clusterSizes = glm::ivec2{16, 16},
			.resolution = glm::vec2{ windowSize.x, windowSize.y },
			.near = cameraCmp.nearClip,
			.far = cameraCmp.farClip,
			.clusterCountZ = 24,
			.maxClusterLightCount = 64,
			.lightCount = static_cast<int32_t>(m_lights.size())
		};
		glNamedBufferSubData(
			meshRenderingCtx.globalClusteringDataUbo, 0, sizeof(globalLightClusteringData), &globalLightClusteringData);

		glUseProgram(meshRenderingCtx.lightClusteringProgram);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, meshRenderingCtx.globalClusteringDataUbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, meshRenderingCtx.lightsSsbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, meshRenderingCtx.clustersLightCountSsbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, meshRenderingCtx.clustersLightIndicesSsbo);

		uint32_t workGroups = (meshRenderingCtx.clusterCount + 128 - 1) / 128;
		glDispatchCompute(workGroups, 1, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		// 5. Opaque pass
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_EQUAL);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDisable(GL_BLEND);
		GraphicId currentMaterialProgram = k_invalidId;
		GraphicId currentInstanceUbo = k_invalidId;
		for (auto const& staticMesh : m_staticMeshes)
		{
			if (currentMaterialProgram != staticMesh.materialProgram)
			{
				currentMaterialProgram = staticMesh.materialProgram;
				glUseProgram(currentMaterialProgram);
				glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_globalMeshRenderingDataUbo);
				glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_modelMeshRenderingDataUbo);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, meshRenderingCtx.lightsSsbo);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, meshRenderingCtx.clustersLightCountSsbo);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, meshRenderingCtx.clustersLightIndicesSsbo);
				currentInstanceUbo = k_invalidId;
			}

			if (currentInstanceUbo != staticMesh.instanceUbo)
			{
				currentInstanceUbo = staticMesh.instanceUbo;
				glBindBufferBase(GL_UNIFORM_BUFFER, 1, currentInstanceUbo);
			}

			auto const meshRenderingData = ModelMeshRenderingData{
				.model = staticMesh.transform
			};

			glNamedBufferSubData(
				m_modelMeshRenderingDataUbo, 0, sizeof(meshRenderingData), &meshRenderingData);
			glBindVertexArray(staticMesh.meshVao);
			glDrawElements(GL_TRIANGLES, staticMesh.indexCount, GL_UNSIGNED_INT, nullptr);
		}

		// 6. TODO: Transparent pass

		// 7. TODO: Skybox pass

		m_staticMeshes.clear();
		m_lights.clear();
	}
}
