#include "vob/aoe/rendering/RenderSceneResourcesUtils.h"

#include "vob/aoe/debug/Check.h"
#include "vob/aoe/rendering/contexts/DebugMeshContext.h"
#include "vob/aoe/rendering/ProgramUtils.h"
#include "vob/aoe/rendering/RenderSceneConfigUtils.h"

#include "vob/misc/std/container_util.h"

#include <algorithm>
#include <array>
#include <cstddef>


namespace vob::aoegl
{
	namespace
	{
		glm::ivec2 computeScaledResolution(glm::ivec2 a_resolution, float a_scale)
		{
			return glm::ivec2{glm::round(glm::vec2{a_resolution} * a_scale)};
		}

		int32_t computeLightClusterCount(glm::ivec2 a_shadingResolution, RenderSceneConfig::Lighting const& a_config)
		{
			auto const lightClusterXYCount =
				(a_shadingResolution + a_config.clusterTileSize - 1) / a_config.clusterTileSize;
			return lightClusterXYCount.x * lightClusterXYCount.y * a_config.clusterZCount;
		}

		GpuBuffer createBuffer(GLsizeiptr a_size, void const* a_data, GLbitfield a_flags, GpuDeleteQueue& a_deleteQueue)
		{
			GraphicId id;
			glCreateBuffers(1, &id);
			glNamedBufferStorage(id, a_size, a_data, a_flags);
			return GpuBuffer{a_deleteQueue, id};
		}

		GpuTexture createTexture2d(glm::ivec2 a_resolution, GpuDeleteQueue& a_deleteQueue, auto const& a_setup)
		{
			GraphicId id;
			glCreateTextures(GL_TEXTURE_2D, 1, &id);
			a_setup(id, a_resolution);
			return GpuTexture{a_deleteQueue, id};
		}

		GpuFramebuffer createFramebuffer(GpuDeleteQueue& a_deleteQueue, auto const& a_setup)
		{
			GraphicId id;
			glCreateFramebuffers(1, &id);
			a_setup(id);
			return GpuFramebuffer{a_deleteQueue, id};
		}

		GpuVertexArray createVertexArray(GpuDeleteQueue& a_deleteQueue)
		{
			GraphicId id;
			glCreateVertexArrays(1, &id);
			return GpuVertexArray{a_deleteQueue, id};
		}

		GpuTexture createFullResColorTexture(glm::ivec2 a_shadingResolution, GpuDeleteQueue& a_deleteQueue)
		{
			return createTexture2d(
				a_shadingResolution,
				a_deleteQueue,
				[](GraphicId a_id, glm::ivec2 a_resolution)
				{
					glTextureStorage2D(a_id, 1, GL_RGB16F, a_resolution.x, a_resolution.y);
					glTextureParameteri(a_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTextureParameteri(a_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				});
		}

		template <std::size_t t_capacity>
		mistd::bounded_vector<GpuRenderTarget, t_capacity> createMipTargets(
			int32_t a_mipLevels, GraphicId a_texture, glm::ivec2 a_resolution, GpuDeleteQueue& a_deleteQueue)
		{
			auto const drawBuffers = std::array<GraphicEnum, 1>{GL_COLOR_ATTACHMENT0};

			auto mipTargets = mistd::bounded_vector<GpuRenderTarget, t_capacity>{};
			auto mipResolution = a_resolution;
			for (int32_t mipIndex = 0; mipIndex < a_mipLevels; ++mipIndex)
			{
				if (mipIndex > 0)
				{
					mipResolution = glm::max(mipResolution / 2, glm::ivec2{1});
				}

				auto& mipTarget = mipTargets.emplace_back();
				mipTarget.resolution = mipResolution;
				mipTarget.framebuffer = createFramebuffer(
					a_deleteQueue,
					[&](GraphicId a_id)
					{
						glNamedFramebufferTexture(a_id, GL_COLOR_ATTACHMENT0, a_texture, mipIndex);
						glNamedFramebufferDrawBuffers(a_id, mistd::isize(drawBuffers), drawBuffers.data());
						VOB_AOE_CHECK_TERMINATE_SLOW(
							glCheckNamedFramebufferStatus(a_id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
							"Incomplete framebuffer.");
					});
			}

			return mipTargets;
		}

		mistd::bounded_vector<GpuRenderSceneResources::BloomMip, k_bloomMipsCapacity> createBloomMips(
			glm::ivec2 a_shadingResolution, RenderSceneConfig::Bloom const& a_config, GpuDeleteQueue& a_deleteQueue)
		{
			auto const minMipWidth = computeBloomMinMipWidth(a_config);

			auto mips = mistd::bounded_vector<GpuRenderSceneResources::BloomMip, k_bloomMipsCapacity>{};
			auto resolution = computeScaledResolution(a_shadingResolution, a_config.scale);
			do
			{
				auto& mip = mips.emplace_back();
				mip.target.resolution = resolution;

				GraphicId colorTexture;
				glCreateTextures(GL_TEXTURE_2D, 1, &colorTexture);
				glTextureStorage2D(colorTexture, 1, GL_RGB16F, resolution.x, resolution.y);
				glTextureParameteri(colorTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTextureParameteri(colorTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTextureParameteri(colorTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTextureParameteri(colorTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				mip.colorTexture = GpuTexture{a_deleteQueue, colorTexture};

				GraphicId framebuffer;
				glCreateFramebuffers(1, &framebuffer);
				glNamedFramebufferTexture(framebuffer, GL_COLOR_ATTACHMENT0, colorTexture, 0);
				VOB_AOE_CHECK_TERMINATE_SLOW(
					glCheckNamedFramebufferStatus(framebuffer, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
					"Incomplete framebuffer.");
				mip.target.framebuffer = GpuFramebuffer{a_deleteQueue, framebuffer};

				resolution = glm::max(resolution / 2, glm::ivec2{1});
			} while (resolution.x >= minMipWidth && static_cast<int32_t>(mips.size()) < k_bloomMipsCapacity);

			return mips;
		}

		void updateSsaoParamsUbo(RenderSceneConfig::Ssao const& a_config, GpuRenderSceneResources& o_resources)
		{
			auto const ssaoParams = createUniformSsaoParams(a_config);
			glNamedBufferSubData(o_resources.ssaoParamsUbo, 0, sizeof(ssaoParams), &ssaoParams);
		}

		void updateSsrParamsUbo(RenderSceneConfig::Ssr const& a_config, GpuRenderSceneResources& o_resources)
		{
			auto const ssrParams = createUniformSsrParams(a_config);
			glNamedBufferSubData(o_resources.ssrParamsUbo, 0, sizeof(ssrParams), &ssrParams);
		}

		void updateBloomParamsUbo(RenderSceneConfig::Bloom const& a_config, GpuRenderSceneResources& o_resources)
		{
			auto const bloomParams = createUniformBloomParams(a_config, mistd::isize(o_resources.bloomMips));
			glNamedBufferSubData(o_resources.bloomParamsUbo, 0, sizeof(bloomParams), &bloomParams);
		}

		void updateTonemapParamsUbo(RenderSceneConfig::Tonemap const& a_config, GpuRenderSceneResources& o_resources)
		{
			auto const tonemapParams = createUniformTonemapParams(a_config);
			glNamedBufferSubData(o_resources.tonemapParamsUbo, 0, sizeof(tonemapParams), &tonemapParams);
		}

		void createInvariantResources(
			RenderSceneProgramSources const& a_programSources,
			GpuDeleteQueue& a_deleteQueue,
			GpuRenderSceneResources& o_resources)
		{
			o_resources.globalParamsUbo =
				createBuffer(sizeof(UniformGlobalParams), nullptr, GL_DYNAMIC_STORAGE_BIT, a_deleteQueue);
			o_resources.viewParamsUbo =
				createBuffer(sizeof(UniformViewParams), nullptr, GL_DYNAMIC_STORAGE_BIT, a_deleteQueue);
			o_resources.targetParamsUbo =
				createBuffer(sizeof(UniformTargetParams), nullptr, GL_DYNAMIC_STORAGE_BIT, a_deleteQueue);
			o_resources.lightViewParamsUbo =
				createBuffer(sizeof(UniformViewParams), nullptr, GL_DYNAMIC_STORAGE_BIT, a_deleteQueue);
			o_resources.lightingParamsUbo =
				createBuffer(sizeof(UniformLightingParams), nullptr, GL_DYNAMIC_STORAGE_BIT, a_deleteQueue);
			o_resources.shadowParamsUbo =
				createBuffer(sizeof(UniformShadowParams), nullptr, GL_DYNAMIC_STORAGE_BIT, a_deleteQueue);

			o_resources.ssaoParamsUbo =
				createBuffer(sizeof(UniformSsaoParams), nullptr, GL_DYNAMIC_STORAGE_BIT, a_deleteQueue);
			o_resources.ssrParamsUbo =
				createBuffer(sizeof(UniformSsrParams), nullptr, GL_DYNAMIC_STORAGE_BIT, a_deleteQueue);
			o_resources.bloomParamsUbo =
				createBuffer(sizeof(UniformBloomParams), nullptr, GL_DYNAMIC_STORAGE_BIT, a_deleteQueue);
			o_resources.tonemapParamsUbo =
				createBuffer(sizeof(UniformTonemapParams), nullptr, GL_DYNAMIC_STORAGE_BIT, a_deleteQueue);

			o_resources.debugParamsUbo =
				createBuffer(sizeof(UniformDebugParams), nullptr, GL_DYNAMIC_STORAGE_BIT, a_deleteQueue);

			o_resources.skyIrradianceSsbo =
				createBuffer(k_skyIrradianceCoefficientCount * sizeof(glm::vec4), nullptr, 0, a_deleteQueue);

			o_resources.lightClusteringProgram = GpuProgram{a_deleteQueue, createLightClusteringProgram()};
			o_resources.ssaoDepthProgram = GpuProgram{a_deleteQueue, createSsaoDepthProgram()};
			o_resources.ssaoProgram = GpuProgram{a_deleteQueue, createSsaoProgram()};
			o_resources.ssaoBlurProgram = GpuProgram{a_deleteQueue, createSsaoBlurProgram()};
			o_resources.ssrProgram = GpuProgram{a_deleteQueue, createSsrProgram(a_programSources.skyPartial)};
			o_resources.hiZReduceProgram = GpuProgram{a_deleteQueue, createHiZReduceProgram()};
			o_resources.ssrPrefilterProgram = GpuProgram{a_deleteQueue, createSsrPrefilterProgram()};
			o_resources.ssrDownsampleProgram = GpuProgram{a_deleteQueue, createSsrDownsampleProgram()};
			o_resources.opaqueCompositionProgram = GpuProgram{a_deleteQueue, createOpaqueCompositionProgram()};
			o_resources.skyBoxProgram = GpuProgram{a_deleteQueue, createSkyProgram(a_programSources.skyPartial)};
			o_resources.skyIrradianceProgram =
				GpuProgram{a_deleteQueue, createSkyIrradianceProgram(a_programSources.skyPartial)};
			o_resources.debugProgram = GpuProgram{a_deleteQueue, createDebugProgram()};
			o_resources.debugGeometryProgram = GpuProgram{a_deleteQueue, createDebugGeometryProgram()};
			o_resources.bloomDownsampleProgram =
				GpuProgram{a_deleteQueue, createQuadProgram(a_programSources.bloomDownsample)};
			o_resources.bloomUpsampleProgram =
				GpuProgram{a_deleteQueue, createQuadProgram(a_programSources.bloomUpsample)};
			o_resources.bloomCombineProgram =
				GpuProgram{a_deleteQueue, createQuadProgram(a_programSources.bloomCombine)};
			o_resources.tonemapProgram = GpuProgram{a_deleteQueue, createQuadProgram(a_programSources.tonemap)};
			o_resources.aaProgram = GpuProgram{a_deleteQueue, createQuadProgram(a_programSources.antiAliasing)};
			o_resources.presentProgram = GpuProgram{a_deleteQueue, createQuadProgram(a_programSources.present)};

			o_resources.postProcessVao = createVertexArray(a_deleteQueue);

			o_resources.debugGeometryVao = createVertexArray(a_deleteQueue);
			glBindVertexArray(o_resources.debugGeometryVao);
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);

			glEnableVertexAttribArray(0);
			glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
			glVertexAttribBinding(0, 0);

			glEnableVertexAttribArray(1);
			glVertexAttribFormat(1, 4, GL_FLOAT, GL_FALSE, 0);
			glVertexAttribBinding(1, 1);

			GraphicId debugGeometryVbo;
			glCreateBuffers(1, &debugGeometryVbo);
			o_resources.debugGeometryVbo = GpuBuffer{a_deleteQueue, debugGeometryVbo};
			glBindVertexBuffer(0, debugGeometryVbo, offsetof(WorldDebugVertex, position), sizeof(WorldDebugVertex));
			glBindVertexBuffer(1, debugGeometryVbo, offsetof(WorldDebugVertex, color), sizeof(WorldDebugVertex));

			GraphicId debugGeometryEbo;
			glCreateBuffers(1, &debugGeometryEbo);
			o_resources.debugGeometryEbo = GpuBuffer{a_deleteQueue, debugGeometryEbo};
			glVertexArrayElementBuffer(o_resources.debugGeometryVao, debugGeometryEbo);
		}

		void createSunShadowResources(
			RenderSceneConfig::Shadow const& a_config,
			GpuDeleteQueue& a_deleteQueue,
			GpuRenderSceneResources& o_resources)
		{
			GraphicId id;
			glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &id);
			glTextureStorage3D(
				id,
				1,
				GL_DEPTH_COMPONENT32F,
				a_config.sunResolution.x,
				a_config.sunResolution.y,
				mistd::isize(a_config.sunCascadeFarClips));
			glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
			glTextureParameterfv(id, GL_TEXTURE_BORDER_COLOR, borderColor);
			o_resources.sunShadowMapDepthTextureArray = GpuTexture{a_deleteQueue, id};

			o_resources.sunShadowMap.resolution = a_config.sunResolution;
			o_resources.sunShadowMap.framebuffer = createFramebuffer(
				a_deleteQueue,
				[](GraphicId a_id)
				{
					glNamedFramebufferDrawBuffer(a_id, GL_NONE);
					glNamedFramebufferReadBuffer(a_id, GL_NONE);
				});
		}

		GpuRenderSceneResources::SpotLightShadowMapTarget createSpotShadowMapTarget(
			glm::ivec2 a_resolution, GpuDeleteQueue& a_deleteQueue)
		{
			auto spotTarget = GpuRenderSceneResources::SpotLightShadowMapTarget{};
			spotTarget.target.resolution = a_resolution;

			GraphicId id;
			glCreateTextures(GL_TEXTURE_2D, 1, &id);
			glTextureStorage2D(id, 1, GL_DEPTH_COMPONENT32F, a_resolution.x, a_resolution.y);
			glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
			glTextureParameterfv(id, GL_TEXTURE_BORDER_COLOR, borderColor);
			spotTarget.depthTexture = GpuTexture{a_deleteQueue, id};

			spotTarget.target.framebuffer = createFramebuffer(
				a_deleteQueue,
				[&spotTarget](GraphicId a_id)
				{
					glNamedFramebufferTexture(a_id, GL_DEPTH_ATTACHMENT, spotTarget.depthTexture, 0);
					glNamedFramebufferDrawBuffer(a_id, GL_NONE);
					glNamedFramebufferReadBuffer(a_id, GL_NONE);
					VOB_AOE_CHECK_TERMINATE_SLOW(
						glCheckNamedFramebufferStatus(a_id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
						"Incomplete framebuffer.");
				});

			return spotTarget;
		}

		void createShadingResources(
			glm::ivec2 a_shadingResolution, GpuDeleteQueue& a_deleteQueue, GpuRenderSceneResources& o_resources)
		{
			o_resources.shadingResolution = a_shadingResolution;
			o_resources.depthTarget.resolution = a_shadingResolution;
			o_resources.directOpaqueTarget.resolution = a_shadingResolution;

			o_resources.opaqueGeometricNormalTexture = createTexture2d(
				a_shadingResolution,
				a_deleteQueue,
				[](GraphicId a_id, glm::ivec2 a_resolution)
				{
					glTextureStorage2D(a_id, 1, GL_RGB16F, a_resolution.x, a_resolution.y);
					glTextureParameteri(a_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTextureParameteri(a_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				});

			o_resources.opaqueDepthTexture = createTexture2d(
				a_shadingResolution,
				a_deleteQueue,
				[](GraphicId a_id, glm::ivec2 a_resolution)
				{
					glTextureStorage2D(a_id, 1, GL_DEPTH_COMPONENT32F, a_resolution.x, a_resolution.y);
					glTextureParameteri(a_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTextureParameteri(a_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				});

			o_resources.directOpaqueColorTexture = createTexture2d(
				a_shadingResolution,
				a_deleteQueue,
				[](GraphicId a_id, glm::ivec2 a_resolution)
				{
					glTextureStorage2D(a_id, 1, GL_RGB16F, a_resolution.x, a_resolution.y);
					glTextureParameteri(a_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTextureParameteri(a_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				});

			o_resources.opaqueNormalTexture = createTexture2d(
				a_shadingResolution,
				a_deleteQueue,
				[](GraphicId a_id, glm::ivec2 a_resolution)
				{
					glTextureStorage2D(a_id, 1, GL_RGB16F, a_resolution.x, a_resolution.y);
					glTextureParameteri(a_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTextureParameteri(a_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				});

			o_resources.opaqueSurfaceTexture = createTexture2d(
				a_shadingResolution,
				a_deleteQueue,
				[](GraphicId a_id, glm::ivec2 a_resolution)
				{
					glTextureStorage2D(a_id, 1, GL_RGBA8, a_resolution.x, a_resolution.y);
					glTextureParameteri(a_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTextureParameteri(a_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				});

			o_resources.finalColorTexture = createFullResColorTexture(a_shadingResolution, a_deleteQueue);
			o_resources.finalTarget.resolution = a_shadingResolution;

			for (auto& postProcessTarget : o_resources.postProcessTargets)
			{
				postProcessTarget.target.resolution = a_shadingResolution;
				postProcessTarget.colorTexture = createTexture2d(
					a_shadingResolution,
					a_deleteQueue,
					[](GraphicId a_id, glm::ivec2 a_resolution)
					{
						glTextureStorage2D(a_id, 1, GL_RGB8, a_resolution.x, a_resolution.y);
						glTextureParameteri(a_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTextureParameteri(a_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					});
			}

			auto const depthDrawFramebuffers = std::array<GraphicEnum, 1>{GL_COLOR_ATTACHMENT0};
			o_resources.depthTarget.framebuffer = createFramebuffer(
				a_deleteQueue,
				[&](GraphicId a_id)
				{
					glNamedFramebufferTexture(a_id, GL_COLOR_ATTACHMENT0, o_resources.opaqueGeometricNormalTexture, 0);
					glNamedFramebufferTexture(a_id, GL_DEPTH_ATTACHMENT, o_resources.opaqueDepthTexture, 0);
					glNamedFramebufferDrawBuffers(
						a_id, mistd::isize(depthDrawFramebuffers), depthDrawFramebuffers.data());
					VOB_AOE_CHECK_TERMINATE_SLOW(
						glCheckNamedFramebufferStatus(a_id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
						"Incomplete framebuffer.");
				});

			auto const directOpaqueDrawBuffers =
				std::array<GraphicEnum, 3>{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
			o_resources.directOpaqueTarget.framebuffer = createFramebuffer(
				a_deleteQueue,
				[&](GraphicId a_id)
				{
					glNamedFramebufferTexture(a_id, GL_COLOR_ATTACHMENT0, o_resources.directOpaqueColorTexture, 0);
					glNamedFramebufferTexture(a_id, GL_COLOR_ATTACHMENT1, o_resources.opaqueNormalTexture, 0);
					glNamedFramebufferTexture(a_id, GL_COLOR_ATTACHMENT2, o_resources.opaqueSurfaceTexture, 0);
					glNamedFramebufferTexture(a_id, GL_DEPTH_ATTACHMENT, o_resources.opaqueDepthTexture, 0);
					glNamedFramebufferDrawBuffers(
						a_id, mistd::isize(directOpaqueDrawBuffers), directOpaqueDrawBuffers.data());
					VOB_AOE_CHECK_TERMINATE_SLOW(
						glCheckNamedFramebufferStatus(a_id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
						"Incomplete framebuffer.");
				});

			auto const finalDrawBuffers = std::array<GraphicEnum, 1>{GL_COLOR_ATTACHMENT0};
			o_resources.finalTarget.framebuffer = createFramebuffer(
				a_deleteQueue,
				[&](GraphicId a_id)
				{
					glNamedFramebufferTexture(a_id, GL_COLOR_ATTACHMENT0, o_resources.finalColorTexture, 0);
					glNamedFramebufferTexture(a_id, GL_DEPTH_ATTACHMENT, o_resources.opaqueDepthTexture, 0);
					glNamedFramebufferDrawBuffers(a_id, mistd::isize(finalDrawBuffers), finalDrawBuffers.data());
					VOB_AOE_CHECK_TERMINATE_SLOW(
						glCheckNamedFramebufferStatus(a_id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
						"Incomplete framebuffer.");
				});

			auto const postProcessDrawBuffers = std::array<GraphicEnum, 1>{GL_COLOR_ATTACHMENT0};
			for (auto& postProcessTarget : o_resources.postProcessTargets)
			{
				postProcessTarget.target.framebuffer = createFramebuffer(
					a_deleteQueue,
					[&](GraphicId a_id)
					{
						glNamedFramebufferTexture(a_id, GL_COLOR_ATTACHMENT0, postProcessTarget.colorTexture, 0);
						glNamedFramebufferDrawBuffers(
							a_id, mistd::isize(postProcessDrawBuffers), postProcessDrawBuffers.data());
						VOB_AOE_CHECK_TERMINATE_SLOW(
							glCheckNamedFramebufferStatus(a_id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
							"Incomplete framebuffer.");
					});
			}
		}

		void createSsaoResources(
			glm::ivec2 a_ssaoResolution, GpuDeleteQueue& a_deleteQueue, GpuRenderSceneResources& o_resources)
		{
			o_resources.ssaoDepthTarget.resolution = a_ssaoResolution;
			o_resources.ssaoRawTarget.resolution = a_ssaoResolution;
			o_resources.ssaoTarget.resolution = a_ssaoResolution;

			o_resources.ssaoLinearDepthTexture = createTexture2d(
				a_ssaoResolution,
				a_deleteQueue,
				[](GraphicId a_id, glm::ivec2 a_resolution)
				{
					glTextureStorage2D(a_id, 1, GL_R32F, a_resolution.x, a_resolution.y);
					glTextureParameteri(a_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTextureParameteri(a_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				});

			o_resources.ssaoRawOcclusionTexture = createTexture2d(
				a_ssaoResolution,
				a_deleteQueue,
				[](GraphicId a_id, glm::ivec2 a_resolution)
				{
					glTextureStorage2D(a_id, 1, GL_R8, a_resolution.x, a_resolution.y);
					glTextureParameteri(a_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTextureParameteri(a_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				});

			o_resources.ambientOcclusionTexture = createTexture2d(
				a_ssaoResolution,
				a_deleteQueue,
				[](GraphicId a_id, glm::ivec2 a_resolution)
				{
					glTextureStorage2D(a_id, 1, GL_R8, a_resolution.x, a_resolution.y);
					glTextureParameteri(a_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTextureParameteri(a_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				});

			auto const litOpaqueDrawBuffers = std::array<GraphicEnum, 1>{GL_COLOR_ATTACHMENT0};

			auto const createSsaoFramebuffer = [&](GraphicId a_colorTexture)
			{
				return createFramebuffer(
					a_deleteQueue,
					[&](GraphicId a_id)
					{
						glNamedFramebufferTexture(a_id, GL_COLOR_ATTACHMENT0, a_colorTexture, 0);
						glNamedFramebufferDrawBuffers(
							a_id, mistd::isize(litOpaqueDrawBuffers), litOpaqueDrawBuffers.data());
						VOB_AOE_CHECK_TERMINATE_SLOW(
							glCheckNamedFramebufferStatus(a_id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
							"Incomplete framebuffer.");
					});
			};

			o_resources.ssaoDepthTarget.framebuffer = createSsaoFramebuffer(o_resources.ssaoLinearDepthTexture);
			o_resources.ssaoRawTarget.framebuffer = createSsaoFramebuffer(o_resources.ssaoRawOcclusionTexture);
			o_resources.ssaoTarget.framebuffer = createSsaoFramebuffer(o_resources.ambientOcclusionTexture);
		}

		void releaseSsaoResources(GpuRenderSceneResources& o_resources)
		{
			o_resources.ssaoLinearDepthTexture = GpuTexture{};
			o_resources.ssaoRawOcclusionTexture = GpuTexture{};
			o_resources.ambientOcclusionTexture = GpuTexture{};
			o_resources.ssaoDepthTarget = GpuRenderTarget{};
			o_resources.ssaoRawTarget = GpuRenderTarget{};
			o_resources.ssaoTarget = GpuRenderTarget{};
		}

		void createSsrResources(
			glm::ivec2 a_shadingResolution,
			glm::ivec2 a_ssrResolution,
			GpuDeleteQueue& a_deleteQueue,
			GpuRenderSceneResources& o_resources)
		{
			auto const hiZMipLevels = std::min(computeFullMipChainLength(a_shadingResolution), k_hiZMipsCapacity);

			o_resources.hiZDepthTexture = createTexture2d(
				a_shadingResolution,
				a_deleteQueue,
				[hiZMipLevels](GraphicId a_id, glm::ivec2 a_resolution)
				{
					glTextureStorage2D(a_id, hiZMipLevels, GL_RG32F, a_resolution.x, a_resolution.y);
					glTextureParameteri(a_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
					glTextureParameteri(a_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTextureParameteri(a_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTextureParameteri(a_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				});

			o_resources.hiZMipTargets = createMipTargets<k_hiZMipsCapacity>(
				hiZMipLevels, o_resources.hiZDepthTexture, a_shadingResolution, a_deleteQueue);

			o_resources.ssrRawTarget.resolution = a_ssrResolution;

			auto const ssrMipLevels = std::min(computeFullMipChainLength(a_ssrResolution), k_ssrMipsCapacity);

			o_resources.ssrColorTexture = createTexture2d(
				a_ssrResolution,
				a_deleteQueue,
				[ssrMipLevels](GraphicId a_id, glm::ivec2 a_resolution)
				{
					glTextureStorage2D(a_id, ssrMipLevels, GL_RGBA16F, a_resolution.x, a_resolution.y);
					glTextureParameteri(a_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTextureParameteri(a_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				});

			o_resources.ssrRawColorTexture = createTexture2d(
				a_ssrResolution,
				a_deleteQueue,
				[](GraphicId a_id, glm::ivec2 a_resolution)
				{
					glTextureStorage2D(a_id, 1, GL_RGBA16F, a_resolution.x, a_resolution.y);
					glTextureParameteri(a_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTextureParameteri(a_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTextureParameteri(a_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTextureParameteri(a_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				});

			auto const ssrDrawBuffers = std::array<GraphicEnum, 1>{GL_COLOR_ATTACHMENT0};
			o_resources.ssrRawTarget.framebuffer = createFramebuffer(
				a_deleteQueue,
				[&](GraphicId a_id)
				{
					glNamedFramebufferTexture(a_id, GL_COLOR_ATTACHMENT0, o_resources.ssrRawColorTexture, 0);
					glNamedFramebufferDrawBuffers(a_id, mistd::isize(ssrDrawBuffers), ssrDrawBuffers.data());
				});

			o_resources.ssrMipTargets = createMipTargets<k_ssrMipsCapacity>(
				ssrMipLevels, o_resources.ssrColorTexture, a_ssrResolution, a_deleteQueue);
		}

		void releaseSsrResources(GpuRenderSceneResources& o_resources)
		{
			o_resources.hiZDepthTexture = GpuTexture{};
			o_resources.hiZMipTargets.clear();
			o_resources.ssrColorTexture = GpuTexture{};
			o_resources.ssrRawColorTexture = GpuTexture{};
			o_resources.ssrRawTarget = GpuRenderTarget{};
			o_resources.ssrMipTargets.clear();
		}

		void createBloomResources(
			glm::ivec2 a_shadingResolution,
			RenderSceneConfig::Bloom const& a_config,
			GpuDeleteQueue& a_deleteQueue,
			GpuRenderSceneResources& o_resources)
		{
			o_resources.bloomMips = createBloomMips(a_shadingResolution, a_config, a_deleteQueue);
			updateBloomParamsUbo(a_config, o_resources);

			o_resources.bloomCombinedColorTexture = createFullResColorTexture(a_shadingResolution, a_deleteQueue);
			o_resources.bloomCombinedTarget.resolution = a_shadingResolution;
			o_resources.bloomCombinedTarget.framebuffer = createFramebuffer(
				a_deleteQueue,
				[&](GraphicId a_id)
				{
					glNamedFramebufferTexture(a_id, GL_COLOR_ATTACHMENT0, o_resources.bloomCombinedColorTexture, 0);
					VOB_AOE_CHECK_TERMINATE_SLOW(
						glCheckNamedFramebufferStatus(a_id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
						"Incomplete framebuffer.");
				});
		}

		void releaseBloomResources(GpuRenderSceneResources& o_resources)
		{
			o_resources.bloomMips.clear();
			o_resources.bloomCombinedColorTexture = GpuTexture{};
			o_resources.bloomCombinedTarget = GpuRenderTarget{};
		}

		void createLightsResources(
			RenderSceneConfig::Lighting const& a_config,
			GpuDeleteQueue& a_deleteQueue,
			GpuRenderSceneResources& o_resources)
		{
			o_resources.lightsSsbo =
				createBuffer(a_config.maxLightCount * sizeof(GpuLight), nullptr, GL_DYNAMIC_STORAGE_BIT, a_deleteQueue);
		}

		void createLightClusterResources(
			glm::ivec2 a_shadingResolution,
			RenderSceneConfig::Lighting const& a_config,
			GpuDeleteQueue& a_deleteQueue,
			GpuRenderSceneResources& o_resources)
		{
			auto const lightClusterCount = computeLightClusterCount(a_shadingResolution, a_config);

			o_resources.lightClusterSizesSsbo =
				createBuffer(lightClusterCount * sizeof(int8_t), nullptr, 0, a_deleteQueue);
			o_resources.lightClusterIndicesSsbo =
				createBuffer(a_config.clusterCapacity * lightClusterCount * sizeof(int16_t), nullptr, 0, a_deleteQueue);
		}
	}

	GpuRenderSceneResources createGpuRenderSceneResources(
		RenderSceneConfig const& a_config,
		glm::ivec2 const a_windowSize,
		RenderSceneProgramSources const& a_programSources,
		GpuDeleteQueue& a_deleteQueue)
	{
		auto resources = GpuRenderSceneResources{};

		auto const shadingResolution = computeScaledResolution(a_windowSize, a_config.renderScale);

		createInvariantResources(a_programSources, a_deleteQueue, resources);
		updateSsaoParamsUbo(a_config.ssao, resources);
		updateSsrParamsUbo(a_config.ssr, resources);
		updateTonemapParamsUbo(a_config.tonemap, resources);
		createSunShadowResources(a_config.shadow, a_deleteQueue, resources);
		auto const spotShadowMapCount = computeSpotShadowMapCount(a_config.shadow);
		for (int32_t i = 0; i < spotShadowMapCount; ++i)
		{
			resources.spotLightShadowMapTargets.push_back(
				createSpotShadowMapTarget(computeSpotShadowMapResolution(a_config.shadow, i), a_deleteQueue));
		}
		createShadingResources(shadingResolution, a_deleteQueue, resources);
		if (a_config.ssao.isEnabled)
		{
			createSsaoResources(
				computeScaledResolution(shadingResolution, a_config.ssao.scale), a_deleteQueue, resources);
		}
		if (a_config.ssr.isEnabled)
		{
			createSsrResources(
				shadingResolution,
				computeScaledResolution(shadingResolution, a_config.ssr.scale),
				a_deleteQueue,
				resources);
		}
		if (a_config.bloom.isEnabled)
		{
			createBloomResources(shadingResolution, a_config.bloom, a_deleteQueue, resources);
		}
		createLightsResources(a_config.lighting, a_deleteQueue, resources);
		createLightClusterResources(shadingResolution, a_config.lighting, a_deleteQueue, resources);

		return resources;
	}

	void updateGpuRenderSceneResources(
		RenderSceneConfig const& a_appliedConfig,
		glm::ivec2 const a_appliedWindowSize,
		RenderSceneConfig const& a_config,
		glm::ivec2 const a_windowSize,
		GpuDeleteQueue& a_deleteQueue,
		GpuRenderSceneResources& o_resources)
	{
		auto const shadingResolution = computeScaledResolution(a_windowSize, a_config.renderScale);
		auto const appliedShadingResolution = computeScaledResolution(a_appliedWindowSize, a_appliedConfig.renderScale);

		if (a_config.shadow.sunResolution != a_appliedConfig.shadow.sunResolution
			|| a_config.shadow.sunCascadeFarClips.size() != a_appliedConfig.shadow.sunCascadeFarClips.size())
		{
			createSunShadowResources(a_config.shadow, a_deleteQueue, o_resources);
		}

		auto const spotShadowMapCount = computeSpotShadowMapCount(a_config.shadow);
		auto const appliedSpotShadowMapCount = computeSpotShadowMapCount(a_appliedConfig.shadow);
		for (int32_t i = spotShadowMapCount; i < appliedSpotShadowMapCount; ++i)
		{
			o_resources.spotLightShadowMapTargets.pop_back();
		}
		for (int32_t i = 0; i < spotShadowMapCount; ++i)
		{
			auto const resolution = computeSpotShadowMapResolution(a_config.shadow, i);
			if (i >= appliedSpotShadowMapCount)
			{
				o_resources.spotLightShadowMapTargets.push_back(createSpotShadowMapTarget(resolution, a_deleteQueue));
			}
			else if (resolution != computeSpotShadowMapResolution(a_appliedConfig.shadow, i))
			{
				o_resources.spotLightShadowMapTargets[i] = createSpotShadowMapTarget(resolution, a_deleteQueue);
			}
		}

		if (shadingResolution != appliedShadingResolution)
		{
			createShadingResources(shadingResolution, a_deleteQueue, o_resources);
		}

		auto const ssaoResolution = computeScaledResolution(shadingResolution, a_config.ssao.scale);
		auto const appliedSsaoResolution =
			computeScaledResolution(appliedShadingResolution, a_appliedConfig.ssao.scale);
		if (a_config.ssao.isEnabled != a_appliedConfig.ssao.isEnabled || ssaoResolution != appliedSsaoResolution)
		{
			if (a_config.ssao.isEnabled)
			{
				createSsaoResources(ssaoResolution, a_deleteQueue, o_resources);
			}
			else
			{
				releaseSsaoResources(o_resources);
			}
		}

		if (a_config.ssao != a_appliedConfig.ssao)
		{
			updateSsaoParamsUbo(a_config.ssao, o_resources);
		}

		auto const ssrResolution = computeScaledResolution(shadingResolution, a_config.ssr.scale);
		auto const appliedSsrResolution = computeScaledResolution(appliedShadingResolution, a_appliedConfig.ssr.scale);
		if (a_config.ssr.isEnabled != a_appliedConfig.ssr.isEnabled || shadingResolution != appliedShadingResolution
			|| ssrResolution != appliedSsrResolution)
		{
			if (a_config.ssr.isEnabled)
			{
				createSsrResources(shadingResolution, ssrResolution, a_deleteQueue, o_resources);
			}
			else
			{
				releaseSsrResources(o_resources);
			}
		}

		if (a_config.ssr != a_appliedConfig.ssr)
		{
			updateSsrParamsUbo(a_config.ssr, o_resources);
		}

		auto const bloomResolution = computeScaledResolution(shadingResolution, a_config.bloom.scale);
		auto const appliedBloomResolution =
			computeScaledResolution(appliedShadingResolution, a_appliedConfig.bloom.scale);
		if (a_config.bloom.isEnabled != a_appliedConfig.bloom.isEnabled || shadingResolution != appliedShadingResolution
			|| bloomResolution != appliedBloomResolution
			|| computeBloomMinMipWidth(a_config.bloom) != computeBloomMinMipWidth(a_appliedConfig.bloom))
		{
			if (a_config.bloom.isEnabled)
			{
				createBloomResources(shadingResolution, a_config.bloom, a_deleteQueue, o_resources);
			}
			else
			{
				releaseBloomResources(o_resources);
			}
		}
		else if (a_config.bloom != a_appliedConfig.bloom)
		{
			updateBloomParamsUbo(a_config.bloom, o_resources);
		}

		if (a_config.lighting.maxLightCount != a_appliedConfig.lighting.maxLightCount)
		{
			createLightsResources(a_config.lighting, a_deleteQueue, o_resources);
		}

		auto const lightClusterCount = computeLightClusterCount(shadingResolution, a_config.lighting);
		auto const appliedLightClusterCount =
			computeLightClusterCount(appliedShadingResolution, a_appliedConfig.lighting);
		if (lightClusterCount != appliedLightClusterCount
			|| a_config.lighting.clusterCapacity != a_appliedConfig.lighting.clusterCapacity)
		{
			createLightClusterResources(shadingResolution, a_config.lighting, a_deleteQueue, o_resources);
		}

		if (a_config.tonemap != a_appliedConfig.tonemap)
		{
			updateTonemapParamsUbo(a_config.tonemap, o_resources);
		}
	}
}
