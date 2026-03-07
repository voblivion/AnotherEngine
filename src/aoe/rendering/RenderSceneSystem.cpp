#include <vob/aoe/rendering/RenderSceneSystem.h>

#include <vob/aoe/rendering/GpuObjects.h>
#include <vob/aoe/rendering/ModelComponent.h>
#include <vob/aoe/rendering/ProgramUtils.h>
#include <vob/aoe/rendering/UniformUtils.h>

#include "vob/aoe/debug/DebugNameUtils.h"

#include <vob/misc/std/enum_traits.h>

#include <glm/gtx/quaternion.hpp>
#include <imgui.h>

#include <array>


const char* debugVertexShaderSource = R"(
#version 450
layout(location = 0) in vec3 aPosition;

out vec2 TexCoords;

void main()
{
	gl_Position = vec4(aPosition, 1.0);
	TexCoords = aPosition.xy * 0.5 + 0.5;
}
)";

const char* debugFragmentShaderSource = R"(
#version 450

out vec4 FragColor;
in vec2 TexCoords;

layout(binding = 0) uniform sampler2D uColorTexture;
layout(binding = 1) uniform sampler2D uDepthTexture;
uniform int uDebugMode = 0;
uniform float uNear = 0.1;
uniform float uFar = 100.0;

void main()
{
	vec4 color = texture(uColorTexture, TexCoords);
	float depth = texture(uDepthTexture, TexCoords).r;
	float z = depth * 2.0 - 1.0;
	float linearDepth = 2.0 * uNear / (uFar + uNear - z * (uFar - uNear));

	if (uDebugMode == 0)
	{
		FragColor = color;
	}
	else if (uDebugMode == 1)
	{
		FragColor = vec4(vec3(linearDepth), 1.0);
	}
}
)";

namespace vob::aoegl
{
	struct CameraProperties
	{
		glm::vec3 position;
		glm::quat rotation;
		float nearClip;
		float farClip;
		float fov;
	};

	template <typename TCameraView>
	CameraProperties getCameraProperties(
		entt::entity a_camera,
		TCameraView const& a_cameraEntities)
	{
		if (!a_cameraEntities.contains(a_camera))
		{
			return CameraProperties{
				.position = glm::vec3{0.0f},
				.rotation = glm::quat{},
				.nearClip = 0.1f,
				.farClip = 1000.0f,
				.fov = 70.0f * std::numbers::pi_v<float> / 360.0f
			};
		}

		auto const [position, rotation, cameraCmp] = a_cameraEntities.get(a_camera);
		return CameraProperties{
			.position = position,
			.rotation = rotation,
			.nearClip = cameraCmp.nearClip,
			.farClip = cameraCmp.farClip,
			.fov = cameraCmp.fov
		};
	}

	std::array<std::pair<glm::vec3, float>, 6> computeCameraFrustumPlanes(
		glm::vec3 const& a_position,
		glm::quat const& a_rotation,
		float a_nearClip,
		float a_farClip,
		float a_fov,
		float a_aspectRatio)
	{
		auto const cameraForward = a_rotation * glm::vec3{ 0.0f, 0.0f, -1.0f };
		auto const cameraRight = a_rotation * glm::vec3{ 1.0f, 0.0f, 0.0f };
		auto const cameraUp = a_rotation * glm::vec3{ 0.0f, 1.0f, 0.0f };
		auto const nearCenter = a_position + cameraForward * a_nearClip;
		auto const farCenter = a_position + cameraForward * a_farClip;
		auto const tanHalfFov = std::tan(a_fov * 0.5f);
		auto const nearHalfHeight = a_nearClip * tanHalfFov;
		auto const nearHalfWidth = nearHalfHeight * a_aspectRatio;
		auto const farHalfHeight = a_farClip * tanHalfFov;
		auto const farHalfWidth = farHalfHeight * a_aspectRatio;

		auto const leftPlaneNormal = glm::normalize(glm::cross(nearCenter - cameraRight * nearHalfWidth - a_position, cameraUp));
		auto const rightPlaneNormal = glm::normalize(glm::cross(cameraUp, nearCenter + cameraRight * nearHalfWidth - a_position));
		auto const upPlaneNormal = glm::normalize(glm::cross(nearCenter + cameraUp * nearHalfHeight - a_position, cameraRight));
		auto const downPlaneNormal = glm::normalize(glm::cross(cameraRight, nearCenter - cameraUp * nearHalfHeight - a_position));

		return std::array{
			std::pair{cameraForward, -glm::dot(cameraForward, nearCenter)},
			std::pair{-cameraForward, -glm::dot(-cameraForward, farCenter)},
			std::pair{leftPlaneNormal, -glm::dot(leftPlaneNormal, a_position)},
			std::pair{rightPlaneNormal, -glm::dot(rightPlaneNormal, a_position)},
			std::pair{upPlaneNormal, -glm::dot(upPlaneNormal, a_position)},
			std::pair{downPlaneNormal, -glm::dot(downPlaneNormal, a_position)}
		};
	}


	bool testCameraFrustumIntersect(
		std::array<std::pair<glm::vec3, float>, 6> const& a_cameraFrustumPlanes,
		glm::vec3 const& a_position,
		float a_radius)
	{
		for (auto const& plane : a_cameraFrustumPlanes)
		{
			if (glm::dot(plane.first, a_position) + plane.second < -a_radius)
			{
				return false;
			}
		}

		return true;
	}

	struct CulledStaticMesh
	{
		GraphicId materialProgram;
		GraphicId instanceUbo;
		GraphicId meshVao;
		int32_t indexCount;
		glm::mat4 transform;
	};

	struct CulledStaticMesh2
	{
		GraphicId forwardProgram;
		GraphicId materialParamsUbo;
		GraphicId meshVao;
		GraphicId modelParamsUbo;
		int32_t indexCount;
	};

	struct CulledRiggedMesh
	{
		GraphicId forwardProgram;
		GraphicId materialParamsUbo;
		GraphicId meshVao;
		GraphicId modelParamsUbo;
		GraphicId rigParamsUbo;
		int32_t indexCount;
	};

	void RenderSceneSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{

	}

	void RenderSceneSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		// TODO: where do these go?
		constexpr int32_t kLightClusterWorkGroupSize = 128;
		constexpr GraphicInt k_viewPostProcessConfigUboLocation = 0;
		constexpr GraphicInt k_lightPostProcessConfigUboLocation = 1;
		constexpr GraphicInt k_customPostProcessConfigUboLocation = 2;

		auto const& renderSceneContext = m_renderSceneContext.get(a_wdap);
		auto const& window = m_windowContext.get(a_wdap).window.get();
		auto& cameraDirectorContext = m_cameraDirectorContext.get(a_wdap);
		auto const cameraEntities = m_cameraEntities.get(a_wdap);

#ifdef VOB_AOEGL_DEBUG
		// O PTICK_PUSH("0 - Debug");
		static DebugMode::Type k_debugMode = DebugMode::None;
		if (ImGui::Begin("Render"))
		{
			auto const activeDebugModeName = mistd::enum_traits<DebugMode::Type>::cast(k_debugMode).value_or("None");
			auto const activeDebugModeStr = std::string{ activeDebugModeName.substr(activeDebugModeName.rfind(":") + 1) };
			if (ImGui::BeginCombo("Debug Mode", activeDebugModeStr.c_str()))
			{
				for (auto const [debugMode, debugModeName] : mistd::enum_traits<DebugMode::Type>::valid_value_name_pairs)
				{
					auto const optionLabel = std::string{ debugModeName.substr(debugModeName.rfind(":") + 1) };
					if (ImGui::Selectable(optionLabel.c_str(), debugMode == k_debugMode))
					{
						k_debugMode = debugMode;
					}

					if (debugMode == k_debugMode)
					{
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}

			ImGui::End();
		}

		if (k_debugMode != DebugMode::None)
		{
			auto const debugParams = DebugParams{
				.mode = k_debugMode
			};
			glNamedBufferSubData(renderSceneContext.debugParamsUbo, 0, sizeof(debugParams), &debugParams);

		}
		// O PTICK_POP();
#endif

		// TODO: separate window size from rendering size
		auto const windowSize = window.getSize();
		auto const aspectRatio = static_cast<float>(windowSize.x) / windowSize.y;
		auto const cameraProperties = getCameraProperties(cameraDirectorContext.activeCameraEntity, cameraEntities);
		auto const cameraTransform = aoest::combine(cameraProperties.position, cameraProperties.rotation);
		auto const cameraFrustumPlanes = computeCameraFrustumPlanes(
			cameraProperties.position,
			cameraProperties.rotation,
			cameraProperties.nearClip,
			cameraProperties.farClip,
			cameraProperties.fov,
			aspectRatio);
		auto const view = glm::inverse(cameraTransform);
		auto const projection = glm::perspective(cameraProperties.fov, aspectRatio, cameraProperties.nearClip, cameraProperties.farClip);
		auto const viewProjection = projection * view;
		auto const invProjection = glm::inverse(projection);

		// 1. Cull lights
		// O PTICK_PUSH("1 - Light Culling");
		// TODO: why magic?
		static std::pmr::vector<Light> culledLights;
		culledLights.clear();
		auto const lightEntities = m_lightEntities.get(a_wdap);
		for (auto const [entity, position, rotation, lightCmp] : lightEntities.each())
		{
			if (testCameraFrustumIntersect(cameraFrustumPlanes, position, lightCmp.radius))
			{
				auto const type = lightCmp.type == LightComponent::Type::Point ? 0.0f : 1.0f;
				culledLights.emplace_back(
					glm::vec4{ position, lightCmp.radius },
					glm::vec4{ lightCmp.color, lightCmp.intensity },
					glm::vec4{ glm::rotate(rotation, glm::vec3{0.0f, 0.0f, -1.0f}), type },
					glm::vec4{ std::cos(lightCmp.outerAngle), std::cos(lightCmp.innerAngle), 0.0f, 0.0f });
			}
		}
		glNamedBufferSubData(renderSceneContext.lightsSsbo, 0, culledLights.size() * sizeof(Light), culledLights.data());
		// O PTICK_POP();

		// 2. Set global scene rendering config
		// O PTICK_PUSH("2 - Global UBO");
		auto const viewParams = ViewParams{
			.worldToView = view,
			.viewToProjected = projection,
			.worldToProjected = viewProjection,
			.projectedToView = invProjection,
			.viewPosition = cameraProperties.position
		};
		glNamedBufferSubData(renderSceneContext.viewParamsUbo, 0, sizeof(ViewParams), &viewParams);
		auto const lightParams = LightParams{
			.lightClusterSizes = glm::ivec2{16},
			.resolution = renderSceneContext.sceneFramebufferSize,
			.near = cameraProperties.nearClip,
			.far = cameraProperties.farClip,
			.lightClusterCountZ = 24,
			.maxLightCountPerCluster = 64,
			.totalLightCount = static_cast<int32_t>(culledLights.size())
		};
		glNamedBufferSubData(renderSceneContext.lightParamsUbo, 0, sizeof(LightParams), &lightParams);
		// O PTICK_POP();

		// 3. Compute light clusters
		// O PTICK_PUSH("3 - Light Clustering");
		glBeginQuery(GL_TIME_ELAPSED, renderSceneContext.timerQueries[0]);
		glUseProgram(renderSceneContext.lightClusteringProgram);
		glBindBufferBase(GL_UNIFORM_BUFFER, k_viewParamsUboLocation, renderSceneContext.viewParamsUbo);
		glBindBufferBase(GL_UNIFORM_BUFFER, k_lightParamsUboLocation, renderSceneContext.lightParamsUbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, k_lightsSsboLocation, renderSceneContext.lightsSsbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, k_lightClusterSizesSsboLocation, renderSceneContext.lightClusterSizesSsbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, k_lightClusterIndicesSsboLocation, renderSceneContext.lightClusterIndicesSsbo);
		auto const workGroupCount = static_cast<uint32_t>(
			(renderSceneContext.lightClusterCount + renderSceneContext.lightClusteringWorkGroupSize - 1) / renderSceneContext.lightClusteringWorkGroupSize);
		glDispatchCompute(workGroupCount, 1, 1);
		// O PTICK_POP();

		// 4. Cull meshes
		// O PTICK_PUSH("4 - Mesh Culling");
		static std::pmr::vector<CulledStaticMesh2> culledStaticMeshes;
		culledStaticMeshes.clear();
		auto const staticModelEntities = m_staticModelEntities.get(a_wdap);
		for (auto const [entity, position, rotation, staticModelCmp] : staticModelEntities.each())
		{
			if (testCameraFrustumIntersect(cameraFrustumPlanes, position, staticModelCmp.boundingRadius))
			{
				// TODO: don't need for static, do elsewhere
				auto const modelParams = ModelParams{ .modelToWorld = aoest::combine(position, rotation) };
				glNamedBufferSubData(staticModelCmp.modelParamsUbo, 0, sizeof(ModelParams), &modelParams);

				for (auto const& mesh : staticModelCmp.meshes)
				{
					culledStaticMeshes.emplace_back(
						mesh.program,
						mesh.materialParamsUbo,
						mesh.meshVao,
						staticModelCmp.modelParamsUbo,
						mesh.indexCount);
				}
			}
		}
		std::sort(
			culledStaticMeshes.begin(),
			culledStaticMeshes.end(),
			[](auto const& a_lhs, auto const& a_rhs)
			{
				return a_lhs.forwardProgram < a_rhs.forwardProgram
					|| (a_lhs.forwardProgram == a_rhs.forwardProgram && a_lhs.materialParamsUbo < a_rhs.materialParamsUbo);
			});
		static std::pmr::vector<CulledRiggedMesh> culledRiggedMeshes;
		culledRiggedMeshes.clear();
		auto const riggedModelEntities = m_riggedModelEntities.get(a_wdap);
		for (auto const [entity, position, rotation, riggedModelCmp] : riggedModelEntities.each())
		{
			if (testCameraFrustumIntersect(cameraFrustumPlanes, position, riggedModelCmp.boundingRadius))
			{
				// TODO: don't need for static, do elsewhere
				auto const modelParams = ModelParams{ .modelToWorld = aoest::combine(position, rotation) };
				glNamedBufferSubData(riggedModelCmp.modelParamsUbo, 0, sizeof(ModelParams), &modelParams);

				for (auto const& mesh : riggedModelCmp.meshes)
				{
					culledRiggedMeshes.emplace_back(
						mesh.program,
						mesh.materialParamsUbo,
						mesh.meshVao,
						riggedModelCmp.modelParamsUbo,
						riggedModelCmp.rigParamsUbo,
						mesh.indexCount);
				}
			}
		}
		std::sort(
			culledRiggedMeshes.begin(),
			culledRiggedMeshes.end(),
			[](auto const& a_lhs, auto const& a_rhs)
			{
				return a_lhs.forwardProgram < a_rhs.forwardProgram
					|| (a_lhs.forwardProgram == a_rhs.forwardProgram && a_lhs.materialParamsUbo < a_rhs.materialParamsUbo);
			});
		// O PTICK_POP();

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		glEndQuery(GL_TIME_ELAPSED);

		// 5. Depth pre-pass
		// O PTICK_PUSH("5 - Depth Pre Pass");
		glBeginQuery(GL_TIME_ELAPSED, renderSceneContext.timerQueries[1]);
		glBindFramebuffer(GL_FRAMEBUFFER, renderSceneContext.sceneFramebuffer);
		glClearColor(k_blueprint.r, k_blueprint.g, k_blueprint.b, k_blueprint.a);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
#ifdef VOB_AOEGL_DEBUG
		if (k_debugMode != DebugMode::DebugOnly)
#endif
		{
			glUseProgram(renderSceneContext.staticDepthProgram);
			glBindBufferBase(GL_UNIFORM_BUFFER, k_viewParamsUboLocation, renderSceneContext.viewParamsUbo);
			for (auto const& staticMesh : culledStaticMeshes)
			{
				glBindBufferBase(GL_UNIFORM_BUFFER, k_modelParamsUboLocation, staticMesh.modelParamsUbo);
				glBindVertexArray(staticMesh.meshVao);
				glDrawElements(GL_TRIANGLES, staticMesh.indexCount, GL_UNSIGNED_INT, nullptr);
			}
			glUseProgram(renderSceneContext.riggedDepthProgram);
			glBindBufferBase(GL_UNIFORM_BUFFER, k_viewParamsUboLocation, renderSceneContext.viewParamsUbo);
			for (auto const& riggedMesh : culledRiggedMeshes)
			{
				glBindBufferBase(GL_UNIFORM_BUFFER, k_modelParamsUboLocation, riggedMesh.modelParamsUbo);
				glBindBufferBase(GL_UNIFORM_BUFFER, k_rigParamsUboLocation, riggedMesh.rigParamsUbo);
				glBindVertexArray(riggedMesh.meshVao);
				glDrawElements(GL_TRIANGLES, riggedMesh.indexCount, GL_UNSIGNED_INT, nullptr);
			}
		}
		/*glBindBufferBase(GL_UNIFORM_BUFFER, k_globalRenderSceneConfigUboLocation, renderSceneContext.globalUbo);
		glBindBufferBase(GL_UNIFORM_BUFFER, k_meshRenderSceneConfigUboLocation, renderSceneContext.meshUbo);
		for (auto const& staticMesh : culledStaticMeshes)
		{
			auto const meshRenderSceneConfig = MeshRenderSceneConfig{
				.model = staticMesh.transform
			};
			glNamedBufferSubData(renderSceneContext.meshUbo, 0, sizeof(meshRenderSceneConfig), &meshRenderSceneConfig);

			glBindVertexArray(staticMesh.meshVao);
			glDrawElements(GL_TRIANGLES, staticMesh.indexCount, GL_UNSIGNED_INT, nullptr);
		}*/
		// NEW
		// TODO
		glEndQuery(GL_TIME_ELAPSED);
		// O PTICK_POP();

		// 6. Opaque pass
		// O PTICK_PUSH("6 - Opaque Pass");
		glBeginQuery(GL_TIME_ELAPSED, renderSceneContext.timerQueries[2]);
		glDepthFunc(GL_LEQUAL);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		GraphicId currentForwardProgram = k_invalidId;
		GraphicId currentMaterialParamsUbo = k_invalidId;
#ifdef VOB_AOEGL_DEBUG
		if (k_debugMode != DebugMode::DebugOnly)
#endif
		{
#ifdef VOB_AOEGL_DEBUG
			if (k_debugMode != DebugMode::None)
			{
				glUseProgram(renderSceneContext.staticDebugForwardProgram);
				glBindBufferBase(GL_UNIFORM_BUFFER, k_viewParamsUboLocation, renderSceneContext.viewParamsUbo);
				glBindBufferBase(GL_UNIFORM_BUFFER, k_lightParamsUboLocation, renderSceneContext.lightParamsUbo);
				glBindBufferBase(GL_UNIFORM_BUFFER, k_materialParamsUboLocation, renderSceneContext.debugParamsUbo);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, k_lightsSsboLocation, renderSceneContext.lightsSsbo);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, k_lightClusterSizesSsboLocation, renderSceneContext.lightClusterSizesSsbo);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, k_lightClusterIndicesSsboLocation, renderSceneContext.lightClusterIndicesSsbo);
			}
#endif
			for (auto const& staticMesh : culledStaticMeshes)
			{
#ifdef VOB_AOEGL_DEBUG
				if (k_debugMode == DebugMode::None)
#endif
				{
					if (currentForwardProgram != staticMesh.forwardProgram)
					{
						glUseProgram(staticMesh.forwardProgram);
						currentForwardProgram = staticMesh.forwardProgram;
						glBindBufferBase(GL_UNIFORM_BUFFER, k_viewParamsUboLocation, renderSceneContext.viewParamsUbo);
						glBindBufferBase(GL_UNIFORM_BUFFER, k_lightParamsUboLocation, renderSceneContext.lightParamsUbo);
						glBindBufferBase(GL_SHADER_STORAGE_BUFFER, k_lightsSsboLocation, renderSceneContext.lightsSsbo);
						glBindBufferBase(GL_SHADER_STORAGE_BUFFER, k_lightClusterSizesSsboLocation, renderSceneContext.lightClusterSizesSsbo);
						glBindBufferBase(GL_SHADER_STORAGE_BUFFER, k_lightClusterIndicesSsboLocation, renderSceneContext.lightClusterIndicesSsbo);
						currentMaterialParamsUbo = k_invalidId;
					}

					if (currentMaterialParamsUbo != staticMesh.materialParamsUbo)
					{
						glBindBufferBase(GL_UNIFORM_BUFFER, k_materialParamsUboLocation, staticMesh.materialParamsUbo);
						currentMaterialParamsUbo = staticMesh.materialParamsUbo;
					}
				}

				glBindBufferBase(GL_UNIFORM_BUFFER, k_modelParamsUboLocation, staticMesh.modelParamsUbo);
				glBindVertexArray(staticMesh.meshVao);
				glDrawElements(GL_TRIANGLES, staticMesh.indexCount, GL_UNSIGNED_INT, nullptr);
			}
#ifdef VOB_AOEGL_DEBUG
			if (k_debugMode != DebugMode::None)
			{
				glUseProgram(renderSceneContext.riggedDebugForwardProgram);
				glBindBufferBase(GL_UNIFORM_BUFFER, k_viewParamsUboLocation, renderSceneContext.viewParamsUbo);
				glBindBufferBase(GL_UNIFORM_BUFFER, k_lightParamsUboLocation, renderSceneContext.lightParamsUbo);
				glBindBufferBase(GL_UNIFORM_BUFFER, k_materialParamsUboLocation, renderSceneContext.debugParamsUbo);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, k_lightsSsboLocation, renderSceneContext.lightsSsbo);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, k_lightClusterSizesSsboLocation, renderSceneContext.lightClusterSizesSsbo);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, k_lightClusterIndicesSsboLocation, renderSceneContext.lightClusterIndicesSsbo);
			}
#endif
			for (auto const& riggedMesh : culledRiggedMeshes)
			{
#ifdef VOB_AOEGL_DEBUG
				if (k_debugMode == DebugMode::None)
#endif
				{
					if (currentForwardProgram != riggedMesh.forwardProgram)
					{
						glUseProgram(riggedMesh.forwardProgram);
						currentForwardProgram = riggedMesh.forwardProgram;
						glBindBufferBase(GL_UNIFORM_BUFFER, k_viewParamsUboLocation, renderSceneContext.viewParamsUbo);
						glBindBufferBase(GL_UNIFORM_BUFFER, k_lightParamsUboLocation, renderSceneContext.lightParamsUbo);
						glBindBufferBase(GL_SHADER_STORAGE_BUFFER, k_lightsSsboLocation, renderSceneContext.lightsSsbo);
						glBindBufferBase(GL_SHADER_STORAGE_BUFFER, k_lightClusterSizesSsboLocation, renderSceneContext.lightClusterSizesSsbo);
						glBindBufferBase(GL_SHADER_STORAGE_BUFFER, k_lightClusterIndicesSsboLocation, renderSceneContext.lightClusterIndicesSsbo);
						currentMaterialParamsUbo = k_invalidId;
					}

					if (currentMaterialParamsUbo != riggedMesh.materialParamsUbo)
					{
						glBindBufferBase(GL_UNIFORM_BUFFER, k_materialParamsUboLocation, riggedMesh.materialParamsUbo);
						currentMaterialParamsUbo = riggedMesh.materialParamsUbo;
					}
				}

				glBindBufferBase(GL_UNIFORM_BUFFER, k_modelParamsUboLocation, riggedMesh.modelParamsUbo);
				glBindBufferBase(GL_UNIFORM_BUFFER, k_rigParamsUboLocation, riggedMesh.rigParamsUbo);
				glBindVertexArray(riggedMesh.meshVao);
				glDrawElements(GL_TRIANGLES, riggedMesh.indexCount, GL_UNSIGNED_INT, nullptr);
			}
		}
		glEndQuery(GL_TIME_ELAPSED);
		// O PTICK_POP();

		// 7. Transparent pass
		// TODO
		
		// 8. Skybox pass
		// TODO

		// 9. Reflection pass
		// TODO

		// 10. Post process(es)
		// O PTICK_PUSH("10 - Post Process");
		glBeginQuery(GL_TIME_ELAPSED, renderSceneContext.timerQueries[3]);
		// TODO: implement various post process + ping-pong pattern
		glBindFramebuffer(GL_FRAMEBUFFER, window.getDefaultFramebufferId());
		glClearColor(k_black.r, k_black.g, k_black.b, k_black.a);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

#ifdef VOB_AOEGL_DEBUG
		if (k_debugMode != DebugMode::None)
		{
			glUseProgram(renderSceneContext.debugPostProcessProgram);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, renderSceneContext.sceneDepthTexture);
			glBindBufferBase(GL_UNIFORM_BUFFER, k_customPostProcessConfigUboLocation, renderSceneContext.debugParamsUbo);
		}
		else
#endif
		{
			glUseProgram(renderSceneContext.postProcessProgram);
			glBindBufferBase(GL_UNIFORM_BUFFER, k_customPostProcessConfigUboLocation, renderSceneContext.postProcessUbo);
		}
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderSceneContext.sceneColorTexture);
		glBindBufferBase(GL_UNIFORM_BUFFER, k_viewPostProcessConfigUboLocation, renderSceneContext.viewParamsUbo);
		glBindBufferBase(GL_UNIFORM_BUFFER, k_lightPostProcessConfigUboLocation, renderSceneContext.lightParamsUbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, renderSceneContext.lightClusterSizesSsbo);
		glBindVertexArray(renderSceneContext.postProcessVao);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glEndQuery(GL_TIME_ELAPSED);
		// O PTICK_POP();

#ifdef VOB_AOEGL_DEBUG
		if (ImGui::Begin("Render Performance"))
		{
			glFinish();
			static int32_t accumulationCount = 50;
			static std::array<float, 4> durations{ 0.0f };
			static std::array<uint64_t, 4> accumulations{ 0 };
			static int32_t accumulationIndex = 0;
			for (int i = 0; i < 4; ++i)
			{
				uint64_t durationMs;
				glGetQueryObjectui64v(renderSceneContext.timerQueries[i], GL_QUERY_RESULT, &durationMs);
				accumulations[i] += durationMs;
			}
			if (++accumulationIndex == accumulationCount)
			{
				for (int i = 0; i < 4; ++i)
				{
					durations[i] = (accumulations[i] / 1000000.0f) / accumulationCount;
					accumulations[i] = 0;
				}
				accumulationIndex = 0;
			}

			ImGui::BeginDisabled();
			int32_t lightCount = static_cast<int32_t>(culledLights.size());
			ImGui::InputInt("Light Count", &lightCount);
			int32_t staticMeshCount = static_cast<int32_t>(culledStaticMeshes.size());
			ImGui::InputInt("Static Mesh Count", &staticMeshCount);
			ImGui::InputFloat("Light Clustering (ms)", &durations[0]);
			ImGui::InputFloat("Depth Pre Pass (ms)", &durations[1]);
			ImGui::InputFloat("Opaque Pass (ms)", &durations[2]);
			ImGui::InputFloat("Post Process (ms)", &durations[3]);
			ImGui::EndDisabled();

			ImGui::End();
		}
#endif
	}
}
