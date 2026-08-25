#include "vob/aoe/rendering/systems/RenderSceneSystem.h"

#include "vob/aoe/rendering/CameraUtils.h"
#include "vob/aoe/rendering/components/InstancedModelsComponent.h"
#include "vob/aoe/rendering/components/ModelComponent.h"
#include "vob/aoe/rendering/components/ModelTransformComponent.h"
#include "vob/aoe/rendering/GpuState.h"
#include "vob/aoe/rendering/MaterialUtils.h"
#include "vob/aoe/rendering/ProgramUtils.h"

#include "vob/aoe/debug/Check.h"
#include "vob/aoe/debug/DebugNameUtils.h"
#include "vob/aoe/debug/ImGuiUtils.h"
#include "vob/aoe/spacetime/TransformUtils.h"

#include <vob/misc/std/container_util.h>
#include <vob/misc/std/enum_traits.h>

#include "vob/aoe/rendering/shaders/defines.h"

#include <glm/gtx/quaternion.hpp>
#include <imgui.h>

#include <algorithm>
#include <array>
#include <limits>


namespace vob::aoegl
{
	void RenderSceneSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_timeContext.init(a_wdar);
		m_cameraDirectorContext.init(a_wdar);
		m_renderSceneCtx.init(a_wdar);
		m_renderProfilingCtx.init(a_wdar);
		m_gpuResourceRegistriesContext.init(a_wdar);
		m_debugProgramContext.init(a_wdar);
		m_debugMaterialContext.init(a_wdar);
		m_debugMeshContext.init(a_wdar);
		m_debugRenderInspectorCtx.init(a_wdar);
		m_windowContext.init(a_wdar);
		m_focusEntities.init(a_wdar);
		m_cameraEntities.init(a_wdar);
		m_lightEntities.init(a_wdar);
		m_modelEntities.init(a_wdar);
		m_staticModelEntities.init(a_wdar);
		m_riggedModelEntities.init(a_wdar);
		m_instancedModelsEntities.init(a_wdar);
	}

	namespace
	{
		struct CulledLight
		{
			float importance;
			glm::vec3 position;
			glm::quat rotation;
			LightComponent const* lightComponent;
		};

		UniformGlobalParams createGlobalParams(aoest::TimeContext const& a_timeCtx)
		{
			return UniformGlobalParams{
				.worldTime = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - a_timeCtx.worldStartTime).count()
			};
		}

		std::pair<UniformViewParams, glm::dvec3> createViewParams(
			aoewi::WindowContext const& a_windowCtx,
			entt::entity a_cameraEntity,
			entt::view<entt::get_t<aoest::PositionComponent const, aoest::RotationComponent const, CameraComponent const>> a_cameraEntities)
		{
			auto const displayResolution = a_windowCtx.window.get().getSize();
			auto const aspectRatio = static_cast<float>(displayResolution.x) / displayResolution.y;

			auto const [realCameraPosition, rotation, nearClip, farClip, fov] = getCameraProperties(a_cameraEntities, a_cameraEntity);
			
			auto const worldOriginPosition = 1024.0 * glm::floor(realCameraPosition / 1024.0);
			auto const cameraPosition = glm::vec3{ realCameraPosition - worldOriginPosition };
			
			auto const viewToWorld = aoest::combine(cameraPosition, rotation);
			auto const worldToView = glm::inverse(viewToWorld);
			auto const viewToClip = glm::perspective(fov, aspectRatio, nearClip, farClip);
			auto const worldToClip = viewToClip * worldToView;
			auto const clipToView = glm::inverse(viewToClip);
			
			return std::make_pair(UniformViewParams{
				.worldToView = worldToView,
				.viewToClip = viewToClip,
				.worldToClip = worldToClip,
				.clipToView = clipToView,
				.viewToWorld = viewToWorld,
				.nearClip = nearClip,
				.farClip = farClip,
				.fov = fov,
				.aspectRatio = aspectRatio
			}, worldOriginPosition);
		}

		template <typename TMesh, typename TGetProgram>
		void sortCulledMeshes(std::vector<TMesh>& a_meshes, TGetProgram a_getProgram)
		{
			std::sort(a_meshes.begin(), a_meshes.end(), [a_getProgram](TMesh const& a_lhs, TMesh const& a_rhs)
				{
					if (a_getProgram(a_lhs) != a_getProgram(a_rhs))
					{
						return a_getProgram(a_lhs) < a_getProgram(a_rhs);
					}
					if (a_lhs.material != a_rhs.material)
					{
						return a_lhs.material < a_rhs.material;
					}
					return a_lhs.vao < a_rhs.vao;
				});
		}

		static float texelSize = 1.0;
		std::tuple<UniformLightingParams, UniformShadowParams, int32_t> createLightingAndShadowParams(
			ViewFrustumPlanes const& a_viewFrustumPlanes,
			glm::dvec3 const& a_lightFocusPosition,
			entt::view<entt::get_t<aoest::PositionComponent const, aoest::RotationComponent const, LightComponent const>> a_lightEntities,
			int32_t a_lightsCapacity,
			glm::ivec2 const& a_lightClusterResolution,
			glm::ivec2 const& a_lightClusterTileSize,
			int32_t a_lightClusterZCount,
			int32_t a_lightClusterCapacity,
			glm::dvec3 const& a_worldOriginPosition,
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
				if (!testViewFrustumPlanes(a_viewFrustumPlanes, positionCmp.value - a_worldOriginPosition, lightCmp.radius))
				{
					continue;
				}

				auto const distanceImportance = 1.0f - static_cast<float>(glm::length(positionCmp.value - a_lightFocusPosition)) / lightCmp.radius;
				auto const colorImportance = glm::dot(lightCmp.color, glm::vec3{ 0.299, 0.587, 0.114f });
				auto const intensityImportance = lightCmp.intensity;
				auto const importance = distanceImportance * colorImportance * intensityImportance;

				culledLights.emplace_back(importance, positionCmp.value - a_worldOriginPosition, rotationCmp.value, &lightCmp);
			}
			std::sort(culledLights.begin(), culledLights.end(), [](auto const& lhs, auto const& rhs) { return lhs.importance > rhs.importance; });
			auto const lightingParams = UniformLightingParams{
				.ambientIntensity = 1.0f,
				.lightCount = std::min(mistd::isize(culledLights), a_lightsCapacity),
				.lightClusterResolution = a_lightClusterResolution,
				.lightClusterTileSize = a_lightClusterTileSize,
				.lightClusterZCount = a_lightClusterZCount,
				.lightClusterCapacity = a_lightClusterCapacity,
				.sunColor = glm::vec3{ 1.0f, 0.5f, 0.4f },
				.sunIntensity = 3.0f,
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

		void beginPass(
			GpuState& a_gpuState
			, GraphicId a_framebuffer
			, glm::ivec2 a_resolution
			, GraphicId a_targetParamsUbo)
		{
			a_gpuState.bindFramebuffer<GpuStateChange::SurelyYes>(a_framebuffer);
			a_gpuState.setViewport<GpuStateChange::LikelyYes>(glm::ivec4{ 0, 0, a_resolution });

			auto const targetParams = UniformTargetParams{
				.resolution = a_resolution,
				.invResolution = 1.0f / glm::vec2{ a_resolution } };
			glNamedBufferSubData(a_targetParamsUbo, 0, sizeof(targetParams), &targetParams);
			a_gpuState.bindUbo<GpuStateChange::LikelyNo>(k_bindingUboTarget, a_targetParamsUbo);
		}

		void captureDebugRenderOutput(
			DebugRenderInspectorContext& a_inspectorCtx
			, GraphicId a_texture
			, GraphicEnum a_sourceTarget
			, int32_t a_sourceLayer
			, int32_t a_sourceLevel
			, DebugType a_type
			, glm::vec2 a_depthRange)
		{
			auto resolution = glm::ivec2{};
			auto internalFormat = GraphicInt{};
			glGetTextureLevelParameteriv(a_texture, a_sourceLevel, GL_TEXTURE_WIDTH, &resolution.x);
			glGetTextureLevelParameteriv(a_texture, a_sourceLevel, GL_TEXTURE_HEIGHT, &resolution.y);
			glGetTextureLevelParameteriv(a_texture, a_sourceLevel, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);

			if (a_inspectorCtx.capturedTexture == k_invalidId
				|| a_inspectorCtx.capturedResolution != resolution
				|| a_inspectorCtx.capturedInternalFormat != static_cast<GraphicEnum>(internalFormat))
			{
				if (a_inspectorCtx.capturedTexture != k_invalidId)
				{
					glDeleteTextures(1, &a_inspectorCtx.capturedTexture);
				}
				glCreateTextures(GL_TEXTURE_2D, 1, &a_inspectorCtx.capturedTexture);
				glTextureStorage2D(a_inspectorCtx.capturedTexture, 1, internalFormat, resolution.x, resolution.y);
				glTextureParameteri(a_inspectorCtx.capturedTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTextureParameteri(a_inspectorCtx.capturedTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				a_inspectorCtx.capturedResolution = resolution;
				a_inspectorCtx.capturedInternalFormat = static_cast<GraphicEnum>(internalFormat);
			}

			glCopyImageSubData(
				a_texture, a_sourceTarget, a_sourceLevel, 0, 0, a_sourceLayer
				, a_inspectorCtx.capturedTexture, GL_TEXTURE_2D, 0, 0, 0, 0
				, resolution.x, resolution.y, 1);

			a_inspectorCtx.capturedType = a_type;
			a_inspectorCtx.capturedDepthRange = a_depthRange;
		}

		void debugInspectRenderOutputMipChannel(
			DebugRenderInspectorContext& a_inspectorCtx
			, std::string_view a_name
			, GraphicId a_texture
			, int32_t a_level
			, int32_t a_channel
			, DebugType a_type
			, glm::vec2 a_depthRange = glm::vec2{ 0.0f })
		{
			a_inspectorCtx.names.push_back(a_name);
			if (a_name != a_inspectorCtx.selectedName)
			{
				return;
			}

			captureDebugRenderOutput(a_inspectorCtx, a_texture, GL_TEXTURE_2D, 0, a_level, a_type, a_depthRange);
			a_inspectorCtx.capturedChannel = a_channel;
		}

		int32_t debugInspectIndex(
			DebugRenderInspectorContext& a_inspectorCtx
			, std::string_view a_name
			, int32_t a_count)
		{
			if (a_name != a_inspectorCtx.selectedName || a_count <= 0)
			{
				return 0;
			}

			a_inspectorCtx.selectedIndexCount = a_count;
			return std::clamp(a_inspectorCtx.selectedIndex, 0, a_count - 1);
		}

		void debugInspectRenderOutput(
			DebugRenderInspectorContext& a_inspectorCtx
			, std::string_view a_name
			, GraphicId a_texture
			, DebugType a_type
			, glm::vec2 a_depthRange = glm::vec2{ 0.0f })
		{
			a_inspectorCtx.names.push_back(a_name);
			if (a_name != a_inspectorCtx.selectedName)
			{
				return;
			}

			captureDebugRenderOutput(a_inspectorCtx, a_texture, GL_TEXTURE_2D, 0, 0, a_type, a_depthRange);
			a_inspectorCtx.capturedChannel = 0;
		}

		void debugInspectRenderOutputMip(
			DebugRenderInspectorContext& a_inspectorCtx
			, std::string_view a_name
			, GraphicId a_texture
			, int32_t a_level
			, DebugType a_type
			, glm::vec2 a_depthRange = glm::vec2{ 0.0f })
		{
			a_inspectorCtx.names.push_back(a_name);
			if (a_name != a_inspectorCtx.selectedName)
			{
				return;
			}

			captureDebugRenderOutput(a_inspectorCtx, a_texture, GL_TEXTURE_2D, 0, a_level, a_type, a_depthRange);
		}

		void debugInspectRenderOutputLayer(
			DebugRenderInspectorContext& a_inspectorCtx
			, std::string_view a_name
			, GraphicId a_textureArray
			, int32_t a_layer
			, DebugType a_type
			, glm::vec2 a_depthRange = glm::vec2{ 0.0f })
		{
			a_inspectorCtx.names.push_back(a_name);
			if (a_name != a_inspectorCtx.selectedName)
			{
				return;
			}

			captureDebugRenderOutput(
				a_inspectorCtx, a_textureArray, GL_TEXTURE_2D_ARRAY, a_layer, 0, a_type, a_depthRange);
		}

		void debugDrawInspectedRenderOutput(
			GpuState& a_gpuState
			, RenderSceneContext const& a_renderSceneCtx
			, DebugRenderInspectorContext const& a_inspectorCtx
			, GraphicId a_framebuffer
			, glm::ivec2 a_resolution)
		{
			auto const debugParams = UniformDebugParams{
				.depthRange = a_inspectorCtx.capturedDepthRange,
				.exposure = a_inspectorCtx.exposure,
				.channel = a_inspectorCtx.capturedChannel,
				.type = static_cast<int8_t>(a_inspectorCtx.capturedType)
			};
			glNamedBufferSubData(a_renderSceneCtx.debugParamsUbo, 0, sizeof(debugParams), &debugParams);

			a_gpuState.disableDepthTest<GpuStateChange::LikelyNo>();
			a_gpuState.disableBlend<GpuStateChange::LikelyNo>();
			a_gpuState.enableColorWrite<GpuStateChange::LikelyNo>();
			a_gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboView, a_renderSceneCtx.viewParamsUbo);
			a_gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboDebug, a_renderSceneCtx.debugParamsUbo);
			a_gpuState.bindTexture<GpuStateChange::LikelyYes>(k_bindingTextureDebugSource, a_inspectorCtx.capturedTexture);

			beginPass(a_gpuState, a_framebuffer, a_resolution, a_renderSceneCtx.targetParamsUbo);
			a_gpuState.useProgram<GpuStateChange::LikelyYes>(a_renderSceneCtx.debugProgram);

			glBindVertexArray(a_renderSceneCtx.postProcessVao);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}

	void RenderSceneSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto& debugMeshCtx = m_debugMeshContext.get(a_wdap);
		auto& renderSceneCtx = m_renderSceneCtx.get(a_wdap);
		auto& renderProfilingCtx = m_renderProfilingCtx.get(a_wdap);
		auto& debugRenderInspectorCtx = m_debugRenderInspectorCtx.get(a_wdap);
		auto& debugProgramCtx = m_debugProgramContext.get(a_wdap);
		auto const& gpuResourceRegistriesCtx = m_gpuResourceRegistriesContext.get(a_wdap);
		auto const& materialRegistry = *gpuResourceRegistriesCtx.materialRegistry;
		auto const& shaderRegistry = *gpuResourceRegistriesCtx.shaderRegistry;
		auto const& window = m_windowContext.get(a_wdap).window.get();
		auto const& cameraDirectorCtx = m_cameraDirectorContext.get(a_wdap);
		auto modelEntities = m_modelEntities.get(a_wdap);
		auto staticModelEntities = m_staticModelEntities.get(a_wdap);
		auto riggedModelEntities = m_riggedModelEntities.get(a_wdap);
		auto instancedModelsEntities = m_instancedModelsEntities.get(a_wdap);
		GpuState gpuState;

		// HACK: make sun rotate
		{
			static auto startT = std::chrono::high_resolution_clock::now();
			static auto prevT = startT;
			auto const t = std::chrono::high_resolution_clock::now();
			if (window.isGamepadButtonPressed(0, aoein::Gamepad::Button::LS))
			{
				startT -= 9 * (t - prevT);
			}
			prevT = t;
			auto const d = 60.0f + std::chrono::duration<float>(t - startT).count();
			auto const a = std::fmod(0.03f * d, 2.0f * std::numbers::pi_v<float>);
			auto const sunDir = glm::vec3{ glm::rotate(glm::mat4{1.0f}, a, glm::vec3{0.0f, 1.0f, 0.0f})
				* glm::normalize(glm::vec4{ 0.7f, 0.15f, -1.0f, 0.0f }) };
			renderSceneCtx.sunDir = sunDir;
		}

		// 0 - Prepare Debug
		static bool k_debugCameraFrustum = false;
		static bool k_ssaoEnabled = true;
		static bool k_ssrEnabled = true;
		static bool k_bloomEnabled = true;
		if (ImGui::Begin("Render Debug"))
		{
			auto& inspectedName = debugRenderInspectorCtx.selectedName;
			if (ImGui::BeginCombo("Inspect", inspectedName.empty() ? "None" : inspectedName.data()))
			{
				if (ImGui::Selectable("None", inspectedName.empty()))
				{
					inspectedName = {};
				}
				for (auto const name : debugRenderInspectorCtx.names)
				{
					if (ImGui::Selectable(name.data(), name == inspectedName))
					{
						inspectedName = name;
					}
				}
				ImGui::EndCombo();
			}
			ImGui::SliderFloat("Inspect Exposure", &debugRenderInspectorCtx.exposure, 0.01f, 100.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
			if (debugRenderInspectorCtx.selectedIndexCount > 0)
			{
				ImGui::InputInt("Inspect Index", &debugRenderInspectorCtx.selectedIndex);
				debugRenderInspectorCtx.selectedIndex = std::clamp(
					debugRenderInspectorCtx.selectedIndex, 0, debugRenderInspectorCtx.selectedIndexCount - 1);
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
			static int k_ssrStepCount = 64;
			ImGui::SliderInt("Step Count", &k_ssrStepCount, 4, 256);
			static float k_ssrMaxRange = 500.0f;
			ImGui::SliderFloat("Max Range", &k_ssrMaxRange, 1.0f, 1000.0f, "%.1f", ImGuiSliderFlags_Logarithmic);

			static bool k_ssrDebugExitReason = false;
			ImGui::Checkbox("Debug Exit Reason##ssr", &k_ssrDebugExitReason);

			static bool k_ssrDebugPenetration = false;
			ImGui::Checkbox("Debug Penetration##ssr", &k_ssrDebugPenetration);

			static bool k_ssrDebugRay = false;
			static glm::ivec2 k_ssrDebugRayPixel = renderSceneCtx.shadingResolution / 2;
			ImGui::Checkbox("Debug Ray##ssr", &k_ssrDebugRay);
			if (k_ssrDebugRay)
			{
				ImGui::SliderInt("Ray Pixel X", &k_ssrDebugRayPixel.x, 0, renderSceneCtx.shadingResolution.x - 1);
				ImGui::SliderInt("Ray Pixel Y", &k_ssrDebugRayPixel.y, 0, renderSceneCtx.shadingResolution.y - 1);
			}

			static float k_ssrPenetrationBlockedRatio = 0.003f;
			ImGui::SliderFloat(
				"Penetration Blocked Below", &k_ssrPenetrationBlockedRatio, 0.0001f, 2.0f, "%.4f", ImGuiSliderFlags_Logarithmic);
			static float k_ssrPenetrationThroughRatio = 0.3f;
			ImGui::SliderFloat(
				"Penetration Through Above", &k_ssrPenetrationThroughRatio, 0.0001f, 2.0f, "%.4f", ImGuiSliderFlags_Logarithmic);


			auto const ssrParams = UniformSsrParams{
				.stepCount = k_ssrStepCount,
				.debugExitReason = k_ssrDebugExitReason ? 1 : 0,
				.maxRange = k_ssrMaxRange,
				.debugRay = k_ssrDebugRay ? 1 : 0,
				.debugRayPixel = k_ssrDebugRayPixel,
				.penetrationBlockedRatio = k_ssrPenetrationBlockedRatio,
				.penetrationThroughRatio = k_ssrPenetrationThroughRatio,
				.debugPenetration = k_ssrDebugPenetration ? 1 : 0
			};
			glNamedBufferSubData(renderSceneCtx.ssrParamsUbo, 0, sizeof(ssrParams), &ssrParams);

			ImGui::SeparatorText("Bloom");
			ImGui::Checkbox("Enable##bloom", &k_bloomEnabled);
			static float k_bloomScatter = 0.5f;
			ImGui::SliderFloat("Scatter", &k_bloomScatter, 0.0f, 3.0f);
			static float k_bloomStrength = 0.05f;
			ImGui::SliderFloat("Strength", &k_bloomStrength, 0.0f, 1.0f);
			static float k_bloomFilterRadius = 1.0f;
			ImGui::SliderFloat("Filter Radius", &k_bloomFilterRadius, 0.5f, 3.0f);
			static bool k_bloomKarisAverage = true;
			ImGui::Checkbox("Karis Average", &k_bloomKarisAverage);

			auto const bloomLevelCount = static_cast<float>(mistd::isize(renderSceneCtx.bloomMips));
			auto const bloomParams = UniformBloomParams{
				.filterRadius = k_bloomFilterRadius,
				.scatter = k_bloomScatter,
				.strength = k_bloomStrength,
				.useKarisAverage = k_bloomKarisAverage ? 1 : 0,
				.totalWeight = std::abs(1.0f - k_bloomScatter) < 1e-4f
					? bloomLevelCount
					: (1.0f - std::pow(k_bloomScatter, bloomLevelCount)) / (1.0f - k_bloomScatter)
			};
			glNamedBufferSubData(renderSceneCtx.bloomParamsUbo, 0, sizeof(bloomParams), &bloomParams);

			ImGui::SeparatorText("Tonemap");
			static float k_tonemapExposure = 1.0f;
			ImGui::SliderFloat("Exposure", &k_tonemapExposure, 0.05f, 20.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
			static glm::vec3 k_tonemapColorFilter{ 1.0f };
			ImGui::ColorEdit3("Color Filter", &k_tonemapColorFilter.x);
			static float k_tonemapContrast = 1.0f;
			ImGui::SliderFloat("Contrast", &k_tonemapContrast, 0.0f, 2.0f);
			static float k_tonemapSaturation = 1.0f;
			ImGui::SliderFloat("Saturation", &k_tonemapSaturation, 0.0f, 2.0f);

			auto const tonemapParams = UniformTonemapParams{
				.colorFilter = k_tonemapColorFilter,
				.exposure = k_tonemapExposure,
				.contrast = k_tonemapContrast,
				.saturation = k_tonemapSaturation
			};
			glNamedBufferSubData(renderSceneCtx.tonemapParamsUbo, 0, sizeof(tonemapParams), &tonemapParams);

			ImGui::SeparatorText("Shaders");
			auto& activeShaderIndex = debugProgramCtx.activeShaderIndex;
			auto const toSmallStr = [](std::string_view a_stringView)
				{
					constexpr size_t k_maxSize = 16;
					auto size = std::min(a_stringView.size(), k_maxSize);
					std::array<char, k_maxSize + 1> smallStr;
					std::memcpy(smallStr.data(), a_stringView.data(), size);
					smallStr[size] = 0;
					return smallStr;
				};

			activeShaderIndex = std::min(activeShaderIndex, mistd::isize(debugProgramCtx.shaders) - 1);


			if (ImGui::Button("Recompile Ssao Program"))
			{
				tryExportCoreShaders();
				createSsaoProgram(debugProgramCtx.ssaoProgram);
			}

			if (ImGui::Button("Recompile Opaque Composition Program"))
			{
				tryExportCoreShaders();
				createOpaqueCompositionProgram(debugProgramCtx.opaqueCompositionProgram);
			}

			auto const recompileWithSkyPartial = [&](auto a_recompile)
				{
					tryExportCoreShaders();
					auto const skyPartialSource = debugProgramCtx.stringDatabase.find(
						debugProgramCtx.filesystemIndexer.get_runtime_id(debugProgramCtx.skyPartialSourcePath));
					if (VOB_AOE_CHECK_LOG(skyPartialSource != nullptr, "Sky partial source not found."))
					{
						a_recompile(*skyPartialSource);
					}
				};

			if (ImGui::Button("Recompile Sky Box Program"))
			{
				recompileWithSkyPartial([&](std::string_view a_source)
					{
						createSkyProgram(a_source, debugProgramCtx.skyBoxProgram);
					});
			}

			if (ImGui::Button("Recompile Ssr Program"))
			{
				recompileWithSkyPartial([&](std::string_view a_source)
					{
						createSsrProgram(a_source, debugProgramCtx.ssrProgram);
					});
			}

			if (ImGui::Button("Recompile Sky Irradiance Program"))
			{
				recompileWithSkyPartial([&](std::string_view a_source)
					{
						createSkyIrradianceProgram(a_source, debugProgramCtx.skyIrradianceProgram);
					});
			}

			auto const activeShaderStr = toSmallStr(debugProgramCtx.shaders[activeShaderIndex].shaderDefinition->name);
			if (ImGui::BeginCombo("Shader", activeShaderStr.data()))
			{
				for (int32_t i = 0; i < mistd::isize(debugProgramCtx.shaders); ++i)
				{
					auto const shaderStr = toSmallStr(debugProgramCtx.shaders[i].shaderDefinition->name);
					if (ImGui::Selectable(shaderStr.data(), i == activeShaderIndex))
					{
						activeShaderIndex = i;
					}

					if (i == activeShaderIndex)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
			if (ImGui::Button("Recompile Shader"))
			{
				tryExportCoreShaders();
				auto const& shader = debugProgramCtx.shaders[activeShaderIndex];
				auto const& shaderDefinition = *shader.shaderDefinition;
				auto const& sourcePath = shaderDefinition.partialSourcePath;
				auto const source = debugProgramCtx.stringDatabase.find(
					debugProgramCtx.filesystemIndexer.get_runtime_id(sourcePath));
				if (VOB_AOE_CHECK_LOG(source != nullptr, "Shader source not found: {}.", sourcePath.string()))
				{
					auto const paramsLayout = computeMaterialParamsLayout(shaderDefinition.uniformDefaults);
					auto const& gpuShader = shaderRegistry.get(shader.shader);

					auto const recompileShading = [&](ModelType a_modelType, GraphicId a_programId)
						{
							createShadingProgram(
								*source
								, shaderDefinition.defines
								, paramsLayout
								, shaderDefinition.shadingPass
								, a_modelType
								, shaderDefinition.isAlphaMasked
								, a_programId);
						};

					recompileShading(ModelType::Static, gpuShader.staticProgram.id);
					recompileShading(ModelType::Rigged, gpuShader.riggedProgram.id);
					recompileShading(ModelType::Instanced, gpuShader.instancedProgram.id);

					// Unmasked shaders alias the core depth/shadow programs, which no partial contributes to.
					if (shaderDefinition.isAlphaMasked)
					{
						auto const recompileDepth = [&](ModelType a_modelType, GraphicId a_programId)
							{
								createAlphaMaskedDepthProgram(
									*source, shaderDefinition.defines, paramsLayout, a_modelType, a_programId);
							};
						auto const recompileShadowMap = [&](ModelType a_modelType, GraphicId a_programId)
							{
								createAlphaMaskedShadowMapProgram(
									*source, shaderDefinition.defines, paramsLayout, a_modelType, a_programId);
							};

						recompileDepth(ModelType::Static, gpuShader.staticDepthProgram.id);
						recompileDepth(ModelType::Rigged, gpuShader.riggedDepthProgram.id);
						recompileDepth(ModelType::Instanced, gpuShader.instancedDepthProgram.id);

						recompileShadowMap(ModelType::Static, gpuShader.staticShadowMapProgram.id);
						recompileShadowMap(ModelType::Rigged, gpuShader.riggedShadowMapProgram.id);
						recompileShadowMap(ModelType::Instanced, gpuShader.instancedShadowMapProgram.id);
					}
				}
			}

			auto& debugMaterialCtx = m_debugMaterialContext.get(a_wdap);
			if (!debugMaterialCtx.materials.empty())
			{
				ImGui::SeparatorText("Materials");
				auto& activeMaterialIndex = debugMaterialCtx.activeMaterialIndex;
				activeMaterialIndex =
					std::clamp(activeMaterialIndex, 0, mistd::isize(debugMaterialCtx.materials) - 1);

				auto const activeShaderName = debugMaterialCtx.materials[activeMaterialIndex].shaderName;
				if (ImGui::BeginCombo("Material shader", activeShaderName.c_str()))
				{
					for (int32_t i = 0; i < mistd::isize(debugMaterialCtx.materials); ++i)
					{
						auto const& shaderName = debugMaterialCtx.materials[i].shaderName;
						auto isFirstOfShader = true;
						for (int32_t j = 0; j < i && isFirstOfShader; ++j)
						{
							isFirstOfShader = debugMaterialCtx.materials[j].shaderName != shaderName;
						}

						if (!isFirstOfShader)
						{
							continue;
						}

						if (ImGui::Selectable(shaderName.c_str(), shaderName == activeShaderName))
						{
							activeMaterialIndex = i;
						}

						if (shaderName == activeShaderName)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				if (ImGui::BeginCombo("Material", debugMaterialCtx.materials[activeMaterialIndex].name.c_str()))
				{
					for (int32_t i = 0; i < mistd::isize(debugMaterialCtx.materials); ++i)
					{
						if (debugMaterialCtx.materials[i].shaderName != activeShaderName)
						{
							continue;
						}

						if (ImGui::Selectable(
							debugMaterialCtx.materials[i].name.c_str(), i == activeMaterialIndex))
						{
							activeMaterialIndex = i;
						}

						if (i == activeMaterialIndex)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				auto const& material =
					materialRegistry.get(debugMaterialCtx.materials[activeMaterialIndex].material);
				for (auto const& [name, slot] : shaderRegistry.get(material.shader).paramsLayout.slots)
				{
					auto value = readMaterialParam(material.paramsUbo, slot);
					auto const nameStr = std::string{ name.view() };
					auto changed = false;
					switch (slot.variantIndex)
					{
					case 0:
						changed = ImGui::DragInt(nameStr.c_str(), &std::get<int32_t>(value));
						break;
					case 1:
						changed = ImGui::DragFloat(nameStr.c_str(), &std::get<float>(value), 0.01f);
						break;
					case 2:
						changed = ImGui::DragFloat2(nameStr.c_str(), &std::get<glm::vec2>(value).x, 0.01f);
						break;
					case 3:
						changed = ImGui::DragFloat3(nameStr.c_str(), &std::get<glm::vec3>(value).x, 0.01f);
						break;
					case 4:
						changed = ImGui::DragFloat4(nameStr.c_str(), &std::get<glm::vec4>(value).x, 0.01f);
						break;
					}

					if (changed)
					{
						writeMaterialParam(material.paramsUbo, slot, value);
					}
				}
			}
		}
		ImGui::End();

		debugRenderInspectorCtx.names.clear();
		debugRenderInspectorCtx.selectedIndexCount = 0;

		VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Scene");

		// I - Prepare Scene
		auto const globalParams = createGlobalParams(m_timeContext.get(a_wdap));

		auto [viewParams, worldOriginPosition] = createViewParams(
			m_windowContext.get(a_wdap), cameraDirectorCtx.activeCameraEntity, m_cameraEntities.get(a_wdap));
		auto const [debugViewParams, debugWorldOriginPosition] = createViewParams(
			m_windowContext.get(a_wdap),
			cameraDirectorCtx.debugCameraEntity != entt::null ? cameraDirectorCtx.debugCameraEntity : cameraDirectorCtx.activeCameraEntity,
			m_cameraEntities.get(a_wdap));

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
			renderSceneCtx.shadingResolution,
			renderSceneCtx.lightClusterTileSize,
			renderSceneCtx.lightClusterZCount,
			renderSceneCtx.lightClusterCapacity,
			worldOriginPosition,
			glm::inverse(debugViewParams.worldToClip),
			debugViewParams.viewToWorld,
			debugViewParams.nearClip,
			debugViewParams.farClip,
			renderSceneCtx.sunDir,
			renderSceneCtx.sunShadowMapFrustumFarClips,
			gpuLights);
		{
			VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Update Buffers");
			glNamedBufferSubData(renderSceneCtx.globalParamsUbo, 0, sizeof(globalParams), &globalParams);
			glNamedBufferSubData(renderSceneCtx.viewParamsUbo, 0, sizeof(viewParams), &viewParams);
			glNamedBufferSubData(renderSceneCtx.lightingParamsUbo, 0, sizeof(lightingParams), &lightingParams);
			glNamedBufferSubData(renderSceneCtx.shadowParamsUbo, 0, sizeof(shadowParams), &shadowParams);
			glNamedBufferSubData(renderSceneCtx.lightsSsbo, 0, gpuLights.size() * sizeof(gpuLights[0]), gpuLights.data());
		}

		// III - Cluster Lights
		{
			VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Light Clustering");
			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.lightClusteringProgram);
			gpuState.bindUbo<GpuStateChange::SurelyYes>(k_bindingUboGlobal, renderSceneCtx.globalParamsUbo);
			gpuState.bindUbo<GpuStateChange::SurelyYes>(k_bindingUboView, renderSceneCtx.viewParamsUbo);
			gpuState.bindUbo<GpuStateChange::SurelyYes>(k_bindingUboLighting, renderSceneCtx.lightingParamsUbo);
			gpuState.bindUbo<GpuStateChange::SurelyYes>(k_bindingUboShadow, renderSceneCtx.shadowParamsUbo);
			gpuState.bindSsbo<GpuStateChange::SurelyYes>(k_bindingSsboLights, renderSceneCtx.lightsSsbo);
			gpuState.bindSsbo<GpuStateChange::SurelyYes>(k_bindingSsboLightClusterSizes, renderSceneCtx.lightClusterSizesSsbo);
			gpuState.bindSsbo<GpuStateChange::SurelyYes>(k_bindingSsboLightClusterIndices, renderSceneCtx.lightClusterIndicesSsbo);

			auto const lightClusterXYCount = (lightingParams.lightClusterResolution + lightingParams.lightClusterTileSize - 1) / lightingParams.lightClusterTileSize;
			auto const lightClusterCount = lightClusterXYCount.x * lightClusterXYCount.y * lightingParams.lightClusterZCount;
			auto const workGroupCount = (lightClusterCount + renderSceneCtx.lightClusteringWorkGroupSize - 1) / renderSceneCtx.lightClusteringWorkGroupSize;
			glDispatchCompute(static_cast<uint32_t>(workGroupCount), 1, 1);
		}

		// III bis - Project Sky Irradiance
		{
			VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Sky Irradiance");
			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.skyIrradianceProgram);
			gpuState.bindUbo<GpuStateChange::SurelyNo>(k_bindingUboGlobal, renderSceneCtx.globalParamsUbo);
			gpuState.bindUbo<GpuStateChange::SurelyNo>(k_bindingUboLighting, renderSceneCtx.lightingParamsUbo);
			gpuState.bindSsbo<GpuStateChange::SurelyYes>(k_bindingSsboSkyIrradiance, renderSceneCtx.skyIrradianceSsbo);

			glDispatchCompute(1, 1, 1);
		}

		// IV - Prepare Meshes
		for (auto const [entity, positionCmp, rotationCmp, modelTransformCmp] : modelEntities.each())
		{
			auto const modelParams = UniformModelParams{ .modelToWorld = aoest::combine(positionCmp.value - worldOriginPosition, rotationCmp.value) };
			if (modelTransformCmp.prevModelParams != modelParams)
			{
				modelTransformCmp.prevModelParams = modelParams;
				glNamedBufferSubData(modelTransformCmp.modelParamsUbo, 0, sizeof(modelParams), &modelParams);
			}
		}

		struct CulledStaticMesh
		{
			ResolvedShader shader;
			WeakHandle<GpuMaterial> material;
			GraphicId modelParamsUbo;
			GraphicId vao;
			int32_t indexCount;
		};

		auto const drawStaticMesh = [&](auto& staticMesh)
			{
				gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboModel, staticMesh.modelParamsUbo);
				glBindVertexArray(staticMesh.vao);
				glDrawElements(GL_TRIANGLES, staticMesh.indexCount, GL_UNSIGNED_INT, nullptr);
			};

		struct CulledRiggedMesh
		{
			ResolvedShader shader;
			WeakHandle<GpuMaterial> material;
			GraphicId modelParamsUbo;
			GraphicId rigParamsUbo;
			GraphicId vao;
			int32_t indexCount;
		};

		auto const drawRiggedMesh = [&](auto& riggedMesh)
			{
				gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboModel, riggedMesh.modelParamsUbo);
				gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboRig, riggedMesh.rigParamsUbo);
				glBindVertexArray(riggedMesh.vao);
				glDrawElements(GL_TRIANGLES, riggedMesh.indexCount, GL_UNSIGNED_INT, nullptr);
			};

		struct CulledInstancedMesh
		{
			ResolvedShader shader;
			WeakHandle<GpuMaterial> material;
			GraphicId modelParamsUbo;
			GraphicId instanceTransformsVbo;
			int32_t instanceCount;
			GraphicId vao;
			int32_t indexCount;
		};

		struct CulledMeshes
		{
			std::vector<CulledStaticMesh> staticOpaqueMeshes;
			std::vector<CulledRiggedMesh> riggedOpaqueMeshes;
			std::vector<CulledInstancedMesh> instancedOpaqueMeshes;
			std::vector<CulledStaticMesh> staticTranslucentMeshes;
			std::vector<CulledRiggedMesh> riggedTranslucentMeshes;
			std::vector<CulledInstancedMesh> instancedTranslucentMeshes;

			void clear()
			{
				staticOpaqueMeshes.clear();
				riggedOpaqueMeshes.clear();
				instancedOpaqueMeshes.clear();
				staticTranslucentMeshes.clear();
				riggedTranslucentMeshes.clear();
				instancedTranslucentMeshes.clear();
			}

			void sort()
			{
				auto const getProgram = [](auto const& a_mesh) { return a_mesh.shader.depthProgram; };
				sortCulledMeshes(staticOpaqueMeshes, getProgram);
				sortCulledMeshes(riggedOpaqueMeshes, getProgram);
				sortCulledMeshes(instancedOpaqueMeshes, getProgram);
			}

			void addStaticMesh(ShadedMesh const& a_mesh, ModelTransformComponent const& a_modelTransformCmp)
			{
				switch (a_mesh.shadingPass)
				{
				case ShadingPass::Opaque:
					staticOpaqueMeshes.emplace_back(
						a_mesh.shader, a_mesh.material, a_modelTransformCmp.modelParamsUbo, a_mesh.vao, a_mesh.indexCount);
					break;
				case ShadingPass::Translucent:
					staticTranslucentMeshes.emplace_back(
						a_mesh.shader, a_mesh.material, a_modelTransformCmp.modelParamsUbo, a_mesh.vao, a_mesh.indexCount);
					break;
				default:
					break;
				}
			}

			void addRiggedMesh(ShadedMesh const& a_mesh, ModelTransformComponent const& a_modelTransformCmp, GraphicId a_rigParamsUbo)
			{
				switch (a_mesh.shadingPass)
				{
				case ShadingPass::Opaque:
					riggedOpaqueMeshes.emplace_back(
						a_mesh.shader, a_mesh.material, a_modelTransformCmp.modelParamsUbo, a_rigParamsUbo, a_mesh.vao, a_mesh.indexCount);
					break;
				case ShadingPass::Translucent:
					riggedTranslucentMeshes.emplace_back(
						a_mesh.shader, a_mesh.material, a_modelTransformCmp.modelParamsUbo, a_rigParamsUbo, a_mesh.vao, a_mesh.indexCount);
					break;
				default:
					break;
				}
			}

			void addInstancedMesh(ShadedMesh const& a_mesh, ModelTransformComponent const& a_modelTransformCmp, GraphicId a_instanceTransformsVbo, int32_t a_instanceCount)
			{
				switch (a_mesh.shadingPass)
				{
				case ShadingPass::Opaque:
					instancedOpaqueMeshes.emplace_back(
						a_mesh.shader, a_mesh.material, a_modelTransformCmp.modelParamsUbo, a_instanceTransformsVbo, a_instanceCount, a_mesh.vao, a_mesh.indexCount);
					break;
				case ShadingPass::Translucent:
					instancedTranslucentMeshes.emplace_back(
						a_mesh.shader, a_mesh.material, a_modelTransformCmp.modelParamsUbo, a_instanceTransformsVbo, a_instanceCount, a_mesh.vao, a_mesh.indexCount);
					break;
				default:
					break;
				}
			}
		};
		
		struct CulledShadowMeshes
		{
			std::vector<CulledStaticMesh> staticOpaqueMeshes;
			std::vector<CulledRiggedMesh> riggedOpaqueMeshes;
			std::vector<CulledInstancedMesh> instancedOpaqueMeshes;

			void clear()
			{
				staticOpaqueMeshes.clear();
				riggedOpaqueMeshes.clear();
				instancedOpaqueMeshes.clear();
			}

			void sort()
			{
				auto const getProgram = [](auto const& a_mesh) { return a_mesh.shader.shadowMapProgram; };
				sortCulledMeshes(staticOpaqueMeshes, getProgram);
				sortCulledMeshes(riggedOpaqueMeshes, getProgram);
				sortCulledMeshes(instancedOpaqueMeshes, getProgram);
			}

			void addStaticMesh(ShadedMesh const& a_mesh, ModelTransformComponent const& a_modelTransformCmp)
			{
				switch (a_mesh.shadingPass)
				{
				case ShadingPass::Opaque:
					staticOpaqueMeshes.emplace_back(
						a_mesh.shader, a_mesh.material, a_modelTransformCmp.modelParamsUbo, a_mesh.vao, a_mesh.indexCount);
					break;
				default:
					break;
				}
			}

			void addRiggedMesh(ShadedMesh const& a_mesh, ModelTransformComponent const& a_modelTransformCmp, GraphicId a_rigParamsUbo)
			{
				switch (a_mesh.shadingPass)
				{
				case ShadingPass::Opaque:
					riggedOpaqueMeshes.emplace_back(
						a_mesh.shader, a_mesh.material, a_modelTransformCmp.modelParamsUbo, a_rigParamsUbo, a_mesh.vao, a_mesh.indexCount);
					break;
				default:
					break;
				}
			}

			void addInstancedMesh(ShadedMesh const& a_mesh, ModelTransformComponent const& a_modelTransformCmp, GraphicId a_instanceTransformsVbo, int32_t a_instanceCount)
			{
				switch (a_mesh.shadingPass)
				{
				case ShadingPass::Opaque:
					instancedOpaqueMeshes.emplace_back(
						a_mesh.shader, a_mesh.material, a_modelTransformCmp.modelParamsUbo, a_instanceTransformsVbo, a_instanceCount, a_mesh.vao, a_mesh.indexCount);
					break;
				default:
					break;
				}
			}
		};

		auto const drawInstancedMesh = [&](auto& instancedMesh)
			{
				gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboModel, instancedMesh.modelParamsUbo);
				glBindVertexArray(instancedMesh.vao);
				glBindVertexBuffer(
					1,
					instancedMesh.instanceTransformsVbo,
					0 /* offset */,
					sizeof(glm::mat4));
				glDrawElementsInstanced(GL_TRIANGLES, instancedMesh.indexCount, GL_UNSIGNED_INT, nullptr, instancedMesh.instanceCount);
			};

		auto const cullView = [&](auto const& a_viewFrustumPlanes, auto& a_culledMeshes)
			{
				a_culledMeshes.clear();

				for (auto const [entity, positionCmp, modelTransformCmp, staticModelCmp] : staticModelEntities.each())
				{
					if (testViewFrustumPlanes(a_viewFrustumPlanes, positionCmp.value - worldOriginPosition, modelTransformCmp.boundingRadius))
					{
						for (auto const& mesh : staticModelCmp.meshes)
						{
							a_culledMeshes.addStaticMesh(mesh, modelTransformCmp);
						}
					}
				}

				for (auto const [entity, positionCmp, modelTransformCmp, riggedModelCmp] : riggedModelEntities.each())
				{
					if (testViewFrustumPlanes(a_viewFrustumPlanes, positionCmp.value - worldOriginPosition, modelTransformCmp.boundingRadius))
					{
						for (auto const& mesh : riggedModelCmp.meshes)
						{
							a_culledMeshes.addRiggedMesh(mesh, modelTransformCmp, riggedModelCmp.rigParamsUbo);
						}
					}
				}

				for (auto const [entity, positionCmp, modelTransformCmp, instancedModelsCmp] : instancedModelsEntities.each())
				{
					if (testViewFrustumPlanes(a_viewFrustumPlanes, positionCmp.value - worldOriginPosition, modelTransformCmp.boundingRadius))
					{
						for (auto const& model : instancedModelsCmp.models)
						{
							for (auto const& mesh : model.meshes)
							{
								a_culledMeshes.addInstancedMesh(
									mesh, modelTransformCmp, model.instanceTransformsVbo, model.instanceCount);
							}
						}
					}
				}

				a_culledMeshes.sort();
			};

		auto const drawOpaqueMeshes = [&](auto const& a_culledMeshes, auto a_applyMeshState)
			{
				for (auto const& staticMesh : a_culledMeshes.staticOpaqueMeshes)
				{
					a_applyMeshState(staticMesh);
					drawStaticMesh(staticMesh);
				}
				for (auto const& riggedMesh : a_culledMeshes.riggedOpaqueMeshes)
				{
					a_applyMeshState(riggedMesh);
					drawRiggedMesh(riggedMesh);
				}
				for (auto const& instancedMesh : a_culledMeshes.instancedOpaqueMeshes)
				{
					a_applyMeshState(instancedMesh);
					drawInstancedMesh(instancedMesh);
				}
			};

		auto const applyFaceCulling = [&](bool a_isTwoSided)
			{
				if (a_isTwoSided)
				{
					gpuState.disableFaceCulling<GpuStateChange::LikelyNo>();
				}
				else
				{
					gpuState.enableFaceCulling<GpuStateChange::LikelyNo>();
				}
			};

		auto const applyDepthOnlyMaterialParams = [&](auto const& a_mesh, WeakHandle<GpuMaterial>& a_currentMaterial)
			{
				if (a_currentMaterial == a_mesh.material)
				{
					return;
				}
				a_currentMaterial = a_mesh.material;

				if (!a_mesh.material.isValid())
				{
					applyFaceCulling(false);
					return;
				}

				auto const& material = materialRegistry.get(a_mesh.material);
				applyFaceCulling(material.isTwoSided);

				if (!a_mesh.shader.isAlphaMasked)
				{
					return;
				}

				gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboMaterial, material.paramsUbo);
				for (auto const slotIndex : material.depthOnlyTextureSlotIndices)
				{
					gpuState.bindTexture<GpuStateChange::LikelyYes>(
						k_bindingTextureShadingMaterialBegin + slotIndex, material.textures[slotIndex].id);
				}
			};

		static CulledMeshes culledMeshes;
		cullView(viewFrustumPlanes, culledMeshes);

		auto const cullAndDrawShadowView = [&](auto const& a_viewFrustumPlanes, auto const& a_viewParams)
			{
				static CulledShadowMeshes culledShadowMeshes;
				cullView(a_viewFrustumPlanes, culledShadowMeshes);

				glNamedBufferSubData(renderSceneCtx.lightViewParamsUbo, 0, sizeof(a_viewParams), &a_viewParams);
				glClear(GL_DEPTH_BUFFER_BIT);

				auto currentMaterial = WeakHandle<GpuMaterial>{};
				drawOpaqueMeshes(culledShadowMeshes, [&](auto const& a_mesh)
					{
						gpuState.useProgram<GpuStateChange::LikelyNo>(a_mesh.shader.shadowMapProgram);
						applyDepthOnlyMaterialParams(a_mesh, currentMaterial);
					});
			};

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		// V - Compute Shadow Maps
		float debugSunNear = 0.0f;
		float debugSunFar = 0.0f;
		if (k_debugCameraFrustum)
		{
			auto const clipToWorld = glm::inverse(debugViewParams.worldToClip);
			auto p0 = glm::dvec3{ aoest::transformPositionSkewed(clipToWorld, glm::vec3{ -1.0f, -1.0f, -1.0f }) };
			auto p1 = glm::dvec3{ aoest::transformPositionSkewed(clipToWorld, glm::vec3{ -1.0f, -1.0f, 1.0f }) };
			auto p2 = glm::dvec3{ aoest::transformPositionSkewed(clipToWorld, glm::vec3{ -1.0f, 1.0f, -1.0f }) };
			auto p3 = glm::dvec3{ aoest::transformPositionSkewed(clipToWorld, glm::vec3{ -1.0f, 1.0f, 1.0f }) };
			auto p4 = glm::dvec3{ aoest::transformPositionSkewed(clipToWorld, glm::vec3{ 1.0f, -1.0f, -1.0f }) };
			auto p5 = glm::dvec3{ aoest::transformPositionSkewed(clipToWorld, glm::vec3{ 1.0f, -1.0f, 1.0f }) };
			auto p6 = glm::dvec3{ aoest::transformPositionSkewed(clipToWorld, glm::vec3{ 1.0f, 1.0f, -1.0f }) };
			auto p7 = glm::dvec3{ aoest::transformPositionSkewed(clipToWorld, glm::vec3{ 1.0f, 1.0f, 1.0f }) };
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
			VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Shadow Maps");
			gpuState.enableDepthTest<GpuStateChange::SurelyYes>();
			gpuState.enableDepthWrite<GpuStateChange::SurelyYes>();
			gpuState.setDepthFunc<GpuStateChange::SurelyYes>(GpuDepthFunc::Less);
			gpuState.setClearDepth<GpuStateChange::SurelyYes>(1.0);
			gpuState.disableColorWrite<GpuStateChange::SurelyYes>();
			gpuState.disableBlend<GpuStateChange::SurelyYes>();
			gpuState.enableFaceCulling<GpuStateChange::LikelyYes>();
			gpuState.setCullFace<GpuStateChange::LikelyYes>(GpuCullFace::Front);
			gpuState.bindUbo<GpuStateChange::SurelyYes>(k_bindingUboView, renderSceneCtx.lightViewParamsUbo);

			// A - Sun CSM
			{
				beginPass(gpuState, renderSceneCtx.sunShadowMapFramebuffer, renderSceneCtx.sunShadowMapResolution, renderSceneCtx.targetParamsUbo);
				VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Sun CSM");
				auto const debugSunCsmIndex = std::clamp(
					debugRenderInspectorCtx.selectedIndex, 0, mistd::isize(renderSceneCtx.sunShadowMapFrustumFarClips) - 1);
				for (int32_t csmIndex = 0; csmIndex < mistd::isize(renderSceneCtx.sunShadowMapFrustumFarClips); ++csmIndex)
				{
					auto const& sunShadowParams = shadowParams.sun[csmIndex];
					if (csmIndex == debugSunCsmIndex)
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
						auto s0 = glm::dvec3{ aoest::transformPositionSkewed(viewClipToWorld, glm::vec3{ -1.0f, -1.0f, clipZ(sunShadowParams.maxViewDepth) }) };
						auto s1 = glm::dvec3{ aoest::transformPositionSkewed(viewClipToWorld, glm::vec3{ -1.0f, 1.0f, clipZ(sunShadowParams.maxViewDepth) }) };
						auto s2 = glm::dvec3{ aoest::transformPositionSkewed(viewClipToWorld, glm::vec3{ 1.0f, -1.0f, clipZ(sunShadowParams.maxViewDepth) }) };
						auto s3 = glm::dvec3{ aoest::transformPositionSkewed(viewClipToWorld, glm::vec3{ 1.0f, 1.0f, clipZ(sunShadowParams.maxViewDepth) }) };
						debugMeshCtx.addLine(s0, s1, aoegl::k_yellow);
						debugMeshCtx.addLine(s0, s2, aoegl::k_yellow);
						debugMeshCtx.addLine(s1, s3, aoegl::k_yellow);
						debugMeshCtx.addLine(s2, s3, aoegl::k_yellow);

						auto const sunClipToWorld = glm::inverse(sunShadowParams.worldToClip);
						auto p0 = glm::dvec3{ aoest::transformPositionSkewed(sunClipToWorld, glm::vec3{ -1.0f, -1.0f, -1.0f }) };
						auto p1 = glm::dvec3{ aoest::transformPositionSkewed(sunClipToWorld, glm::vec3{ -1.0f, -1.0f, 1.0f }) };
						auto p2 = glm::dvec3{ aoest::transformPositionSkewed(sunClipToWorld, glm::vec3{ -1.0f, 1.0f, -1.0f }) };
						auto p3 = glm::dvec3{ aoest::transformPositionSkewed(sunClipToWorld, glm::vec3{ -1.0f, 1.0f, 1.0f }) };
						auto p4 = glm::dvec3{ aoest::transformPositionSkewed(sunClipToWorld, glm::vec3{ 1.0f, -1.0f, -1.0f }) };
						auto p5 = glm::dvec3{ aoest::transformPositionSkewed(sunClipToWorld, glm::vec3{ 1.0f, -1.0f, 1.0f }) };
						auto p6 = glm::dvec3{ aoest::transformPositionSkewed(sunClipToWorld, glm::vec3{ 1.0f, 1.0f, -1.0f }) };
						auto p7 = glm::dvec3{ aoest::transformPositionSkewed(sunClipToWorld, glm::vec3{ 1.0f, 1.0f, 1.0f }) };
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
					glNamedFramebufferTextureLayer(
						renderSceneCtx.sunShadowMapFramebuffer,
						GL_DEPTH_ATTACHMENT,
						renderSceneCtx.sunShadowMapDepthTextureArray,
						0 /* mip level */,
						csmIndex);

					auto const sunViewParams = UniformViewParams{
						.worldToClip = sunShadowParams.worldToClip,
						.nearClip = sunShadowParams.nearClip,
						.farClip = sunShadowParams.farClip,
						.fov = 0.0f,
						.aspectRatio = 0.0f
					};

					cullAndDrawShadowView(sunViewFrustumPlanes, sunViewParams);
				}
			}
			auto const sunCsmIndex = debugInspectIndex(
				debugRenderInspectorCtx, "Sun Shadow Map", mistd::isize(renderSceneCtx.sunShadowMapFrustumFarClips));
			debugInspectRenderOutputLayer(
				debugRenderInspectorCtx
				, "Sun Shadow Map"
				, renderSceneCtx.sunShadowMapDepthTextureArray
				, sunCsmIndex
				, DebugType::DepthTexture
				, glm::vec2{ debugSunNear, debugSunFar });

			// B - Spot Lights
			{
				VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Spot Lights");

				for (int32_t i = 0; i < spotLightShadowMapCount; ++i)
				{
					auto const& spotLightShadowMapTarget = renderSceneCtx.spotLightShadowMapTargets[i];

					auto const spotLightViewParams = UniformViewParams{
						.worldToClip = shadowParams.spotLights[i].worldToClip,
						.viewToWorld = shadowParams.spotLights[i].viewToWorld,
						.nearClip = shadowParams.spotLights[i].nearClip,
						.farClip = shadowParams.spotLights[i].farClip,
						.fov = shadowParams.spotLights[i].fov,
						.aspectRatio = 1.0f
					};

					auto const spotLightViewFrustumPlanes = computeViewFrustumPlanes(spotLightViewParams.worldToClip);

					beginPass(gpuState, spotLightShadowMapTarget.framebuffer, spotLightShadowMapTarget.resolution, renderSceneCtx.targetParamsUbo);

					cullAndDrawShadowView(spotLightViewFrustumPlanes, spotLightViewParams);
				}
			}

			if (spotLightShadowMapCount > 0)
			{
				auto const spotLightIndex = debugInspectIndex(
					debugRenderInspectorCtx, "Spot Shadow Map", spotLightShadowMapCount);
				debugInspectRenderOutput(
					debugRenderInspectorCtx
					, "Spot Shadow Map"
					, renderSceneCtx.spotLightShadowMapTargets[spotLightIndex].depthTexture
					, DebugType::DepthTexture
					, glm::vec2{ shadowParams.spotLights[spotLightIndex].nearClip, shadowParams.spotLights[spotLightIndex].farClip });
			}
		}

		// VI - Depth Pre-Pass
		{
			VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Depth Pre-Pass");
			gpuState.enableDepthTest<GpuStateChange::SurelyNo>();
			gpuState.enableDepthWrite<GpuStateChange::SurelyNo>();
			gpuState.setDepthFunc<GpuStateChange::SurelyNo>(GpuDepthFunc::Less);
			gpuState.setClearDepth<GpuStateChange::SurelyNo>(1.0);
			gpuState.enableColorWrite<GpuStateChange::SurelyYes>();
			gpuState.setClearColor<GpuStateChange::SurelyYes>(glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f });
			gpuState.disableBlend<GpuStateChange::SurelyNo>();
			gpuState.enableFaceCulling<GpuStateChange::LikelyNo>();
			gpuState.setCullFace<GpuStateChange::LikelyYes>(GpuCullFace::Back);
			gpuState.bindUbo<GpuStateChange::SurelyNo>(k_bindingUboGlobal, renderSceneCtx.globalParamsUbo);
			gpuState.bindUbo<GpuStateChange::SurelyYes>(k_bindingUboView, renderSceneCtx.viewParamsUbo);

			beginPass(gpuState, renderSceneCtx.depthFramebuffer, renderSceneCtx.shadingResolution, renderSceneCtx.targetParamsUbo);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

			auto currentDepthMaterial = WeakHandle<GpuMaterial>{};
			drawOpaqueMeshes(culledMeshes, [&](auto const& a_mesh)
				{
					gpuState.useProgram<GpuStateChange::LikelyNo>(a_mesh.shader.depthProgram);
					applyDepthOnlyMaterialParams(a_mesh, currentDepthMaterial);
				});

			debugInspectRenderOutput(debugRenderInspectorCtx, "Opaque Geometric Normal", renderSceneCtx.opaqueGeometricNormalTexture, DebugType::DirectionTexture);
			debugInspectRenderOutput(debugRenderInspectorCtx, "Opaque Depth", renderSceneCtx.opaqueDepthTexture, DebugType::DepthTexture, glm::vec2{ viewParams.nearClip, viewParams.farClip });
			debugInspectRenderOutput(debugRenderInspectorCtx, "Light Clusters", renderSceneCtx.opaqueDepthTexture, DebugType::LightClusters);
		}

		// VII - SSAO
		{
			VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "SSAO");
			gpuState.disableDepthTest<GpuStateChange::SurelyYes>();
			gpuState.disableDepthWrite<GpuStateChange::SurelyYes>();
			gpuState.disableBlend<GpuStateChange::SurelyNo>();
			gpuState.disableFaceCulling<GpuStateChange::LikelyYes>();
			gpuState.bindUbo<GpuStateChange::SurelyNo>(k_bindingUboGlobal, renderSceneCtx.globalParamsUbo);
			gpuState.bindUbo<GpuStateChange::SurelyNo>(k_bindingUboView, renderSceneCtx.viewParamsUbo);
			gpuState.bindUbo<GpuStateChange::SurelyYes>(k_bindingUboSsao, renderSceneCtx.ssaoParamsUbo);
			gpuState.bindTexture<GpuStateChange::SurelyYes>(k_bindingTextureSsaoOpaqueDepth, renderSceneCtx.opaqueDepthTexture);
			gpuState.bindTexture<GpuStateChange::SurelyYes>(k_bindingTextureSsaoOpaqueGeometricNormal, renderSceneCtx.opaqueGeometricNormalTexture);
			beginPass(gpuState, renderSceneCtx.ssaoFramebuffer, renderSceneCtx.ssaoResolution, renderSceneCtx.targetParamsUbo);
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

			debugInspectRenderOutput(debugRenderInspectorCtx, "Ambient Occlusion", renderSceneCtx.ambientOcclusionTexture, DebugType::ShadesTexture);
		}

		// VIII - Direct Opaque Lighting
		{
			VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Direct Opaque");
			gpuState.enableDepthTest<GpuStateChange::SurelyYes>();
			gpuState.disableDepthWrite<GpuStateChange::SurelyNo>();
			gpuState.setDepthFunc<GpuStateChange::SurelyYes>(GpuDepthFunc::Equal);
			gpuState.setClearDepth<GpuStateChange::SurelyNo>(1.0);
			gpuState.enableColorWrite<GpuStateChange::SurelyNo>();
			gpuState.setClearColor<GpuStateChange::LikelyYes>(glm::vec4{ 0.0 });
			gpuState.disableBlend<GpuStateChange::SurelyNo>();
			gpuState.enableFaceCulling<GpuStateChange::LikelyNo>();
			gpuState.setCullFace<GpuStateChange::LikelyNo>(GpuCullFace::Back);
			gpuState.bindUbo<GpuStateChange::SurelyNo>(k_bindingUboGlobal, renderSceneCtx.globalParamsUbo);
			gpuState.bindUbo<GpuStateChange::SurelyNo>(k_bindingUboView, renderSceneCtx.viewParamsUbo);
			gpuState.bindTexture<GpuStateChange::SurelyYes>(k_bindingTextureShadingAmbientOcclusion, renderSceneCtx.ambientOcclusionTexture);
			gpuState.bindTexture<GpuStateChange::SurelyYes>(k_bindingTextureShadingSunShadowMap, renderSceneCtx.sunShadowMapDepthTextureArray);
			for (int32_t i = 0; i < spotLightShadowMapCount; ++i)
			{
				gpuState.bindTexture<GpuStateChange::SurelyYes>(
					k_bindingTextureShadingSpotLightShadowMapsBegin + i, renderSceneCtx.spotLightShadowMapTargets[i].depthTexture);
			}
			beginPass(gpuState, renderSceneCtx.directOpaqueFramebuffer, renderSceneCtx.shadingResolution, renderSceneCtx.targetParamsUbo);
			glClear(GL_COLOR_BUFFER_BIT);
			WeakHandle<GpuMaterial> currentMaterial;
			auto const applyMeshShadingParams = [&](auto const& mesh)
				{
					gpuState.useProgram<GpuStateChange::LikelyNo>(mesh.shader.program);
					if (currentMaterial != mesh.material && mesh.material.isValid())
					{
						auto const& material = materialRegistry.get(mesh.material);
						applyFaceCulling(material.isTwoSided);
						gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboMaterial, material.paramsUbo);
						for (int32_t i = 0; i < mistd::isize(material.textures); ++i)
						{
							gpuState.bindTexture<GpuStateChange::LikelyYes>(k_bindingTextureShadingMaterialBegin + i, material.textures[i].id);
						}
						currentMaterial = mesh.material;
					}
				};
			drawOpaqueMeshes(culledMeshes, applyMeshShadingParams);

			debugInspectRenderOutput(debugRenderInspectorCtx, "Direct Opaque Color", renderSceneCtx.directOpaqueColorTexture, DebugType::ColorTexture);
			debugInspectRenderOutput(debugRenderInspectorCtx, "Opaque Normal", renderSceneCtx.opaqueNormalTexture, DebugType::DirectionTexture);
			debugInspectRenderOutput(debugRenderInspectorCtx, "Opaque Surface", renderSceneCtx.opaqueSurfaceTexture, DebugType::ColorTexture);
		}

		// VIII bis - Hi-Z depth pyramid
		{
			VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Hi-Z");
			gpuState.disableDepthTest<GpuStateChange::SurelyYes>();
			gpuState.disableDepthWrite<GpuStateChange::SurelyNo>();
			gpuState.enableColorWrite<GpuStateChange::SurelyNo>();
			gpuState.disableBlend<GpuStateChange::SurelyNo>();
			gpuState.disableFaceCulling<GpuStateChange::LikelyYes>();
			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.hiZReduceProgram);
			glBindVertexArray(renderSceneCtx.postProcessVao);

			auto mipResolution = renderSceneCtx.shadingResolution;
			for (int32_t mipIndex = 0; mipIndex < mistd::isize(renderSceneCtx.hiZMipFramebuffers); ++mipIndex)
			{
				if (mipIndex == 0)
				{
					gpuState.bindTexture<GpuStateChange::LikelyYes>(
						k_bindingTextureSsrFilterSource, renderSceneCtx.opaqueDepthTexture);
				}
				else
				{
					mipResolution = glm::max(mipResolution / 2, glm::ivec2{ 1 });
					glTextureParameteri(renderSceneCtx.hiZDepthTexture, GL_TEXTURE_BASE_LEVEL, mipIndex - 1);
					glTextureParameteri(renderSceneCtx.hiZDepthTexture, GL_TEXTURE_MAX_LEVEL, mipIndex - 1);
					gpuState.bindTexture<GpuStateChange::SurelyYes>(
						k_bindingTextureSsrFilterSource, renderSceneCtx.hiZDepthTexture);
				}

				beginPass(
					gpuState
					, renderSceneCtx.hiZMipFramebuffers[mipIndex]
					, mipResolution
					, renderSceneCtx.targetParamsUbo);
				glDrawArrays(GL_TRIANGLES, 0, 3);
			}

			glTextureParameteri(renderSceneCtx.hiZDepthTexture, GL_TEXTURE_BASE_LEVEL, 0);
			glTextureParameteri(
				renderSceneCtx.hiZDepthTexture, GL_TEXTURE_MAX_LEVEL, renderSceneCtx.hiZMipLevels - 1);

			auto const inspectedHiZNearLevel = debugInspectIndex(
				debugRenderInspectorCtx, "Hi-Z Near", renderSceneCtx.hiZMipLevels);
			debugInspectRenderOutputMipChannel(
				debugRenderInspectorCtx
				, "Hi-Z Near"
				, renderSceneCtx.hiZDepthTexture
				, inspectedHiZNearLevel
				, 0
				, DebugType::DepthTexture
				, glm::vec2{ viewParams.nearClip, viewParams.farClip });

			auto const inspectedHiZFarLevel = debugInspectIndex(
				debugRenderInspectorCtx, "Hi-Z Far", renderSceneCtx.hiZMipLevels);
			debugInspectRenderOutputMipChannel(
				debugRenderInspectorCtx
				, "Hi-Z Far"
				, renderSceneCtx.hiZDepthTexture
				, inspectedHiZFarLevel
				, 1
				, DebugType::DepthTexture
				, glm::vec2{ viewParams.nearClip, viewParams.farClip });
		}

		// IX - SSR
		{
			VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "SSR");
			gpuState.bindTexture<GpuStateChange::LikelyYes>(k_bindingTextureSsrHiZDepth, renderSceneCtx.hiZDepthTexture);
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

			beginPass(gpuState, renderSceneCtx.ssrRawFramebuffer, renderSceneCtx.ssrResolution, renderSceneCtx.targetParamsUbo);
			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.ssrProgram);

			if (k_ssrEnabled)
			{
				glBindVertexArray(renderSceneCtx.postProcessVao);
				glDrawArrays(GL_TRIANGLES, 0, 3);
			}
			else
			{
				gpuState.setClearColor<GpuStateChange::LikelyYes>(glm::vec4{ 0.0 });
				glClear(GL_COLOR_BUFFER_BIT);
			}

			debugInspectRenderOutput(
				debugRenderInspectorCtx, "Ssr Raw", renderSceneCtx.ssrRawColorTexture, DebugType::ColorTexture);

			glBindVertexArray(renderSceneCtx.postProcessVao);
			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.ssrPrefilterProgram);
			gpuState.bindTexture<GpuStateChange::SurelyYes>(
				k_bindingTextureSsrFilterSource, renderSceneCtx.ssrRawColorTexture);
			gpuState.bindTexture<GpuStateChange::LikelyYes>(
				k_bindingTextureSsrFilterOpaqueDepth, renderSceneCtx.opaqueDepthTexture);
			gpuState.bindTexture<GpuStateChange::LikelyYes>(
				k_bindingTextureSsrFilterOpaqueNormal, renderSceneCtx.opaqueNormalTexture);
			beginPass(
				gpuState
				, renderSceneCtx.ssrMipFramebuffers[0]
				, renderSceneCtx.ssrResolution
				, renderSceneCtx.targetParamsUbo);
			glDrawArrays(GL_TRIANGLES, 0, 3);

			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.ssrDownsampleProgram);
			gpuState.bindTexture<GpuStateChange::SurelyYes>(
				k_bindingTextureSsrFilterSource, renderSceneCtx.ssrColorTexture);
			auto mipResolution = renderSceneCtx.ssrResolution;
			for (int32_t mipIndex = 1; mipIndex < mistd::isize(renderSceneCtx.ssrMipFramebuffers); ++mipIndex)
			{
				mipResolution = glm::max(mipResolution / 2, glm::ivec2{ 1 });
				glTextureParameteri(renderSceneCtx.ssrColorTexture, GL_TEXTURE_BASE_LEVEL, mipIndex - 1);
				glTextureParameteri(renderSceneCtx.ssrColorTexture, GL_TEXTURE_MAX_LEVEL, mipIndex - 1);
				beginPass(
					gpuState
					, renderSceneCtx.ssrMipFramebuffers[mipIndex]
					, mipResolution
					, renderSceneCtx.targetParamsUbo);
				glDrawArrays(GL_TRIANGLES, 0, 3);
			}

			glTextureParameteri(renderSceneCtx.ssrColorTexture, GL_TEXTURE_BASE_LEVEL, 0);
			glTextureParameteri(
				renderSceneCtx.ssrColorTexture, GL_TEXTURE_MAX_LEVEL, renderSceneCtx.ssrMipLevels - 1);

			auto const inspectedSsrLevel = debugInspectIndex(
				debugRenderInspectorCtx, "Ssr Color", renderSceneCtx.ssrMipLevels);
			debugInspectRenderOutputMip(
				debugRenderInspectorCtx
				, "Ssr Color"
				, renderSceneCtx.ssrColorTexture
				, inspectedSsrLevel
				, DebugType::ColorTexture);
		}

		// X - Opaque Composition
		{
			VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Opaque Composition");
			gpuState.disableDepthTest<GpuStateChange::SurelyNo>();
			gpuState.disableDepthWrite<GpuStateChange::SurelyNo>();
			gpuState.enableColorWrite<GpuStateChange::SurelyNo>();
			gpuState.disableBlend<GpuStateChange::SurelyNo>();
			gpuState.bindTexture<GpuStateChange::LikelyNo>(k_bindingTextureOpaqueCompositionDirectOpaqueColor, renderSceneCtx.directOpaqueColorTexture);
			gpuState.bindTexture<GpuStateChange::LikelyNo>(k_bindingTextureOpaqueCompositionOpaqueSurface, renderSceneCtx.opaqueSurfaceTexture);
			gpuState.bindTexture<GpuStateChange::SurelyYes>(k_bindingTextureOpaqueCompositionSsrColor, renderSceneCtx.ssrColorTexture);
			gpuState.bindTexture<GpuStateChange::LikelyNo>(k_bindingTextureOpaqueCompositionOpaqueNormal, renderSceneCtx.opaqueNormalTexture);
			beginPass(gpuState, renderSceneCtx.finalFramebuffer, renderSceneCtx.shadingResolution, renderSceneCtx.targetParamsUbo);
			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.opaqueCompositionProgram);

			// glBindVertexArray(renderSceneCtx.postProcessVao);
			glDrawArrays(GL_TRIANGLES, 0, 3);

			debugInspectRenderOutput(debugRenderInspectorCtx, "Opaque Composition", renderSceneCtx.finalColorTexture, DebugType::ColorTexture);
		}

		// XI - Translucent
		{
			VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Translucent");
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
			VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Sky Box");
			gpuState.enableDepthTest<GpuStateChange::SurelyNo>();
			gpuState.setDepthFunc<GpuStateChange::SurelyYes>(GpuDepthFunc::Equal);
			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.skyBoxProgram);
			// TODO: ubo?
			glBindVertexArray(renderSceneCtx.postProcessVao);
			glDrawArrays(GL_TRIANGLES, 0, 3);

			debugInspectRenderOutput(debugRenderInspectorCtx, "Sky Box", renderSceneCtx.finalColorTexture, DebugType::ColorTexture);
		}

		auto const bloomEnabled = k_bloomEnabled && !renderSceneCtx.bloomMips.empty();

		// XII-bis - Bloom Downsample
		if (bloomEnabled)
		{
			VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Bloom Downsample");
			gpuState.disableBlend<GpuStateChange::SurelyYes>();
			gpuState.bindUbo<GpuStateChange::SurelyYes>(k_bindingUboBloom, renderSceneCtx.bloomParamsUbo);
			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.bloomDownsampleProgram);
			glBindVertexArray(renderSceneCtx.postProcessVao);

			auto const& firstMip = renderSceneCtx.bloomMips[0];
			gpuState.bindTexture<GpuStateChange::SurelyYes>(k_bindingTextureBloomSource, renderSceneCtx.finalColorTexture);
			beginPass(gpuState, firstMip.framebuffer, firstMip.resolution, renderSceneCtx.targetParamsUbo);
			glDrawArrays(GL_TRIANGLES, 0, 3);

			auto const karisOff = 0;
			glNamedBufferSubData(
				renderSceneCtx.bloomParamsUbo, offsetof(UniformBloomParams, useKarisAverage), sizeof(karisOff), &karisOff);

			for (int32_t mipIndex = 1; mipIndex < mistd::isize(renderSceneCtx.bloomMips); ++mipIndex)
			{
				auto const& mip = renderSceneCtx.bloomMips[mipIndex];
				gpuState.bindTexture<GpuStateChange::SurelyYes>(
					k_bindingTextureBloomSource, renderSceneCtx.bloomMips[mipIndex - 1].colorTexture);
				beginPass(gpuState, mip.framebuffer, mip.resolution, renderSceneCtx.targetParamsUbo);
				glDrawArrays(GL_TRIANGLES, 0, 3);
			}

			auto const inspectedMipIndex = debugInspectIndex(
				debugRenderInspectorCtx, "Bloom Downsample", mistd::isize(renderSceneCtx.bloomMips));
			debugInspectRenderOutput(
				debugRenderInspectorCtx
				, "Bloom Downsample"
				, renderSceneCtx.bloomMips[inspectedMipIndex].colorTexture
				, DebugType::ColorTexture);
		}

		// XII-ter - Bloom Upsample
		if (bloomEnabled)
		{
			VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Bloom Upsample");
			gpuState.enableBlend<GpuStateChange::SurelyYes>();
			glBlendFunc(GL_ONE, GL_ONE);
			gpuState.bindUbo<GpuStateChange::LikelyNo>(k_bindingUboBloom, renderSceneCtx.bloomParamsUbo);
			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.bloomUpsampleProgram);
			glBindVertexArray(renderSceneCtx.postProcessVao);

			for (int32_t mipIndex = mistd::isize(renderSceneCtx.bloomMips) - 2; mipIndex >= 0; --mipIndex)
			{
				auto const& mip = renderSceneCtx.bloomMips[mipIndex];
				gpuState.bindTexture<GpuStateChange::SurelyYes>(
					k_bindingTextureBloomSource, renderSceneCtx.bloomMips[mipIndex + 1].colorTexture);
				beginPass(gpuState, mip.framebuffer, mip.resolution, renderSceneCtx.targetParamsUbo);
				glDrawArrays(GL_TRIANGLES, 0, 3);
			}

			gpuState.disableBlend<GpuStateChange::SurelyYes>();

			auto const inspectedMipIndex = debugInspectIndex(
				debugRenderInspectorCtx, "Bloom Upsample", mistd::isize(renderSceneCtx.bloomMips));
			debugInspectRenderOutput(
				debugRenderInspectorCtx
				, "Bloom Upsample"
				, renderSceneCtx.bloomMips[inspectedMipIndex].colorTexture
				, DebugType::ColorTexture);
		}

		// XII-quater - Bloom Combine
		if (bloomEnabled)
		{
			VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Bloom Combine");
			gpuState.bindUbo<GpuStateChange::LikelyNo>(k_bindingUboBloom, renderSceneCtx.bloomParamsUbo);
			gpuState.bindTexture<GpuStateChange::LikelyYes>(k_bindingTextureBloomCombineScene, renderSceneCtx.finalColorTexture);
			gpuState.bindTexture<GpuStateChange::LikelyYes>(k_bindingTextureBloomCombineBloom, renderSceneCtx.bloomMips[0].colorTexture);
			beginPass(gpuState, renderSceneCtx.bloomCombinedFramebuffer, renderSceneCtx.shadingResolution, renderSceneCtx.targetParamsUbo);
			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.bloomCombineProgram);
			glDrawArrays(GL_TRIANGLES, 0, 3);

			debugInspectRenderOutput(
				debugRenderInspectorCtx
				, "Bloom Combine"
				, renderSceneCtx.bloomCombinedColorTexture
				, DebugType::ColorTexture);
		}

		// XIII - Post Processes
		{
			VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Post Processes");
			gpuState.disableDepthTest<GpuStateChange::SurelyYes>();
			gpuState.enableColorWrite<GpuStateChange::SurelyNo>();
			gpuState.disableBlend<GpuStateChange::LikelyNo>();
			glBindVertexArray(renderSceneCtx.postProcessVao);

			// Tonemap
			{
				VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Tonemap");
				gpuState.bindUbo<GpuStateChange::SurelyYes>(k_bindingUboTonemap, renderSceneCtx.tonemapParamsUbo);
				gpuState.bindTexture<GpuStateChange::SurelyYes>(
					k_bindingTextureTonemapSource
					, bloomEnabled ? renderSceneCtx.bloomCombinedColorTexture : renderSceneCtx.finalColorTexture);
				beginPass(gpuState, renderSceneCtx.postProcessTargets[0].framebuffer, renderSceneCtx.shadingResolution, renderSceneCtx.targetParamsUbo);
				gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.tonemapProgram);
				glDrawArrays(GL_TRIANGLES, 0, 3);

				debugInspectRenderOutput(debugRenderInspectorCtx, "Tonemap", renderSceneCtx.postProcessTargets[0].colorTexture, DebugType::ColorTexture);
			}

			// Anti Aliasing
			{
				VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Anti Aliasing");
				gpuState.bindTexture<GpuStateChange::SurelyYes>(k_bindingTextureAntiAliasingSource, renderSceneCtx.postProcessTargets[0].colorTexture);
				beginPass(gpuState, renderSceneCtx.postProcessTargets[1].framebuffer, renderSceneCtx.shadingResolution, renderSceneCtx.targetParamsUbo);
				gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.aaProgram);
				glDrawArrays(GL_TRIANGLES, 0, 3);

				debugInspectRenderOutput(debugRenderInspectorCtx, "Anti Aliasing", renderSceneCtx.postProcessTargets[1].colorTexture, DebugType::ColorTexture);
			}

			// Present
			{
				VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Present");
				// TODO: should allow passing ivec4 so I can pass window's desired viewport directly (editor...).
				gpuState.bindTexture<GpuStateChange::SurelyYes>(k_bindingTexturePresentSource, renderSceneCtx.postProcessTargets[1].colorTexture);
				beginPass(gpuState, window.getDefaultFramebufferId(), window.getSize(), renderSceneCtx.targetParamsUbo);
				gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.presentProgram);
				glDrawArrays(GL_TRIANGLES, 0, 3);
			}

			// Debug Geometry
			if (!debugMeshCtx.lines.empty())
			{
				VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Debug Geometry");
				gpuState.disableDepthTest<GpuStateChange::SurelyNo>();
				gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.debugGeometryProgram);
				glLineWidth(2);

				glBindVertexArray(renderSceneCtx.debugGeometryVao);

				static std::vector<WorldDebugVertex> worldDebugVertices;
				worldDebugVertices.clear();
				for (auto const& debugVertex : debugMeshCtx.vertices)
				{
					worldDebugVertices.emplace_back(glm::vec3{ debugVertex.position - worldOriginPosition }, glm::vec4{ debugVertex.color });
				}

				glNamedBufferData(
					renderSceneCtx.debugGeometryVbo,
					worldDebugVertices.size() * sizeof(decltype(worldDebugVertices.front())),
					worldDebugVertices.data(),
					GL_STREAM_DRAW);

				glNamedBufferData(
					renderSceneCtx.debugGeometryEbo,
					debugMeshCtx.lines.size() * sizeof(decltype(debugMeshCtx.lines.front())),
					debugMeshCtx.lines.data(),
					GL_STREAM_DRAW);

				glDrawElements(GL_LINES, 2 * mistd::isize(debugMeshCtx.lines), GL_UNSIGNED_INT, nullptr);
			}

			// Hud
			{
				VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Hud");
				gpuState.enableBlend<GpuStateChange::SurelyYes>();
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				gpuState.bindUbo<GpuStateChange::LikelyNo>(k_bindingUboPostProcess, renderSceneCtx.hudParamsUbo);
				gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.hudProgram);
				glDrawArrays(GL_TRIANGLES, 0, 3);
				gpuState.disableBlend<GpuStateChange::SurelyYes>();
			}
		}

		if (std::ranges::find(debugRenderInspectorCtx.names, debugRenderInspectorCtx.selectedName)
			!= debugRenderInspectorCtx.names.end())
		{
			VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "Debug Inspector");
			debugDrawInspectedRenderOutput(
				gpuState
				, renderSceneCtx
				, debugRenderInspectorCtx
				, window.getDefaultFramebufferId()
				, window.getSize());
		}

		debugMeshCtx.clear();

		renderProfilingCtx.lightCount = mistd::isize(gpuLights);
		renderProfilingCtx.staticOpaqueMeshCount = mistd::isize(culledMeshes.staticOpaqueMeshes);
		renderProfilingCtx.riggedOpaqueMeshCount = mistd::isize(culledMeshes.riggedOpaqueMeshes);
	}
}
