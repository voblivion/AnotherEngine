#include "DefaultWorld.h"

#include <vob/aoe/engine/EcsWorld.h>
#include <vob/aoe/engine/MultiWorld.h>

#include <vob/aoe/exchange/EcsExchangeData.h>

#include <vob/aoe/spacetime/TimeSystem.h>
#include <vob/aoe/spacetime/InterpolationTimeComponent.h>
#include <vob/aoe/spacetime/TransformInterpolationSystem.h>
#include <vob/aoe/spacetime/InterpolatedTransform.h>
#include <vob/aoe/spacetime/InterpolationContext.h>
#include <vob/aoe/spacetime/InterpolationExchangeContext.h>
#include <vob/aoe/spacetime/FixedRateLimitingSystem.h>
#include <vob/aoe/spacetime/FixedRateTimeSystem.h>
#include <vob/aoe/spacetime/SoftFollowComponent.h>
#include <vob/aoe/spacetime/SoftFollowSystem.h>
#include <vob/aoe/window/PollEventsSystem.h>
#include <vob/aoe/input/InputBindingSystem.h>
#include <vob/aoe/input/WindowInputBindingSystem.h>
#include <vob/aoe/debug/GhostControllerSystem.h>
#include <vob/aoe/physics/CarControllerSystem.h>
#include <vob/aoe/physics/CollisionSystem.h>
#include <vob/aoe/physics/DebugRenderCollidersSystem.h>
#include <vob/aoe/physics/CarMaterialsComponent.h>
#include <vob/aoe/physics/CarMaterialsSystem.h>
#include <vob/aoe/window/SwapBuffersSystem.h>
#include <vob/aoe/rendering/BindSceneFramebufferSystem.h>
#include <vob/aoe/rendering/RenderDebugMeshSystem.h>
#include <vob/aoe/rendering/RenderMeshSystem.h>
#include <vob/aoe/rendering/BindWindowFramebufferSystem.h>
#include <vob/aoe/rendering/DebugRenderLightsSystem.h>
#include <vob/aoe/rendering/RenderSceneSystem.h>
#include <vob/aoe/rendering/StaticMesh.h>
#include <vob/aoe/rendering/StaticMeshComponent.h>
#include <vob/aoe/rendering/PrepareImGuiFrameSystem.h>
#include <vob/aoe/rendering/MeshRenderingContext.h>
#include <vob/aoe/rendering/LightComponent.h>
#include <vob/aoe/rendering/RenderImGuiFrameSystem.h>
#include <vob/aoe/rendering/StaticMeshLoader.h>

#include <vob/misc/type/registry.h>
#include <vob/misc/type/factory.h>
#include <vob/misc/type/applicator.h>
#include <vob/misc/visitor/accept.h>

#include <vob/aoe/data/filesystem_indexer.h>
#include <vob/aoe/data/multi_database.h>
#include <vob/aoe/data/filesystem_database.h>
#include <vob/aoe/data/single_file_loader.h>
#include <vob/aoe/data/json_file_loader.h>
#include <vob/aoe/data/string_loader.h>
#include <vob/aoe/data/filesystem_visitor_context.h>
#include <vob/aoe/rendering/ProgramData.h>
#include <vob/aoe/data/filesystem_util.h>
#include <vob/aoe/rendering/ProgramUtils.h>
#include <vob/aoe/input/GameInputUtils.h>
#include <vob/aoe/input/InputBindingUtils.h>

#include <entt/entt.hpp>

#include <imgui.h>

#include <random>
#include <vector>
#include <unordered_map>

namespace vob::misvi
{
	template <typename TData>
	bool accept(
		pmr::json_reader<aoedt::filesystem_visitor_context>& a_visitor
		, std::shared_ptr<TData const>& a_data)
	{
		std::string rawPath;
		a_visitor.visit(rawPath);

		auto const& context = a_visitor.get_context();
		auto const path = aoedt::filesystem_util::normalize(rawPath, context.get_base_path());
		auto const& indexer = context.get_indexer();
		a_data = context.get_multi_database().find<TData>(indexer.get_runtime_id(path));
		return true;
	}
}

/*  */

namespace vob::aoein {

	struct PresentationInputContext
	{
		std::vector<float> values;
	};

	struct PresentationInputBindingContext
	{
		std::vector<std::pair<GameInputValueId, GameInputValueId>> inputValueIds;
		std::unordered_map<GameInputEventId, GameInputEventId> inputEventIds;
	};
}

namespace vob
{
	struct SimulationExchangeContext
	{
		std::shared_ptr<aoexc::EcsExchangeData> data;
	};

	struct PresentationExchangeContext
	{
		std::shared_ptr<aoexc::EcsExchangeData> data;
	};

	struct SimulationDebugMeshContext : public aoegl::DebugMeshContext
	{

	};

	class PresentationExportSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
		{

		}

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
		{
			entt::registry excRegistry;
			aoexc::EventPool excEventPool;

			auto& presentationInputCtx = excRegistry.ctx().emplace<aoein::PresentationInputContext>();
			auto& gameInputCtx = m_gameInputContext.get(a_wdap);
			for (auto& inputValue : gameInputCtx.getValues())
			{
				presentationInputCtx.values.push_back(inputValue);
			}

			std::vector<aoein::GameInputEventId> inputEvents;
			for (auto inputEvent : gameInputCtx.getEvents())
			{
				inputEvents.push_back(inputEvent);
			}
			excEventPool.addEvents(std::move(inputEvents));

			m_presentationExchangeContext.get(a_wdap).data->store(std::move(excRegistry), std::move(excEventPool));
		}

	private:
		aoeng::EcsWorldContextRef<PresentationExchangeContext> m_presentationExchangeContext;
		aoeng::EcsWorldContextRef<aoein::GameInputContext> m_gameInputContext;
	};

	class PresentationImportSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
		{

		}

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
		{
			auto [excRegistry, excEventPool] = m_presentationExchangeContext.get(a_wdap).data->load();
			if (!excRegistry.ctx().contains<aoein::PresentationInputContext>())
			{
				return;
			}

			auto const& presentationInputBindingCtx = m_presentationInputBindingContext.get(a_wdap);
			auto const& presentationInputValues = excRegistry.ctx().get<aoein::PresentationInputContext>().values;
			auto& gameInputCtx = m_gameInputContext.get(a_wdap);
			for (auto [presInputValueId, simInputValueId] : presentationInputBindingCtx.inputValueIds)
			{
				gameInputCtx.setValue(simInputValueId, presentationInputValues[presInputValueId]);
			}

			gameInputCtx.flushEvents();
			std::vector<aoein::GameInputEventId> presInputEvents;
			excEventPool.pollEvents(presInputEvents);
			for (auto presInputEventId : presInputEvents)
			{
				if (presentationInputBindingCtx.inputEventIds.contains(presInputEventId))
				{
					gameInputCtx.addEvent(presentationInputBindingCtx.inputEventIds.at(presInputEventId));
				}
			}
		}

	private:
		aoeng::EcsWorldContextRef<PresentationExchangeContext> m_presentationExchangeContext;
		aoeng::EcsWorldContextRef<aoein::PresentationInputBindingContext> m_presentationInputBindingContext;
		aoeng::EcsWorldContextRef<aoein::GameInputContext> m_gameInputContext;
	};

	class SimulationExportSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
		{

		}

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
		{
			entt::registry excRegistry;
			aoexc::EventPool excEventPool;

			// Car Colliders
			for (auto [entity, position, rotation, carCollider, carController] : m_carEntities.get(a_wdap).each())
			{
				assert(entity == excRegistry.create(entity));
				excRegistry.emplace<aoest::Position>(entity, position);
				excRegistry.emplace<aoest::Rotation>(entity, rotation);
				excRegistry.emplace<aoeph::CarCollider>(entity, carCollider);
				excRegistry.emplace<aoeph::CarControllerComponent>(entity, carController);
			}

			// Interpolation
			excRegistry.ctx().emplace<aoest::TimeContext>(m_timeContext.get(a_wdap));
			excRegistry.ctx().emplace<aoest::InterpolationExchangeContext>(std::chrono::high_resolution_clock::now());

			// Debug Mesh
			auto& excDebugMeshContext = excRegistry.ctx().emplace<aoegl::DebugMeshContext>();
			std::swap(excDebugMeshContext.vertices, m_debugMeshContext.get(a_wdap).vertices);
			std::swap(excDebugMeshContext.lines, m_debugMeshContext.get(a_wdap).lines);

			m_simulationExchangeContext.get(a_wdap).data->store(std::move(excRegistry), std::move(excEventPool));
		}

	private:
		aoeng::EcsWorldContextRef<aoest::TimeContext> m_timeContext;
		aoeng::EcsWorldContextRef<aoegl::DebugMeshContext> m_debugMeshContext;
		aoeng::EcsWorldContextRef<SimulationExchangeContext> m_simulationExchangeContext;
		aoeng::EcsWorldViewRef<aoest::Position, aoest::Rotation, aoeph::CarCollider, aoeph::CarControllerComponent> m_carEntities;
	};

	class SimulationImportSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
		{

		}

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
		{
			auto [excRegistry, excEventPool] = m_simulationExchangeContext.get(a_wdap).data->load();
			if (!excRegistry.ctx().contains<aoest::InterpolationExchangeContext>())
			{
				return;
			}

			// Interpolation
			auto sourceTime = m_timeContext.get(a_wdap).tickStartTime;
			auto targetTime = excRegistry.ctx().get<aoest::InterpolationExchangeContext>().targetTime + m_interpolationContext.get(a_wdap).offset;

			// Car Colliders
			for (auto [entity, excPosition, excRotation, excCarCollider, excCarController]
				: excRegistry.view<aoest::Position, aoest::Rotation, aoeph::CarCollider, aoeph::CarControllerComponent>().each())
			{
				auto [position, rotation, interpolatedPosition, interpolatedRotation, interpolationComponent, carCollider, carController] = m_carEntities.get(a_wdap).get(entity);
				interpolatedPosition.source = position;
				interpolatedPosition.target = excPosition;
				interpolatedRotation.source = rotation;
				interpolatedRotation.target = excRotation;
				carCollider = excCarCollider;
				carController = excCarController;
				interpolationComponent.sourceTime = sourceTime;
				interpolationComponent.targetTime = targetTime;
				interpolationComponent.times[interpolationComponent.endIndex] = excRegistry.ctx().get<aoest::InterpolationExchangeContext>().targetTime;
				interpolationComponent.endIndex = (interpolationComponent.endIndex + 1) % interpolationComponent.times.size();
				interpolatedPosition.positions[interpolatedPosition.endIndex] = excPosition;
				interpolatedPosition.endIndex = interpolationComponent.endIndex;
				interpolatedRotation.rotations[interpolatedRotation.endIndex] = excRotation;
				interpolatedRotation.endIndex = interpolationComponent.endIndex;
			}

			// Debug Mesh
			auto& excDebugMeshContext = excRegistry.ctx().get<aoegl::DebugMeshContext>();
			auto& simDebugMeshContext = m_simDebugMeshContext.get(a_wdap);
			std::swap(excDebugMeshContext.vertices, simDebugMeshContext.vertices);
			std::swap(excDebugMeshContext.lines, simDebugMeshContext.lines);
		}

	private:
		aoeng::EcsWorldContextRef<SimulationExchangeContext> m_simulationExchangeContext;
		aoeng::EcsWorldContextRef<aoest::TimeContext> m_timeContext;
		aoeng::EcsWorldContextRef<aoest::InterpolationContext> m_interpolationContext;
		aoeng::EcsWorldContextRef<SimulationDebugMeshContext> m_simDebugMeshContext;
		aoeng::EcsWorldViewRef<aoest::Position, aoest::Rotation, aoest::InterpolatedPosition, aoest::InterpolatedRotation, aoest::InterpolationTimeComponent, aoeph::CarCollider, aoeph::CarControllerComponent> m_carEntities;
	};

	class SimulationDebugRenderSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
		{

		}

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
		{
			auto& debugMeshContext = m_debugMeshContext.get(a_wdap);
			auto const& simDebugMeshContext = m_simDebugMeshContext.get(a_wdap);
			auto const vertexOffset = static_cast<aoegl::GraphicIndex>(debugMeshContext.vertices.size());
			for (auto const& simVertex : simDebugMeshContext.vertices)
			{
				debugMeshContext.vertices.push_back(simVertex);
			}
			for (auto const& simLine : simDebugMeshContext.lines)
			{
				debugMeshContext.lines.emplace_back(simLine.v0 + vertexOffset, simLine.v1 + vertexOffset);
			}
		}

	private:
		aoeng::EcsWorldContextRef<aoegl::DebugMeshContext> m_debugMeshContext;
		aoeng::EcsWorldContextRef<SimulationDebugMeshContext> m_simDebugMeshContext;
	};

	template <typename TSystem>
	int32_t addSystem(std::vector<std::shared_ptr<aoeng::IEcsSystem>>& a_ecsSystems)
	{
		auto const id = static_cast<int32_t>(a_ecsSystems.size());
		a_ecsSystems.push_back(std::make_shared<aoeng::BasicEcsSystem<TSystem>>());
		return id;
	}

	std::pair<std::vector<std::shared_ptr<aoeng::IEcsSystem>>, aoeng::EcsSchedule> createPresentationEcsSystems()
	{
		std::vector<std::shared_ptr<aoeng::IEcsSystem>> ecsSystems;
		auto timeId = addSystem<aoest::TimeSystem>(ecsSystems);
		auto softFollowId = addSystem<aoest::SoftFollowSystem>(ecsSystems);
		auto pollEventsId = addSystem<aoewi::PollEventsSystem>(ecsSystems);
		auto prepareImGuiFrameId = addSystem<aoegl::PrepareImGuiFrameSystem>(ecsSystems);
		auto windowInputBindingId = addSystem<aoein::WindowInputBindingSystem>(ecsSystems);
		auto ghostControllerId = addSystem<aoedb::GhostControllerSystem>(ecsSystems);
		auto carMaterialsId = addSystem<aoeph::CarMaterialsSystem>(ecsSystems);

		auto renderSceneId = addSystem<aoegl::RenderSceneSystem>(ecsSystems);

		auto debugRenderLightsId = addSystem<aoegl::DebugRenderLightsSystem>(ecsSystems);
		auto renderDebugMeshId = addSystem<aoegl::RenderDebugMeshSystem>(ecsSystems);
		auto renderImGuiFrameId = addSystem<aoegl::RenderImGuiFrameSystem>(ecsSystems);

		auto swapBuffersId = addSystem<aoewi::SwapBuffersSystem>(ecsSystems);

		auto simulationImportId = addSystem<SimulationImportSystem>(ecsSystems);
		auto transformInterpolationId = addSystem<aoest::TransformInterpolationSystem>(ecsSystems);
		auto simDebugRenderId = addSystem<SimulationDebugRenderSystem>(ecsSystems);
		auto presExportId = addSystem<PresentationExportSystem>(ecsSystems);

		aoeng::EcsSchedule ecsSchedule({ { "Presentation", {
			{ timeId },
			{ simulationImportId },
			{ transformInterpolationId },
			{ carMaterialsId },
			{ simDebugRenderId },
			{ softFollowId },
			{ pollEventsId },
			{ prepareImGuiFrameId },
			{ windowInputBindingId },
			{ ghostControllerId },
			{ renderSceneId },
			{ debugRenderLightsId },
			{ renderDebugMeshId },
			{ renderImGuiFrameId },
			{ swapBuffersId },
			{ presExportId },
		} } });

		return { ecsSystems, ecsSchedule };
	}

	std::pair<std::vector<std::shared_ptr<aoeng::IEcsSystem>>, aoeng::EcsSchedule> createSimulationEcsSystems()
	{
		std::vector<std::shared_ptr<aoeng::IEcsSystem>> ecsSystems;
		auto timeId = addSystem<aoest::TimeSystem>(ecsSystems);
		auto presImportId = addSystem<PresentationImportSystem>(ecsSystems);
		auto fixedRateTimeId = addSystem<aoest::FixedRateTimeSystem>(ecsSystems);
		auto carControllerId = addSystem<aoeph::CarControllerSystem>(ecsSystems);
		auto collisionId = addSystem<aoeph::CollisionSystem>(ecsSystems);
		auto debugRenderCollidersId = addSystem<aoeph::DebugRenderCollidersSystem>(ecsSystems);
		auto simExportId = addSystem<SimulationExportSystem>(ecsSystems);
		auto fixedRateLimitingId = addSystem<aoest::FixedRateLimitingSystem>(ecsSystems);

		aoeng::EcsSchedule ecsSchedule({ { "Simulation", {
			{ timeId },
			{ presImportId },
			{ fixedRateTimeId },
			{ carControllerId },
			{ collisionId },
			{ debugRenderCollidersId },
			{ simExportId },
			{ fixedRateLimitingId },
		} } });

		/*
		* -> bindings/inputs
		* <- debug mesh
		* <- transform
		* 
		*/

		return { ecsSystems, ecsSchedule };
	}

#pragma optimize("", off)
	std::pair<entt::registry, entt::registry> createSimulationEcsRegistries(aoewi::IWindow& a_window)
	{
		// Prepare database
		misty::pmr::registry typeRegistry;
		misty::pmr::factory factory{ typeRegistry };
		aoedt::filesystem_indexer filesystemIndexer;
		aoedt::multi_database multiDatabase;
		misvi::pmr::applicator<false, misvi::pmr::json_reader<aoedt::filesystem_visitor_context>> jsonLoadApplicator;
		aoedt::filesystem_visitor_context_factory contextFactory{ factory, filesystemIndexer, multiDatabase };

		aoedt::filesystem_database<aoedt::single_file_loader<aoedt::string_loader>> stringDatabase{ filesystemIndexer };
		aoedt::filesystem_database<aoedt::json_file_loader<aoegl::ProgramData, aoedt::filesystem_visitor_context_factory>> shaderProgramDatabase{
			filesystemIndexer, jsonLoadApplicator, contextFactory };
		aoedt::filesystem_database<aoegl::StaticMeshLoader> staticMeshDatabase{ filesystemIndexer };
		multiDatabase.register_database(stringDatabase);
		multiDatabase.register_database(shaderProgramDatabase);
		multiDatabase.register_database(staticMeshDatabase);

		auto simExchangeData = std::make_shared<aoexc::EcsExchangeData>();
		auto presExchangeData = std::make_shared<aoexc::EcsExchangeData>();

		auto const currentTime = std::chrono::high_resolution_clock::now();

		// Prepare contexts
		entt::registry preRegistry;
		preRegistry.ctx().emplace<aoest::TimeContext>();
		preRegistry.ctx().emplace<SimulationExchangeContext>(simExchangeData);
		preRegistry.ctx().emplace<aoest::InterpolationContext>();
		preRegistry.ctx().emplace<aoewi::WindowInputContext>();
		preRegistry.ctx().emplace<aoewi::WindowContext>(a_window);
		preRegistry.ctx().emplace<aoegl::DebugMeshContext>();
		auto& meshRenderingCtx = preRegistry.ctx().emplace<aoegl::MeshRenderingContext>();
		{
			auto const clusterShaderSource = stringDatabase.find(filesystemIndexer.get_runtime_id("data/new/shaders/cluster.comp"));
			meshRenderingCtx.lightClusteringProgram = aoegl::createComputeProgram(*clusterShaderSource);
			glGenBuffers(1, &meshRenderingCtx.globalClusteringDataUbo);
			glBindBuffer(GL_UNIFORM_BUFFER, meshRenderingCtx.globalClusteringDataUbo);
			glNamedBufferStorage(
				meshRenderingCtx.globalClusteringDataUbo,
				sizeof(aoegl::GlobalLightClusteringData),
				nullptr,
				GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

			glGenBuffers(1, &meshRenderingCtx.lightsSsbo);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, meshRenderingCtx.lightsSsbo);
			glNamedBufferStorage(
				meshRenderingCtx.lightsSsbo,
				meshRenderingCtx.maxLightCount * sizeof(aoegl::CulledLight),
				nullptr,
				GL_DYNAMIC_STORAGE_BIT);
			
			auto const windowSize = a_window.getSize();
			auto const clusterCountX = static_cast<int32_t>(std::ceil(static_cast<float>(windowSize.x) / 16));
			auto const clusterCountY = static_cast<int32_t>(std::ceil(static_cast<float>(windowSize.y) / 16));
			auto const clusterCountZ = 24;
			meshRenderingCtx.clusterCount = clusterCountX * clusterCountY * clusterCountZ;
			auto const maxClusterLightCount = 64;
			
			glGenBuffers(1, &meshRenderingCtx.clustersLightCountSsbo);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, meshRenderingCtx.clustersLightCountSsbo);
			glNamedBufferStorage(
				meshRenderingCtx.clustersLightCountSsbo,
				meshRenderingCtx.clusterCount * sizeof(int32_t),
				nullptr,
				GL_DYNAMIC_STORAGE_BIT);

			glGenBuffers(1, &meshRenderingCtx.clustersLightIndicesSsbo);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, meshRenderingCtx.clustersLightIndicesSsbo);
			glNamedBufferStorage(
				meshRenderingCtx.clustersLightIndicesSsbo,
				meshRenderingCtx.clusterCount * maxClusterLightCount * sizeof(int32_t),
				nullptr,
				GL_DYNAMIC_STORAGE_BIT);
		}
		preRegistry.ctx().emplace<SimulationDebugMeshContext>();
		preRegistry.ctx().emplace<PresentationExchangeContext>(presExchangeData);
		auto& presGameInputCtx = preRegistry.ctx().emplace<aoein::GameInputContext>();
		auto& presGameInputBindingCtx = preRegistry.ctx().emplace<aoein::GameInputBindingContext>();
		auto presForwardInputValueId = aoein::GameInputUtils::addInputValueBinding(presGameInputCtx, presGameInputBindingCtx,
			std::make_shared<aoein::GamepadButtonInputValueBinding>(0, aoein::Gamepad::Button::A));
		auto presBackwardInputValueId = aoein::GameInputUtils::addInputValueBinding(presGameInputCtx, presGameInputBindingCtx,
			std::make_shared<aoein::GamepadButtonInputValueBinding>(0, aoein::Gamepad::Button::B));
		auto presSteeringInputValueId = aoein::GameInputUtils::addInputValueBinding(presGameInputCtx, presGameInputBindingCtx,
			std::make_shared<aoein::GamepadAxisInputValueBinding>(0, aoein::Gamepad::Axis::LeftX));

		auto& preInputBindings = preRegistry.ctx().emplace<aoein::InputBindings>();
		auto& cameraDirectorContext = preRegistry.ctx().emplace<aoegl::CameraDirectorContext>();
		auto& sceneTextureContext = preRegistry.ctx().emplace<aoegl::SceneTextureContext>();
		{
			auto& sceneTexture = sceneTextureContext.texture;

			static constexpr uint32_t k_multiSampling = 1;
			auto const width = static_cast<GLsizei>(a_window.getSize().x * std::sqrt(k_multiSampling));
			auto const height = static_cast<GLsizei>(a_window.getSize().y * std::sqrt(k_multiSampling));

			// 1. color texture
			glGenTextures(1, &sceneTexture.texture);
			glBindTexture(GL_TEXTURE_2D, sceneTexture.texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// 2. renderbuffer
			glGenTextures(1, &sceneTexture.renderbuffer);
			glBindTexture(GL_TEXTURE_2D, sceneTexture.renderbuffer);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			/* OPTIMIZED DEPTH
			glGenRenderbuffers(1, &sceneTexture.renderbuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, sceneTexture.renderbuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
			*/
			glGenFramebuffers(1, &sceneTexture.framebuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, sceneTexture.framebuffer);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneTexture.texture, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sceneTexture.renderbuffer, 0);
			/* OPTIMIZED DEPTH
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, sceneTexture.renderbuffer);
			*/
			glClearColor(0.3f, 0.1f, 0.1f, 1.0f);
			glEnable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			/*aoegl::graphic_enum drawBuffer = GL_COLOR_ATTACHMENT0;
			glDrawBuffers(1, &drawBuffer);*/

			assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		}
		auto& debugRenderContext = preRegistry.ctx().emplace<aoegl::DebugRenderContext>();
		{
			auto const debugProgramData = shaderProgramDatabase.find(filesystemIndexer.get_runtime_id("data/new/shaders/debug_program.json"));
			if (debugProgramData != nullptr)
			{
				aoegl::createProgram(*debugProgramData, debugRenderContext.debugProgram);
			}

			// vao
			{
				glCreateVertexArrays(1, &(debugRenderContext.vao));
				glBindVertexArray(debugRenderContext.vao);
				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);

				// position
				glEnableVertexAttribArray(0);
				glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
				glVertexAttribBinding(0, 0);

				// color
				glEnableVertexAttribArray(1);
				glVertexAttribFormat(1, 4, GL_FLOAT, GL_FALSE, 0);
				glVertexAttribBinding(1, 1);
			}
			// vbo
			{
				glCreateBuffers(1, &(debugRenderContext.vbo));
				glBindVertexBuffer(
					0,
					debugRenderContext.vbo,
					offsetof(aoegl::DebugVertex, position),
					sizeof(aoegl::DebugVertex));
				glBindVertexBuffer(
					1,
					debugRenderContext.vbo,
					offsetof(aoegl::DebugVertex, color),
					sizeof(aoegl::DebugVertex));
			}
			// ebo
			{
				glCreateBuffers(1, &(debugRenderContext.ebo));
			}
		}
		auto& postProcessRenderContext = preRegistry.ctx().emplace<aoegl::PostProcessRenderContext>();
		{
			auto const postProcessProgramData = shaderProgramDatabase.find(filesystemIndexer.get_runtime_id("data/new/shaders/post_process_program.json"));
			if (postProcessProgramData != nullptr)
			{
				aoegl::createProgram(*postProcessProgramData, postProcessRenderContext.postProcessProgram);
			}
		}

		auto& renderSceneContext = preRegistry.ctx().emplace<aoegl::RenderSceneContext>();
		{
			constexpr int32_t k_maxLightCount = 2048;
			static const glm::ivec2 k_lightClusterSize = glm::ivec2{ 16, 16 };
			constexpr int32_t k_lightClusterCountZ = 24;
			static const glm::ivec2 k_sceneFramebufferSize = a_window.getSize();
			constexpr int32_t k_maxLightCountPerCluster = 128;

			auto const lightClusterCountXY = glm::ivec2{ glm::ceil(glm::vec2{ k_sceneFramebufferSize } / glm::vec2{ k_lightClusterSize }) };
			auto const lightClusterCount = lightClusterCountXY.x * lightClusterCountXY.y * k_lightClusterCountZ;

			renderSceneContext.sceneFramebufferSize = k_sceneFramebufferSize;
			renderSceneContext.maxLightCount = k_maxLightCount;
			renderSceneContext.lightClusterCount = lightClusterCount;

			// TODO: this context doesn't own shit
			glCreateBuffers(1, &renderSceneContext.globalUbo);
			// TODO remove? glBindBuffer(GL_UNIFORM_BUFFER, renderSceneContext.globalUbo);
			glNamedBufferStorage(renderSceneContext.globalUbo, sizeof(aoegl::GlobalRenderSceneConfig), nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
			
			glCreateBuffers(1, &renderSceneContext.meshUbo);
			// TODO remove? glBindBuffer(GL_UNIFORM_BUFFER, renderSceneContext.meshUbo);
			glNamedBufferStorage(renderSceneContext.meshUbo, sizeof(aoegl::MeshRenderSceneConfig), nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

			glCreateBuffers(1, &renderSceneContext.lightsSsbo);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, renderSceneContext.lightsSsbo);
			glNamedBufferStorage(renderSceneContext.lightsSsbo, k_maxLightCount * sizeof(aoegl::CulledLight2), nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

			glCreateBuffers(1, &renderSceneContext.lightCountPerClusterSsbo);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, renderSceneContext.lightCountPerClusterSsbo);
			glNamedBufferStorage(renderSceneContext.lightCountPerClusterSsbo, lightClusterCount * sizeof(int32_t), nullptr, GL_DYNAMIC_STORAGE_BIT);

			glCreateBuffers(1, &renderSceneContext.lightIndicesPerClusterSsbo);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, renderSceneContext.lightIndicesPerClusterSsbo);
			glNamedBufferStorage(renderSceneContext.lightIndicesPerClusterSsbo, lightClusterCount* k_maxLightCountPerCluster * sizeof(int32_t), nullptr, GL_DYNAMIC_STORAGE_BIT);

			glCreateTextures(GL_TEXTURE_2D, 1, &renderSceneContext.sceneColorTexture);
			glTextureStorage2D(renderSceneContext.sceneColorTexture, 1, GL_RGB8, k_sceneFramebufferSize.x, k_sceneFramebufferSize.y);
			glTextureParameteri(renderSceneContext.sceneColorTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(renderSceneContext.sceneColorTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glCreateTextures(GL_TEXTURE_2D, 1, &renderSceneContext.sceneDepthTexture);
			glTextureStorage2D(renderSceneContext.sceneDepthTexture, 1, GL_DEPTH_COMPONENT24, k_sceneFramebufferSize.x, k_sceneFramebufferSize.y);
			glTextureParameteri(renderSceneContext.sceneDepthTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteri(renderSceneContext.sceneDepthTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glCreateFramebuffers(1, &renderSceneContext.sceneFramebuffer);
			glNamedFramebufferTexture(renderSceneContext.sceneFramebuffer, GL_COLOR_ATTACHMENT0, renderSceneContext.sceneColorTexture, 0);
			glNamedFramebufferTexture(renderSceneContext.sceneFramebuffer, GL_DEPTH_ATTACHMENT, renderSceneContext.sceneDepthTexture, 0);
			assert(glCheckNamedFramebufferStatus(renderSceneContext.sceneFramebuffer, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

			glCreateVertexArrays(1, &renderSceneContext.postProcessVao);
		}
		{
			auto const depthShaderProgramData = shaderProgramDatabase.find(filesystemIndexer.get_runtime_id("data/new/shaders/depth.json"));
			assert(depthShaderProgramData != nullptr);
			assert(depthShaderProgramData->vertexShaderSource != nullptr && depthShaderProgramData->fragmentShaderSource != nullptr);
			renderSceneContext.depthPrePassProgram = aoegl::createProgram(
				*depthShaderProgramData->vertexShaderSource, *depthShaderProgramData->fragmentShaderSource);
		}
		{
			auto const defaultPostProcessProgramData = shaderProgramDatabase.find(filesystemIndexer.get_runtime_id("data/new/shaders/default_post_process.json"));
			assert(defaultPostProcessProgramData != nullptr);
			assert(defaultPostProcessProgramData->vertexShaderSource != nullptr && defaultPostProcessProgramData->fragmentShaderSource != nullptr);
			renderSceneContext.postProcessProgram = aoegl::createProgram(
				*defaultPostProcessProgramData->vertexShaderSource, *defaultPostProcessProgramData->fragmentShaderSource);

		}
		{
			auto const debugPostProcessProgramData = shaderProgramDatabase.find(filesystemIndexer.get_runtime_id("data/new/shaders/debug_post_process.json"));
			assert(debugPostProcessProgramData != nullptr);
			assert(debugPostProcessProgramData->vertexShaderSource != nullptr && debugPostProcessProgramData->fragmentShaderSource != nullptr);
			renderSceneContext.debugPostProcessProgram = aoegl::createProgram(
				*debugPostProcessProgramData->vertexShaderSource, *debugPostProcessProgramData->fragmentShaderSource);
		}
		{
			// TODO: doesn't belong here
			auto const debugPostProcessConfig = aoegl::DebugPostProcessConfig{
				.mode = aoegl::DebugMode::None
			};
			glCreateBuffers(1, &renderSceneContext.debugUbo);
			glNamedBufferStorage(renderSceneContext.debugUbo, sizeof(debugPostProcessConfig), &debugPostProcessConfig, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
		}
		{
			auto const debugMeshProgramData = shaderProgramDatabase.find(filesystemIndexer.get_runtime_id("data/new/shaders/debug_mesh.json"));
			assert(debugMeshProgramData != nullptr);
			assert(debugMeshProgramData->vertexShaderSource != nullptr && debugMeshProgramData->fragmentShaderSource != nullptr);
			renderSceneContext.debugMeshProgram = aoegl::createProgram(
				*debugMeshProgramData->vertexShaderSource, *debugMeshProgramData->fragmentShaderSource);
		}
		{
			auto const clusterShaderSource = stringDatabase.find(filesystemIndexer.get_runtime_id("data/new/shaders/cluster.comp"));
			renderSceneContext.computeLightClustersProgram = aoegl::createComputeProgram(*clusterShaderSource);
		}
		{
			glGenQueries(renderSceneContext.timerQueries.size(), renderSceneContext.timerQueries.data());
		}

		entt::registry simRegistry;

		auto& simGameInputCtx = simRegistry.ctx().emplace<aoein::GameInputContext>();
		auto simForwardInputValueId = simGameInputCtx.registerValue();
		auto simBackwardInputValueId = simGameInputCtx.registerValue();
		auto simSteeringInputValueId = simGameInputCtx.registerValue();
		simRegistry.ctx().emplace<PresentationExchangeContext>(presExchangeData);

		auto& simPresentationInputBindingCtx = simRegistry.ctx().emplace<aoein::PresentationInputBindingContext>();
		simPresentationInputBindingCtx.inputValueIds.emplace_back(presForwardInputValueId, simForwardInputValueId);
		simPresentationInputBindingCtx.inputValueIds.emplace_back(presBackwardInputValueId, simBackwardInputValueId);
		simPresentationInputBindingCtx.inputValueIds.emplace_back(presSteeringInputValueId, simSteeringInputValueId);

		simRegistry.ctx().emplace<aoest::TimeContext>();
		simRegistry.ctx().emplace<SimulationExchangeContext>(simExchangeData);
		simRegistry.ctx().emplace<aoest::FixedRateTimeContext>();
		simRegistry.ctx().emplace<aoegl::DebugMeshContext>();
		simRegistry.ctx().emplace<aoeph::CollisionContext>();

		aoegl::GraphicId forwardMaterialProgram = aoegl::k_invalidId;
		auto const forwardMaterialData = shaderProgramDatabase.find(filesystemIndexer.get_runtime_id("data/new/shaders/basic_mesh.json"));
		if (forwardMaterialData != nullptr)
		{
			if (forwardMaterialData->vertexShaderSource != nullptr && forwardMaterialData->fragmentShaderSource != nullptr)
			{
				forwardMaterialProgram = aoegl::createProgram(*(forwardMaterialData->vertexShaderSource), *(forwardMaterialData->fragmentShaderSource));
			}
		}
		aoegl::GraphicId wheelShaderProgram = aoegl::k_invalidId;
		auto const wheelShaderData = shaderProgramDatabase.find(filesystemIndexer.get_runtime_id("data/new/shaders/wheel_mesh.json"));
		if (wheelShaderData != nullptr)
		{
			if (wheelShaderData->vertexShaderSource != nullptr && wheelShaderData->fragmentShaderSource != nullptr)
			{
				wheelShaderProgram = aoegl::createProgram(*(wheelShaderData->vertexShaderSource), *(wheelShaderData->fragmentShaderSource));
			}
		}
		struct alignas(16) CustomRenderSceneConfig
		{
			glm::vec3 albedo;
			float metallic;
			float roughness;
		};

		// Prepare entities
		{
			// ==== STATIC COLLIDER

			auto e = simRegistry.create();
			auto& pos = simRegistry.emplace<aoest::Position>(e);
			auto& rot = simRegistry.emplace<aoest::Rotation>(e, glm::quat());
			auto& col = simRegistry.emplace<aoeph::StaticCollider>(e);
			auto& par = col.parts.emplace_back();
			for (auto i = -1; i < 51; ++i)
			{
				par.triangles.emplace_back(
					glm::vec3{ -120.0f, 0.0f, -(i + 1) * 20.0f },
					glm::vec3{ -120.0f, 0.0f, -i * 20.0f },
					glm::vec3{ 120.0f, 0.0f, -i * 20.0f });
				par.triangles.emplace_back(
					glm::vec3{ -120.0f, 0.0f, -(i + 1) * 20.0f },
					glm::vec3{ 120.0f, 0.0f, -i * 20.0f },
					glm::vec3{ 120.0f, 0.0f, -(i + 1) * 20.0f });
			}
			col.bounds = { glm::vec3{-120.0f, 0.0f, -1020.0f}, glm::vec3{120.0f, 1.0f, 20.0f} };

			assert(e == preRegistry.create(e));
			preRegistry.emplace<aoest::Position>(e, simRegistry.get<aoest::Position>(e));
			preRegistry.emplace<aoest::Rotation>(e, simRegistry.get<aoest::Rotation>(e));
			auto& sm = preRegistry.emplace<aoegl::StaticMeshComponent>(e);
			
			std::vector<aoegl::StaticVertex> vertices;
			std::vector<uint32_t> indices;
			for (auto const& triangle : par.triangles)
			{
				auto const n = glm::normalize(glm::cross(triangle.p1 - triangle.p0, triangle.p2 - triangle.p0));
				vertices.emplace_back(triangle.p0, n, glm::vec3{ 0.0f });
				vertices.emplace_back(triangle.p1, n, glm::vec3{ 0.0f });
				vertices.emplace_back(triangle.p2, n, glm::vec3{ 0.0f });
				indices.emplace_back(static_cast<uint32_t>(indices.size()));
				indices.emplace_back(static_cast<uint32_t>(indices.size()));
				indices.emplace_back(static_cast<uint32_t>(indices.size()));
			}

			// PROGRAM

			// MESH
			aoegl::GraphicId vao, vbo, ebo;
			glCreateVertexArrays(1, &vao);
			glCreateBuffers(1, &vbo);
			glCreateBuffers(1, &ebo);

			glNamedBufferStorage(vbo, vertices.size() * sizeof(aoegl::StaticVertex), vertices.data(), 0);
			glNamedBufferStorage(ebo, indices.size() * sizeof(uint32_t), indices.data(), 0);

			glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(aoegl::StaticVertex));
			glVertexArrayElementBuffer(vao, ebo);

			glEnableVertexArrayAttrib(vao, 0);
			glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(aoegl::StaticVertex, position));
			glVertexArrayAttribBinding(vao, 0, 0);

			glEnableVertexArrayAttrib(vao, 1);
			glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(aoegl::StaticVertex, normal));
			glVertexArrayAttribBinding(vao, 1, 0);

			glEnableVertexArrayAttrib(vao, 2);
			glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(aoegl::StaticVertex, uv));
			glVertexArrayAttribBinding(vao, 2, 0);

			// MATERIAL
			aoegl::GraphicId materialUbo = aoegl::k_invalidId;
			auto const materialData = CustomRenderSceneConfig{
				.albedo = glm::vec3{0.05f},
				.metallic = 0.05f,
				.roughness = 0.95f
			};
			glCreateBuffers(1, &materialUbo);
			glNamedBufferStorage(materialUbo, sizeof(CustomRenderSceneConfig), &materialData, GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT);
			sm.parts.emplace_back(forwardMaterialProgram, materialUbo, vao, static_cast<int32_t>(indices.size()));
			for (auto const& vertex : vertices)
			{
				if (sm.boundingRadius * sm.boundingRadius < glm::dot(vertex.position, vertex.position))
				{
					sm.boundingRadius = glm::length(vertex.position);
				}
			}
		}

		{
			// ==== GHOST CAMERA

			auto e = preRegistry.create();
			preRegistry.emplace<aoest::Position>(e, glm::vec3{ 0.0f, 0.0f, 10.0f });
			preRegistry.emplace<aoest::Rotation>(e, glm::quat{});
			auto& gcc = preRegistry.emplace<aoedb::GhostControllerComponent>(e);

			gcc.lateralMoveValueId = aoein::GameInputUtils::addInputValueBinding(
				presGameInputCtx,
				presGameInputBindingCtx,
				std::make_shared<aoein::UpDownInputValueBinding>(
					std::make_shared<aoein::KeyboardKeyInputValueBinding>(aoein::Keyboard::Key::S),
					std::make_shared<aoein::KeyboardKeyInputValueBinding>(aoein::Keyboard::Key::F)
				)
			);
			gcc.longitudinalMoveValueId = aoein::GameInputUtils::addInputValueBinding(
				presGameInputCtx,
				presGameInputBindingCtx,
				std::make_shared<aoein::UpDownInputValueBinding>(
					std::make_shared<aoein::KeyboardKeyInputValueBinding>(aoein::Keyboard::Key::E),
					std::make_shared<aoein::KeyboardKeyInputValueBinding>(aoein::Keyboard::Key::D)
				)
			);
			gcc.verticalMoveValueId = aoein::GameInputUtils::addInputValueBinding(
				presGameInputCtx,
				presGameInputBindingCtx,
				std::make_shared<aoein::UpDownInputValueBinding>(
					std::make_shared<aoein::KeyboardKeyInputValueBinding>(aoein::Keyboard::Key::W),
					std::make_shared<aoein::KeyboardKeyInputValueBinding>(aoein::Keyboard::Key::R)
				)
			);
			gcc.pitchValueId = aoein::GameInputUtils::addInputValueBinding(
				presGameInputCtx,
				presGameInputBindingCtx,
				std::make_shared<aoein::DerivedInputValueBinding>(
					std::make_shared<aoein::MouseAxisInputValueBinding>(aoein::Mouse::Axis::Y),
					0.001f
				)
			);
			gcc.yawValueId = aoein::GameInputUtils::addInputValueBinding(
				presGameInputCtx,
				presGameInputBindingCtx,
				std::make_shared<aoein::DerivedInputValueBinding>(
					std::make_shared<aoein::MouseAxisInputValueBinding>(aoein::Mouse::Axis::X),
					0.001f
				)
			);
			gcc.enableRotationValueId = aoein::GameInputUtils::addInputValueBinding(
				presGameInputCtx,
				presGameInputBindingCtx,
				std::make_shared<aoein::MouseButtonInputValueBinding>(aoein::Mouse::Button::Right)
			);

			gcc.decreaseSpeedEventId = aoein::GameInputUtils::addInputEventBinding(
				presGameInputCtx,
				presGameInputBindingCtx,
				std::make_shared<aoein::MouseScrollEventBinding>(false));
			gcc.increaseSpeedEventId = aoein::GameInputUtils::addInputEventBinding(
				presGameInputCtx,
				presGameInputBindingCtx,
				std::make_shared<aoein::MouseScrollEventBinding>(true));

			preRegistry.emplace<aoedb::IsControlledTag>(e);
			preRegistry.emplace<aoegl::CameraComponent>(e);
			cameraDirectorContext.activeCameraEntity = e;
		}
		auto carEntity = preRegistry.create();
		assert(carEntity == simRegistry.create(carEntity));
		{
			// ==== FOLLOW CAMERA

			auto e = preRegistry.create();
			preRegistry.emplace<aoest::Position>(e, glm::vec3{ 0.0f, 0.0f, 10.0f });
			preRegistry.emplace<aoest::Rotation>(e, glm::quat{});
			preRegistry.emplace<aoegl::CameraComponent>(e);
			preRegistry.emplace<aoest::SoftFollowComponent>(e,
				carEntity, glm::vec3{ 0.0f, 3.0f, 5.0f }, glm::vec3{ 0.0f, 0.0f, -4.0f }, 3000.0f, 1.0f, 500.0f);
			cameraDirectorContext.activeCameraEntity = e;
		}
		{
			// ==== CAR

			auto niceMesh = staticMeshDatabase.find(filesystemIndexer.get_runtime_id("data/new/models/ToyCar.glb"));

			auto e = carEntity;
			simRegistry.emplace<aoest::Position>(e, glm::vec3{ 0.0f, 3.0f, -20.0f });
			simRegistry.emplace<aoest::Rotation>(e, glm::quat{});// glm::angleAxis(3.14f, glm::vec3{ 1.0f, 0.0f, 0.0f }));
			auto& carCollider = simRegistry.emplace<aoeph::CarCollider>(e);
			// front axel
			carCollider.chassisParts.emplace_back(glm::vec3{ -0.01553f, 0.36325f, -1.75357f }, glm::quat{ glm::vec3{0.0f} }, glm::vec3{ 0.905f, 0.283f, 0.385f });
			// mid axel
			carCollider.chassisParts.emplace_back(glm::vec3{ 0.0f, 0.471f, -0.219f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.439f, 0.362f, 1.902f });
			// cockpit
			carCollider.chassisParts.emplace_back(glm::vec3{ 0.0f, 0.65281f, 0.89763f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 1.021f, 0.515f, 1.038f });
			// chassis
			carCollider.chassisParts.emplace_back(glm::vec3{ 0.0f, 0.44878f, 0.20792f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.968f, 0.363f, 1.682f });

			// front left wheel
			carCollider.wheels[0] = { glm::vec3{ -0.86301f, 0.3525f, -1.78209f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.182f, 0.364f, 0.364f } };
			// carCollider.wheels[0].turnFactor = 1.0f;
			// front right wheel
			carCollider.wheels[1] = { glm::vec3{ 0.86299f, 0.3525f, -1.78209f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.182f, 0.364f, 0.364f } };
			// .turnFactor = 1.0f;
			// rear left wheel
			carCollider.wheels[2] = { glm::vec3{ -0.885f, 0.3525f, 1.2055f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.182f, 0.364f, 0.364f } };
			// rear right wheel
			carCollider.wheels[3] = { glm::vec3{ 0.885f, 0.3525f, 1.2055f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.182f, 0.364f, 0.364f } };
			carCollider.mass = 1'500.0f;
			carCollider.barycenterLocal = glm::vec3{ 0.0f, 0.175f, -0.288295f };
			carCollider.inertiaLocal = glm::mat3{ carCollider.mass / 5.0f };
			carCollider.inertiaLocal[0][0] *= (0.7f * 0.7f + 1.6f * 1.6f);
			carCollider.inertiaLocal[1][1] *= (1.6f * 1.6f + 0.9f * 0.9f);
			carCollider.inertiaLocal[2][2] *= (0.9f * 0.9f + 0.7f * 0.7f);
			carCollider.boundsHalfExtentsLocal = glm::vec3{ 3.5f };


			auto& carController = simRegistry.emplace<aoeph::CarControllerComponent>(e);
			carController.wheels[0].steeringFactor = 1.0f;
			carController.wheels[1].steeringFactor = 1.0f;
			// carController.forwardInputId = inputBindings.switches.add(aoein::InputBindingUtils::makeSwitch(aoein::SwitchReference{ 0, aoein::Gamepad::Button::A }));
			// carController.backwardInputId = inputBindings.switches.add(aoein::InputBindingUtils::makeSwitch(aoein::SwitchReference{ 0, aoein::Gamepad::Button::B }));
			// carController.steeringInputId = inputBindings.axes.add(aoein::InputBindingUtils::makeAxis(aoein::AxisReference{ 0, aoein::Gamepad::Axis::LeftX }));
			carController.forwardInputValueId = simForwardInputValueId;
			carController.backwardInputValueId = simBackwardInputValueId;
			carController.steeringInputValueId = simSteeringInputValueId;

			preRegistry.emplace<aoest::Position>(e, simRegistry.get<aoest::Position>(e));
			preRegistry.emplace<aoest::Rotation>(e, simRegistry.get<aoest::Rotation>(e));
			preRegistry.emplace<aoeph::CarCollider>(e, simRegistry.get<aoeph::CarCollider>(e));
			preRegistry.emplace<aoeph::CarControllerComponent>(e, simRegistry.get<aoeph::CarControllerComponent>(e));
			preRegistry.emplace<aoest::InterpolatedPosition>(e).positions.fill(preRegistry.get<aoest::Position>(e));
			preRegistry.emplace<aoest::InterpolatedRotation>(e).rotations.fill(preRegistry.get<aoest::Rotation>(e));
			preRegistry.emplace<aoest::InterpolationTimeComponent>(e).times.fill(currentTime);

			auto& sm = preRegistry.emplace<aoegl::StaticMeshComponent>(e);
			auto& cm = preRegistry.emplace<aoeph::CarMaterialsComponent>(e);

			// MESH
			aoegl::GraphicId chassisMaterialUbo = aoegl::k_invalidId;
			auto const chassisMaterialData = CustomRenderSceneConfig{
				.albedo = glm::vec3{ 0.9f },
				.metallic = 0.7f,
				.roughness = 0.2f
			};
			glCreateBuffers(1, &chassisMaterialUbo);
			glNamedBufferStorage(chassisMaterialUbo, sizeof(CustomRenderSceneConfig), &chassisMaterialData, GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT);

			auto i = 0;
			for (auto& niceMeshPart : niceMesh->parts)
			{
				aoegl::GraphicId vao, vbo, ebo;
				glCreateVertexArrays(1, &vao);
				glCreateBuffers(1, &vbo);
				glCreateBuffers(1, &ebo);

				glNamedBufferStorage(vbo, niceMeshPart.vertices.size() * sizeof(aoegl::StaticVertex), niceMeshPart.vertices.data(), 0);
				glNamedBufferStorage(ebo, niceMeshPart.indices.size() * sizeof(uint32_t), niceMeshPart.indices.data(), 0);

				glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(aoegl::StaticVertex));
				glVertexArrayElementBuffer(vao, ebo);

				glEnableVertexArrayAttrib(vao, 0);
				glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(aoegl::StaticVertex, position));
				glVertexArrayAttribBinding(vao, 0, 0);

				glEnableVertexArrayAttrib(vao, 1);
				glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(aoegl::StaticVertex, normal));
				glVertexArrayAttribBinding(vao, 1, 0);

				glEnableVertexArrayAttrib(vao, 2);
				glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(aoegl::StaticVertex, uv));
				glVertexArrayAttribBinding(vao, 2, 0);

				if (i == 0)
				{
					sm.parts.emplace_back(forwardMaterialProgram, chassisMaterialUbo, vao, niceMeshPart.indices.size());
				}
				else
				{
					auto const wheelIndex = (i - 1) / 2;
					auto const isTire = (i - 1) % 2 == 0;
					aoegl::GraphicId materialUbo;
					glCreateBuffers(1, &materialUbo);
					glNamedBufferStorage(materialUbo, sizeof(aoeph::WheelRenderSceneConfig), nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT);
					
					cm.parts.emplace_back(
						wheelIndex,
						isTire ? glm::vec3{ 0.0f } : glm::vec3{ 1.0f, 0.0f, 0.0f } /* albedo */,
						0.0f /* metallic */,
						isTire ? 0.8f : 0.2f /* roughness */,
						materialUbo);

					sm.parts.emplace_back(wheelShaderProgram, materialUbo, vao, niceMeshPart.indices.size());
				}

				for (auto const& vertex : niceMeshPart.vertices)
				{
					if (sm.boundingRadius * sm.boundingRadius < glm::dot(vertex.position, vertex.position))
					{
						sm.boundingRadius = glm::length(vertex.position);
					}
				}

				++i;
			}
		}

		{
			auto streetLampMesh = staticMeshDatabase.find(filesystemIndexer.get_runtime_id("data/new/models/StreetLamp.glb"));
			std::vector<aoegl::StaticMeshComponent::Part> streetLampParts;
			float streetLampBoundingRadius = 0.0f;

			auto i = 0;
			for (auto& streetLampMeshPart : streetLampMesh->parts)
			{
				aoegl::GraphicId materialUbo;
				glCreateBuffers(1, &materialUbo);
				CustomRenderSceneConfig materialData{
					.albedo = (i == 0 ? glm::vec3{0.0f} : glm::vec3{10.f}),
					.metallic = (i == 0 ? 0.3f : 0.9f),
					.roughness = (i == 0 ? 0.7f : 0.1f)
				};
				++i;
				glNamedBufferStorage(materialUbo, sizeof(CustomRenderSceneConfig), &materialData, GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT);
				aoegl::GraphicId vao, vbo, ebo;
				glCreateVertexArrays(1, &vao);
				glCreateBuffers(1, &vbo);
				glCreateBuffers(1, &ebo);

				glNamedBufferStorage(vbo, streetLampMeshPart.vertices.size() * sizeof(aoegl::StaticVertex), streetLampMeshPart.vertices.data(), 0);
				glNamedBufferStorage(ebo, streetLampMeshPart.indices.size() * sizeof(uint32_t), streetLampMeshPart.indices.data(), 0);

				glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(aoegl::StaticVertex));
				glVertexArrayElementBuffer(vao, ebo);

				glEnableVertexArrayAttrib(vao, 0);
				glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(aoegl::StaticVertex, position));
				glVertexArrayAttribBinding(vao, 0, 0);

				glEnableVertexArrayAttrib(vao, 1);
				glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(aoegl::StaticVertex, normal));
				glVertexArrayAttribBinding(vao, 1, 0);

				glEnableVertexArrayAttrib(vao, 2);
				glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(aoegl::StaticVertex, uv));
				glVertexArrayAttribBinding(vao, 2, 0);

				// MATERIAL
				streetLampParts.emplace_back(forwardMaterialProgram, materialUbo, vao, streetLampMeshPart.indices.size());
				for (auto const& vertex : streetLampMeshPart.vertices)
				{
					if (streetLampBoundingRadius * streetLampBoundingRadius < glm::dot(vertex.position, vertex.position))
					{
						streetLampBoundingRadius = glm::length(vertex.position);
					}
				}
			}

			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
			auto rng = [&]() { return distribution(gen); };
			static int k_rows = 50;
			static int k_cols = 6;
			static float k_minX = -40.0f;
			static float k_maxX = 40.0f;
			static float k_minZ = -1020.0f;
			static float k_maxZ = 20.0f;

			for (int j = 0; j < k_cols; ++j)
			{
				for (int i = 0; i < k_rows; ++i)
				{
					auto const x = k_minX + (j + 0.5f + 0.0f * rng()) * ((k_maxX - k_minX) / k_cols);
					auto const y = 0.0f;
					auto const z = k_minZ + (i + 0.5f + 0.0f * rng()) * ((k_maxZ - k_minZ) / k_rows);
					auto const r = glm::angleAxis(2.0f * std::numbers::pi_v<float> * (j % 2 == 0 ? 0.5f : 0.0f), glm::vec3{ 0.0f, 1.0f, 0.0f });

					auto const lamp = preRegistry.create();
					preRegistry.emplace<aoest::Position>(lamp, glm::vec3{ x, y, z });
					preRegistry.emplace<aoest::Rotation>(lamp, r);
					assert(lamp == simRegistry.create(lamp));
					simRegistry.emplace<aoest::Position>(lamp, preRegistry.get<aoest::Position>(lamp));
					simRegistry.emplace<aoest::Rotation>(lamp, preRegistry.get<aoest::Rotation>(lamp));
					auto& staticMeshCmp = preRegistry.emplace<aoegl::StaticMeshComponent>(lamp);
					staticMeshCmp.parts = streetLampParts;
					staticMeshCmp.boundingRadius = streetLampBoundingRadius;
					auto& col = simRegistry.emplace<aoeph::StaticCollider>(lamp);
					auto& par = col.parts.emplace_back();
					static auto const rr = 0.1f;
					static auto const n = 8;
					for (auto i = 0; i < n; ++i)
					{
						auto const a0 = (2.0f * std::numbers::pi_v<float> *i) / n;
						auto const a1 = (2.0f * std::numbers::pi_v<float> *(i + 1)) / n;
						auto const c0 = rr * std::cos(a0);
						auto const c1 = rr * std::cos(a1);
						auto const s0 = rr * std::sin(a0);
						auto const s1 = rr * std::sin(a1);

						par.triangles.emplace_back(
							glm::vec3{ c1, 0.0f, s1 },
							glm::vec3{c0, 0.0f, s0},
							glm::vec3{c0, 2.0f, s0});
						par.triangles.emplace_back(
							glm::vec3{ c1, 0.0f, s1 },
							glm::vec3{ c0, 2.0f, s0 },
							glm::vec3{ c1, 2.0f, s1 });
					}
					col.bounds = { glm::vec3{x-.1f, 0.0f, z-.1f}, glm::vec3{x+.1f, 2.0f, z+.1f} };

					auto const light = preRegistry.create();
					preRegistry.emplace<aoest::Position>(light, glm::vec3{ x, y, z } + r * glm::vec3{ -0.66f, 4.66f, 0.0f });
					preRegistry.emplace<aoest::Rotation>(light, r);
					preRegistry.emplace<aoegl::LightComponent>(light,
						aoegl::LightComponent::Type::Point,
						12.0f /* radius */,
						2.0f /* intensity */,
						glm::vec3{ 1.0f, 0.84f, 0.7f } /* color */);
				}
			}
		}

		{
			auto sphereMesh = staticMeshDatabase.find(filesystemIndexer.get_runtime_id("data/new/models/Sphere.glb"));
			aoegl::GraphicId sphereVao;
			if (sphereMesh->parts.size() > 0)
			{
				auto& sphereMeshPart = sphereMesh->parts[0];
				float sphereRadius = 0.0f;
				for (auto const& vertex : sphereMeshPart.vertices)
				{
					if (sphereRadius * sphereRadius < glm::dot(vertex.position, vertex.position))
					{
						sphereRadius = glm::length(vertex.position);
					}
				}
				aoegl::GraphicId vbo, ebo;
				glCreateVertexArrays(1, &sphereVao);
				glCreateBuffers(1, &vbo);
				glCreateBuffers(1, &ebo);

				glNamedBufferStorage(vbo, sphereMeshPart.vertices.size() * sizeof(aoegl::StaticVertex), sphereMeshPart.vertices.data(), 0);
				glNamedBufferStorage(ebo, sphereMeshPart.indices.size() * sizeof(uint32_t), sphereMeshPart.indices.data(), 0);

				glVertexArrayVertexBuffer(sphereVao, 0, vbo, 0, sizeof(aoegl::StaticVertex));
				glVertexArrayElementBuffer(sphereVao, ebo);

				glEnableVertexArrayAttrib(sphereVao, 0);
				glVertexArrayAttribFormat(sphereVao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(aoegl::StaticVertex, position));
				glVertexArrayAttribBinding(sphereVao, 0, 0);

				glEnableVertexArrayAttrib(sphereVao, 1);
				glVertexArrayAttribFormat(sphereVao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(aoegl::StaticVertex, normal));
				glVertexArrayAttribBinding(sphereVao, 1, 0);

				glEnableVertexArrayAttrib(sphereVao, 2);
				glVertexArrayAttribFormat(sphereVao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(aoegl::StaticVertex, uv));
				glVertexArrayAttribBinding(sphereVao, 2, 0);

				constexpr auto k_cols = 10;
				constexpr auto k_rows = 10;
				constexpr float k_dist = 0.02f;

				constexpr float k_minMetallic = 0.01f;
				constexpr float k_maxMetallic = 1.0f;
				constexpr float k_minRoughness = 0.01f;
				constexpr float k_maxRoughness = 1.0f;

				for (int i = 0; i < k_cols; ++i)
				{
					for (int j = 0; j < k_rows; ++j)
					{

						auto e = preRegistry.create();
						preRegistry.emplace<aoest::Position>(e, glm::vec3{ i * (k_dist + 2.0f * sphereRadius), sphereRadius + j * (k_dist + 2.0f * sphereRadius), 0.0f });
						preRegistry.emplace<aoest::Rotation>(e, glm::quat{});
						auto& sm = preRegistry.emplace<aoegl::StaticMeshComponent>(e);

						aoegl::GraphicId materialUbo = aoegl::k_invalidId;
						auto const materialData = CustomRenderSceneConfig{
							.albedo = glm::vec3{1.0f, 0.0f, 0.2f},
							.metallic = k_minMetallic + static_cast<float>(i) / k_cols * (k_maxMetallic - k_minMetallic),
							.roughness = k_minRoughness + static_cast<float>(j) / k_rows * (k_maxRoughness - k_minRoughness)
						};
						glCreateBuffers(1, &materialUbo);
						glNamedBufferStorage(materialUbo, sizeof(CustomRenderSceneConfig), &materialData, GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT);


						// MATERIAL
						sm.parts.emplace_back(forwardMaterialProgram, materialUbo, sphereVao, sphereMeshPart.indices.size());
						sm.boundingRadius = sphereRadius;
					}
				}
			}
		}

		return { std::move(preRegistry), std::move(simRegistry) };
	}

	std::shared_ptr<aoeng::IWorld> createDefaultWorld(aoewi::IWindow& a_window)
	{
		auto [presentationSystems, presentationSchedule] = createPresentationEcsSystems();
		auto [simulationSystems, simulationSchedule] = createSimulationEcsSystems();

		auto [preRegistry, simRegistry] = createSimulationEcsRegistries(a_window);

		auto preWorld = std::make_shared<aoeng::EcsWorld>(presentationSystems, presentationSchedule, std::move(preRegistry));
		auto simWorld = std::make_shared<aoeng::EcsWorld>(simulationSystems, simulationSchedule, std::move(simRegistry));
		return std::make_shared<aoeng::MultiWorld>(std::vector<std::shared_ptr<aoeng::IWorld>>({ preWorld, simWorld }));
	}
}
