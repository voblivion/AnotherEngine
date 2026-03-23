#include <vob/aoe/rendering/RenderSceneSystem.h>

#include <vob/aoe/rendering/GpuObjects.h>
#include <vob/aoe/rendering/ModelComponent.h>
#include <vob/aoe/rendering/ProgramUtils.h>
#include <vob/aoe/rendering/UniformUtils.h>

#include "vob/aoe/debug/DebugNameUtils.h"

#include <vob/misc/std/container_util.h>
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
#pragma optimize("", off)
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

	std::array<std::pair<glm::vec3, float>, 6> computeSpotLightFrustumPlanes(
		glm::vec3 const& a_position,
		glm::quat const& a_rotation,
		float a_nearClip,
		float a_farClip,
		float a_outerAngle)
	{
		return computeCameraFrustumPlanes(a_position, a_rotation, a_nearClip, a_farClip, 2.0f * a_outerAngle, 1.0f /* aspect ratio */);
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
		GraphicId forwardProgram;
		int32_t materialIndex;
		GraphicId meshVao;
		GraphicId modelParamsUbo;
		int32_t indexCount;
	};

	struct CulledRiggedMesh
	{
		GraphicId forwardProgram;
		int32_t materialIndex;
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
		constexpr auto k_spotLightNear = 0.01f;

		auto const& timeContext = m_timeContext.get(a_wdap);
		auto const& renderSceneContext = m_renderSceneContext.get(a_wdap);
		auto const& window = m_windowContext.get(a_wdap).window.get();
		auto& cameraDirectorContext = m_cameraDirectorContext.get(a_wdap);
		auto const cameraEntities = m_cameraEntities.get(a_wdap);
		auto const& materialManager = *m_materialManagerContext.get(a_wdap).materialManager;

#define VOB_AOEGL_DEBUG
#ifdef VOB_AOEGL_DEBUG
		static DebugMode::Type k_debugMode = DebugMode::None;
		static int32_t k_debugActiveForwardProgramIndex = 0;
		if (ImGui::Begin("Render"))
		{
			auto const toSmallStr = [](std::string_view a_stringView)
				{
					constexpr size_t k_maxSize = 16;
					auto size = std::min(a_stringView.size(), k_maxSize);
					std::array<char, k_maxSize + 1> smallStr;
					std::memcpy(smallStr.data(), a_stringView.data(), size);
					smallStr[size] = 0;
					return smallStr;
				};

			auto const activeDebugModeName = mistd::enum_traits<DebugMode::Type>::cast(k_debugMode).value_or("None");
			auto const activeDebugModeStr = toSmallStr(activeDebugModeName.substr(activeDebugModeName.rfind(":") + 1));
			if (ImGui::BeginCombo("Debug Mode", activeDebugModeStr.data()))
			{
				for (auto const [debugMode, debugModeName] : mistd::enum_traits<DebugMode::Type>::valid_value_name_pairs)
				{
					auto const optionLabel = toSmallStr(debugModeName.substr(debugModeName.rfind(":") + 1));
					if (ImGui::Selectable(optionLabel.data(), debugMode == k_debugMode))
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

			auto const& debugProgramCtx = m_debugProgramContext.get(a_wdap);
			if (!debugProgramCtx.forwardPrograms.empty())
			{
				k_debugActiveForwardProgramIndex =
					std::min(k_debugActiveForwardProgramIndex, mistd::isize(debugProgramCtx.forwardPrograms) - 1);

				auto const activeForwardProgramNameStr = toSmallStr(debugProgramCtx.forwardPrograms[k_debugActiveForwardProgramIndex].name);
				if (ImGui::BeginCombo("Forward Program", activeForwardProgramNameStr.data()))
				{
					for (int32_t i = 0; i < mistd::isize(debugProgramCtx.forwardPrograms); ++i)
					{
						auto const forwardProgramNameStr = toSmallStr(debugProgramCtx.forwardPrograms[i].name);
						if (ImGui::Selectable(forwardProgramNameStr.data(), i == k_debugActiveForwardProgramIndex))
						{
							k_debugActiveForwardProgramIndex = i;
						}

						if (i == k_debugActiveForwardProgramIndex)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				if (ImGui::Button("Recompile Forward Program"))
				{
					auto const& forwardProgram = debugProgramCtx.forwardPrograms[k_debugActiveForwardProgramIndex];
					auto const shadingSource = debugProgramCtx.stringDatabase.find(
						debugProgramCtx.filesystemIndexer.get_runtime_id(forwardProgram.shadingSourcePath));
					createForwardProgram(*shadingSource, false /* use rig */, forwardProgram.staticProgram);
					createForwardProgram(*shadingSource, true /* use rig */, forwardProgram.riggedProgram);
				}
			}
		}
		ImGui::End();

		if (k_debugMode != DebugMode::None)
		{
			auto const debugParams = DebugParams{
				.mode = k_debugMode
			};
			glNamedBufferSubData(renderSceneContext.debugParamsUbo, 0, sizeof(debugParams), &debugParams);

		}
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
		// BEGIN WIP
		auto const sunPosition = cameraProperties.position * glm::vec3{ 1.0f, 0.0f, 1.0f } + glm::vec3{ 0.0f, 24.0f, 0.0f };
		auto const sunNearPlane = 1.0f;
		auto const sunFarPlane = 128.0f;
		auto const sunProjection = glm::ortho(-32.0f, 32.0f, -16.0f, 48.0f, sunNearPlane, sunFarPlane);
		auto const cameraFacing = cameraProperties.rotation * glm::vec3{ 0.0f, 0.0f, -1.0f };
		auto const sunView = glm::lookAt(sunPosition, cameraProperties.position * glm::vec3{ 1.0f, 0.0f, 1.0f }, cameraFacing);
		auto const sunSpace = sunProjection * sunView;
		// END WIP

		auto const shadowFocusPoint = cameraProperties.position
			+ cameraProperties.rotation * (16.0f * glm::vec3{ 0.0f, 0.0f, -1.0f });
		struct ShadowSpotLight
		{
			int32_t culledLightIndex;
			float importance;
			glm::quat rotation;
			float outerAngle;
			float lightSize;
		};

		// 1. Cull lights
		// TODO: why magic?
		static std::pmr::vector<Light> culledLights;
		static std::pmr::vector<ShadowSpotLight> shadowSpotLights;
		culledLights.clear();
		shadowSpotLights.clear();
		auto const lightEntities = m_lightEntities.get(a_wdap);
		for (auto const [entity, position, rotation, lightCmp] : lightEntities.each())
		{
			if (testCameraFrustumIntersect(cameraFrustumPlanes, position, lightCmp.radius))
			{
				auto const direction = glm::rotate(rotation, glm::vec3{ 0.0f, 0.0f, -1.0f });
				auto const outerCosAngle = std::cos(lightCmp.outerAngle);
				culledLights.emplace_back(
					position,
					lightCmp.radius,
					lightCmp.color,
					lightCmp.intensity,
					direction,
					lightCmp.type == LightComponent::Type::Point ? 0.0f : 1.0f,
					outerCosAngle,
					std::cos(lightCmp.innerAngle),
					-1 /* shadowMapIndex */);

				if (lightCmp.type == LightComponent::Type::Spot)
				{
					// TODO: improve logic for selecting most important spot light
					auto const distance = glm::length(shadowFocusPoint - position);
					auto const distanceFactor = lightCmp.radius / std::max(0.1f, distance);
					auto const colorFactor = glm::dot(lightCmp.color, glm::vec3{ 0.299f, 0.587f, 0.114f });
					auto const intensityFactor = lightCmp.intensity;
					shadowSpotLights.emplace_back(
						mistd::isize(culledLights) - 1, distanceFactor * colorFactor * intensityFactor, rotation, lightCmp.outerAngle, lightCmp.shadowLightSize);
				}
			}
		}
		std::array<ShadowParams, k_maxSpotLightShadowMapCount> spotLightShadowParams;
		std::sort(shadowSpotLights.begin(), shadowSpotLights.end(), [](auto const& lhs, auto const& rhs) { return lhs.importance > rhs.importance; });
		shadowSpotLights.resize(std::min(shadowSpotLights.size(), renderSceneContext.spotShadowMaps.size()));
		for (int32_t i = 0; i < mistd::isize(shadowSpotLights); ++i)
		{
			auto& culledLight = culledLights[shadowSpotLights[i].culledLightIndex];
			culledLight.spotShadowMapIndex = i;

			auto const lightViewToProjected = glm::perspective(2.0f * shadowSpotLights[i].outerAngle, 1.0f, k_spotLightNear, culledLight.radius);
			auto const lightUpHint = shadowSpotLights[i].rotation * glm::vec3{ 0.0f, 1.0f, 0.0f };
			auto const lightWorldToView = glm::lookAt(culledLight.position, culledLight.position + culledLight.direction, lightUpHint);
			auto const lightWorldToProjected = lightViewToProjected * lightWorldToView;
			spotLightShadowParams[i] = ShadowParams{
				.worldToProjected = lightWorldToProjected,
				.nearPlane = k_spotLightNear,
				.farPlane = culledLight.radius,
				.lightSize = shadowSpotLights[i].lightSize
			};
		}
		glNamedBufferSubData(renderSceneContext.lightsSsbo, 0, culledLights.size() * sizeof(Light), culledLights.data());

		// 2. Set global scene rendering config
		auto const globalParams = GlobalParams{
			.worldTime = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - timeContext.worldStartTime).count()
		};
		glNamedBufferSubData(renderSceneContext.globalParamsUbo, 0, sizeof(GlobalParams), &globalParams);
		glBindBufferBase(GL_UNIFORM_BUFFER, k_globalParamsUboLocation, renderSceneContext.globalParamsUbo);

		auto const viewParams = ViewParams{
			.worldToView = view,
			.viewToProjected = projection,
			.worldToProjected = viewProjection,
			.projectedToView = invProjection,
			.viewPosition = cameraProperties.position
		};
		glNamedBufferSubData(renderSceneContext.viewParamsUbo, 0, sizeof(ViewParams), &viewParams);

		auto const lightParams = LightParams{
			.sunShadowParams = { sunSpace, 1.0, sunFarPlane },
			.spotLightShadowParams = spotLightShadowParams,
			.lightClusterSizes = glm::ivec2{ 16 },
			.resolution = renderSceneContext.sceneFramebufferSize,
			.near = cameraProperties.nearClip,
			.far = cameraProperties.farClip,
			.lightClusterCountZ = 24,
			.maxLightCountPerCluster = 64,
			.totalLightCount = mistd::isize(culledLights)
		};
		glNamedBufferSubData(renderSceneContext.lightParamsUbo, 0, sizeof(LightParams), &lightParams);

		// 3. Compute light clusters
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

		// 4. Cull meshes
		static std::pmr::vector<CulledStaticMesh> culledStaticMeshes;
		culledStaticMeshes.clear();
		auto const staticModelEntities = m_staticModelEntities.get(a_wdap);
		for (auto const [entity, position, rotation, staticModelCmp] : staticModelEntities.each())
		{
			if (testCameraFrustumIntersect(cameraFrustumPlanes, position, staticModelCmp.boundingRadius))
			{
				auto const modelParams = ModelParams{ .modelToWorld = aoest::combine(position, rotation) };
				if (modelParams != staticModelCmp.modelParams)
				{
					staticModelCmp.modelParams = modelParams;
					glNamedBufferSubData(staticModelCmp.modelParamsUbo, 0, sizeof(ModelParams), &modelParams);
				}

				for (auto const& mesh : staticModelCmp.meshes)
				{
					culledStaticMeshes.emplace_back(
						mesh.program,
						mesh.materialIndex,
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
					|| (a_lhs.forwardProgram == a_rhs.forwardProgram && a_lhs.materialIndex < a_rhs.materialIndex);
			});
		static std::pmr::vector<CulledRiggedMesh> culledRiggedMeshes;
		culledRiggedMeshes.clear();
		auto const riggedModelEntities = m_riggedModelEntities.get(a_wdap);
		for (auto const [entity, position, rotation, riggedModelCmp] : riggedModelEntities.each())
		{
			if (testCameraFrustumIntersect(cameraFrustumPlanes, position, riggedModelCmp.boundingRadius))
			{
				auto const modelParams = ModelParams{ .modelToWorld = aoest::combine(position, rotation) };
				if (modelParams != riggedModelCmp.modelParams)
				{
					riggedModelCmp.modelParams = modelParams;
					glNamedBufferSubData(riggedModelCmp.modelParamsUbo, 0, sizeof(ModelParams), &modelParams);
				}

				for (auto const& mesh : riggedModelCmp.meshes)
				{
					culledRiggedMeshes.emplace_back(
						mesh.program,
						mesh.materialIndex,
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
					|| (a_lhs.forwardProgram == a_rhs.forwardProgram && a_lhs.materialIndex < a_rhs.materialIndex);
			});

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		glEndQuery(GL_TIME_ELAPSED);

		// 5. Sun Shadow
		// BEGIN WIP
		glBeginQuery(GL_TIME_ELAPSED, renderSceneContext.timerQueries[1]);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		for (int32_t i = 0; i < mistd::isize(shadowSpotLights); ++i)
		{
			auto const culledSpotLight = culledLights[shadowSpotLights[i].culledLightIndex];
			auto const spotLightFrustumPlanes = computeSpotLightFrustumPlanes(
				culledSpotLight.position, shadowSpotLights[i].rotation, k_spotLightNear, culledSpotLight.radius, shadowSpotLights[i].outerAngle);

			auto const lightViewParams = ViewParams{
				.worldToView = glm::mat4{},
				.viewToProjected = glm::mat4{},
				.worldToProjected = spotLightShadowParams[i].worldToProjected,
				.projectedToView = glm::mat4{},
				.viewPosition = glm::vec3{}
			};
			glNamedBufferSubData(renderSceneContext.lightViewParamsUbo, 0, sizeof(ViewParams), &lightViewParams);

			glBindFramebuffer(GL_FRAMEBUFFER, renderSceneContext.spotShadowMaps[i].framebuffer);
			glViewport(0, 0, renderSceneContext.spotShadowMaps[i].size.x, renderSceneContext.spotShadowMaps[i].size.y);
			glClear(GL_DEPTH_BUFFER_BIT);

			glUseProgram(renderSceneContext.staticDepthProgram);
			glBindBufferBase(GL_UNIFORM_BUFFER, k_viewParamsUboLocation, renderSceneContext.lightViewParamsUbo);
			for (auto const [entity, position, rotation, staticModelCmp] : staticModelEntities.each())
			{
				if (testCameraFrustumIntersect(spotLightFrustumPlanes, position, staticModelCmp.boundingRadius))
				{
					auto const modelParams = ModelParams{ .modelToWorld = aoest::combine(position, rotation) };
					if (modelParams != staticModelCmp.modelParams)
					{
						staticModelCmp.modelParams = modelParams;
						glNamedBufferSubData(staticModelCmp.modelParamsUbo, 0, sizeof(ModelParams), &modelParams);
					}

					glBindBufferBase(GL_UNIFORM_BUFFER, k_modelParamsUboLocation, staticModelCmp.modelParamsUbo);
					for (auto const& mesh : staticModelCmp.meshes)
					{
						glBindVertexArray(mesh.meshVao);
						glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
					}
				}
			}

			glUseProgram(renderSceneContext.riggedDepthProgram);
			glBindBufferBase(GL_UNIFORM_BUFFER, k_viewParamsUboLocation, renderSceneContext.lightViewParamsUbo);
			for (auto const [entity, position, rotation, riggedModelCmp] : riggedModelEntities.each())
			{
				if (testCameraFrustumIntersect(spotLightFrustumPlanes, position, riggedModelCmp.boundingRadius))
				{
					auto const modelParams = ModelParams{ .modelToWorld = aoest::combine(position, rotation) };
					if (modelParams != riggedModelCmp.modelParams)
					{
						riggedModelCmp.modelParams = modelParams;
						glNamedBufferSubData(riggedModelCmp.modelParamsUbo, 0, sizeof(ModelParams), &modelParams);
					}

					glBindBufferBase(GL_UNIFORM_BUFFER, k_modelParamsUboLocation, riggedModelCmp.modelParamsUbo);
					glBindBufferBase(GL_UNIFORM_BUFFER, k_rigParamsUboLocation, riggedModelCmp.rigParamsUbo);
					for (auto const& mesh : riggedModelCmp.meshes)
					{
						glBindVertexArray(mesh.meshVao);
						glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
					}
				}
			}
		}
		{
			// A. Prepare Data
			auto const sunViewParams = ViewParams{
				.worldToView = glm::mat4{},
				.viewToProjected = glm::mat4{},
				.worldToProjected = sunSpace,
				.projectedToView = glm::mat4{},
				.viewPosition = glm::vec3{}
			};
			glNamedBufferSubData(renderSceneContext.lightViewParamsUbo, 0, sizeof(ViewParams), &sunViewParams);

			// B. Render Static Meshes
			glBindFramebuffer(GL_FRAMEBUFFER, renderSceneContext.sunShadowMap.framebuffer);
			glViewport(0, 0, renderSceneContext.sunShadowMap.size.x, renderSceneContext.sunShadowMap.size.y);
			glClear(GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
			glDepthFunc(GL_LESS);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

			glUseProgram(renderSceneContext.staticDepthProgram);
			glBindBufferBase(GL_UNIFORM_BUFFER, k_viewParamsUboLocation, renderSceneContext.lightViewParamsUbo);
			for (auto const& staticMesh : culledStaticMeshes)
			{
				glBindBufferBase(GL_UNIFORM_BUFFER, k_modelParamsUboLocation, staticMesh.modelParamsUbo);
				glBindVertexArray(staticMesh.meshVao);
				glDrawElements(GL_TRIANGLES, staticMesh.indexCount, GL_UNSIGNED_INT, nullptr);
			}
			glUseProgram(renderSceneContext.riggedDepthProgram);
			glBindBufferBase(GL_UNIFORM_BUFFER, k_viewParamsUboLocation, renderSceneContext.lightViewParamsUbo);
			for (auto const& riggedMesh : culledRiggedMeshes)
			{
				glBindBufferBase(GL_UNIFORM_BUFFER, k_modelParamsUboLocation, riggedMesh.modelParamsUbo);
				glBindBufferBase(GL_UNIFORM_BUFFER, k_rigParamsUboLocation, riggedMesh.rigParamsUbo);
				glBindVertexArray(riggedMesh.meshVao);
				glDrawElements(GL_TRIANGLES, riggedMesh.indexCount, GL_UNSIGNED_INT, nullptr);
			}
		}
		glEndQuery(GL_TIME_ELAPSED);
		// END WIP

		// 5. Depth pre-pass
		glBeginQuery(GL_TIME_ELAPSED, renderSceneContext.timerQueries[2]);
		glBindFramebuffer(GL_FRAMEBUFFER, renderSceneContext.sceneFramebuffer);
		glViewport(0, 0, renderSceneContext.sceneFramebufferSize.x, renderSceneContext.sceneFramebufferSize.y);
		glClearDepth(1.0);
		glClear(GL_DEPTH_BUFFER_BIT);
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

		// 6. Opaque pass
		glBeginQuery(GL_TIME_ELAPSED, renderSceneContext.timerQueries[3]);
		glDepthFunc(GL_LEQUAL);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glClearColor(k_blueprint.r, k_blueprint.g, k_blueprint.b, k_blueprint.a);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindTextureUnit(k_sunShadowMapTextureIndex, renderSceneContext.sunShadowMap.depthTexture);
		for (int32_t i = 0; i < mistd::isize(shadowSpotLights); ++i)
		{
			glBindTextureUnit(k_spotLightShadowMapsFirstIndex + i, renderSceneContext.spotShadowMaps[i].depthTexture);
		}
		GraphicId currentForwardProgram = k_invalidId;
		int32_t currentMaterialIndex = -1;
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
						currentMaterialIndex = -1;
					}

					if (currentMaterialIndex != staticMesh.materialIndex)
					{
						auto const& material = materialManager.getMaterial(staticMesh.materialIndex);
						currentMaterialIndex = staticMesh.materialIndex;
						glBindBufferBase(GL_UNIFORM_BUFFER, k_materialParamsUboLocation, material.paramsUbo);
						for (int32_t slotIndex = 0; slotIndex < std::ssize(material.textureIds); ++slotIndex)
						{
							glBindTextureUnit(k_materialTexturesFirstIndex + static_cast<GraphicId>(slotIndex), material.textureIds[slotIndex]);
						}
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
						currentMaterialIndex = -1;
					}

					if (currentMaterialIndex != riggedMesh.materialIndex)
					{
						auto const& material = materialManager.getMaterial(riggedMesh.materialIndex);
						currentMaterialIndex = riggedMesh.materialIndex;
						glBindBufferBase(GL_UNIFORM_BUFFER, k_materialParamsUboLocation, material.paramsUbo);
						for (int32_t slotIndex = 0; slotIndex < std::ssize(material.textureIds); ++slotIndex)
						{
							glBindTextureUnit(k_materialTexturesFirstIndex + static_cast<GraphicId>(slotIndex), material.textureIds[slotIndex]);
						}
					}
				}

				glBindBufferBase(GL_UNIFORM_BUFFER, k_modelParamsUboLocation, riggedMesh.modelParamsUbo);
				glBindBufferBase(GL_UNIFORM_BUFFER, k_rigParamsUboLocation, riggedMesh.rigParamsUbo);
				glBindVertexArray(riggedMesh.meshVao);
				glDrawElements(GL_TRIANGLES, riggedMesh.indexCount, GL_UNSIGNED_INT, nullptr);
			}
		}
		glEndQuery(GL_TIME_ELAPSED);

		// 7. Transparent pass
		// TODO
		
		// 8. Skybox pass
		// TODO

		// 9. Reflection pass
		// TODO

		// 10. Post process(es)
		glBeginQuery(GL_TIME_ELAPSED, renderSceneContext.timerQueries[4]);
#ifdef VOB_AOEGL_DEBUG
		if (k_debugMode != DebugMode::None)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, window.getDefaultFramebufferId());
			glClearColor(k_black.r, k_black.g, k_black.b, k_black.a);
			glClear(GL_COLOR_BUFFER_BIT);
			glDisable(GL_DEPTH_TEST);

			glUseProgram(renderSceneContext.debugPostProcessProgram);
			glBindTextureUnit(0, renderSceneContext.sceneColorTexture);
			glBindTextureUnit(1, renderSceneContext.sceneDepthTexture);
			glBindTextureUnit(2, renderSceneContext.sunShadowMap.depthTexture);
			for (int32_t i = 0; i < mistd::isize(shadowSpotLights); ++i)
			{
				glBindTextureUnit(3 + i, renderSceneContext.spotShadowMaps[i].depthTexture);
			}
			glBindBufferBase(GL_UNIFORM_BUFFER, k_customPostProcessConfigUboLocation, renderSceneContext.debugParamsUbo);
			glBindBufferBase(GL_UNIFORM_BUFFER, k_viewPostProcessConfigUboLocation, renderSceneContext.viewParamsUbo);
			glBindBufferBase(GL_UNIFORM_BUFFER, k_lightPostProcessConfigUboLocation, renderSceneContext.lightParamsUbo);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, renderSceneContext.lightClusterSizesSsbo);
			glBindVertexArray(renderSceneContext.postProcessVao);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
		else
#endif
		{
			glClearColor(k_black.r, k_black.g, k_black.b, k_black.a);
			glDisable(GL_DEPTH_TEST);
			glBindTextureUnit(1, renderSceneContext.sceneDepthTexture);
			glBindBufferBase(GL_UNIFORM_BUFFER, k_viewPostProcessConfigUboLocation, renderSceneContext.viewParamsUbo);
			glBindBufferBase(GL_UNIFORM_BUFFER, k_lightPostProcessConfigUboLocation, renderSceneContext.lightParamsUbo);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, renderSceneContext.lightClusterSizesSsbo);
			glBindVertexArray(renderSceneContext.postProcessVao);

			auto const postProcessFramebuffers = std::array{
				renderSceneContext.sceneFramebuffer, renderSceneContext.postProcessFramebuffer };
			auto const postProcessColorTextures = std::array{
				renderSceneContext.sceneColorTexture, renderSceneContext.postProcessColorTexture };
			for (int32_t i = 0; i < mistd::isize(renderSceneContext.postProcesses); ++i)
			{
				auto const isLastPostProcess = i + 1 == mistd::isize(renderSceneContext.postProcesses);
				auto const framebuffer = isLastPostProcess ? window.getDefaultFramebufferId() : postProcessFramebuffers[(i + 1) % 2];
				glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
				glClear(GL_COLOR_BUFFER_BIT);

				glBindTextureUnit(0, postProcessColorTextures[i % 2]);

				auto const& postProcess = renderSceneContext.postProcesses[i];
				glUseProgram(postProcess.program);
				glBindBufferBase(GL_UNIFORM_BUFFER, k_customPostProcessConfigUboLocation, postProcess.ubo);
				glDrawArrays(GL_TRIANGLES, 0, 3);
			}

		}
		glEndQuery(GL_TIME_ELAPSED);

#ifdef VOB_AOEGL_DEBUG
		if (ImGui::Begin("Render Performance"))
		{
			glFinish();
			static int32_t accumulationCount = 50;
			static std::array<float, 5> durations{ 0.0f };
			static std::array<uint64_t, 5> accumulations{ 0 };
			static int32_t accumulationIndex = 0;
			for (int i = 0; i < 5; ++i)
			{
				uint64_t durationMs;
				glGetQueryObjectui64v(renderSceneContext.timerQueries[i], GL_QUERY_RESULT, &durationMs);
				accumulations[i] += durationMs;
			}
			if (++accumulationIndex == accumulationCount)
			{
				for (int i = 0; i < 5; ++i)
				{
					durations[i] = (accumulations[i] / 1000000.0f) / accumulationCount;
					accumulations[i] = 0;
				}
				accumulationIndex = 0;
			}

			ImGui::BeginDisabled();
			int32_t lightCount = mistd::isize(culledLights);
			ImGui::InputInt("Light Count", &lightCount);
			int32_t staticMeshCount = mistd::isize(culledStaticMeshes);
			ImGui::InputInt("Static Mesh Count", &staticMeshCount);
			ImGui::InputFloat("Light Clustering (ms)", &durations[0]);
			ImGui::InputFloat("Sun Shadow Map (ms)", &durations[1]);
			ImGui::InputFloat("Depth Pre Pass (ms)", &durations[2]);
			ImGui::InputFloat("Opaque Pass (ms)", &durations[3]);
			ImGui::InputFloat("Post Process (ms)", &durations[4]);
			ImGui::EndDisabled();
		}
		ImGui::End();
#endif
	}
}
