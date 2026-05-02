#include "vob/aoe/rendering/systems/RenderSceneSystem.h"

#include "vob/aoe/rendering/CameraUtils.h"
#include "vob/aoe/rendering/components/InstancedModelsComponent.h"
#include "vob/aoe/rendering/components/ModelComponent.h"
#include "vob/aoe/rendering/components/ModelTransformComponent.h"
#include "vob/aoe/rendering/GpuState.h"
#include "vob/aoe/rendering/ProgramUtils.h"

#include "vob/aoe/debug/DebugNameUtils.h"
#include "vob/aoe/debug/ImGuiUtils.h"
#include "vob/aoe/spacetime/TransformUtils.h"

#include <vob/misc/std/container_util.h>
#include <vob/misc/std/enum_traits.h>

#include "vob/aoe/rendering/shaders/defines.h"

#include <glm/gtx/quaternion.hpp>
#include <imgui.h>

#include <array>
#include <limits>


namespace vob::aoegl
{
	void RenderSceneSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{

	}

	namespace
	{
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

		struct CulledLight
		{
			float importance;
			glm::vec3 position;
			glm::quat rotation;
			LightComponent const* lightComponent;
		};

		struct ScopedGpuTimerQuery
		{
			ScopedGpuTimerQuery(GraphicId queryId)
			{
				glBeginQuery(GL_TIME_ELAPSED, queryId);
			}

			~ScopedGpuTimerQuery()
			{
				glEndQuery(GL_TIME_ELAPSED);
			}
		};

		UniformGlobalParams createGlobalParams(aoest::TimeContext const& a_timeCtx)
		{
			return UniformGlobalParams{
				.worldTime = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - a_timeCtx.worldStartTime).count()
			};
		}

		UniformViewParams createViewParams(
			aoewi::WindowContext const& a_windowCtx,
			entt::entity a_cameraEntity,
			entt::view<entt::get_t<aoest::PositionComponent const, aoest::RotationComponent const, CameraComponent const>> a_cameraEntities)
		{
			auto const resolution = a_windowCtx.window.get().getSize();
			auto const invResolution = 1.0f / glm::vec2{ resolution };
			auto const aspectRatio = static_cast<float>(resolution.x) / resolution.y;

			auto const [position, rotation, nearClip, farClip, fov] = getCameraProperties(a_cameraEntities, a_cameraEntity);
			auto const viewToWorld = aoest::combine(position, rotation);
			auto const worldToView = glm::inverse(viewToWorld);
			auto const viewToClip = glm::perspective(fov, aspectRatio, nearClip, farClip);
			auto const worldToClip = viewToClip * worldToView;
			auto const clipToView = glm::inverse(viewToClip);
			
			return UniformViewParams{
				.worldToView = worldToView,
				.viewToClip = viewToClip,
				.worldToClip = worldToClip,
				.clipToView = clipToView,
				.viewToWorld = viewToWorld,
				.resolution = resolution,
				.invResolution = invResolution,
				.nearClip = nearClip,
				.farClip = farClip,
				.fov = fov,
				.aspectRatio = aspectRatio
			};
		}

		static float texelSize = 1.0;
		std::tuple<UniformLightingParams, UniformShadowParams, int32_t> createLightingAndShadowParams(
			ViewFrustumPlanes const& a_viewFrustumPlanes,
			glm::vec3 const& a_lightFocusPosition,
			entt::view<entt::get_t<aoest::PositionComponent const, aoest::RotationComponent const, LightComponent const>> a_lightEntities,
			int32_t a_lightsCapacity,
			glm::ivec2 const& a_lightClusterTileSize,
			int32_t a_lightClusterZCount,
			int32_t a_lightClusterCapacity,
			glm::mat4 a_clipToWorld,
			glm::mat4 a_viewToWorld,
			float a_nearClip,
			float a_farClip,
			glm::vec3 const& a_sunDir,
			mistd::bounded_vector<float, k_sunCascadingShadowMapsCapacity> const& a_sunFarClips,
			std::vector<GpuLight>& o_gpuLights)
		{
			// TODO: remove magic
			static std::vector<CulledLight> culledLights;
			culledLights.clear();
			for (auto const [entity, positionCmp, rotationCmp, lightCmp] : a_lightEntities.each())
			{
				if (!testViewFrustumPlanes(a_viewFrustumPlanes, positionCmp.value, lightCmp.radius))
				{
					continue;
				}

				auto const distanceImportance = 1.0f - glm::length(positionCmp.value - a_lightFocusPosition) / lightCmp.radius;
				auto const colorImportance = glm::dot(lightCmp.color, glm::vec3{ 0.299, 0.587, 0.114f });
				auto const intensityImportance = lightCmp.intensity;
				auto const importance = distanceImportance * colorImportance * intensityImportance;

				culledLights.emplace_back(importance, positionCmp.value, rotationCmp.value, &lightCmp);
			}
			std::sort(culledLights.begin(), culledLights.end(), [](auto const& lhs, auto const& rhs) { return lhs.importance > rhs.importance; });
			auto const lightingParams = UniformLightingParams{
				.ambientColor = glm::vec3{ 0.5f },
				.lightCount = std::min(mistd::isize(culledLights), a_lightsCapacity),
				.lightClusterTileSize = a_lightClusterTileSize,
				.lightClusterZCount = a_lightClusterZCount,
				.lightClusterCapacity = a_lightClusterCapacity,
				.sunColor = glm::vec3{ 1.0f, 0.5f, 0.4f },
				.sunIntensity = 1.0f,
				.sunDir = a_sunDir
			};

			auto shadowParams = UniformShadowParams{};

			auto const sunZ = -glm::normalize(a_sunDir);
			auto const sunX = glm::normalize(glm::cross(
				std::abs(sunZ.y) < 0.9f ? glm::vec3{ 0.0f, 1.0f, 0.0f } : glm::vec3{ 0.0f, 0.0f, 1.0f }, sunZ));
			auto const sunY = glm::cross(sunZ, sunX);
			auto const sunWorldToView = glm::mat4(glm::transpose(glm::mat3{ sunX, sunY, sunZ }));
			auto const clipZ = [&a_nearClip, &a_farClip](auto const a_clip)
				{
					return (a_nearClip + a_farClip - 2.0f * a_nearClip * a_farClip / a_clip) / (a_farClip - a_nearClip);
				};
			auto nearZ = clipZ(a_nearClip);
			for (auto i = 0; i < mistd::isize(a_sunFarClips); ++i)
			{
				auto const farClip = a_sunFarClips[i];
				auto const farZ = clipZ(farClip);
				auto minX = std::numeric_limits<float>::infinity();
				auto maxX = -std::numeric_limits<float>::infinity();
				auto minY = std::numeric_limits<float>::infinity();
				auto maxY = -std::numeric_limits<float>::infinity();
				auto minZ = std::numeric_limits<float>::infinity();
				auto maxZ = -std::numeric_limits<float>::infinity();
				for (auto x : {-1.0f, 1.0f})
				{
					for (auto y : {-1.0f, 1.0f})
					{
						auto const clampZ = [](auto const& v) { return v; return  glm::vec3{ v.x, std::clamp(v.y, -1.0f, 9.0f), v.z }; };
						auto const nearPoint = aoest::transformPositionSkewed(a_clipToWorld, clampZ(glm::vec3{ x, y, nearZ }));
						auto const farPoint = aoest::transformPositionSkewed(a_clipToWorld, clampZ(glm::vec3{ x, y, farZ }));
						for (auto const& point : { nearPoint, farPoint })
						{
							minX = std::min(minX, glm::dot(point, sunX));
							maxX = std::max(maxX, glm::dot(point, sunX));
							minY = std::min(minY, glm::dot(point, sunY));
							maxY = std::max(maxY, glm::dot(point, sunY));
							minZ = std::min(minZ, glm::dot(point, -sunZ));
							maxZ = std::max(maxZ, glm::dot(point, -sunZ));
						}
					}
				}

				minX = glm::floor(minX / texelSize) * texelSize;
				maxX = glm::ceil(maxX / texelSize) * texelSize;
				minY = glm::floor(minY / texelSize) * texelSize;
				maxY = glm::ceil(maxY / texelSize) * texelSize;
				minZ = glm::floor(minZ / texelSize) * texelSize;
				maxZ = glm::ceil(maxZ / texelSize) * texelSize;

				auto const sunViewToClip = glm::ortho(minX, maxX, minY, maxY, maxZ + 100.0f, minZ);
				auto const sunWorldToClip = sunViewToClip * sunWorldToView;

				shadowParams.sun[i] = GpuSunCascadingShadow{
					.worldToClip = sunWorldToClip,
					.maxViewDepth = farClip,
					.nearClip = minZ,
					.farClip = maxZ
				};

				nearZ = farZ;
			}
			shadowParams.sunReferenceViewToWorld = a_viewToWorld;
			shadowParams.sunCascadingShadowMapCount = mistd::isize(a_sunFarClips);

			o_gpuLights.reserve(lightingParams.lightCount);
			int32_t spotLightShadowMapCount = 0;
			for (auto i = 0; i < lightingParams.lightCount; ++i)
			{
				auto const& culledLight = culledLights[i];
				auto const isPointLight = culledLight.lightComponent->type == LightType::Point;
				auto const spotOuterAngleCos = std::cos(culledLight.lightComponent->outerAngle);
				auto const spotInnerAngleCos = std::cos(culledLight.lightComponent->innerAngle);

				o_gpuLights.emplace_back(
					culledLight.position,
					culledLight.lightComponent->radius,
					culledLight.lightComponent->color,
					culledLight.lightComponent->intensity,
					culledLight.rotation * glm::vec3{ 0.0f, 0.0f, -1.0f },
					isPointLight ? 0 : 1,
					spotOuterAngleCos,
					spotInnerAngleCos,
					-1 /* spot shadow map index */);

				if (culledLight.lightComponent->castsShadow && spotLightShadowMapCount < k_spotLightShadowMapsCapacity)
				{
					auto const& lightCmp = *culledLight.lightComponent;
					auto const lightViewToClip = glm::perspective(2.0f * lightCmp.outerAngle, 1.0f, lightCmp.nearClip, lightCmp.radius);
					auto const lightForward = culledLight.rotation * glm::vec3{ 0.0f, 0.0f, -1.0f };
					auto const lightUp = culledLight.rotation * glm::vec3{ 0.0f, 1.0f, 0.0f };
					auto const worldToLightView = glm::lookAt(culledLight.position, culledLight.position + lightForward, lightUp);
					auto const lightViewToWorld = glm::inverse(worldToLightView);

					auto const spotLightShadowMapIndex = spotLightShadowMapCount++;
					o_gpuLights.back().shadowMapIndex = spotLightShadowMapIndex;
					shadowParams.spotLights[spotLightShadowMapIndex] = GpuSpotLightShadow{
						.worldToClip = lightViewToClip * worldToLightView,
						.nearClip = lightCmp.nearClip,
						.farClip = lightCmp.radius,
						.size = lightCmp.size,
						.fov = 2.0f * lightCmp.outerAngle,
						.viewToWorld = lightViewToWorld
					};
				}
			}

			return { lightingParams, shadowParams, spotLightShadowMapCount };
		}
	}

	void RenderSceneSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto& debugMeshCtx = m_debugMeshContext.get(a_wdap);
		auto& renderSceneCtx = m_renderSceneCtx.get(a_wdap);
		auto const& debugProgramCtx = m_debugProgramContext.get(a_wdap);
		auto const& materialManager = *m_materialManagerContext.get(a_wdap).materialManager;
		auto const& window = m_windowContext.get(a_wdap).window.get();
		auto const& cameraDirectorCtx = m_cameraDirectorContext.get(a_wdap);
		auto staticModelEntities = m_staticModelEntities.get(a_wdap);
		auto riggedModelEntities = m_riggedModelEntities.get(a_wdap);
		auto instancedModelsEntities = m_instancedModelsEntities.get(a_wdap);
		GpuState gpuState;

		glQueryCounter(renderSceneCtx.totalTimerQueries[0], GL_TIMESTAMP);
		// 0 - Prepare Debug
		enum class DebugMode2
		{
			None,
			LightClusters,
			SunShadowMap,
			SpotLightShadowMap,
			OpaqueGeometricNormal,
			OpaqueDepth,
			AmbientOcclusion,
			DirectOpaqueColor,
			OpaqueNormal,
			OpaqueSurface,
			SsrColor,
			FinalColor,
		};
		static DebugMode2 k_debugMode = DebugMode2::None;
		static int32_t k_debugShadowMapIndex = 0;
		static bool k_debugCameraFrustum = false;
		static bool k_ssaoEnabled = true;
		static bool k_ssrEnabled = true;
		if (ImGui::Begin("Render Debug"))
		{
			aoedb::ImGuiEnumCombo("Debug Mode", &k_debugMode);
			ImGui::BeginDisabled(k_debugMode != DebugMode2::SunShadowMap && k_debugMode != DebugMode2::SpotLightShadowMap);
			ImGui::InputInt("Shadow Map Index", &k_debugShadowMapIndex);
			ImGui::EndDisabled();
			if (k_debugMode == DebugMode2::SunShadowMap)
			{
				k_debugShadowMapIndex = std::clamp(k_debugShadowMapIndex, 0, mistd::isize(renderSceneCtx.sunShadowMapFrustumFarClips) - 1);
			}
			else if (k_debugMode == DebugMode2::SpotLightShadowMap)
			{
				k_debugShadowMapIndex = std::clamp(k_debugShadowMapIndex, 0, k_spotLightShadowMapsCapacity - 1);
			}
			ImGui::Checkbox("Frustum", &k_debugCameraFrustum);

			ImGui::SeparatorText("SSAO");
			ImGui::Checkbox("Enable##ssao", &k_ssaoEnabled);
			static int k_ssaoSampleCount = 8;
			ImGui::InputInt("Sample Count", &k_ssaoSampleCount);
			static float k_ssaoRadius = 25.0f;
			ImGui::SliderFloat("Radius", &k_ssaoRadius, 0.0f, 100.0f);
			static float k_ssaoAttenuationBias = 0.4f;
			ImGui::SliderFloat("Attenuation Bias", &k_ssaoAttenuationBias, 0.0f, 1.0f);
			static float k_ssaoAttenuationScale = 1.6f;
			ImGui::SliderFloat("Attenuation Scale", &k_ssaoAttenuationScale, 0.0f, 3.0f);
			static float k_ssaoThreshold = 0.25f;
			ImGui::SliderFloat("Threshold", &k_ssaoThreshold, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_Logarithmic);

			auto const ssaoParams = UniformSsaoParams{
				.sampleCount = k_ssaoSampleCount,
				.radius = k_ssaoRadius,
				.attenuationBias = k_ssaoAttenuationBias,
				.attenuationScale = k_ssaoAttenuationScale,
				.threshold = k_ssaoThreshold
			};
			glNamedBufferSubData(renderSceneCtx.ssaoParamsUbo, 0, sizeof(ssaoParams), &ssaoParams);

			ImGui::SeparatorText("SSR");
			ImGui::Checkbox("Enable##ssr", &k_ssrEnabled);
			static int k_ssrLog2Step = 7;
			ImGui::InputInt("Log2 Step", &k_ssrLog2Step);
			static int k_ssrLog2SubStep = 3;
			ImGui::InputInt("Log2 Sub Step", &k_ssrLog2SubStep);
			static float k_ssrThicknessRatio = 0.1f;
			ImGui::SliderFloat("Thickness Ratio", &k_ssrThicknessRatio, 0.01f, 100.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
			static float k_ssrMaxRangeRatio = 0.02f;
			ImGui::SliderFloat("Max Range Ratio", &k_ssrMaxRangeRatio, 0.01f, 100.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
			static float k_ssrInitialBiasRatio = 0.005f;
			ImGui::SliderFloat("Initial Bias Ratio", &k_ssrInitialBiasRatio, 0.001f, 1.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
			static float k_ssrMaxThickness = 0.01f;
			ImGui::SliderFloat("Max Thickness", &k_ssrMaxThickness, 0.01f, 10.0f);

			auto const ssrParams = UniformSsrParams{
				.log2Step = k_ssrLog2Step,
				.log2SubStep = k_ssrLog2SubStep,
				.thicknessRatio = k_ssrThicknessRatio,
				.maxRangeRatio = k_ssrMaxRangeRatio,
				.initialBiasRatio = k_ssrInitialBiasRatio,
				.maxThickness = k_ssrMaxThickness
			};
			glNamedBufferSubData(renderSceneCtx.ssrParamsUbo, 0, sizeof(ssrParams), &ssrParams);

			ImGui::SeparatorText("Shaders");
			static int32_t k_activeShadingProgramIndex = 0;
			auto const toSmallStr = [](std::string_view a_stringView)
				{
					constexpr size_t k_maxSize = 16;
					auto size = std::min(a_stringView.size(), k_maxSize);
					std::array<char, k_maxSize + 1> smallStr;
					std::memcpy(smallStr.data(), a_stringView.data(), size);
					smallStr[size] = 0;
					return smallStr;
				};

			k_activeShadingProgramIndex = std::min(k_activeShadingProgramIndex, mistd::isize(debugProgramCtx.forwardPrograms) - 1);


			if (ImGui::Button("Recompile Ssao Program"))
			{
				std::system("tools\\GLSLGenerator\\bin\\x64\\Release\\GLSLGenerator.exe include\\vob\\aoe\\rendering\\shaders data\\shaders\\core");
				createSsaoProgram(debugProgramCtx.ssaoProgram);
			}

			if (ImGui::Button("Recompile Ssr Program"))
			{
				std::system("tools\\GLSLGenerator\\bin\\x64\\Release\\GLSLGenerator.exe include\\vob\\aoe\\rendering\\shaders data\\shaders\\core");
				createSsrProgram(debugProgramCtx.ssrProgram);
			}

			if (ImGui::Button("Recompile Sky Box Program"))
			{
				std::system("tools\\GLSLGenerator\\bin\\x64\\Release\\GLSLGenerator.exe include\\vob\\aoe\\rendering\\shaders data\\shaders\\core");
				auto const skyBoxSource = debugProgramCtx.stringDatabase.find(
					debugProgramCtx.filesystemIndexer.get_runtime_id(debugProgramCtx.skyBoxSourcePath));
				createQuadProgram(*skyBoxSource, debugProgramCtx.skyBoxProgram);
			}

			auto const activeShadingProgramStr = toSmallStr(debugProgramCtx.forwardPrograms[k_activeShadingProgramIndex].name);
			if (ImGui::BeginCombo("Shading Program", activeShadingProgramStr.data()))
			{
				for (int32_t i = 0; i < mistd::isize(debugProgramCtx.forwardPrograms); ++i)
				{
					auto const shadingProgramStr = toSmallStr(debugProgramCtx.forwardPrograms[i].name);
					if (ImGui::Selectable(shadingProgramStr.data(), i == k_activeShadingProgramIndex))
					{
						k_activeShadingProgramIndex = i;
					}

					if (i == k_activeShadingProgramIndex)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
			if (ImGui::Button("Recompile Shading Program"))
			{
				std::system("tools\\GLSLGenerator\\bin\\x64\\Release\\GLSLGenerator.exe include\\vob\\aoe\\rendering\\shaders data\\shaders\\core");
				auto const& forwardProgram = debugProgramCtx.forwardPrograms[k_activeShadingProgramIndex];
				auto const shadingSource = debugProgramCtx.stringDatabase.find(
					debugProgramCtx.filesystemIndexer.get_runtime_id(forwardProgram.shadingSourcePath));
				createShadingProgram(*shadingSource, ModelType::Static, forwardProgram.staticProgram);
				createShadingProgram(*shadingSource, ModelType::Rigged, forwardProgram.riggedProgram);
				createShadingProgram(*shadingSource, ModelType::Instanced, forwardProgram.instancedProgram);
			}

		}
		ImGui::End();

		// I - Prepare Scene
		auto const globalParams = createGlobalParams(m_timeContext.get(a_wdap));
		glNamedBufferSubData(renderSceneCtx.globalParamsUbo, 0, sizeof(globalParams), &globalParams);

		auto viewParams = createViewParams(
			m_windowContext.get(a_wdap), cameraDirectorCtx.activeCameraEntity, m_cameraEntities.get(a_wdap));
		auto const debugViewParams = createViewParams(
			m_windowContext.get(a_wdap),
			cameraDirectorCtx.debugCameraEntity != entt::null ? cameraDirectorCtx.debugCameraEntity : cameraDirectorCtx.activeCameraEntity,
			m_cameraEntities.get(a_wdap));
		glNamedBufferSubData(renderSceneCtx.viewParamsUbo, 0, sizeof(viewParams), &viewParams);

		auto const viewFrustumPlanes = computeViewFrustumPlanes(debugViewParams.worldToClip);

		// II - Prepare Lights & Shadows
		auto const lightFocusPosition = getFocusPosition(m_focusEntities.get(a_wdap), cameraDirectorCtx.focusEntity);
		// TODO: remove magic
		static std::vector<GpuLight> gpuLights;
		gpuLights.clear();
		auto const [lightingParams, shadowParams, spotLightShadowMapCount] = createLightingAndShadowParams(
			viewFrustumPlanes,
			lightFocusPosition,
			m_lightEntities.get(a_wdap),
			renderSceneCtx.lightsCapacity,
			renderSceneCtx.lightClusterTileSize,
			renderSceneCtx.lightClusterZCount,
			renderSceneCtx.lightClusterCapacity,
			glm::inverse(debugViewParams.worldToClip),
			debugViewParams.viewToWorld,
			debugViewParams.nearClip,
			debugViewParams.farClip,
			renderSceneCtx.sunDir,
			renderSceneCtx.sunShadowMapFrustumFarClips,
			gpuLights);
		glNamedBufferSubData(renderSceneCtx.lightingParamsUbo, 0, sizeof(lightingParams), &lightingParams);
		glNamedBufferSubData(renderSceneCtx.shadowParamsUbo, 0, sizeof(shadowParams), &shadowParams);
		glNamedBufferSubData(renderSceneCtx.lightsSsbo, 0, gpuLights.size() * sizeof(gpuLights[0]), gpuLights.data());

		// III - Cluster Lights
		{
			ScopedGpuTimerQuery const timerQuery(renderSceneCtx.renderPassTimerQueries[RenderPass::LightClustering]);
			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.lightClusteringProgram);
			gpuState.bindUbo<GpuStateChange::SurelyYes>(k_bindingUboGlobal, renderSceneCtx.globalParamsUbo);
			gpuState.bindUbo<GpuStateChange::SurelyYes>(k_bindingUboView, renderSceneCtx.viewParamsUbo);
			gpuState.bindUbo<GpuStateChange::SurelyYes>(k_bindingUboLighting, renderSceneCtx.lightingParamsUbo);
			gpuState.bindUbo<GpuStateChange::SurelyYes>(k_bindingUboShadow, renderSceneCtx.shadowParamsUbo);
			gpuState.bindSsbo<GpuStateChange::SurelyYes>(k_bindingSsboLights, renderSceneCtx.lightsSsbo);
			gpuState.bindSsbo<GpuStateChange::SurelyYes>(k_bindingSsboLightClusterSizes, renderSceneCtx.lightClusterSizesSsbo);
			gpuState.bindSsbo<GpuStateChange::SurelyYes>(k_bindingSsboLightClusterIndices, renderSceneCtx.lightClusterIndicesSsbo);

			auto const lightClusterXYCount = (renderSceneCtx.shadingResolution + renderSceneCtx.lightClusterTileSize - 1) / renderSceneCtx.lightClusterTileSize;
			auto const lightClusterCount = lightClusterXYCount.x * lightClusterXYCount.y * renderSceneCtx.lightClusterZCount;
			auto const workGroupCount = (lightClusterCount + renderSceneCtx.lightClusteringWorkGroupSize - 1) / renderSceneCtx.lightClusteringWorkGroupSize;
			glDispatchCompute(static_cast<uint32_t>(workGroupCount), 1, 1);
		}

		// IV - Prepare Meshes
		struct CulledStaticMesh
		{
			GraphicId shadingProgram;
			int32_t materialIndex;
			GraphicId modelParamsUbo;
			GraphicId meshVao;
			int32_t indexCount;
		};
		static std::vector<CulledStaticMesh> culledOpaqueStaticMeshes;
		culledOpaqueStaticMeshes.clear();
		static std::vector<CulledStaticMesh> culledTranslucentStaticMeshes;
		culledTranslucentStaticMeshes.clear();
		for (auto const [entity, positionCmp, rotationCmp, staticModelCmp, modelTransformCmp] : staticModelEntities.each())
		{
			if (testViewFrustumPlanes(viewFrustumPlanes, positionCmp.value, modelTransformCmp.boundingRadius))
			{
				auto const modelParams = UniformModelParams{ .modelToWorld = aoest::combine(positionCmp.value, rotationCmp.value) };
				if (modelTransformCmp.modelParams != modelParams)
				{
					modelTransformCmp.modelParams = modelParams;
					glNamedBufferSubData(modelTransformCmp.modelParamsUbo, 0, sizeof(modelParams), &modelParams);
				}

				for (auto const& mesh : staticModelCmp.meshes)
				{
					switch (mesh.shadingPass)
					{
					case ShadingPass::Opaque:
						culledOpaqueStaticMeshes.emplace_back(mesh.program, mesh.materialIndex, modelTransformCmp.modelParamsUbo, mesh.meshVao, mesh.indexCount);
						break;
					case ShadingPass::Translucent:
						culledTranslucentStaticMeshes.emplace_back(mesh.program, mesh.materialIndex, modelTransformCmp.modelParamsUbo, mesh.meshVao, mesh.indexCount);
						break;
					default:
						break;
					}
				}
			}
		}
		struct CulledRiggedMesh
		{
			GraphicId shadingProgram;
			int32_t materialIndex;
			GraphicId modelParamsUbo;
			GraphicId rigParamsUbo;
			GraphicId meshVao;
			int32_t indexCount;
		};
		static std::vector<CulledRiggedMesh> culledOpaqueRiggedMeshes;
		culledOpaqueRiggedMeshes.clear();
		static std::vector<CulledRiggedMesh> culledTranslucentRiggedMeshes;
		culledTranslucentRiggedMeshes.clear();
		for (auto const [entity, positionCmp, rotationCmp, riggedModelCmp, modelTransformCmp] : riggedModelEntities.each())
		{
			if (testViewFrustumPlanes(viewFrustumPlanes, positionCmp.value, modelTransformCmp.boundingRadius))
			{
				auto const modelParams = UniformModelParams{ .modelToWorld = aoest::combine(positionCmp.value, rotationCmp.value) };
				if (modelTransformCmp.modelParams != modelParams)
				{
					modelTransformCmp.modelParams = modelParams;
					glNamedBufferSubData(modelTransformCmp.modelParamsUbo, 0, sizeof(modelParams), &modelParams);
				}

				for (auto const& mesh : riggedModelCmp.meshes)
				{
					switch (mesh.shadingPass)
					{
					case ShadingPass::Opaque:
						culledOpaqueRiggedMeshes.emplace_back(
							mesh.program, mesh.materialIndex, modelTransformCmp.modelParamsUbo, riggedModelCmp.rigParamsUbo, mesh.meshVao, mesh.indexCount);
						break;
					case ShadingPass::Translucent:
						culledTranslucentRiggedMeshes.emplace_back(
							mesh.program, mesh.materialIndex, modelTransformCmp.modelParamsUbo, riggedModelCmp.rigParamsUbo, mesh.meshVao, mesh.indexCount);
						break;
					default:
						break;
					}
				}
			}
		}
		struct CulledInstancedMesh
		{
			GraphicId shadingProgram;
			int32_t materialIndex;
			GraphicId modelParamsUbo;
			GraphicId instanceTransformVbo;
			int32_t instanceCount;
			GraphicId meshVao;
			int32_t indexCount;
		};
		static std::vector<CulledInstancedMesh> culledOpaqueInstancedMeshes;
		culledOpaqueInstancedMeshes.clear();
		static std::vector<CulledInstancedMesh> culledTranslucentInstancedMeshes;
		culledTranslucentInstancedMeshes.clear();
		for (auto const [entity, positionCmp, rotationCmp, instancedModelsCmp, modelTransformCmp] : instancedModelsEntities.each())
		{
			if (testViewFrustumPlanes(viewFrustumPlanes, positionCmp.value, modelTransformCmp.boundingRadius))
			{
				auto const modelParams = UniformModelParams{ .modelToWorld = aoest::combine(positionCmp.value, rotationCmp.value) };
				if (modelTransformCmp.modelParams != modelParams)
				{
					modelTransformCmp.modelParams = modelParams;
					glNamedBufferSubData(modelTransformCmp.modelParamsUbo, 0, sizeof(modelParams), &modelParams);
				}

				for (auto const& model : instancedModelsCmp.models)
				{
					for (auto const& mesh : model.meshes)
					{
						switch (mesh.shadingPass)
						{
						case ShadingPass::Opaque:
							culledOpaqueInstancedMeshes.emplace_back(
								mesh.program, mesh.materialIndex, modelTransformCmp.modelParamsUbo, model.instanceTransformVbo, model.instanceCount, mesh.meshVao, mesh.indexCount);
							break;
						case ShadingPass::Translucent:
							culledTranslucentInstancedMeshes.emplace_back(
								mesh.program, mesh.materialIndex, modelTransformCmp.modelParamsUbo, model.instanceTransformVbo, model.instanceCount, mesh.meshVao, mesh.indexCount);
							break;
						default:
							break;
						}
					}
				}
			}
		}

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		// V - Compute Shadow Maps
		float debugSunNear = 0.0f;
		float debugSunFar = 0.0f;
		if (k_debugCameraFrustum)
		{
			auto const clipToWorld = glm::inverse(debugViewParams.worldToClip);
			auto p0 = aoest::transformPositionSkewed(clipToWorld, glm::vec3{ -1.0f, -1.0f, -1.0f });
			auto p1 = aoest::transformPositionSkewed(clipToWorld, glm::vec3{ -1.0f, -1.0f, 1.0f });
			auto p2 = aoest::transformPositionSkewed(clipToWorld, glm::vec3{ -1.0f, 1.0f, -1.0f });
			auto p3 = aoest::transformPositionSkewed(clipToWorld, glm::vec3{ -1.0f, 1.0f, 1.0f });
			auto p4 = aoest::transformPositionSkewed(clipToWorld, glm::vec3{ 1.0f, -1.0f, -1.0f });
			auto p5 = aoest::transformPositionSkewed(clipToWorld, glm::vec3{ 1.0f, -1.0f, 1.0f });
			auto p6 = aoest::transformPositionSkewed(clipToWorld, glm::vec3{ 1.0f, 1.0f, -1.0f });
			auto p7 = aoest::transformPositionSkewed(clipToWorld, glm::vec3{ 1.0f, 1.0f, 1.0f });
			debugMeshCtx.addLine(p0, p1, aoegl::k_orange);
			debugMeshCtx.addLine(p0, p2, aoegl::k_orange);
			debugMeshCtx.addLine(p0, p4, aoegl::k_orange);
			debugMeshCtx.addLine(p1, p3, aoegl::k_orange);
			debugMeshCtx.addLine(p1, p5, aoegl::k_orange);
			debugMeshCtx.addLine(p2, p3, aoegl::k_orange);
			debugMeshCtx.addLine(p2, p6, aoegl::k_orange);
			debugMeshCtx.addLine(p3, p7, aoegl::k_orange);
			debugMeshCtx.addLine(p4, p5, aoegl::k_orange);
			debugMeshCtx.addLine(p4, p6, aoegl::k_orange);
			debugMeshCtx.addLine(p5, p7, aoegl::k_orange);
			debugMeshCtx.addLine(p6, p7, aoegl::k_orange);
		}

		{
			ScopedGpuTimerQuery const timerQuery(renderSceneCtx.renderPassTimerQueries[RenderPass::ShadowMaps]);
			gpuState.enableDepthTest<GpuStateChange::SurelyYes>();
			gpuState.enableDepthWrite<GpuStateChange::SurelyYes>();
			gpuState.setDepthFunc<GpuStateChange::SurelyYes>(GpuDepthFunc::Less);
			gpuState.setClearDepth<GpuStateChange::SurelyYes>(1.0);
			gpuState.disableColorWrite<GpuStateChange::SurelyYes>();
			gpuState.disableBlend<GpuStateChange::SurelyYes>();
			gpuState.bindUbo<GpuStateChange::SurelyYes>(k_bindingUboView, renderSceneCtx.lightViewParamsUbo);

			// A - Sun CSM
			gpuState.bindFramebuffer<GpuStateChange::SurelyYes>(renderSceneCtx.sunShadowMapFramebuffer);
			gpuState.setViewport<GpuStateChange::LikelyYes>(glm::ivec4{ 0, 0, renderSceneCtx.sunShadowMapResolution });

			for (int32_t csmIndex = 0; csmIndex < mistd::isize(renderSceneCtx.sunShadowMapFrustumFarClips); ++csmIndex)
			{
				auto const& sunShadowParams = shadowParams.sun[csmIndex];
				if (csmIndex == k_debugShadowMapIndex)
				{
					debugSunNear = sunShadowParams.nearClip;
					debugSunFar = sunShadowParams.farClip;
				}
				if (k_debugCameraFrustum)
				{
					auto const viewClipToWorld = glm::inverse(debugViewParams.worldToClip);
					auto const viewNearClip = debugViewParams.nearClip;
					auto const viewFarClip = debugViewParams.farClip;
					auto const clipZ = [viewNearClip, viewFarClip](auto const a_clip)
						{
							return (viewNearClip + viewFarClip - 2.0f * viewNearClip * viewFarClip / a_clip) / (viewFarClip - viewNearClip);
						};
					auto s0 = aoest::transformPositionSkewed(viewClipToWorld, glm::vec3{ -1.0f, -1.0f, clipZ(sunShadowParams.maxViewDepth) });
					auto s1 = aoest::transformPositionSkewed(viewClipToWorld, glm::vec3{ -1.0f, 1.0f, clipZ(sunShadowParams.maxViewDepth) });
					auto s2 = aoest::transformPositionSkewed(viewClipToWorld, glm::vec3{ 1.0f, -1.0f, clipZ(sunShadowParams.maxViewDepth) });
					auto s3 = aoest::transformPositionSkewed(viewClipToWorld, glm::vec3{ 1.0f, 1.0f, clipZ(sunShadowParams.maxViewDepth) });
					debugMeshCtx.addLine(s0, s1, aoegl::k_yellow);
					debugMeshCtx.addLine(s0, s2, aoegl::k_yellow);
					debugMeshCtx.addLine(s1, s3, aoegl::k_yellow);
					debugMeshCtx.addLine(s2, s3, aoegl::k_yellow);

					auto const sunClipToWorld = glm::inverse(sunShadowParams.worldToClip);
					auto p0 = aoest::transformPositionSkewed(sunClipToWorld, glm::vec3{ -1.0f, -1.0f, -1.0f });
					auto p1 = aoest::transformPositionSkewed(sunClipToWorld, glm::vec3{ -1.0f, -1.0f, 1.0f });
					auto p2 = aoest::transformPositionSkewed(sunClipToWorld, glm::vec3{ -1.0f, 1.0f, -1.0f });
					auto p3 = aoest::transformPositionSkewed(sunClipToWorld, glm::vec3{ -1.0f, 1.0f, 1.0f });
					auto p4 = aoest::transformPositionSkewed(sunClipToWorld, glm::vec3{ 1.0f, -1.0f, -1.0f });
					auto p5 = aoest::transformPositionSkewed(sunClipToWorld, glm::vec3{ 1.0f, -1.0f, 1.0f });
					auto p6 = aoest::transformPositionSkewed(sunClipToWorld, glm::vec3{ 1.0f, 1.0f, -1.0f });
					auto p7 = aoest::transformPositionSkewed(sunClipToWorld, glm::vec3{ 1.0f, 1.0f, 1.0f });
					debugMeshCtx.addLine(p0, p1, aoegl::k_gray);
					debugMeshCtx.addLine(p0, p2, aoegl::k_gray);
					debugMeshCtx.addLine(p0, p4, aoegl::k_gray);
					debugMeshCtx.addLine(p1, p3, aoegl::k_gray);
					debugMeshCtx.addLine(p1, p5, aoegl::k_gray);
					debugMeshCtx.addLine(p2, p3, aoegl::k_gray);
					debugMeshCtx.addLine(p2, p6, aoegl::k_gray);
					debugMeshCtx.addLine(p3, p7, aoegl::k_gray);
					debugMeshCtx.addLine(p4, p5, aoegl::k_gray);
					debugMeshCtx.addLine(p4, p6, aoegl::k_gray);
					debugMeshCtx.addLine(p5, p7, aoegl::k_gray);
					debugMeshCtx.addLine(p6, p7, aoegl::k_gray);
				}
				
				auto const sunViewFrustumPlanes = computeViewFrustumPlanes(sunShadowParams.worldToClip);

				auto const sunViewParams = UniformViewParams{
					.worldToClip = sunShadowParams.worldToClip,
					.resolution = renderSceneCtx.sunShadowMapResolution,
					.nearClip = sunShadowParams.nearClip,
					.farClip = sunShadowParams.farClip,
					.fov = 0.0f,
					.aspectRatio = 0.0f
				};
				glNamedBufferSubData(renderSceneCtx.lightViewParamsUbo, 0, sizeof(sunViewParams), &sunViewParams);

				glNamedFramebufferTextureLayer(
					renderSceneCtx.sunShadowMapFramebuffer,
					GL_DEPTH_ATTACHMENT,
					renderSceneCtx.sunShadowMapDepthTextureArray,
					0 /* mip level */,
					csmIndex);
				glClear(GL_DEPTH_BUFFER_BIT);

				gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.staticShadowMapProgram);
				for (auto const [entity, positionCmp, rotationCmp, staticModelCmp, modelTransformCmp] : staticModelEntities.each())
				{
					if (testViewFrustumPlanes(sunViewFrustumPlanes, positionCmp.value, modelTransformCmp.boundingRadius))
					{
						auto const modelParams = UniformModelParams{ .modelToWorld = aoest::combine(positionCmp.value, rotationCmp.value) };
						if (modelTransformCmp.modelParams != modelParams)
						{
							modelTransformCmp.modelParams = modelParams;
							glNamedBufferSubData(modelTransformCmp.modelParamsUbo, 0, sizeof(modelParams), &modelParams);
						}

						gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboModel, modelTransformCmp.modelParamsUbo);
						for (auto const& mesh : staticModelCmp.meshes)
						{
							glBindVertexArray(mesh.meshVao);
							glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
						}
					}
				}

				gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.riggedShadowMapProgram);
				for (auto const [entity, positionCmp, rotationCmp, riggedModelCmp, modelTransformCmp] : riggedModelEntities.each())
				{
					if (testViewFrustumPlanes(sunViewFrustumPlanes, positionCmp.value, modelTransformCmp.boundingRadius))
					{
						auto const modelParams = UniformModelParams{ .modelToWorld = aoest::combine(positionCmp.value, rotationCmp.value) };
						if (modelTransformCmp.modelParams != modelParams)
						{
							modelTransformCmp.modelParams = modelParams;
							glNamedBufferSubData(modelTransformCmp.modelParamsUbo, 0, sizeof(modelParams), &modelParams);
						}

						gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboModel, modelTransformCmp.modelParamsUbo);
						gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboRig, riggedModelCmp.rigParamsUbo);
						for (auto const& mesh : riggedModelCmp.meshes)
						{
							glBindVertexArray(mesh.meshVao);
							glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
						}
					}
				}

				gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.instancedShadowMapProgram);
				for (auto const [entity, positionCmp, rotationCmp, instancedModelsCmp, modelTransformCmp] : instancedModelsEntities.each())
				{
					if (testViewFrustumPlanes(viewFrustumPlanes, positionCmp.value, modelTransformCmp.boundingRadius))
					{
						auto const modelParams = UniformModelParams{ .modelToWorld = aoest::combine(positionCmp.value, rotationCmp.value) };
						if (modelTransformCmp.modelParams != modelParams)
						{
							modelTransformCmp.modelParams = modelParams;
							glNamedBufferSubData(modelTransformCmp.modelParamsUbo, 0, sizeof(modelParams), &modelParams);
						}

					}
					if (testViewFrustumPlanes(sunViewFrustumPlanes, positionCmp.value, modelTransformCmp.boundingRadius))
					{
						auto const modelParams = UniformModelParams{ .modelToWorld = aoest::combine(positionCmp, rotationCmp) };
						if (modelTransformCmp.modelParams != modelParams)
						{
							modelTransformCmp.modelParams = modelParams;
							glNamedBufferSubData(modelTransformCmp.modelParamsUbo, 0, sizeof(modelParams), &modelParams);
						}

						gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboModel, modelTransformCmp.modelParamsUbo);

						for (auto const& model : instancedModelsCmp.models)
						{
							for (auto const& mesh : model.meshes)
							{
								glBindVertexArray(mesh.meshVao);
								glBindVertexBuffer(
									1,
									model.instanceTransformVbo,
									0 /* offset */,
									sizeof(glm::mat4));
								glDrawElementsInstanced(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr, model.instanceCount);
							}
						}
					}
				}
			}
			// B - Spot Lights
			for (int32_t i = 0; i < spotLightShadowMapCount; ++i)
			{
				auto const spotLightViewParams = UniformViewParams{
					.worldToClip = shadowParams.spotLights[i].worldToClip,
					.viewToWorld = shadowParams.spotLights[i].viewToWorld,
					.resolution = renderSceneCtx.spotLightShadowMapTargets[i].resolution,
					.nearClip = shadowParams.spotLights[i].nearClip,
					.farClip = shadowParams.spotLights[i].farClip,
					.fov = shadowParams.spotLights[i].fov,
					.aspectRatio = 1.0f
				};
				glNamedBufferSubData(renderSceneCtx.lightViewParamsUbo, 0, sizeof(spotLightViewParams), &spotLightViewParams);

				auto const spotLightViewFrustumPlanes = computeViewFrustumPlanes(spotLightViewParams.worldToClip);

				gpuState.bindFramebuffer<GpuStateChange::SurelyYes>(renderSceneCtx.spotLightShadowMapTargets[i].framebuffer);
				gpuState.setViewport<GpuStateChange::LikelyYes>(glm::ivec4{ 0, 0, spotLightViewParams.resolution });
				glClear(GL_DEPTH_BUFFER_BIT);

				gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.staticShadowMapProgram);
				for (auto const [entity, positionCmp, rotationCmp, staticModelCmp, modelTransformCmp] : staticModelEntities.each())
				{
					if (testViewFrustumPlanes(spotLightViewFrustumPlanes, positionCmp.value, modelTransformCmp.boundingRadius))
					{
						auto const modelParams = UniformModelParams{ .modelToWorld = aoest::combine(positionCmp.value, rotationCmp.value) };
						if (modelTransformCmp.modelParams != modelParams)
						{
							modelTransformCmp.modelParams = modelParams;
							glNamedBufferSubData(modelTransformCmp.modelParamsUbo, 0, sizeof(modelParams), &modelParams);
						}

						gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboModel, modelTransformCmp.modelParamsUbo);
						for (auto const& mesh : staticModelCmp.meshes)
						{
							glBindVertexArray(mesh.meshVao);
							glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
						}
					}
				}

				gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.riggedShadowMapProgram);
				for (auto const [entity, positionCmp, rotationCmp, riggedModelCmp, modelTransformCmp] : riggedModelEntities.each())
				{
					if (testViewFrustumPlanes(spotLightViewFrustumPlanes, positionCmp.value, modelTransformCmp.boundingRadius))
					{
						auto const modelParams = UniformModelParams{ .modelToWorld = aoest::combine(positionCmp.value, rotationCmp.value) };
						if (modelTransformCmp.modelParams != modelParams)
						{
							modelTransformCmp.modelParams = modelParams;
							glNamedBufferSubData(modelTransformCmp.modelParamsUbo, 0, sizeof(modelParams), &modelParams);
						}

						gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboModel, modelTransformCmp.modelParamsUbo);
						gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboRig, riggedModelCmp.rigParamsUbo);
						for (auto const& mesh : riggedModelCmp.meshes)
						{
							glBindVertexArray(mesh.meshVao);
							glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
						}
					}
				}

				gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.instancedShadowMapProgram);
				for (auto const [entity, positionCmp, rotationCmp, instancedModelsCmp, modelTransformCmp] : instancedModelsEntities.each())
				{
					if (testViewFrustumPlanes(spotLightViewFrustumPlanes, positionCmp.value, modelTransformCmp.boundingRadius))
					{
						auto const modelParams = UniformModelParams{ .modelToWorld = aoest::combine(positionCmp.value, rotationCmp.value) };
						if (modelTransformCmp.modelParams != modelParams)
						{
							modelTransformCmp.modelParams = modelParams;
							glNamedBufferSubData(modelTransformCmp.modelParamsUbo, 0, sizeof(modelParams), &modelParams);
						}

						gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboModel, modelTransformCmp.modelParamsUbo);

						for (auto const& model : instancedModelsCmp.models)
						{
							for (auto const& mesh : model.meshes)
							{
								glBindVertexArray(mesh.meshVao);
								glBindVertexBuffer(
									1,
									model.instanceTransformVbo,
									0 /* offset */,
									sizeof(glm::mat4));
								glDrawElementsInstanced(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr, model.instanceCount);
							}
						}
					}
				}
			}

			// TODO: generate sun's shadow map
		}

		// VI - Depth Pre-Pass
		{
			ScopedGpuTimerQuery timerQuery(renderSceneCtx.renderPassTimerQueries[RenderPass::DepthPrePass]);
			gpuState.enableDepthTest<GpuStateChange::SurelyNo>();
			gpuState.enableDepthWrite<GpuStateChange::SurelyNo>();
			gpuState.setDepthFunc<GpuStateChange::SurelyNo>(GpuDepthFunc::Less);
			gpuState.setClearDepth<GpuStateChange::SurelyNo>(1.0);
			gpuState.enableColorWrite<GpuStateChange::SurelyYes>();
			gpuState.setClearColor<GpuStateChange::SurelyYes>(glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f });
			gpuState.disableBlend<GpuStateChange::SurelyNo>();
			gpuState.bindUbo<GpuStateChange::SurelyNo>(k_bindingUboGlobal, renderSceneCtx.globalParamsUbo);
			gpuState.bindUbo<GpuStateChange::SurelyYes>(k_bindingUboView, renderSceneCtx.viewParamsUbo);
			gpuState.bindFramebuffer<GpuStateChange::SurelyYes>(renderSceneCtx.depthFramebuffer);
			gpuState.setViewport<GpuStateChange::LikelyYes>(glm::ivec4{ 0, 0, renderSceneCtx.shadingResolution });
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.staticDepthProgram);
			for (auto const& culledStaticOpaqueMesh : culledOpaqueStaticMeshes)
			{
				gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboModel, culledStaticOpaqueMesh.modelParamsUbo);

				glBindVertexArray(culledStaticOpaqueMesh.meshVao);
				glDrawElements(GL_TRIANGLES, culledStaticOpaqueMesh.indexCount, GL_UNSIGNED_INT, nullptr);
			}

			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.riggedDepthProgram);
			for (auto const& culledOpaqueRiggedMesh : culledOpaqueRiggedMeshes)
			{
				gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboModel, culledOpaqueRiggedMesh.modelParamsUbo);
				gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboRig, culledOpaqueRiggedMesh.rigParamsUbo);

				glBindVertexArray(culledOpaqueRiggedMesh.meshVao);
				glDrawElements(GL_TRIANGLES, culledOpaqueRiggedMesh.indexCount, GL_UNSIGNED_INT, nullptr);
			}

			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.instancedDepthProgram);
			for (auto const& culledOpaqueInstancedMesh : culledOpaqueInstancedMeshes)
			{
				gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboModel, culledOpaqueInstancedMesh.modelParamsUbo);

				glBindVertexArray(culledOpaqueInstancedMesh.meshVao);
				glBindVertexBuffer(
					1,
					culledOpaqueInstancedMesh.instanceTransformVbo,
					0 /* offset */,
					sizeof(glm::mat4));
				glDrawElementsInstanced(GL_TRIANGLES, culledOpaqueInstancedMesh.indexCount, GL_UNSIGNED_INT, nullptr, culledOpaqueInstancedMesh.instanceCount);
			}
		}

		// VII - SSAO
		{
			ScopedGpuTimerQuery const timerQuery(renderSceneCtx.renderPassTimerQueries[RenderPass::SSAO]);
			gpuState.disableDepthTest<GpuStateChange::SurelyYes>();
			gpuState.disableDepthWrite<GpuStateChange::SurelyYes>();
			gpuState.disableBlend<GpuStateChange::SurelyNo>();
			gpuState.bindUbo<GpuStateChange::SurelyNo>(k_bindingUboGlobal, renderSceneCtx.globalParamsUbo);
			gpuState.bindUbo<GpuStateChange::SurelyNo>(k_bindingUboView, renderSceneCtx.viewParamsUbo);
			gpuState.bindUbo<GpuStateChange::SurelyYes>(k_bindingUboSsao, renderSceneCtx.ssaoParamsUbo);
			gpuState.bindTexture<GpuStateChange::SurelyYes>(k_bindingTextureSsaoOpaqueDepth, renderSceneCtx.opaqueDepthTexture);
			gpuState.bindTexture<GpuStateChange::SurelyYes>(k_bindingTextureSsaoOpaqueGeometricNormal, renderSceneCtx.opaqueGeometricNormalTexture);
			gpuState.bindFramebuffer<GpuStateChange::SurelyYes>(renderSceneCtx.ssaoFramebuffer);
			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.ssaoProgram);

			if (k_ssaoEnabled)
			{
				// glBindVertexArray(renderSceneCtx.postProcessVao);
				glDrawArrays(GL_TRIANGLES, 0, 3);
			}
			else
			{
				gpuState.setClearColor<GpuStateChange::LikelyYes>(glm::vec4{ 1.0 });
				glClear(GL_COLOR_BUFFER_BIT);
			}
		}

		// VIII - Direct Opaque Lighting
		{
			ScopedGpuTimerQuery const timerQuery(renderSceneCtx.renderPassTimerQueries[RenderPass::DirectOpaque]);
			gpuState.enableDepthTest<GpuStateChange::SurelyYes>();
			gpuState.disableDepthWrite<GpuStateChange::SurelyNo>();
			gpuState.setDepthFunc<GpuStateChange::SurelyYes>(GpuDepthFunc::Equal);
			gpuState.setClearDepth<GpuStateChange::SurelyNo>(1.0);
			gpuState.enableColorWrite<GpuStateChange::SurelyNo>();
			gpuState.setClearColor<GpuStateChange::LikelyYes>(glm::vec4{ 0.0 });
			gpuState.disableBlend<GpuStateChange::SurelyNo>();
			gpuState.bindUbo<GpuStateChange::SurelyNo>(k_bindingUboGlobal, renderSceneCtx.globalParamsUbo);
			gpuState.bindUbo<GpuStateChange::SurelyNo>(k_bindingUboView, renderSceneCtx.viewParamsUbo);
			gpuState.bindTexture<GpuStateChange::SurelyYes>(k_bindingTextureMaterialAmbientOcclusion, renderSceneCtx.ambientOcclusionTexture);
			gpuState.bindTexture<GpuStateChange::SurelyYes>(k_bindingTextureMaterialSunShadowMap, renderSceneCtx.sunShadowMapDepthTextureArray);
			for (int32_t i = 0; i < spotLightShadowMapCount; ++i)
			{
				gpuState.bindTexture<GpuStateChange::SurelyYes>(
					k_bindingTextureMaterialSpotLightShadowMapsBegin + i, renderSceneCtx.spotLightShadowMapTargets[i].depthTexture);
			}
			gpuState.bindFramebuffer<GpuStateChange::SurelyYes>(renderSceneCtx.directOpaqueFramebuffer);
			glClear(GL_COLOR_BUFFER_BIT);
			int32_t currentMaterialIndex = -1;
			for (auto const& culledOpaqueStaticMesh : culledOpaqueStaticMeshes)
			{
				gpuState.useProgram<GpuStateChange::LikelyNo>(culledOpaqueStaticMesh.shadingProgram);
				if (currentMaterialIndex != culledOpaqueStaticMesh.materialIndex && culledOpaqueStaticMesh.materialIndex != -1)
				{
					auto const& material = materialManager.getMaterial(culledOpaqueStaticMesh.materialIndex);
					gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboMaterial, material.paramsUbo);
					for (int32_t i = 0; i < mistd::isize(material.textureIds); ++i)
					{
						gpuState.bindTexture<GpuStateChange::LikelyYes>(k_bindingTextureMaterialCustomsBegin + i, material.textureIds[i]);
					}
					currentMaterialIndex = culledOpaqueStaticMesh.materialIndex;
				}

				gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboModel, culledOpaqueStaticMesh.modelParamsUbo);

				glBindVertexArray(culledOpaqueStaticMesh.meshVao);
				glDrawElements(GL_TRIANGLES, culledOpaqueStaticMesh.indexCount, GL_UNSIGNED_INT, nullptr);
			}
			for (auto const& culledOpaqueRiggedMesh : culledOpaqueRiggedMeshes)
			{
				gpuState.useProgram<GpuStateChange::LikelyNo>(culledOpaqueRiggedMesh.shadingProgram);
				if (currentMaterialIndex != culledOpaqueRiggedMesh.materialIndex && culledOpaqueRiggedMesh.materialIndex != -1)
				{
					auto const& material = materialManager.getMaterial(culledOpaqueRiggedMesh.materialIndex);
					gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboMaterial, material.paramsUbo);
					for (int32_t i = 0; i < mistd::isize(material.textureIds); ++i)
					{
						gpuState.bindTexture<GpuStateChange::LikelyYes>(k_bindingTextureMaterialCustomsBegin + i, material.textureIds[i]);
					}
					currentMaterialIndex = culledOpaqueRiggedMesh.materialIndex;
				}

				gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboModel, culledOpaqueRiggedMesh.modelParamsUbo);
				gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboRig, culledOpaqueRiggedMesh.rigParamsUbo);

				glBindVertexArray(culledOpaqueRiggedMesh.meshVao);
				glDrawElements(GL_TRIANGLES, culledOpaqueRiggedMesh.indexCount, GL_UNSIGNED_INT, nullptr);
			}
			for (auto const& culledOpaqueInstancedMesh : culledOpaqueInstancedMeshes)
			{
				gpuState.useProgram<GpuStateChange::LikelyNo>(culledOpaqueInstancedMesh.shadingProgram);
				if (currentMaterialIndex != culledOpaqueInstancedMesh.materialIndex && culledOpaqueInstancedMesh.materialIndex != -1)
				{
					auto const& material = materialManager.getMaterial(culledOpaqueInstancedMesh.materialIndex);
					gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboMaterial, material.paramsUbo);
					for (int32_t i = 0; i < mistd::isize(material.textureIds); ++i)
					{
						gpuState.bindTexture<GpuStateChange::LikelyYes>(k_bindingTextureMaterialCustomsBegin + i, material.textureIds[i]);
					}
					currentMaterialIndex = culledOpaqueInstancedMesh.materialIndex;
				}

				gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboModel, culledOpaqueInstancedMesh.modelParamsUbo);

				glBindVertexArray(culledOpaqueInstancedMesh.meshVao);
				glBindVertexBuffer(
					1,
					culledOpaqueInstancedMesh.instanceTransformVbo,
					0 /* offset */,
					sizeof(glm::mat4));
				glDrawElementsInstanced(GL_TRIANGLES, culledOpaqueInstancedMesh.indexCount, GL_UNSIGNED_INT, nullptr, culledOpaqueInstancedMesh.instanceCount);
			}
		}

		// IX - SSR
		{
			ScopedGpuTimerQuery const timerQuery(renderSceneCtx.renderPassTimerQueries[RenderPass::SSR]);
			gpuState.disableDepthTest<GpuStateChange::SurelyYes>();
			gpuState.disableDepthWrite<GpuStateChange::SurelyNo>();
			gpuState.enableColorWrite<GpuStateChange::SurelyNo>();
			gpuState.disableBlend<GpuStateChange::SurelyNo>();
			gpuState.bindUbo<GpuStateChange::SurelyNo>(k_bindingUboGlobal, renderSceneCtx.globalParamsUbo);
			gpuState.bindUbo<GpuStateChange::SurelyNo>(k_bindingUboView, renderSceneCtx.viewParamsUbo);
			gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboSsr, renderSceneCtx.ssrParamsUbo);
			gpuState.bindTexture<GpuStateChange::SurelyYes>(k_bindingTextureSsrDirectOpaqueColor, renderSceneCtx.directOpaqueColorTexture);
			gpuState.bindTexture<GpuStateChange::SurelyYes>(k_bindingTextureSsrOpaqueSurface, renderSceneCtx.opaqueSurfaceTexture);
			gpuState.bindTexture<GpuStateChange::SurelyYes>(k_bindingTextureSsrOpaqueNormal, renderSceneCtx.opaqueNormalTexture);
			gpuState.bindTexture<GpuStateChange::SurelyYes>(k_bindingTextureSsrOpaqueDepth, renderSceneCtx.opaqueDepthTexture);
			gpuState.bindFramebuffer<GpuStateChange::SurelyYes>(renderSceneCtx.ssrFramebuffer);
			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.ssrProgram);

			if (k_ssrEnabled)
			{
				glBindVertexArray(renderSceneCtx.postProcessVao);
				glDrawArrays(GL_TRIANGLES, 0, 3);
				glGenerateTextureMipmap(renderSceneCtx.ssrColorTexture);
			}
			else
			{
				gpuState.setClearColor<GpuStateChange::LikelyYes>(glm::vec4{ 0.0 });
				glClear(GL_COLOR_BUFFER_BIT);
			}
		}

		// X - Opaque Composition
		{
			ScopedGpuTimerQuery const timerQuery(renderSceneCtx.renderPassTimerQueries[RenderPass::OpaqueComposition]);
			gpuState.disableDepthTest<GpuStateChange::SurelyNo>();
			gpuState.disableDepthWrite<GpuStateChange::SurelyNo>();
			gpuState.enableColorWrite<GpuStateChange::SurelyNo>();
			gpuState.disableBlend<GpuStateChange::SurelyNo>();
			gpuState.bindTexture<GpuStateChange::SurelyNo>(k_bindingTextureOpaqueCompositionDirectOpaqueColor, renderSceneCtx.directOpaqueColorTexture);
			gpuState.bindTexture<GpuStateChange::SurelyNo>(k_bindingTextureOpaqueCompositionOpaqueSurface, renderSceneCtx.opaqueSurfaceTexture);
			gpuState.bindTexture<GpuStateChange::SurelyYes>(k_bindingTextureOpaqueCompositionSsrColor, renderSceneCtx.ssrColorTexture);
			gpuState.bindTexture<GpuStateChange::SurelyYes>(k_bindingTextureOpaqueCompositionEnvironmentCubeMap, renderSceneCtx.environmentCubeMap);
			gpuState.bindFramebuffer<GpuStateChange::SurelyYes>(renderSceneCtx.finalFramebuffer);
			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.opaqueCompositionProgram);

			// glBindVertexArray(renderSceneCtx.postProcessVao);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}

		// XI - Translucent
		{
			ScopedGpuTimerQuery const timerQuery(renderSceneCtx.renderPassTimerQueries[RenderPass::Translucent]);
			// TODO
			gpuState.enableDepthTest<GpuStateChange::SurelyYes>();
			gpuState.disableDepthWrite<GpuStateChange::SurelyNo>();
			gpuState.setDepthFunc<GpuStateChange::SurelyYes>(GpuDepthFunc::Less);
			gpuState.enableColorWrite<GpuStateChange::SurelyNo>();
			gpuState.enableBlend<GpuStateChange::SurelyYes>();
		}

		// XII - Sky Box
		if (renderSceneCtx.skyBoxProgram != k_invalidId)
		{
			ScopedGpuTimerQuery const timerQuery(renderSceneCtx.renderPassTimerQueries[RenderPass::SkyBox]);
			gpuState.enableDepthTest<GpuStateChange::SurelyNo>();
			gpuState.setDepthFunc<GpuStateChange::SurelyYes>(GpuDepthFunc::Equal);
			gpuState.bindFramebuffer<GpuStateChange::SurelyNo>(renderSceneCtx.finalFramebuffer);
			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.skyBoxProgram);
			// TODO: ubo?
			glBindVertexArray(renderSceneCtx.postProcessVao);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}

		// XIII - Post Processes
		if (k_debugMode != DebugMode2::None)
		{
			auto debugParams = UniformDebugParams{};
			auto viewParamsUbo = renderSceneCtx.viewParamsUbo;
			auto debugTexture = k_invalidId;
			auto debugTextureArray = k_invalidId;
			switch (k_debugMode)
			{
			case DebugMode2::LightClusters:
			{
				debugTexture = renderSceneCtx.opaqueDepthTexture;
				debugParams.type = static_cast<uint8_t>(DebugType::LightClusters);
				break;
			}
			case DebugMode2::SunShadowMap:
			{
				auto const sunViewParams = UniformViewParams{
					.nearClip = debugSunNear,
					.farClip = debugSunFar
				};
				glNamedBufferSubData(renderSceneCtx.lightViewParamsUbo, 0, sizeof(sunViewParams), &sunViewParams);
				viewParamsUbo = renderSceneCtx.lightViewParamsUbo;
				debugTextureArray = renderSceneCtx.sunShadowMapDepthTextureArray;
				debugParams.type = static_cast<int8_t>(DebugType::DepthTextureArray);
				debugParams.index = static_cast<int8_t>(k_debugShadowMapIndex);
				break;
			}
			case DebugMode2::SpotLightShadowMap:
			{
				auto const spotLightViewParams = UniformViewParams{
					.worldToClip = shadowParams.spotLights[k_debugShadowMapIndex].worldToClip,
					.viewToWorld = shadowParams.spotLights[k_debugShadowMapIndex].viewToWorld,
					.resolution = renderSceneCtx.spotLightShadowMapTargets[k_debugShadowMapIndex].resolution,
					.nearClip = shadowParams.spotLights[k_debugShadowMapIndex].nearClip,
					.farClip = shadowParams.spotLights[k_debugShadowMapIndex].farClip,
					.fov = shadowParams.spotLights[k_debugShadowMapIndex].fov,
					.aspectRatio = 1.0f
				};
				glNamedBufferSubData(renderSceneCtx.lightViewParamsUbo, 0, sizeof(spotLightViewParams), &spotLightViewParams);
				viewParamsUbo = renderSceneCtx.lightViewParamsUbo;
				debugTexture = renderSceneCtx.spotLightShadowMapTargets[k_debugShadowMapIndex].depthTexture;
				debugParams.type = static_cast<uint8_t>(DebugType::DepthTexture);
				break;
			}
			case DebugMode2::OpaqueDepth:
			{
				debugTexture = renderSceneCtx.opaqueDepthTexture;
				debugParams.type = static_cast<uint8_t>(DebugType::DepthTexture);
				break;
			}
			case DebugMode2::OpaqueGeometricNormal:
			{
				debugTexture = renderSceneCtx.opaqueGeometricNormalTexture;
				debugParams.type = static_cast<uint8_t>(DebugType::DirectionTexture);
				break;
			}
			case DebugMode2::AmbientOcclusion:
			{
				debugTexture = renderSceneCtx.ambientOcclusionTexture;
				debugParams.type = static_cast<uint8_t>(DebugType::ShadesTexture);
				break;
			}
			case DebugMode2::DirectOpaqueColor:
			{
				debugTexture = renderSceneCtx.directOpaqueColorTexture;
				debugParams.type = static_cast<uint8_t>(DebugType::ColorTexture);
				break;
			}
			case DebugMode2::OpaqueNormal:
			{
				debugTexture = renderSceneCtx.opaqueNormalTexture;
				debugParams.type = static_cast<uint8_t>(DebugType::DirectionTexture);
				break;
			}
			case DebugMode2::OpaqueSurface:
			{
				debugTexture = renderSceneCtx.opaqueSurfaceTexture;
				debugParams.type = static_cast<uint8_t>(DebugType::ColorTexture);
				break;
			}
			case DebugMode2::SsrColor:
			{
				debugTexture = renderSceneCtx.ssrColorTexture;
				debugParams.type = static_cast<uint8_t>(DebugType::ColorTexture);
				break;
			}
			case DebugMode2::FinalColor:
			{
				debugTexture = renderSceneCtx.finalColorTexture;
				debugParams.type = static_cast<uint8_t>(DebugType::ColorTexture);
				break;
			}
			}
			glNamedBufferSubData(renderSceneCtx.debugParamsUbo, 0, sizeof(debugParams), &debugParams);

			gpuState.disableDepthTest<GpuStateChange::SurelyYes>();
			gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboView, viewParamsUbo);
			gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboDebug, renderSceneCtx.debugParamsUbo);
			gpuState.bindTexture<GpuStateChange::LikelyYes>(k_bindingTextureDebug, debugTexture);
			gpuState.bindTexture<GpuStateChange::LikelyYes>(k_bindingTextureArrayDebug, debugTextureArray);

			gpuState.bindFramebuffer<GpuStateChange::SurelyYes>(window.getDefaultFramebufferId());
			gpuState.setViewport<GpuStateChange::LikelyYes>(glm::ivec4{ 0, 0, renderSceneCtx.postProcessResolution });
			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.debugProgram);

			glBindVertexArray(renderSceneCtx.postProcessVao);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
		else
		{
			ScopedGpuTimerQuery const timerQuery(renderSceneCtx.renderPassTimerQueries[RenderPass::PostProcesses]);
			gpuState.disableDepthTest<GpuStateChange::SurelyYes>();
			gpuState.enableColorWrite<GpuStateChange::SurelyNo>();
			gpuState.disableBlend<GpuStateChange::SurelyYes>();
			glBindVertexArray(renderSceneCtx.postProcessVao);

			if (!renderSceneCtx.postProcesses.empty())
			{
				auto nextTexture = renderSceneCtx.finalColorTexture;
				for (int32_t i = 0; i < mistd::isize(renderSceneCtx.postProcesses) - 1; ++i)
				{
					gpuState.bindTexture<GpuStateChange::SurelyYes>(k_bindingTexturePostProcessColor, nextTexture);
					gpuState.bindFramebuffer<GpuStateChange::SurelyYes>(renderSceneCtx.postProcessTargets[i % 2].framebuffer);
					nextTexture = renderSceneCtx.postProcessTargets[i % 2].colorTexture;

					auto const& postProcess = renderSceneCtx.postProcesses[i];
					// Few post processes need ubo.
					gpuState.bindUbo<GpuStateChange::LikelyNo>(k_bindingUboPostProcess, postProcess.ubo);
					gpuState.useProgram<GpuStateChange::SurelyYes>(postProcess.program);

					// glBindVertexArray(renderSceneCtx.postProcessVao);
					glDrawArrays(GL_TRIANGLES, 0, 3);
				}

				gpuState.bindTexture<GpuStateChange::SurelyYes>(k_bindingTexturePostProcessColor, nextTexture);
				gpuState.bindFramebuffer<GpuStateChange::SurelyYes>(window.getDefaultFramebufferId());

				auto const& lastPostProcess = renderSceneCtx.postProcesses.back();
				gpuState.bindUbo<GpuStateChange::LikelyNo>(k_bindingUboPostProcess, lastPostProcess.ubo);
				gpuState.useProgram<GpuStateChange::SurelyYes>(lastPostProcess.program);

				// glBindVertexArray(renderSceneCtx.postProcessVao);
				glDrawArrays(GL_TRIANGLES, 0, 3);
			}
			else
			{
				glBindFramebuffer(GL_READ_FRAMEBUFFER, renderSceneCtx.finalFramebuffer);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, window.getDefaultFramebufferId());
				glBlitFramebuffer(
					0, 0, renderSceneCtx.shadingResolution.x, renderSceneCtx.shadingResolution.y,
					0, 0, window.getSize().x, window.getSize().y,
					GL_COLOR_BUFFER_BIT,
					GL_NEAREST);
			}
		}
		glQueryCounter(renderSceneCtx.totalTimerQueries[1], GL_TIMESTAMP);

		// XIV - Debug Geometry
		if (!debugMeshCtx.lines.empty())
		{
			glClearDepth(1.0);
			gpuState.disableDepthTest<GpuStateChange::SurelyNo>();
			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.debugGeometryProgram);
			glLineWidth(2);

			glBindVertexArray(renderSceneCtx.debugGeometryVao);

			glBindBuffer(GL_ARRAY_BUFFER, renderSceneCtx.debugGeometryVbo);
			glBufferData(
				GL_ARRAY_BUFFER,
				debugMeshCtx.vertices.size() * sizeof(decltype(debugMeshCtx.vertices.front())),
				debugMeshCtx.vertices.data(),
				GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderSceneCtx.debugGeometryEbo);
			glBufferData(
				GL_ELEMENT_ARRAY_BUFFER,
				debugMeshCtx.lines.size() * sizeof(decltype(debugMeshCtx.lines.front())),
				debugMeshCtx.lines.data(),
				GL_STATIC_DRAW);

			glDrawElements(GL_LINES, 2 * mistd::isize(debugMeshCtx.lines), GL_UNSIGNED_INT, nullptr);

			debugMeshCtx.clear();
		}

		if (ImGui::Begin("Render Performance"))
		{
			glFinish();
			static int32_t accumulationIndex = 0;
			static int32_t accumulationCount = 50;
			static float totalDuration = 0.0f;
			static uint64_t totalAccumulation = 0;
			static mistd::enum_map<RenderPass, float> renderPassDurations;
			static mistd::enum_map<RenderPass, uint64_t> renderPassAccumulations;
			for (auto renderPass : mistd::enum_traits<RenderPass>::valid_values)
			{
				uint64_t durationMs;
				glGetQueryObjectui64v(renderSceneCtx.renderPassTimerQueries[renderPass], GL_QUERY_RESULT, &durationMs);
				renderPassAccumulations[renderPass] += durationMs;
			}
			uint64_t totalStartTimeMs;
			glGetQueryObjectui64v(renderSceneCtx.totalTimerQueries[0], GL_QUERY_RESULT, &totalStartTimeMs);
			uint64_t totalStopTimeMs;
			glGetQueryObjectui64v(renderSceneCtx.totalTimerQueries[1], GL_QUERY_RESULT, &totalStopTimeMs);
			totalAccumulation += totalStopTimeMs - totalStartTimeMs;
			if (++accumulationIndex == accumulationCount)
			{
				for (auto renderPass : mistd::enum_traits<RenderPass>::valid_values)
				{
					renderPassDurations[renderPass] = (float(renderPassAccumulations[renderPass]) / 1'000'000.0f) / accumulationCount;
					renderPassAccumulations[renderPass] = 0;
				}
				totalDuration = (float(totalAccumulation) / 1'000'000.0f) / accumulationCount;
				totalAccumulation = 0;
				accumulationIndex = 0;
			}

			ImGui::BeginDisabled();
			int32_t lightCount = mistd::isize(gpuLights);
			ImGui::InputInt("Light Count", &lightCount);
			int32_t staticOpaqueMeshCount = mistd::isize(culledOpaqueStaticMeshes);
			ImGui::InputInt("Static Mesh Count", &staticOpaqueMeshCount);
			int32_t riggedOpaqueMeshCount = mistd::isize(culledOpaqueRiggedMeshes);
			ImGui::InputInt("Rigged Mesh Count", &riggedOpaqueMeshCount);

			auto const toSmallStr = [](std::string_view a_stringView)
				{
					constexpr size_t k_maxSize = 16;
					auto size = std::min(a_stringView.size(), k_maxSize);
					std::array<char, k_maxSize + 1> smallStr;
					std::memcpy(smallStr.data(), a_stringView.data(), size);
					smallStr[size] = 0;
					return smallStr;
				};
			for (auto renderPass : mistd::enum_traits<RenderPass>::valid_values)
			{
				auto const renderPassName = mistd::enum_traits<RenderPass>::cast(renderPass).value_or("");
				auto const renderPassStr = toSmallStr(renderPassName.substr(renderPassName.rfind(':') + 1));
				ImGui::InputFloat(renderPassStr.data(), &renderPassDurations[renderPass]);
			}
			ImGui::InputFloat("TOTAL", &totalDuration);

			ImGui::EndDisabled();
		}
		ImGui::End();
	}
}
