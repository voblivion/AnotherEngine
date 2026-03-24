#include <vob/aoe/rendering/RenderSceneSystem.h>

#include <vob/aoe/rendering/GpuObjects.h>
#include <vob/aoe/rendering/ModelComponent.h>
#include <vob/aoe/rendering/ProgramUtils.h>
#include <vob/aoe/rendering/UniformUtils.h>

#include "vob/aoe/debug/DebugNameUtils.h"
#include "vob/aoe/debug/ImGuiUtils.h"

#include <vob/misc/std/container_util.h>
#include <vob/misc/std/enum_traits.h>

#include "vob/aoe/rendering/shaders/defines.h"

#include <glm/gtx/quaternion.hpp>
#include <imgui.h>

#include <array>


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

	struct ViewFrustumBounds
	{
		std::pair<glm::vec3, float> nearPlane;
		std::pair<glm::vec3, float> farPlane;
		std::pair<glm::vec3, float> leftPlane;
		std::pair<glm::vec3, float> rightPlane;
		std::pair<glm::vec3, float> topPlane;
		std::pair<glm::vec3, float> bottomPlane;
	};

	ViewFrustumBounds computeViewFrustumBounds(UniformViewParams const& a_viewParams)
	{
		auto const cameraForward = -glm::vec3(a_viewParams.viewToWorld[2]);
		auto const cameraRight = glm::vec3(a_viewParams.viewToWorld[0]);
		auto const cameraUp = glm::vec3(a_viewParams.viewToWorld[1]);
		auto const nearCenter = glm::vec3(a_viewParams.viewToWorld * glm::vec4(0.0, 0.0, -a_viewParams.nearClip, 1.0));
		auto const farCenter = glm::vec3(a_viewParams.viewToWorld * glm::vec4(0.0, 0.0, -a_viewParams.farClip, 1.0));
		auto const tanHalfFov = std::tan(a_viewParams.fov * 0.5f);
		auto const nearHalfHeight = a_viewParams.nearClip * tanHalfFov;
		auto const nearHalfWidth = nearHalfHeight * a_viewParams.aspectRatio;
		auto const farHalfHeight = a_viewParams.farClip * tanHalfFov;
		auto const farHalfWidth = farHalfHeight * a_viewParams.aspectRatio;

		auto const viewPosition = glm::vec3(a_viewParams.viewToWorld[3]);
		auto const leftPlaneNormal = glm::normalize(glm::cross(nearCenter - cameraRight * nearHalfWidth - viewPosition, cameraUp));
		auto const rightPlaneNormal = glm::normalize(glm::cross(cameraUp, nearCenter + cameraRight * nearHalfWidth - viewPosition));
		auto const upPlaneNormal = glm::normalize(glm::cross(nearCenter + cameraUp * nearHalfHeight - viewPosition, cameraRight));
		auto const downPlaneNormal = glm::normalize(glm::cross(cameraRight, nearCenter - cameraUp * nearHalfHeight - viewPosition));

		return ViewFrustumBounds{
			std::pair{cameraForward, -glm::dot(cameraForward, nearCenter)},
			std::pair{-cameraForward, -glm::dot(-cameraForward, farCenter)},
			std::pair{leftPlaneNormal, -glm::dot(leftPlaneNormal, viewPosition)},
			std::pair{rightPlaneNormal, -glm::dot(rightPlaneNormal, viewPosition)},
			std::pair{upPlaneNormal, -glm::dot(upPlaneNormal, viewPosition)},
			std::pair{downPlaneNormal, -glm::dot(downPlaneNormal, viewPosition)}
		};
	}

	bool testIntersectViewFrustumBounds(ViewFrustumBounds const& a_viewFrustumBounds, glm::vec3 const& a_position, float a_radius)
	{
		auto const testPlane = [&a_position, a_radius](std::pair<glm::vec3, float> const& plane) -> bool
			{
				return glm::dot(plane.first, a_position) + plane.second >= -a_radius;
			};

		if (!testPlane(a_viewFrustumBounds.nearPlane))
		{
			return false;
		}
		if (!testPlane(a_viewFrustumBounds.farPlane))
		{
			return false;
		}
		if (!testPlane(a_viewFrustumBounds.leftPlane))
		{
			return false;
		}
		if (!testPlane(a_viewFrustumBounds.rightPlane))
		{
			return false;
		}
		if (!testPlane(a_viewFrustumBounds.topPlane))
		{
			return false;
		}
		if (!testPlane(a_viewFrustumBounds.bottomPlane))
		{
			return false;
		}

		return true;
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

	namespace
	{
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

		enum class GpuDepthFunc : GraphicEnum
		{
			Less = GL_LESS,
			LessEqual = GL_LEQUAL,
			Equal = GL_EQUAL
		};

		enum class GpuStateChange
		{
			// A change must happen, else we are wasting gpu calls
			SurelyYes,
			// A change often happens, no need to branch
			LikelyYes,
			// A change rarely happens, branch
			LikelyNo,
			// A change must not happen, else something went wrong
			SurelyNo
		};

		struct GpuState
		{
			template<GpuStateChange t_expectedChange, typename TValue, typename TPushChangeFunc>
			void changeState(TValue& a_currentState, TValue a_newState, TPushChangeFunc a_pushChangeFunc)
			{
				if constexpr (t_expectedChange == GpuStateChange::SurelyYes)
				{
					assert(a_currentState != a_newState);
					a_pushChangeFunc(a_newState);
					a_currentState = a_newState;
				}
				else if (t_expectedChange == GpuStateChange::LikelyYes)
				{
					a_pushChangeFunc(a_newState);
					a_currentState = a_newState;
				}
				else if constexpr (t_expectedChange == GpuStateChange::LikelyNo)
				{
					if (a_currentState != a_newState)
					{
						a_pushChangeFunc(a_newState);
						a_currentState = a_newState;
					}
				}
				else
				{
					assert(a_currentState == a_newState);
				}
			}

			template<GpuStateChange t_expectedChange>
			void enableDepthTest()
			{
				changeState<t_expectedChange>(m_depthTest, TernaryState::True, [](auto) { glEnable(GL_DEPTH_TEST); });
			}

			template<GpuStateChange t_expectedChange>
			void disableDepthTest()
			{
				changeState<t_expectedChange>(
					m_depthTest, TernaryState::False, [](auto) { glDisable(GL_DEPTH_TEST); });
			}

			template<GpuStateChange t_expectedChange>
			void enableDepthWrite()
			{
				changeState<t_expectedChange>(m_depthWrite, TernaryState::True, [](auto) { glDepthMask(GL_TRUE); });
			}

			template<GpuStateChange t_expectedChange>
			void disableDepthWrite()
			{
				changeState<t_expectedChange>(m_depthWrite, TernaryState::False, [](auto) { glDepthMask(GL_FALSE); });
			}

			template<GpuStateChange t_expectedChange>
			void enableBlend()
			{
				changeState<t_expectedChange>(m_blend, TernaryState::True, [](auto) { glEnable(GL_BLEND); });
			}

			template<GpuStateChange t_expectedChange>
			void disableBlend()
			{
				changeState<t_expectedChange>(m_blend, TernaryState::False, [](auto) { glDisable(GL_BLEND); });
			}

			template<GpuStateChange t_expectedChange>
			void setDepthFunc(GpuDepthFunc a_depthFunc)
			{
				changeState<t_expectedChange>(
					m_depthFunc, std::to_underlying(a_depthFunc), [](auto depthFunc) { glDepthFunc(depthFunc); });
			}

			template<GpuStateChange t_expectedChange>
			void setClearDepth(double a_clearDepth)
			{
				changeState<t_expectedChange>(
					m_clearDepth, a_clearDepth, [](auto clearDepth) { glClearDepth(clearDepth); });
			}

			template<GpuStateChange t_expectedChange>
			void enableColorWrite()
			{
				changeState<t_expectedChange>(
					m_colorWrite, TernaryState::True, [](auto) { glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); });
			}

			template<GpuStateChange t_expectedChange>
			void disableColorWrite()
			{
				changeState<t_expectedChange>(
					m_colorWrite, TernaryState::False, [](auto) { glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); });
			}

			template<GpuStateChange t_expectedChange>
			void setClearColor(glm::vec4 a_clearColor)
			{
				changeState<t_expectedChange>(
					m_clearColor, a_clearColor, [](auto clearColor) { glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w); });
			}

			template<GpuStateChange t_expectedChange>
			void setViewport(glm::ivec4 a_viewport)
			{
				changeState<t_expectedChange>(
					m_viewport,
					a_viewport,
					[](auto viewport) { glViewport(viewport.x, viewport.y, viewport.z, viewport.w); });
			}

			template<GpuStateChange t_expectedChange>
			void bindFramebuffer(GraphicId a_framebuffer)
			{
				changeState<t_expectedChange>(
					m_framebuffer,
					a_framebuffer,
					[](auto framebuffer) { glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); });
			}

			template<GpuStateChange t_expectedChange>
			void unbindFramebuffer()
			{
				changeState<t_expectedChange>(m_framebuffer, k_invalidId, [](auto) {});
			}

			template<GpuStateChange t_expectedChange>
			void bindUbo(GraphicId a_index, GraphicId a_ubo)
			{
				changeState<t_expectedChange>(
					m_ubos[a_index],
					a_ubo,
					[a_index](auto ubo) { glBindBufferBase(GL_UNIFORM_BUFFER, a_index, ubo); });
			}

			template<GpuStateChange t_expectedChange>
			void unbindUbo(GraphicId a_index)
			{
				changeState<t_expectedChange>(m_ubos[a_index], k_invalidId, [](auto) {});
			}

			template<GpuStateChange t_expectedChange>
			void bindSsbo(GraphicId a_index, GraphicId a_ssbo)
			{
				changeState<t_expectedChange>(
					m_ssbos[a_index],
					a_ssbo,
					[a_index](auto ssbo) { glBindBufferBase(GL_SHADER_STORAGE_BUFFER, a_index, ssbo); });
			}

			template<GpuStateChange t_expectedChange>
			void unbindSsbo(GraphicId a_index)
			{
				changeState<t_expectedChange>(m_ubos[a_index], k_invalidId, [a_index](auto) {});
			}

			template<GpuStateChange t_expectedChange>
			void bindTexture(GraphicId a_index, GraphicId a_texture)
			{
				changeState<t_expectedChange>(
					m_textures[a_index],
					a_texture,
					[a_index](auto texture) { glBindTextureUnit(a_index, texture); });
			}

			template<GpuStateChange t_expectedChange>
			void unbindTexture(GraphicId a_index)
			{
				changeState<t_expectedChange>(m_textures[a_index], k_invalidId, [a_index](auto) {});
			}

			template<GpuStateChange t_expectedChange>
			void useProgram(GraphicId a_program, bool expectChange = true)
			{
				changeState<t_expectedChange>(m_program, a_program, [](auto program) { glUseProgram(program); });
			}

		private:
			std::array<GraphicId, 36> m_ubos = { k_invalidId };
			std::array<GraphicId, 8> m_ssbos = { k_invalidId };
			std::array<GraphicId, 32> m_textures = { k_invalidId };

			enum class TernaryState : int8_t
			{
				Unknown = -1,
				False = 0,
				True = 1
			};
			TernaryState m_depthTest = TernaryState::Unknown;
			TernaryState m_blend = TernaryState::Unknown;
			TernaryState m_depthWrite = TernaryState::Unknown;
			TernaryState m_colorWrite = TernaryState::Unknown;
			GraphicEnum m_depthFunc = GL_NEVER;
			GraphicId m_framebuffer = k_invalidId;
			GraphicId m_program = k_invalidId;
			glm::ivec4 m_viewport = glm::ivec4{ -1 };
			double m_clearDepth = -1.0;
			glm::vec4 m_clearColor = glm::vec4{ -1.0f };
		};

		UniformGlobalParams createGlobalParams(aoest::TimeContext const& a_timeCtx)
		{
			return UniformGlobalParams{
				.worldTime = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - a_timeCtx.worldStartTime).count()
			};
		}

		UniformViewParams createViewParams(
			aoewi::WindowContext const& a_windowCtx,
			CameraDirectorContext const& a_cameraDirectorCtx,
			entt::view<entt::get_t<aoest::Position const, aoest::Rotation const, CameraComponent const>> a_cameraEntities)
		{
			auto const resolution = a_windowCtx.window.get().getSize();
			auto const invResolution = 1.0f / glm::vec2{ resolution };
			auto const aspectRatio = static_cast<float>(resolution.x) / resolution.y;

			auto const [position, rotation, nearClip, farClip, fov] = getCameraProperties(a_cameraDirectorCtx.activeCameraEntity, a_cameraEntities);
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

		struct CulledLight
		{
			float importance;
			glm::vec3 position;
			glm::quat rotation;
			LightComponent const* lightComponent;
		};

		std::tuple<UniformLightingParams, UniformShadowParams, int32_t> createLightingAndShadowParams(
			ViewFrustumBounds const& a_viewFrustumBounds,
			glm::vec3 const& a_lightFocusPosition,
			entt::view<entt::get_t<aoest::Position const, aoest::Rotation const, LightComponent const>> a_lightEntities,
			int32_t a_lightsCapacity,
			glm::ivec2 const& a_lightClusterTileSize,
			int32_t a_lightClusterZCount,
			int32_t a_lightClusterCapacity,
			std::vector<GpuLight>& o_gpuLights)
		{
			// TODO: remove magic
			static std::vector<CulledLight> culledLights;
			culledLights.clear();
			for (auto const [entity, position, rotation, lightCmp] : a_lightEntities.each())
			{
				if (!testIntersectViewFrustumBounds(a_viewFrustumBounds, position, lightCmp.radius))
				{
					continue;
				}

				auto const distanceImportance = 1.0f - glm::length(position - a_lightFocusPosition) / lightCmp.radius;
				auto const colorImportance = glm::dot(lightCmp.color, glm::vec3{ 0.299, 0.587, 0.114f });
				auto const intensityImportance = lightCmp.intensity;
				auto const importance = distanceImportance * colorImportance * intensityImportance;

				culledLights.emplace_back(importance, position, rotation, &lightCmp);
			}
			std::sort(culledLights.begin(), culledLights.end(), [](auto const& lhs, auto const& rhs) { return lhs.importance > rhs.importance; });
			auto const lightingParams = UniformLightingParams{
				.ambientColor = glm::vec3{ 0.5f },
				.lightCount = std::min(mistd::isize(culledLights), a_lightsCapacity),
				.lightClusterTileSize = a_lightClusterTileSize,
				.lightClusterZCount = a_lightClusterZCount,
				.lightClusterCapacity = a_lightClusterCapacity
			};

			o_gpuLights.reserve(lightingParams.lightCount);
			int32_t spotLightShadowMapCount = 0;
			auto shadowParams = UniformShadowParams{};
			for (auto i = 0; i < lightingParams.lightCount; ++i)
			{
				auto const& culledLight = culledLights[i];
				auto const isPointLight = culledLight.lightComponent->type == LightComponent::Type::Point;
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
					shadowParams.spotLights[spotLightShadowMapIndex] = GpuShadowLight{
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

#pragma optimize("", off)
	bool RenderSceneSystem::executeNew(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto const& renderSceneCtx = m_newRenderSceneContext.get(a_wdap);
		auto const& debugProgramCtx = m_debugProgramContext.get(a_wdap);
		auto const& materialManager = *m_materialManagerContext.get(a_wdap).materialManager;
		auto const& window = m_windowContext.get(a_wdap).window.get();
		auto staticModelEntities = m_staticModelEntities.get(a_wdap);
		auto riggedModelEntities = m_riggedModelEntities.get(a_wdap);
		GpuState gpuState;

		glQueryCounter(renderSceneCtx.totalTimerQueries[0], GL_TIMESTAMP);
		// 0 - Prepare Debug
		enum class DebugMode2
		{
			None,
			LightClusters,
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
		static int32_t k_debugSpotLightShadowMapIndex = 0;
		static bool k_ssaoEnabled = true;
		static bool k_ssrEnabled = true;
		if (ImGui::Begin("Render Debug"))
		{
			aoedb::ImGuiEnumCombo("Debug Mode", &k_debugMode);
			ImGui::BeginDisabled(k_debugMode != DebugMode2::SpotLightShadowMap);
			ImGui::InputInt("Spot Light Shadow Map Index", &k_debugSpotLightShadowMapIndex);
			ImGui::EndDisabled();
			k_debugSpotLightShadowMapIndex = std::clamp(k_debugSpotLightShadowMapIndex, 0, k_spotLightShadowMapsCapacity - 1);

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
				createShadingProgram(*shadingSource, false /* use rig */, forwardProgram.staticProgram);
				createShadingProgram(*shadingSource, true /* use rig */, forwardProgram.riggedProgram);
			}

		}
		ImGui::End();

		// I - Prepare Scene
		auto const globalParams = createGlobalParams(m_timeContext.get(a_wdap));
		glNamedBufferSubData(renderSceneCtx.globalParamsUbo, 0, sizeof(globalParams), &globalParams);

		auto const viewParams = createViewParams(m_windowContext.get(a_wdap), m_cameraDirectorContext.get(a_wdap), m_cameraEntities.get(a_wdap));
		glNamedBufferSubData(renderSceneCtx.viewParamsUbo, 0, sizeof(viewParams), &viewParams);

		auto const viewFrustumBounds = computeViewFrustumBounds(viewParams);

		// II - Prepare Lights & Shadows
		// TODO: how to define light focus point
		auto const lightFocusPositionH = viewParams.viewToWorld * glm::vec4{ 0.0, 0.0, -7.0f, 1.0f };
		auto const lightFocusPosition = glm::vec3{ lightFocusPositionH } / lightFocusPositionH.w;
		// TODO: remove magic
		static std::vector<GpuLight> gpuLights;
		gpuLights.clear();
		auto const [lightingParams, shadowParams, spotLightShadowMapCount] = createLightingAndShadowParams(
			viewFrustumBounds,
			lightFocusPosition,
			m_lightEntities.get(a_wdap),
			renderSceneCtx.lightsCapacity,
			renderSceneCtx.lightClusterTileSize,
			renderSceneCtx.lightClusterZCount,
			renderSceneCtx.lightClusterCapacity,
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
		for (auto const [entity, position, rotation, staticModelCmp] : staticModelEntities.each())
		{
			if (testIntersectViewFrustumBounds(viewFrustumBounds, position, staticModelCmp.boundingRadius))
			{
				auto const modelParams = UniformModelParams{ .modelToWorld = aoest::combine(position, rotation) };
				if (staticModelCmp.modelParams != modelParams)
				{
					staticModelCmp.modelParams = modelParams;
					glNamedBufferSubData(staticModelCmp.modelParamsUbo, 0, sizeof(modelParams), &modelParams);
				}

				for (auto const& mesh : staticModelCmp.meshes)
				{

					switch (mesh.shadingPass)
					{
					case ShadingPass::Opaque:
						culledOpaqueStaticMeshes.emplace_back(mesh.program, mesh.materialIndex, staticModelCmp.modelParamsUbo, mesh.meshVao, mesh.indexCount);
						break;
					case ShadingPass::Translucent:
						culledTranslucentStaticMeshes.emplace_back(mesh.program, mesh.materialIndex, staticModelCmp.modelParamsUbo, mesh.meshVao, mesh.indexCount);
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
		for (auto const [entity, position, rotation, riggedModelCmp] : riggedModelEntities.each())
		{
			if (testIntersectViewFrustumBounds(viewFrustumBounds, position, riggedModelCmp.boundingRadius))
			{
				auto const modelParams = UniformModelParams{ .modelToWorld = aoest::combine(position, rotation) };
				if (riggedModelCmp.modelParams != modelParams)
				{
					riggedModelCmp.modelParams = modelParams;
					glNamedBufferSubData(riggedModelCmp.modelParamsUbo, 0, sizeof(modelParams), &modelParams);
				}

				for (auto const& mesh : riggedModelCmp.meshes)
				{
					switch (mesh.shadingPass)
					{
					case ShadingPass::Opaque:
						culledOpaqueRiggedMeshes.emplace_back(
							mesh.program, mesh.materialIndex, riggedModelCmp.modelParamsUbo, riggedModelCmp.rigParamsUbo, mesh.meshVao, mesh.indexCount);
						break;
					case ShadingPass::Translucent:
						culledTranslucentRiggedMeshes.emplace_back(
							mesh.program, mesh.materialIndex, riggedModelCmp.modelParamsUbo, riggedModelCmp.rigParamsUbo, mesh.meshVao, mesh.indexCount);
						break;
					default:
						break;
					}
				}
			}
		}

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		// V - Compute Shadow Maps
		{
			ScopedGpuTimerQuery const timerQuery(renderSceneCtx.renderPassTimerQueries[RenderPass::ShadowMaps]);
			gpuState.enableDepthTest<GpuStateChange::SurelyYes>();
			gpuState.enableDepthWrite<GpuStateChange::SurelyYes>();
			gpuState.setDepthFunc<GpuStateChange::SurelyYes>(GpuDepthFunc::Less);
			gpuState.setClearDepth<GpuStateChange::SurelyYes>(1.0);
			gpuState.disableColorWrite<GpuStateChange::SurelyYes>();
			gpuState.disableBlend<GpuStateChange::SurelyYes>();
			gpuState.bindUbo<GpuStateChange::SurelyYes>(k_bindingUboView, renderSceneCtx.lightViewParamsUbo);
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

				auto const spotLightViewFrustumBounds = computeViewFrustumBounds(spotLightViewParams);

				gpuState.bindFramebuffer<GpuStateChange::SurelyYes>(renderSceneCtx.spotLightShadowMapTargets[i].framebuffer);
				gpuState.setViewport<GpuStateChange::LikelyYes>(glm::ivec4{ 0, 0, spotLightViewParams.resolution });
				glClear(GL_DEPTH_BUFFER_BIT);

				gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.staticShadowMapProgram);
				for (auto const [entity, position, rotation, staticModelCmp] : staticModelEntities.each())
				{
					if (testIntersectViewFrustumBounds(spotLightViewFrustumBounds, position, staticModelCmp.boundingRadius))
					{
						auto const modelParams = UniformModelParams{ .modelToWorld = aoest::combine(position, rotation) };
						if (staticModelCmp.modelParams != modelParams)
						{
							staticModelCmp.modelParams = modelParams;
							glNamedBufferSubData(staticModelCmp.modelParamsUbo, 0, sizeof(modelParams), &modelParams);
						}

						gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboModel, staticModelCmp.modelParamsUbo);
						for (auto const& mesh : staticModelCmp.meshes)
						{
							glBindVertexArray(mesh.meshVao);
							glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
						}
					}
				}

				gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.riggedShadowMapProgram);
				for (auto const [entity, position, rotation, riggedModelCmp] : riggedModelEntities.each())
				{
					if (testIntersectViewFrustumBounds(spotLightViewFrustumBounds, position, riggedModelCmp.boundingRadius))
					{
						auto const modelParams = UniformModelParams{ .modelToWorld = aoest::combine(position, rotation) };
						if (riggedModelCmp.modelParams != modelParams)
						{
							riggedModelCmp.modelParams = modelParams;
							glNamedBufferSubData(riggedModelCmp.modelParamsUbo, 0, sizeof(modelParams), &modelParams);
						}

						gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboModel, riggedModelCmp.modelParamsUbo);
						gpuState.bindUbo<GpuStateChange::LikelyYes>(k_bindingUboRig, riggedModelCmp.rigParamsUbo);
						for (auto const& mesh : riggedModelCmp.meshes)
						{
							glBindVertexArray(mesh.meshVao);
							glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
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
			// gpuState.bindTexture<GpuStateChange::SurelyYes>(k_shadingSunShadowMapTextureBindingLocation, renderSceneCtx.sunShadowMap.depthTexture);
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

		// XII - Post Processes
		if (k_debugMode != DebugMode2::None)
		{
			auto debugParams = UniformDebugParams{};
			auto viewParamsUbo = renderSceneCtx.viewParamsUbo;
			auto debugTexture = k_invalidId;
			switch (k_debugMode)
			{
			case DebugMode2::LightClusters:
			{
				debugTexture = renderSceneCtx.opaqueDepthTexture;
				debugParams.type = static_cast<uint8_t>(DebugType::LightClusters);
				break;
			}
			case DebugMode2::SpotLightShadowMap:
			{
				auto const spotLightViewParams = UniformViewParams{
					.worldToClip = shadowParams.spotLights[k_debugSpotLightShadowMapIndex].worldToClip,
					.viewToWorld = shadowParams.spotLights[k_debugSpotLightShadowMapIndex].viewToWorld,
					.resolution = renderSceneCtx.spotLightShadowMapTargets[k_debugSpotLightShadowMapIndex].resolution,
					.nearClip = shadowParams.spotLights[k_debugSpotLightShadowMapIndex].nearClip,
					.farClip = shadowParams.spotLights[k_debugSpotLightShadowMapIndex].farClip,
					.fov = shadowParams.spotLights[k_debugSpotLightShadowMapIndex].fov,
					.aspectRatio = 1.0f
				};
				glNamedBufferSubData(renderSceneCtx.lightViewParamsUbo, 0, sizeof(spotLightViewParams), &spotLightViewParams);
				viewParamsUbo = renderSceneCtx.lightViewParamsUbo;
				debugTexture = renderSceneCtx.spotLightShadowMapTargets[k_debugSpotLightShadowMapIndex].depthTexture;
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

			gpuState.bindFramebuffer<GpuStateChange::SurelyYes>(window.getDefaultFramebufferId());
			gpuState.setViewport<GpuStateChange::LikelyYes>(glm::ivec4{ 0, 0, renderSceneCtx.postProcessResolution });
			gpuState.useProgram<GpuStateChange::SurelyYes>(renderSceneCtx.debugProgram);

			glBindVertexArray(renderSceneCtx.postProcessVao);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
		else
		{
			ScopedGpuTimerQuery const timerQuery(renderSceneCtx.renderPassTimerQueries[RenderPass::PostProcesses]);
			if (!renderSceneCtx.postProcesses.empty())
			{
				gpuState.disableDepthTest<GpuStateChange::SurelyYes>();
				gpuState.enableColorWrite<GpuStateChange::SurelyNo>();
				gpuState.disableBlend<GpuStateChange::SurelyYes>();
				glBindVertexArray(renderSceneCtx.postProcessVao);
				
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

		return true;
	}

	void RenderSceneSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		if (executeNew(a_wdap))
		{
			return;
		}
#ifndef WIP
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
		static int32_t k_ssrMode = 1;
		static int32_t k_ssrLog2Step = 6;
		static int32_t k_ssrLog2SubStep = 3;
		static float k_ssrMaxRangeRatioMin = 0.01f;
		static float k_ssrMaxRangeRatioMax = 100.0f;
		static float k_ssrMaxRangeRatio = 0.1f;
		static float k_ssrThicknessMin = 0.01f;
		static float k_ssrThicknessMax = 0.5f;
		static float k_ssrThickness = 0.05f;
		static float k_ssrMaxThickness = 5.0f;
		static float k_ssrInitialBiasRatio = 0.001f;
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
			if (!debugProgramCtx.oldForwardPrograms.empty())
			{
				ImGui::SeparatorText("Shading");
				k_debugActiveForwardProgramIndex =
					std::min(k_debugActiveForwardProgramIndex, mistd::isize(debugProgramCtx.oldForwardPrograms) - 1);

				auto const activeForwardProgramNameStr = toSmallStr(debugProgramCtx.oldForwardPrograms[k_debugActiveForwardProgramIndex].name);
				if (ImGui::BeginCombo("Forward Program", activeForwardProgramNameStr.data()))
				{
					for (int32_t i = 0; i < mistd::isize(debugProgramCtx.oldForwardPrograms); ++i)
					{
						auto const forwardProgramNameStr = toSmallStr(debugProgramCtx.oldForwardPrograms[i].name);
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
					auto const& forwardProgram = debugProgramCtx.oldForwardPrograms[k_debugActiveForwardProgramIndex];
					auto const shadingSource = debugProgramCtx.stringDatabase.find(
						debugProgramCtx.filesystemIndexer.get_runtime_id(forwardProgram.shadingSourcePath));
					oldCreateForwardProgram(*shadingSource, false /* use rig */, forwardProgram.staticProgram);
					oldCreateForwardProgram(*shadingSource, true /* use rig */, forwardProgram.riggedProgram);
				}
				ImGui::SeparatorText("SSR");
				ImGui::InputInt("Mode", &k_ssrMode);
				ImGui::InputInt("Log2 Step", &k_ssrLog2Step);
				ImGui::InputInt("Log2 Sub Step", &k_ssrLog2SubStep);
				ImGui::InputFloat("Max Range Ratio Min", &k_ssrMaxRangeRatioMin);
				ImGui::InputFloat("Max Range Ratio Max", &k_ssrMaxRangeRatioMax);
				ImGui::SliderFloat("Max Range Ratio", &k_ssrMaxRangeRatio, k_ssrMaxRangeRatioMin, k_ssrMaxRangeRatioMax, "%.6f", ImGuiSliderFlags_Logarithmic);
				ImGui::InputFloat("Thickness Min", &k_ssrThicknessMin);
				ImGui::InputFloat("Thickness Max", &k_ssrThicknessMax);
				ImGui::SliderFloat("Thickness Ratio", &k_ssrThickness, k_ssrThicknessMin, k_ssrThicknessMax, "%.6f", ImGuiSliderFlags_Logarithmic);
				ImGui::SliderFloat("Max Thickness", &k_ssrMaxThickness, 0.1f, 10.0f, "%.6f");
				ImGui::SliderFloat("Initial Bias Ratio", &k_ssrInitialBiasRatio, 0.0001f, 0.5f, "%.6f", ImGuiSliderFlags_Logarithmic);

				k_ssrLog2Step = std::max(0, k_ssrLog2Step);
				k_ssrLog2SubStep = std::clamp(k_ssrLog2SubStep, 0, 12);
				auto const ssrParams = SsrParams{
					.mode = k_ssrMode,
					.log2Step = k_ssrLog2Step,
					.log2SubStep = k_ssrLog2SubStep,
					.maxRangeRatio = k_ssrMaxRangeRatio,
					.thicknessRatio = k_ssrThickness,
					.initialBiasRatio = k_ssrInitialBiasRatio,
					.maxThickness = k_ssrMaxThickness
				};
				glNamedBufferSubData(renderSceneContext.ssrUbo, 0, sizeof(ssrParams), &ssrParams);

				if (ImGui::Button("Recompile SSR Program"))
				{
					auto const ssrSource = debugProgramCtx.stringDatabase.find(
						debugProgramCtx.filesystemIndexer.get_runtime_id("data/shaders/ssr.glsl"));
					oldCreatePostProcessProgram(*ssrSource, renderSceneContext.ssrProgram);
				}
				if (ImGui::Button("Recompile Reflection Program"))
				{
					auto const reflectionSource = debugProgramCtx.stringDatabase.find(
						debugProgramCtx.filesystemIndexer.get_runtime_id("data/shaders/reflection.glsl"));
					oldCreatePostProcessProgram(*reflectionSource, renderSceneContext.reflectionProgram);
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
						mistd::isize(culledLights) - 1, distanceFactor * colorFactor * intensityFactor, rotation, lightCmp.outerAngle, lightCmp.size);
				}
			}
		}
		std::array<ShadowParams, k_maxSpotLightShadowMapCount> spotLightShadowParams = {};
		std::sort(shadowSpotLights.begin(), shadowSpotLights.end(), [](auto const& lhs, auto const& rhs) { return lhs.importance > rhs.importance; });
		shadowSpotLights.resize(std::min(shadowSpotLights.size(), renderSceneContext.spotLightShadowMaps.size()));
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
		glNamedBufferSubData(renderSceneContext.lightingParamsUbo, 0, sizeof(LightParams), &lightParams);

		// 3. Compute light clusters
		glBeginQuery(GL_TIME_ELAPSED, renderSceneContext.timerQueries[0]);
		glUseProgram(renderSceneContext.lightClusteringProgram);
		glBindBufferBase(GL_UNIFORM_BUFFER, k_viewParamsUboLocation, renderSceneContext.viewParamsUbo);
		glBindBufferBase(GL_UNIFORM_BUFFER, k_lightParamsUboLocation, renderSceneContext.lightingParamsUbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, k_lightsSsboLocation, renderSceneContext.lightsSsbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, k_lightClusterSizesSsboLocation, renderSceneContext.lightClusterSizesSsbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, k_lightClusterIndicesSsboLocation, renderSceneContext.lightClusterIndicesSsbo);
		auto const workGroupCount = static_cast<uint32_t>(
			(renderSceneContext.lightClusterCount + renderSceneContext.lightClusteringWorkGroupSize - 1) / renderSceneContext.lightClusteringWorkGroupSize);
		glDispatchCompute(workGroupCount, 1, 1);
#endif

		// 4. Cull meshes
		static std::pmr::vector<CulledStaticMesh> culledStaticMeshes;
		culledStaticMeshes.clear();
		auto const staticModelEntities = m_staticModelEntities.get(a_wdap);
		for (auto const [entity, position, rotation, staticModelCmp] : staticModelEntities.each())
		{
			if (testCameraFrustumIntersect(cameraFrustumPlanes, position, staticModelCmp.boundingRadius))
			{
				auto const modelParams = ModelParams{ .modelToWorld = aoest::combine(position, rotation) };
				if (modelParams != staticModelCmp.oldModelParams)
				{
					staticModelCmp.oldModelParams = modelParams;
					glNamedBufferSubData(staticModelCmp.modelParamsUbo, 0, sizeof(ModelParams), &modelParams);
				}

				for (auto const& mesh : staticModelCmp.meshes)
				{
					culledStaticMeshes.emplace_back(
						mesh.oldProgram,
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
				if (modelParams != riggedModelCmp.oldModelParams)
				{
					riggedModelCmp.oldModelParams = modelParams;
					glNamedBufferSubData(riggedModelCmp.modelParamsUbo, 0, sizeof(ModelParams), &modelParams);
				}

				for (auto const& mesh : riggedModelCmp.meshes)
				{
					culledRiggedMeshes.emplace_back(
						mesh.oldProgram,
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

			glBindFramebuffer(GL_FRAMEBUFFER, renderSceneContext.spotLightShadowMaps[i].framebuffer);
			glViewport(0, 0, renderSceneContext.spotLightShadowMaps[i].size.x, renderSceneContext.spotLightShadowMaps[i].size.y);
			glClear(GL_DEPTH_BUFFER_BIT);

			glUseProgram(renderSceneContext.staticDepthProgram);
			glBindBufferBase(GL_UNIFORM_BUFFER, k_viewParamsUboLocation, renderSceneContext.lightViewParamsUbo);
			for (auto const [entity, position, rotation, staticModelCmp] : staticModelEntities.each())
			{
				if (testCameraFrustumIntersect(spotLightFrustumPlanes, position, staticModelCmp.boundingRadius))
				{
					auto const modelParams = ModelParams{ .modelToWorld = aoest::combine(position, rotation) };
					if (modelParams != staticModelCmp.oldModelParams)
					{
						staticModelCmp.oldModelParams = modelParams;
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
					if (modelParams != riggedModelCmp.oldModelParams)
					{
						riggedModelCmp.oldModelParams = modelParams;
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
			glBindTextureUnit(k_spotLightShadowMapsFirstIndex + i, renderSceneContext.spotLightShadowMaps[i].depthTexture);
		}
		GraphicId currentForwardProgram = k_invalidId;
		int32_t currentMaterialIndex = -1;
		{
			for (auto const& staticMesh : culledStaticMeshes)
			{
				if (currentForwardProgram != staticMesh.forwardProgram)
				{
					glUseProgram(staticMesh.forwardProgram);
					currentForwardProgram = staticMesh.forwardProgram;
					glBindBufferBase(GL_UNIFORM_BUFFER, k_viewParamsUboLocation, renderSceneContext.viewParamsUbo);
					glBindBufferBase(GL_UNIFORM_BUFFER, k_lightParamsUboLocation, renderSceneContext.lightingParamsUbo);
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

				glBindBufferBase(GL_UNIFORM_BUFFER, k_modelParamsUboLocation, staticMesh.modelParamsUbo);
				glBindVertexArray(staticMesh.meshVao);
				glDrawElements(GL_TRIANGLES, staticMesh.indexCount, GL_UNSIGNED_INT, nullptr);
			}

			for (auto const& riggedMesh : culledRiggedMeshes)
			{
				if (currentForwardProgram != riggedMesh.forwardProgram)
				{
					glUseProgram(riggedMesh.forwardProgram);
					currentForwardProgram = riggedMesh.forwardProgram;
					glBindBufferBase(GL_UNIFORM_BUFFER, k_viewParamsUboLocation, renderSceneContext.viewParamsUbo);
					glBindBufferBase(GL_UNIFORM_BUFFER, k_lightParamsUboLocation, renderSceneContext.lightingParamsUbo);
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
		glBindFramebuffer(GL_FRAMEBUFFER, renderSceneContext.ssrFramebuffer);
		glClearColor(k_black.r, k_black.g, k_black.b, k_black.a);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		glUseProgram(renderSceneContext.ssrProgram);
		glBindBufferBase(GL_UNIFORM_BUFFER, k_viewParamsUboLocation, renderSceneContext.viewParamsUbo);
		glBindBufferBase(GL_UNIFORM_BUFFER, k_lightParamsUboLocation, renderSceneContext.lightingParamsUbo);
		glBindBufferBase(GL_UNIFORM_BUFFER, k_customPostProcessConfigUboLocation, renderSceneContext.ssrUbo);
		glBindTextureUnit(0, renderSceneContext.sceneColorTexture);
		glBindTextureUnit(1, renderSceneContext.sceneNormalTexture);
		glBindTextureUnit(2, renderSceneContext.sceneSurfaceTexture);
		glBindTextureUnit(3, renderSceneContext.sceneDepthTexture);
		glBindVertexArray(renderSceneContext.postProcessVao);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glGenerateTextureMipmap(renderSceneContext.ssrColorTexture);

		glBindFramebuffer(GL_FRAMEBUFFER, renderSceneContext.reflectionFramebuffer);
		glClearColor(k_black.r, k_black.g, k_black.b, k_black.a);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(renderSceneContext.reflectionProgram);
		glBindTextureUnit(0, renderSceneContext.sceneColorTexture);
		glBindTextureUnit(1, renderSceneContext.sceneSurfaceTexture);
		glBindTextureUnit(2, renderSceneContext.ssrColorTexture);
		glBindVertexArray(renderSceneContext.postProcessVao);
		glDrawArrays(GL_TRIANGLES, 0, 3);
#endif

		// 10. Post processes
		glBeginQuery(GL_TIME_ELAPSED, renderSceneContext.timerQueries[4]);
#ifdef VOB_AOEGL_DEBUG
		if (k_debugMode != DebugMode::None)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, window.getDefaultFramebufferId());
			glClearColor(k_black.r, k_black.g, k_black.b, k_black.a);
			glClear(GL_COLOR_BUFFER_BIT);
			glDisable(GL_DEPTH_TEST);

			glUseProgram(renderSceneContext.debugPostProcessProgram);
			glBindTextureUnit(0, renderSceneContext.reflectionColorTexture);
			glBindTextureUnit(1, renderSceneContext.sceneNormalTexture);
			glBindTextureUnit(2, renderSceneContext.sceneDepthTexture);
			glBindTextureUnit(3, renderSceneContext.ssrColorTexture);
			glBindTextureUnit(4, renderSceneContext.sunShadowMap.depthTexture);
			for (int32_t i = 0; i < mistd::isize(shadowSpotLights); ++i)
			{
				glBindTextureUnit(5 + i, renderSceneContext.spotLightShadowMaps[i].depthTexture);
			}
			glBindTextureUnit(11, renderSceneContext.sceneColorTexture);
			glBindBufferBase(GL_UNIFORM_BUFFER, k_viewPostProcessConfigUboLocation, renderSceneContext.viewParamsUbo);
			glBindBufferBase(GL_UNIFORM_BUFFER, k_lightPostProcessConfigUboLocation, renderSceneContext.lightingParamsUbo);
			glBindBufferBase(GL_UNIFORM_BUFFER, k_customPostProcessConfigUboLocation, renderSceneContext.debugParamsUbo);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, renderSceneContext.lightClusterSizesSsbo);
			glBindVertexArray(renderSceneContext.postProcessVao);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
		else
#endif
		{
			glClearColor(1.0f, 0.0f, 1.f, 1.0f);
			glDisable(GL_DEPTH_TEST);
			glBindTextureUnit(1, renderSceneContext.sceneDepthTexture);
			glBindBufferBase(GL_UNIFORM_BUFFER, k_viewPostProcessConfigUboLocation, renderSceneContext.viewParamsUbo);
			glBindBufferBase(GL_UNIFORM_BUFFER, k_lightPostProcessConfigUboLocation, renderSceneContext.lightingParamsUbo);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, renderSceneContext.lightClusterSizesSsbo);
			glBindVertexArray(renderSceneContext.postProcessVao);

			auto const postProcessFramebuffers = std::array{
				renderSceneContext.reflectionFramebuffer, renderSceneContext.postProcessFramebuffer };
			auto const postProcessColorTextures = std::array{
				renderSceneContext.reflectionColorTexture, renderSceneContext.postProcessColorTexture };
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
