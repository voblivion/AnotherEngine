#include "DefaultWorld.h"

#include <vob/aoe/engine/EcsWorld.h>
#include <vob/aoe/engine/MultiWorld.h>

#include <vob/aoe/spacetime/TimeSystem.h>
#include <vob/aoe/spacetime/FixedRateTimeSystem.h>
#include <vob/aoe/window/PollEventsSystem.h>
#include <vob/aoe/rendering/PrepareImGuiFrameSystem.h>
#include <vob/aoe/window/WindowInputSystem.h>
#include <vob/aoe/input/InputBindingSystem.h>
#include <vob/aoe/input/WindowInputBindingSystem.h>
#include <vob/aoe/debug/GhostControllerSystem.h>
#include <vob/aoe/physics/CarControllerSystem.h>
#include <vob/aoe/physics/CollisionSystem.h>
#include <vob/aoe/physics/DebugRenderCollidersSystem.h>
#include <vob/aoe/spacetime/FixedRateLimitingSystem.h>
#include <vob/aoe/window/SwapBuffersSystem.h>
#include <vob/aoe/spacetime/SoftFollowSystem.h>
#include <vob/aoe/rendering/BindSceneFramebufferSystem.h>
#include <vob/aoe/rendering/RenderDebugMeshSystem.h>
#include <vob/aoe/rendering/BindWindowFramebufferSystem.h>
#include <vob/aoe/rendering/RenderSceneSystem.h>
#include <vob/aoe/rendering/RenderImGuiFrameSystem.h>

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
#include <vob/aoe/input/InputBindingUtils.h>

#include <entt/entt.hpp>

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
#pragma optimize("", off)

#include <vob/aoe/input/GameInputUtils.h>
#include <imgui.h>

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
	struct InterpolationContext
	{
		std::chrono::nanoseconds offset = std::chrono::milliseconds(10);
	};

	struct InterpolationExchangeContext
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> targetTime = std::chrono::high_resolution_clock::now();
	};

	struct InterpolationComponent
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> sourceTime = std::chrono::high_resolution_clock::now();
		std::chrono::time_point<std::chrono::high_resolution_clock> targetTime = std::chrono::high_resolution_clock::now();
	};

	struct InterpolatedPosition
	{
		glm::vec3 source;
		glm::vec3 target;
	};

	struct InterpolatedRotation
	{
		glm::quat source;
		glm::quat target;
	};

	class EventPool
	{
	public:
		template <typename TEvent>
		void addEvents(std::vector<TEvent> a_events)
		{
			auto eventListEntry = m_eventLists.find(typeid(TEvent));
			if (eventListEntry != m_eventLists.end())
			{
				eventListEntry->second->addEvents(&a_events);
				return;
			}

			m_eventLists.emplace(typeid(TEvent), std::make_shared<EventList<TEvent>>(std::move(a_events)));
		}

		template <typename TEvent>
		void pollEvents(std::vector<TEvent>& a_events)
		{
			auto eventListEntry = m_eventLists.find(typeid(TEvent));
			if (eventListEntry != m_eventLists.end())
			{
				eventListEntry->second->pollEvents(&a_events);
			}
		}

		void merge(EventPool a_eventPool)
		{
			for (auto& newEventListEntry : a_eventPool.m_eventLists)
			{
				auto eventListEntry = m_eventLists.find(newEventListEntry.first);
				if (eventListEntry != m_eventLists.end())
				{
					eventListEntry->second->merge(*newEventListEntry.second);
					continue;
				}

				m_eventLists.emplace(newEventListEntry.first, std::move(newEventListEntry.second));
			}
		}

	private:
		struct AEventList
		{
			virtual ~AEventList() = default;

			virtual void addEvents(void* a_events) = 0;
			virtual void pollEvents(void* a_events) = 0;
			virtual void merge(AEventList& a_eventList) = 0;
		};

		template <typename TEvent>
		struct EventList : AEventList
		{
			explicit EventList(std::vector<TEvent> a_events)
				: m_events{ std::move(a_events) }
			{}

			void addEvents(void* a_events) override
			{
				auto& newEvents = *static_cast<std::vector<TEvent>*>(a_events);
				m_events.insert_range(m_events.end(), newEvents);
			}

			void merge(AEventList& a_eventList) override
			{
				auto& newEvents = static_cast<EventList<TEvent>&>(a_eventList).m_events;
				m_events.insert_range(m_events.end(), newEvents);
			}

			void pollEvents(void* a_events) override
			{
				auto& newEvents = *static_cast<std::vector<TEvent>*>(a_events);
				std::swap(newEvents, m_events);
			}

			std::vector<TEvent> m_events;
		};

		std::unordered_map<std::type_index, std::shared_ptr<AEventList>> m_eventLists;
	};

	class EcsExchangeData
	{
	public:
		void store(entt::registry a_registry, EventPool a_eventPool)
		{
			std::lock_guard lock(m_mutex);
			std::swap(m_registry, a_registry);
			m_eventPool.merge(std::move(a_eventPool));
		}

		std::pair<entt::registry, EventPool> load()
		{
			entt::registry registry;
			EventPool eventPool;

			std::lock_guard lock(m_mutex);
			std::swap(registry, m_registry);
			std::swap(eventPool, m_eventPool);
			return std::make_pair(std::move(registry), std::move(eventPool));
		}

	private:
		std::mutex m_mutex;
		entt::registry m_registry;
		// TODO: if multiple readers, there should be multiple pools, but will there ever be multiple readers?
		EventPool m_eventPool;
	};

	struct SimulationExchangeContext
	{
		std::shared_ptr<EcsExchangeData> data;
	};

	struct PresentationExchangeContext
	{
		std::shared_ptr<EcsExchangeData> data;
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
			EventPool excEventPool;

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
			EventPool excEventPool;

			// Car Colliders
			for (auto [entity, position, rotation, carCollider] : m_carEntities.get(a_wdap).each())
			{
				assert(entity == excRegistry.create(entity));
				excRegistry.emplace<aoest::Position>(entity, position);
				excRegistry.emplace<aoest::Rotation>(entity, rotation);
				excRegistry.emplace<aoeph::CarCollider>(entity, carCollider);
			}

			// Interpolation
			excRegistry.ctx().emplace<aoest::TimeContext>(m_timeContext.get(a_wdap));
			excRegistry.ctx().emplace<InterpolationExchangeContext>(std::chrono::high_resolution_clock::now());

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
		aoeng::EcsWorldViewRef<aoest::Position, aoest::Rotation, aoeph::CarCollider> m_carEntities;
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
			if (!excRegistry.ctx().contains<InterpolationExchangeContext>())
			{
				return;
			}

			// Interpolation
			auto sourceTime = m_timeContext.get(a_wdap).tickStartTime;
			auto targetTime = excRegistry.ctx().get<InterpolationExchangeContext>().targetTime + m_interpolationContext.get(a_wdap).offset;

			// Car Colliders
			for (auto [entity, excPosition, excRotation, excCarCollider] : excRegistry.view<aoest::Position, aoest::Rotation, aoeph::CarCollider>().each())
			{
				auto [position, rotation, interpolatedPosition, interpolatedRotation, interpolationComponent, carCollider] = m_carEntities.get(a_wdap).get(entity);
				interpolatedPosition.source = position;
				interpolatedPosition.target = excPosition;
				interpolatedRotation.source = rotation;
				interpolatedRotation.target = excRotation;
				carCollider = excCarCollider;
				interpolationComponent.sourceTime = sourceTime;
				interpolationComponent.targetTime = targetTime;
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
		aoeng::EcsWorldContextRef<InterpolationContext> m_interpolationContext;
		aoeng::EcsWorldContextRef<SimulationDebugMeshContext> m_simDebugMeshContext;
		aoeng::EcsWorldViewRef<aoest::Position, aoest::Rotation, InterpolatedPosition, InterpolatedRotation, InterpolationComponent, aoeph::CarCollider> m_carEntities;
	};

	class TransformInterpolationSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
		{

		}

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
		{
			auto const& timeContext = m_timeContext.get(a_wdap);

			for (auto [entity, position, rotation, interpolatedPosition, interpolatedRotation, interpolationComponent] : m_transformEntities.get(a_wdap).each())
			{
				auto const sourceToTarget = std::chrono::duration<float>(interpolationComponent.targetTime - interpolationComponent.sourceTime).count();
				if (std::abs(sourceToTarget) < std::numeric_limits<float>::epsilon())
				{

					continue;
				}

				auto const sourceToNow = std::chrono::duration<float>(timeContext.tickStartTime - interpolationComponent.sourceTime).count();
				auto const alpha = std::clamp(sourceToNow / sourceToTarget, 0.0f, 1.0f);

				position = interpolatedPosition.source + alpha * (interpolatedPosition.target - interpolatedPosition.source);
				rotation = glm::slerp(interpolatedRotation.source, interpolatedRotation.target, alpha);
			}
		}

	private:
		aoeng::EcsWorldContextRef<aoest::TimeContext> m_timeContext;
		aoeng::EcsWorldViewRef<aoest::Position, aoest::Rotation, InterpolatedPosition, InterpolatedRotation, InterpolationComponent> m_transformEntities;
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
		auto pollEventsId = addSystem<aoewi::PollEventsSystem>(ecsSystems);
		auto prepareImGuiFrameId = addSystem<aoegl::PrepareImGuiFrameSystem>(ecsSystems);
		auto windowInputId = addSystem<aoewi::WindowInputSystem>(ecsSystems);
		auto inputBindingId = addSystem<aoein::InputBindingSystem>(ecsSystems);
		auto windowInputBindingId = addSystem<aoein::WindowInputBindingSystem>(ecsSystems);
		auto ghostControllerId = addSystem<aoedb::GhostControllerSystem>(ecsSystems);
		// auto carControllerId = addSystem<aoeph::CarControllerSystem>(ecsSystems);
		// auto collisionId = addSystem<aoeph::CollisionSystem>(ecsSystems);
		// auto softFollowId = addSystem<aoest::SoftFollowSystem>(ecsSystems);
		auto bindSceneFramebufferId = addSystem<aoegl::BindSceneFramebufferSystem>(ecsSystems);
		auto renderDebugMeshId = addSystem<aoegl::RenderDebugMeshSystem>(ecsSystems);
		auto bindWindowFramebufferId = addSystem<aoegl::BindWindowFramebufferSystem>(ecsSystems);
		auto renderSceneId = addSystem<aoegl::RenderSceneSystem>(ecsSystems);
		auto renderImGuiFrameId = addSystem<aoegl::RenderImGuiFrameSystem>(ecsSystems);
		auto swapBuffersId = addSystem<aoewi::SwapBuffersSystem>(ecsSystems);
		// auto fixedRateLimitingId = addSystem<aoest::FixedRateLimitingSystem>(ecsSystems);

		auto simulationImportId = addSystem<SimulationImportSystem>(ecsSystems);
		auto transformInterpolationId = addSystem<TransformInterpolationSystem>(ecsSystems);
		auto simDebugRenderId = addSystem<SimulationDebugRenderSystem>(ecsSystems);
		auto presExportId = addSystem<PresentationExportSystem>(ecsSystems);

		aoeng::EcsSchedule ecsSchedule({ { "Presentation", {
			{ timeId },
			{ simulationImportId },
			{ transformInterpolationId },
			{ simDebugRenderId },
			{ pollEventsId },
			{ prepareImGuiFrameId },
			{ windowInputId },
			{ inputBindingId },
			{ windowInputBindingId },
			{ ghostControllerId },
			// { debugRenderCollidersId },
			{ bindSceneFramebufferId },
			{ renderDebugMeshId },
			{ bindWindowFramebufferId },
			{ renderSceneId },
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
		auto inputBindingId = addSystem<aoein::InputBindingSystem>(ecsSystems); // <- should this be in sim?
		auto carControllerId = addSystem<aoeph::CarControllerSystem>(ecsSystems);
		auto collisionId = addSystem<aoeph::CollisionSystem>(ecsSystems);
		auto debugRenderCollidersId = addSystem<aoeph::DebugRenderCollidersSystem>(ecsSystems);
		auto softFollowId = addSystem<aoest::SoftFollowSystem>(ecsSystems);
		auto simExportId = addSystem<SimulationExportSystem>(ecsSystems);
		auto fixedRateLimitingId = addSystem<aoest::FixedRateLimitingSystem>(ecsSystems);

		aoeng::EcsSchedule ecsSchedule({ { "Simulation", {
			{ timeId },
			{ presImportId },
			{ fixedRateTimeId },
			{ inputBindingId },
			{ carControllerId },
			{ collisionId },
			{ debugRenderCollidersId },
			{ softFollowId },
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
		multiDatabase.register_database(stringDatabase);
		multiDatabase.register_database(shaderProgramDatabase);

		auto simExchangeData = std::make_shared<EcsExchangeData>();
		auto presExchangeData = std::make_shared<EcsExchangeData>();

		// Prepare contexts
		entt::registry preRegistry;
		preRegistry.ctx().emplace<aoest::TimeContext>();
		preRegistry.ctx().emplace<SimulationExchangeContext>(simExchangeData);
		preRegistry.ctx().emplace<InterpolationContext>();
		preRegistry.ctx().emplace<aoewi::WindowInputContext>();
		preRegistry.ctx().emplace<aoewi::WindowContext>(a_window);
		preRegistry.ctx().emplace<aoegl::DebugMeshContext>();
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

		preRegistry.ctx().emplace<aoein::Inputs>();
		auto& preInputBindings = preRegistry.ctx().emplace<aoein::InputBindings>();
		auto& cameraDirectorContext = preRegistry.ctx().emplace<aoegl::CameraDirectorContext>();
		auto& sceneTextureContext = preRegistry.ctx().emplace<aoegl::SceneTextureContext>();
		{

			auto& sceneTexture = sceneTextureContext.texture;

			glGenTextures(1, &sceneTexture.texture);

			static constexpr uint32_t k_multiSampling = 1;
			auto const width = static_cast<GLsizei>(a_window.getSize().x * std::sqrt(k_multiSampling));
			auto const height = static_cast<GLsizei>(a_window.getSize().y * std::sqrt(k_multiSampling));

			glBindTexture(GL_TEXTURE_2D, sceneTexture.texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glGenRenderbuffers(1, &sceneTexture.renderbuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, sceneTexture.renderbuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

			glGenFramebuffers(1, &sceneTexture.framebuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, sceneTexture.framebuffer);

			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sceneTexture.texture, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneTexture.texture, 0);

			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, sceneTexture.renderbuffer);

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
		simRegistry.ctx().emplace<aoein::Inputs>();
		auto& inputBindings = simRegistry.ctx().emplace<aoein::InputBindings>();
		simRegistry.ctx().emplace<aoeph::CollisionContext>();

		// Prepare entities
		{
			auto ent = simRegistry.create();
			auto& pos = simRegistry.emplace<aoest::Position>(ent);
			auto& rot = simRegistry.emplace<aoest::Rotation>(ent, glm::quat());
			auto& col = simRegistry.emplace<aoeph::StaticCollider>(ent);
			auto& par = col.parts.emplace_back();
			par.triangles.emplace_back(
				glm::vec3{ -32.0f, 0.0f, -32.0f },
				glm::vec3{ -32.0f, 0.0f, 48.0f },
				glm::vec3{ 64.0f, 0.0f, -32.0f });
			col.bounds = { glm::vec3{-32.0f, 0.0f, -32.0f}, glm::vec3{64.0f, 0.0f, 64.0f} };
		}

		{
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
		{
			auto e = simRegistry.create();
			assert(e == preRegistry.create(e));
			simRegistry.emplace<aoest::Position>(e, glm::vec3{ 0.0f, 2.0f, 0.0f });
			simRegistry.emplace<aoest::Rotation>(e, glm::quat{});
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
			carCollider.wheels[1] = { glm::vec3{ 0.86299f, 0.3525f, -1.78209f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.182f, 0.364f, 0.364f }};
			// .turnFactor = 1.0f;
			// rear left wheel
			carCollider.wheels[2] = { glm::vec3{ -0.885f, 0.3525f, 1.2055f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.182f, 0.364f, 0.364f } };
			// rear right wheel
			carCollider.wheels[3] = { glm::vec3{ 0.885f, 0.3525f, 1.2055f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.182f, 0.364f, 0.364f } };
			carCollider.mass = 1'500.0f;
			carCollider.barycenterLocal = glm::vec3{ 0.0f, 0.0f, -0.288295f };
			carCollider.inertiaLocal = glm::mat3{ carCollider.mass / 5.0f };
			carCollider.inertiaLocal[0][0] *= (0.7f * 0.7f + 1.6f * 1.6f);
			carCollider.inertiaLocal[1][1] *= (1.6f * 1.6f + 0.9f * 0.9f);
			carCollider.inertiaLocal[2][2] *= (0.9f * 0.9f + 0.7f * 0.7f);
			carCollider.boundsHalfExtentsLocal = glm::vec3{ 3.5f };


			auto& carController = simRegistry.emplace<aoeph::CarControllerComponent>(e);
			carController.wheels[0].steeringFactor = 1.0f;
			carController.wheels[1].steeringFactor = 1.0f;
			carController.forwardInputId = inputBindings.switches.add(aoein::InputBindingUtils::makeSwitch(aoein::SwitchReference{ 0, aoein::Gamepad::Button::A }));
			carController.backwardInputId = inputBindings.switches.add(aoein::InputBindingUtils::makeSwitch(aoein::SwitchReference{ 0, aoein::Gamepad::Button::B }));
			carController.steeringInputId = inputBindings.axes.add(aoein::InputBindingUtils::makeAxis(aoein::AxisReference{ 0, aoein::Gamepad::Axis::LeftX }));
			carController.forwardInputValueId = simForwardInputValueId;
			carController.backwardInputValueId = simBackwardInputValueId;
			carController.steeringInputValueId = simSteeringInputValueId;

			preRegistry.emplace<aoest::Position>(e, simRegistry.get<aoest::Position>(e));
			preRegistry.emplace<aoest::Rotation>(e, simRegistry.get<aoest::Rotation>(e));
			preRegistry.emplace<aoeph::CarCollider>(e, simRegistry.get<aoeph::CarCollider>(e));
			preRegistry.emplace<InterpolatedPosition>(e);
			preRegistry.emplace<InterpolatedRotation>(e);
			preRegistry.emplace<InterpolationComponent>(e);
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
