#include "DefaultWorld.h"

#include "vob/aoe/debug/DebugDevblogSystem.h"
#include <vob/aoe/debug/DebugNameComponent.h>
#include <vob/aoe/debug/GhostControllerSystem.h>
#include <vob/aoe/engine/EcsWorld.h>
#include <vob/aoe/engine/MultiWorld.h>
#include <vob/aoe/engine/TracyFrameSystem.h>
#include <vob/aoe/exchange/EcsExchangeData.h>
#include <vob/aoe/input/DebugGameInputContext.h>
#include <vob/aoe/input/DebugGameInputSystem.h>
#include <vob/aoe/input/GameInputUtils.h>
#include <vob/aoe/input/InputBindingSystem.h>
#include <vob/aoe/input/InputBindingUtils.h>
#include <vob/aoe/input/WindowInputBindingSystem.h>
#include <vob/aoe/physics/CarControllerSystem.h>
#include <vob/aoe/physics/CarMaterialsComponent.h>
#include <vob/aoe/physics/CarMaterialsSystem.h>
#include <vob/aoe/physics/CollisionSystem.h>
#include <vob/aoe/physics/DebugCollisionSystem.h>
#include <vob/aoe/physics/MathUtils.h>
#include <vob/aoe/physics/StaticColliderComponent.h>
#include <vob/aoe/rendering/BindSceneFramebufferSystem.h>
#include <vob/aoe/rendering/BindWindowFramebufferSystem.h>
#include <vob/aoe/rendering/CarRigComponent.h>
#include <vob/aoe/rendering/CarRigSystem.h>
#include <vob/aoe/rendering/DebugCameraDirectorContext.h>
#include <vob/aoe/rendering/DebugCameraDirectorSystem.h>
#include <vob/aoe/rendering/DebugRenderLightsSystem.h>
#include <vob/aoe/rendering/GpuObjects.h>
#include <vob/aoe/rendering/ImageLoader.h>
#include <vob/aoe/rendering/LightComponent.h>
#include <vob/aoe/rendering/MaterialManagerContext.h>
#include <vob/aoe/rendering/ModelComponent.h>
#include <vob/aoe/rendering/ModelLoader.h>
#include <vob/aoe/rendering/ModelUtils.h>
#include <vob/aoe/rendering/PrepareImGuiFrameSystem.h>
#include <vob/aoe/rendering/ProgramData.h>
#include <vob/aoe/rendering/ProgramUtils.h>
#include <vob/aoe/rendering/RenderDebugMeshSystem.h>
#include <vob/aoe/rendering/RenderImGuiFrameSystem.h>
#include <vob/aoe/rendering/RenderSceneSystem.h>
#include <vob/aoe/rendering/StaticMesh.h>
#include <vob/aoe/rendering/StaticMeshLoader.h>
#include <vob/aoe/spacetime/AttachmentComponent.h>
#include <vob/aoe/spacetime/AttachmentSystem.h>
#include <vob/aoe/spacetime/DebugDisplaySimulationFrameTimeSystem.h>
#include <vob/aoe/spacetime/DebugFrameTimeContext.h>
#include <vob/aoe/spacetime/DebugSimulationFrameTimeHistoryContext.h>
#include <vob/aoe/spacetime/DebugTrackFrameTimeSystem.h>
#include <vob/aoe/spacetime/FixedRateLimitingSystem.h>
#include <vob/aoe/spacetime/FixedRateTimeSystem.h>
#include <vob/aoe/spacetime/InterpolatedTransform.h>
#include <vob/aoe/spacetime/InterpolationContext.h>
#include <vob/aoe/spacetime/InterpolationExchangeContext.h>
#include <vob/aoe/spacetime/InterpolationTimeComponent.h>
#include <vob/aoe/spacetime/SoftFollowComponent.h>
#include <vob/aoe/spacetime/SoftFollowSystem.h>
#include <vob/aoe/spacetime/TimeSystem.h>
#include <vob/aoe/spacetime/TransformInterpolationSystem.h>
#include <vob/aoe/window/PollEventsSystem.h>
#include <vob/aoe/window/SwapBuffersSystem.h>

#include <vob/misc/std/reflection_util.h>
#include <vob/misc/std/container_util.h>
#include <vob/misc/type/applicator.h>
#include <vob/misc/type/factory.h>
#include <vob/misc/type/registry.h>
#include <vob/misc/visitor/accept.h>

#include <vob/aoe/data/filesystem_database.h>
#include <vob/aoe/data/filesystem_indexer.h>
#include <vob/aoe/data/filesystem_util.h>
#include <vob/aoe/data/filesystem_visitor_context.h>
#include <vob/aoe/data/json_file_loader.h>
#include <vob/aoe/data/multi_database.h>
#include <vob/aoe/data/single_file_loader.h>
#include <vob/aoe/data/string_loader.h>

#include "glm/gtc/quaternion.hpp"
#include <entt/entt.hpp>
#include <imgui.h>

#include <random>
#include <unordered_map>
#include <vector>

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

#pragma optimize("", off )
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

	class PresentationExportSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
		{

		}

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
		{
			auto& excRegistry = m_presentationExchangeContext.get(a_wdap).data->beginStore();
			excRegistry.clear();

			if (!excRegistry.ctx().contains<aoein::PresentationInputContext>())
			{
				excRegistry.ctx().emplace<aoein::PresentationInputContext>();
			}
			auto& presentationInputCtx = excRegistry.ctx().get<aoein::PresentationInputContext>();
			presentationInputCtx.values.clear();
			auto& gameInputCtx = m_gameInputContext.get(a_wdap);
			for (auto& inputValue : gameInputCtx.getValues())
			{
				presentationInputCtx.values.push_back(inputValue);
			}

			m_excEventPool.addEvents(gameInputCtx.getEvents());

			m_presentationExchangeContext.get(a_wdap).data->endStore(m_excEventPool);
		}

	private:
		aoeng::EcsWorldContextRef<PresentationExchangeContext> m_presentationExchangeContext;
		aoeng::EcsWorldContextRef<aoein::GameInputContext> m_gameInputContext;

		// TODO: this could break thread-safety?
		mutable aoexc::EventPool m_excEventPool;
	};

	class PresentationImportSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
		{

		}

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
		{
			auto excData = m_presentationExchangeContext.get(a_wdap).data->tryBeginLoad();
			if (excData == std::nullopt)
			{
				return;
			}

			auto const& excRegistry = *excData->first;
			auto const& excEventPool = *excData->second;

			auto const& presentationInputBindingCtx = m_presentationInputBindingContext.get(a_wdap);
			auto const& presentationInputValues = excRegistry.ctx().get<aoein::PresentationInputContext>().values;
			auto& gameInputCtx = m_gameInputContext.get(a_wdap);
			for (auto [presInputValueId, simInputValueId] : presentationInputBindingCtx.inputValueIds)
			{
				gameInputCtx.setValue(simInputValueId, presentationInputValues[presInputValueId.id]);
			}

			gameInputCtx.flushEvents();
			m_presInputEvents.clear();
			excEventPool.pollEvents(m_presInputEvents);
			for (auto presInputEventId : m_presInputEvents)
			{
				if (presentationInputBindingCtx.inputEventIds.contains(presInputEventId))
				{
					gameInputCtx.addEvent(presentationInputBindingCtx.inputEventIds.at(presInputEventId));
				}
			}

			m_presentationExchangeContext.get(a_wdap).data->endLoad();
		}

	private:
		aoeng::EcsWorldContextRef<PresentationExchangeContext> m_presentationExchangeContext;
		aoeng::EcsWorldContextRef<aoein::PresentationInputBindingContext> m_presentationInputBindingContext;
		aoeng::EcsWorldContextRef<aoein::GameInputContext> m_gameInputContext;

		// TODO: this could break thread-safety?
		mutable aoexc::EventPool m_excEventPool;
		mutable std::vector<aoein::GameInputEventId> m_presInputEvents;
	};

	class SimulationExportSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
		{

		}

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
		{
			auto& excRegistry = m_simulationExchangeContext.get(a_wdap).data->beginStore();

			// TODO: For now destroying all entities that are gone, but if component can be dynamic this would need to extend to gone components.
			std::vector<entt::entity> toDestroy;
			for (auto const entity : excRegistry.view<entt::entity>())
			{
				if (!m_carEntities.get(a_wdap).contains(entity))
				{
					toDestroy.push_back(entity);
				}
			}
			excRegistry.destroy(toDestroy.begin(), toDestroy.end());
			
			// Car Colliders
			for (auto [entity, position, rotation, carCollider, carController] : m_carEntities.get(a_wdap).each())
			{
				if (!excRegistry.valid(entity))
				{
					assert(entity == excRegistry.create(entity));
					excRegistry.emplace<aoest::Position>(entity, position);
					excRegistry.emplace<aoest::Rotation>(entity, rotation);
					excRegistry.emplace<aoeph::CarCollider>(entity, carCollider);
					excRegistry.emplace<aoeph::CarControllerComponent>(entity, carController);
				}
				else
				{
					excRegistry.get<aoest::Position>(entity) = position;
					excRegistry.get<aoest::Rotation>(entity) = rotation;
					excRegistry.get<aoeph::CarCollider>(entity) = carCollider;
					excRegistry.get<aoeph::CarControllerComponent>(entity) = carController;
				}
			}


			// Interpolation
			if (!excRegistry.ctx().contains<aoest::TimeContext>())
			{
				excRegistry.ctx().emplace<aoest::TimeContext>();
			}
			excRegistry.ctx().get<aoest::TimeContext>() = m_timeContext.get(a_wdap);
			if (!excRegistry.ctx().contains<aoest::InterpolationExchangeContext>())
			{
				excRegistry.ctx().emplace<aoest::InterpolationExchangeContext>();
			}
			excRegistry.ctx().get<aoest::InterpolationExchangeContext>().targetTime = std::chrono::high_resolution_clock::now();

			auto const& debugFrameTimeContext = m_debugFrameTimeContext.get(a_wdap);
			m_debugSimulationFrameTimeEvents.clear();
			m_debugSimulationFrameTimeEvents.emplace_back(static_cast<float>(debugFrameTimeContext.frameTime.count()) / 1'000'000);
			m_excEventPool.addEvents(m_debugSimulationFrameTimeEvents);

			m_simulationExchangeContext.get(a_wdap).data->endStore(m_excEventPool);
		}

	private:
		aoeng::EcsWorldContextRef<aoest::TimeContext> m_timeContext;
		aoeng::EcsWorldContextRef<aoest::DebugFrameTimeContext const> m_debugFrameTimeContext;
		aoeng::EcsWorldContextRef<SimulationExchangeContext> m_simulationExchangeContext;
		aoeng::EcsWorldViewRef<aoest::Position, aoest::Rotation, aoeph::CarCollider, aoeph::CarControllerComponent> m_carEntities;

		// TODO: this could break thread-safety?
		mutable std::vector<aoest::DebugSimulationFrameTimeEvent> m_debugSimulationFrameTimeEvents;
		mutable entt::registry m_excRegistry;
		mutable aoexc::EventPool m_excEventPool;
	};

	class SimulationImportSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
		{

		}

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
		{
			auto excData = m_simulationExchangeContext.get(a_wdap).data->tryBeginLoad();
			if (excData == std::nullopt)
			{
				return;
			}
			auto const& excRegistry = *excData->first;
			auto const& excEvents = *excData->second;

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

			excEvents.pollEvents(m_debugSimulationFrameTimeEvents);
			auto& debugSimulationFrameTimeHistoryCtx = m_debugSimulationFrameTimeHistoryContext.get(a_wdap);
			for (auto const debugSimulationFrameTimeEvent : m_debugSimulationFrameTimeEvents)
			{
				debugSimulationFrameTimeHistoryCtx.durationsInMs[debugSimulationFrameTimeHistoryCtx.nextIndex] = debugSimulationFrameTimeEvent.durationInMs;
				debugSimulationFrameTimeHistoryCtx.nextIndex = (debugSimulationFrameTimeHistoryCtx.nextIndex + 1) % debugSimulationFrameTimeHistoryCtx.historyLength;
			}

			m_simulationExchangeContext.get(a_wdap).data->endLoad();
		}

	private:
		aoeng::EcsWorldContextRef<SimulationExchangeContext> m_simulationExchangeContext;
		aoeng::EcsWorldContextRef<aoest::TimeContext> m_timeContext;
		aoeng::EcsWorldContextRef<aoest::DebugSimulationFrameTimeHistoryContext> m_debugSimulationFrameTimeHistoryContext;
		aoeng::EcsWorldContextRef<aoest::InterpolationContext> m_interpolationContext;
		aoeng::EcsWorldViewRef<aoest::Position, aoest::Rotation, aoest::InterpolatedPosition, aoest::InterpolatedRotation, aoest::InterpolationTimeComponent, aoeph::CarCollider, aoeph::CarControllerComponent> m_carEntities;

		// TODO: is this ok/thread-safe? where does this belong to prevent allocating every time?
		mutable std::vector<aoest::DebugSimulationFrameTimeEvent> m_debugSimulationFrameTimeEvents;
	};

	template <typename TSystem>
	int32_t addSystem(std::vector<std::shared_ptr<aoeng::IEcsSystem>>& a_ecsSystems)
	{
		auto const id = mistd::isize(a_ecsSystems);
		a_ecsSystems.push_back(std::make_shared<aoeng::BasicEcsSystem<TSystem>>());
		return id;
	}

	std::pair<std::vector<std::shared_ptr<aoeng::IEcsSystem>>, aoeng::EcsSchedule> createPresentationEcsSystems()
	{
		std::vector<std::shared_ptr<aoeng::IEcsSystem>> ecsSystems;
		auto tracyFrameId = addSystem<aoeng::TracyFrameSystem>(ecsSystems);
		auto timeId = addSystem<aoest::TimeSystem>(ecsSystems);
		auto attachmentId = addSystem<aoest::AttachmentSystem>(ecsSystems);
		auto softFollowId = addSystem<aoest::SoftFollowSystem>(ecsSystems);
		auto pollEventsId = addSystem<aoewi::PollEventsSystem>(ecsSystems);
		auto prepareImGuiFrameId = addSystem<aoegl::PrepareImGuiFrameSystem>(ecsSystems);
		auto windowInputBindingId = addSystem<aoein::WindowInputBindingSystem>(ecsSystems);
		auto debugGameInputId = addSystem<aoein::DebugGameInputSystem>(ecsSystems);
		auto debugCameraDirectorId = addSystem<aoegl::DebugCameraDirectorSystem>(ecsSystems);
		auto debugDevblogId = addSystem<aoedb::DebugDevblogSystem>(ecsSystems);
		auto debugDisplaySimulationFrameTimeId = addSystem<aoest::DebugDisplaySimulationFrameTimeSystem>(ecsSystems);
		auto ghostControllerId = addSystem<aoedb::GhostControllerSystem>(ecsSystems);
		auto carMaterialsId = addSystem<aoeph::CarMaterialsSystem>(ecsSystems);
		auto carRigId = addSystem<aoegl::CarRigSystem>(ecsSystems);

		auto renderSceneId = addSystem<aoegl::RenderSceneSystem>(ecsSystems);

		auto debugCollisionSystemId = addSystem<aoeph::DebugCollisionSystem>(ecsSystems);

		auto debugRenderLightsId = addSystem<aoegl::DebugRenderLightsSystem>(ecsSystems);
		auto renderDebugMeshId = addSystem<aoegl::RenderDebugMeshSystem>(ecsSystems);
		auto renderImGuiFrameId = addSystem<aoegl::RenderImGuiFrameSystem>(ecsSystems);

		auto swapBuffersId = addSystem<aoewi::SwapBuffersSystem>(ecsSystems);

		auto simulationImportId = addSystem<SimulationImportSystem>(ecsSystems);
		auto transformInterpolationId = addSystem<aoest::TransformInterpolationSystem>(ecsSystems);
		auto presExportId = addSystem<PresentationExportSystem>(ecsSystems);

		aoeng::EcsSchedule ecsSchedule({ { "Presentation", {
			{ tracyFrameId },
			{ timeId },
			{ simulationImportId },
			{ transformInterpolationId },
			{ prepareImGuiFrameId },
			{ carMaterialsId },
			{ carRigId },
			{ debugCollisionSystemId },
			{ attachmentId },
			{ softFollowId },
			{ pollEventsId },
			{ windowInputBindingId },
			{ debugGameInputId },
			{ debugCameraDirectorId },
			{ debugDevblogId },
			{ debugDisplaySimulationFrameTimeId },
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
		auto simExportId = addSystem<SimulationExportSystem>(ecsSystems);

		auto debugTrackFrameTimeId = addSystem<aoest::DebugTrackFrameTimeSystem>(ecsSystems);
		auto fixedRateLimitingId = addSystem<aoest::FixedRateLimitingSystem>(ecsSystems);

		aoeng::EcsSchedule ecsSchedule({ { "Simulation", {
			// TIME
			{ timeId },
			{ fixedRateTimeId },

			// IMPORT
			{ presImportId },

			// SIM
			{ carControllerId },
			{ collisionId },

			// EXPORT
			{ simExportId },

			// TIME
			{ debugTrackFrameTimeId },
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

	aoegl::StaticModelData colliderToModel(aoeph::StaticCollider const& a_collider)
	{
		aoegl::StaticModelData modelData;
		for (auto const& part : a_collider.parts)
		{
			auto& meshData = modelData.meshes.emplace_back();
			for (auto const& triangle : part.triangles)
			{
				auto const n = glm::normalize(glm::cross(triangle.p1 - triangle.p0, triangle.p2 - triangle.p0));
				auto const i = mistd::isize(meshData.vertices);
				meshData.vertices.emplace_back(triangle.p0, n);
				meshData.vertices.emplace_back(triangle.p1, n);
				meshData.vertices.emplace_back(triangle.p2, n);
				meshData.indices.emplace_back(i + 0);
				meshData.indices.emplace_back(i + 1);
				meshData.indices.emplace_back(i + 2);
			}
		}
		return modelData;
	}

	void colliderToComponent(
		glm::vec3 const& a_position,
		glm::quat const& a_rotation,
		aoeph::StaticCollider const& a_collider,
		aoeph::StaticColliderComponent& a_colliderCmp)
	{
		std::vector<glm::vec3> vertices;
		std::vector<aoeph::TriangleIndices> triangles;
		for (auto const& part : a_collider.parts)
		{
			vertices.clear();
			triangles.clear();

			for (auto const& triangle : part.triangles)
			{
				auto const i = mistd::isize(vertices);
				vertices.emplace_back(a_position + a_rotation * triangle.p0);
				vertices.emplace_back(a_position + a_rotation * triangle.p1);
				vertices.emplace_back(a_position + a_rotation * triangle.p2);
				triangles.emplace_back(i, i + 1, i + 2);
			}

			a_colliderCmp.parts.emplace_back(
				part.material,
				aoeph::BvhTriangles{ vertices, triangles, 4 }
			);
		}
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
		aoedt::filesystem_database<aoegl::StaticModelLoader> staticModelDatabase{ filesystemIndexer };
		aoedt::filesystem_database<aoegl::RiggedModelLoader> riggedModelDatabase{ filesystemIndexer };
		aoedt::filesystem_database<aoedt::single_file_loader<aoegl::ImageLoader>> imageDatabase{ filesystemIndexer };
		multiDatabase.register_database(stringDatabase);
		multiDatabase.register_database(shaderProgramDatabase);
		multiDatabase.register_database(imageDatabase);

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
		preRegistry.ctx().emplace<aoest::DebugSimulationFrameTimeHistoryContext>();
		preRegistry.ctx().emplace<PresentationExchangeContext>(presExchangeData);
		auto& presGameInputCtx = preRegistry.ctx().emplace<aoein::GameInputContext>();
		auto& presGameInputBindingCtx = preRegistry.ctx().emplace<aoein::GameInputBindingContext>();
		auto presForwardInputValueId = aoein::GameInputUtils::addInputValueBinding(presGameInputCtx, presGameInputBindingCtx,
			std::make_shared<aoein::GamepadButtonInputValueBinding>(0, aoein::Gamepad::Button::A));
		auto presBackwardInputValueId = aoein::GameInputUtils::addInputValueBinding(presGameInputCtx, presGameInputBindingCtx,
			std::make_shared<aoein::GamepadButtonInputValueBinding>(0, aoein::Gamepad::Button::B));
		auto presSteeringInputValueId = aoein::GameInputUtils::addInputValueBinding(presGameInputCtx, presGameInputBindingCtx,
			std::make_shared<aoein::GamepadAxisInputValueBinding>(0, aoein::Gamepad::Axis::LeftX, 0.05f /* dead zone */));
		auto presSetRespawnStateInputEventId = aoein::GameInputUtils::addInputEventBinding(presGameInputCtx, presGameInputBindingCtx,
			std::make_shared<aoein::GamepadButtonEventBinding>(0, aoein::Gamepad::Button::Up));
		auto presRespawnInputEventId = aoein::GameInputUtils::addInputEventBinding(presGameInputCtx, presGameInputBindingCtx,
			std::make_shared<aoein::GamepadButtonEventBinding>(0, aoein::Gamepad::Button::Y));
		auto presInstantBrakeInputEventId = aoein::GameInputUtils::addInputEventBinding(presGameInputCtx, presGameInputBindingCtx,
			std::make_shared<aoein::GamepadButtonEventBinding>(0, aoein::Gamepad::Button::X));

		auto presStepInputEventId = aoein::GameInputUtils::addInputEventBinding(presGameInputCtx, presGameInputBindingCtx,
			std::make_shared<aoein::GamepadButtonEventBinding>(0, aoein::Gamepad::Button::Down));
		auto presPlayInputEventId = aoein::GameInputUtils::addInputEventBinding(presGameInputCtx, presGameInputBindingCtx,
			std::make_shared<aoein::GamepadButtonEventBinding>(0, aoein::Gamepad::Button::Right));
		auto presRevertInputEventId = aoein::GameInputUtils::addInputEventBinding(presGameInputCtx, presGameInputBindingCtx,
			std::make_shared<aoein::GamepadButtonEventBinding>(0, aoein::Gamepad::Button::Left));

		auto presPrevCameraInputEventId = aoein::GameInputUtils::addInputEventBinding(presGameInputCtx, presGameInputBindingCtx,
			std::make_shared<aoein::GamepadButtonEventBinding>(0, aoein::Gamepad::Button::LB));
		auto presNextCameraInputEventId = aoein::GameInputUtils::addInputEventBinding(presGameInputCtx, presGameInputBindingCtx,
			std::make_shared<aoein::GamepadButtonEventBinding>(0, aoein::Gamepad::Button::RB));
		auto quitInputEventId = aoein::GameInputUtils::addInputEventBinding(presGameInputCtx, presGameInputBindingCtx,
			std::make_shared<aoein::GamepadButtonEventBinding>(0, aoein::Gamepad::Button::Start));

		auto& presDebugGameInputCtx = preRegistry.ctx().emplace<aoein::DebugGameInputContext>();
		presDebugGameInputCtx.values.emplace_back("Forward", presForwardInputValueId, std::pair{ 0.0f, 1.1f });
		presDebugGameInputCtx.values.emplace_back("Backward", presBackwardInputValueId, std::pair{ 0.0f, 1.1f });
		presDebugGameInputCtx.values.emplace_back("Steering", presSteeringInputValueId, std::pair{ -1.1f, 1.1f });
		presDebugGameInputCtx.events.emplace_back("Set Respawn State", presSetRespawnStateInputEventId);
		presDebugGameInputCtx.events.emplace_back("Respawn", presRespawnInputEventId);
		presDebugGameInputCtx.events.emplace_back("Instant Brake", presInstantBrakeInputEventId);
		presDebugGameInputCtx.events.emplace_back("Step Simulation", presStepInputEventId);
		presDebugGameInputCtx.events.emplace_back("Play Simulation", presPlayInputEventId);
		presDebugGameInputCtx.events.emplace_back("Revert Simulation", presRevertInputEventId);

		auto& debugCameraDirectorContext = preRegistry.ctx().emplace<aoegl::DebugCameraDirectorContext>();
		debugCameraDirectorContext.prevCameraInputEventId = presPrevCameraInputEventId;
		debugCameraDirectorContext.nextCameraInputEventId = presNextCameraInputEventId;
		debugCameraDirectorContext.quitInputEventId = quitInputEventId;

		auto& preInputBindings = preRegistry.ctx().emplace<aoein::InputBindings>();
		auto& cameraDirectorContext = preRegistry.ctx().emplace<aoegl::CameraDirectorContext>();
		auto& sceneTextureContext = preRegistry.ctx().emplace<aoegl::SceneTextureContext>();
		auto& materialManagerCtx = preRegistry.ctx().emplace<aoegl::MaterialManagerContext>(std::make_shared<aoegl::MaterialManager>());

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
			glGenQueries(static_cast<aoegl::GraphicSize>(renderSceneContext.timerQueries.size()), renderSceneContext.timerQueries.data());
		}
		{
			// SHADOWS
			glGenFramebuffers(mistd::isize(renderSceneContext.shadowMapFramebuffers), renderSceneContext.shadowMapFramebuffers.data());
		}

		auto const createTexture = [](auto const& image)
			{
				assert(1 <= image.channelCount && image.channelCount <= 4);

				static constexpr std::array<aoegl::GraphicEnum, 4> k_internalChannelFormats = { GL_R8, GL_RG8, GL_RGB8, GL_RGBA8 };
				static constexpr std::array<aoegl::GraphicEnum, 4> k_channelFormats = { GL_RED, GL_RG, GL_RGB, GL_RGBA };

				auto const width = image.size.x;
				auto const height = image.size.y;
				auto const internalChannelFormat = k_internalChannelFormats[image.channelCount - 1];
				auto const channelFormat = k_channelFormats[image.channelCount - 1];

				aoegl::GraphicId texture;
				glCreateTextures(GL_TEXTURE_2D, 1, &texture);
				auto const mipLevels = static_cast<int32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
				glTextureStorage2D(texture, mipLevels, internalChannelFormat, width, height);
				glTextureSubImage2D(texture, 0, 0, 0, width, height, channelFormat, GL_UNSIGNED_BYTE, image.data.data());
				glGenerateTextureMipmap(texture);

				glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				return texture;
			};

		aoegl::GraphicId asphaltDiffuseTexture = aoegl::k_invalidId;
		if (auto const image = imageDatabase.find(filesystemIndexer.get_runtime_id("data/new/images/Asphalt_D.png")))
		{
			asphaltDiffuseTexture = createTexture(*image);
		}

		aoegl::GraphicId asphaltNormalMapTexture = aoegl::k_invalidId;
		if (auto const image = imageDatabase.find(filesystemIndexer.get_runtime_id("data/new/images/Asphalt_N.png")))
		{
			asphaltNormalMapTexture = createTexture(*image);
		}

		aoegl::GraphicId asphaltMetallicRoughnessTexture = aoegl::k_invalidId;
		if (auto const image = imageDatabase.find(filesystemIndexer.get_runtime_id("data/new/images/Asphalt_MR.png")))
		{
			asphaltMetallicRoughnessTexture = createTexture(*image);
		}

		auto basicShadingSource = stringDatabase.find(filesystemIndexer.get_runtime_id("data/new/shaders/basic_shading.glsl"));
		auto basicStaticForwardProgram = aoegl::createForwardProgram(*basicShadingSource, false /* use rig */);
		auto basicRiggedForwardProgram = aoegl::createForwardProgram(*basicShadingSource, true /* use rig */);

		auto texturedShadingSource = stringDatabase.find(filesystemIndexer.get_runtime_id("data/new/shaders/texture_shading.glsl"));
		auto texturedStaticForwardProgram = aoegl::createForwardProgram(*texturedShadingSource, false /* use rig */);
		auto texturedRiggedForwardProgram = aoegl::createForwardProgram(*texturedShadingSource, true /* use rig */);

		std::vector<aoegl::GraphicId> asphaltTextureIds = { asphaltDiffuseTexture, asphaltNormalMapTexture, asphaltMetallicRoughnessTexture };
		auto const asphaltMaterialIndex = materialManagerCtx.materialManager->emplaceMaterial(aoegl::k_invalidId, std::move(asphaltTextureIds));

		struct alignas(16) BasicShadingParams
		{
			glm::vec4 albedo;
			float metallic;
			float roughness;
		};

		aoegl::GraphicId rubberParamsUbo;
		auto const rubberParams = BasicShadingParams{
			.albedo = glm::vec4{0.0f, 0.0f, 0.0f, 0.0f},
			.metallic = 0.0f,
			.roughness = 0.99f
		};
		glCreateBuffers(1, &rubberParamsUbo);
		glNamedBufferStorage(rubberParamsUbo, sizeof(rubberParams), &rubberParams, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
		auto const rubberMaterialIndex = materialManagerCtx.materialManager->emplaceMaterial(rubberParamsUbo);

		aoegl::GraphicId bluePaintedMetalParamsUbo;
		auto const bluePaintedMetalParams = BasicShadingParams{
			.albedo = glm::vec4{0.0f, 0.0f, 0.1f, 0.0f},
			.metallic = 0.7f,
			.roughness = 0.3f
		};
		glCreateBuffers(1, &bluePaintedMetalParamsUbo);
		glNamedBufferStorage(bluePaintedMetalParamsUbo, sizeof(bluePaintedMetalParams), &bluePaintedMetalParams, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
		auto const bluePaintedMetalMaterialIndex = materialManagerCtx.materialManager->emplaceMaterial(bluePaintedMetalParamsUbo);

		aoegl::GraphicId purpleMetalParamsUbo;
		auto const purpleMetalParams = BasicShadingParams{
			.albedo = glm::vec4{0.5f, 0.0f, 0.5f, 0.0f},
			.metallic = 0.9f,
			.roughness = 0.2f
		};
		glCreateBuffers(1, &purpleMetalParamsUbo);
		glNamedBufferStorage(purpleMetalParamsUbo, sizeof(purpleMetalParams), &purpleMetalParams, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
		auto const purpleMetalMaterialIndex = materialManagerCtx.materialManager->emplaceMaterial(purpleMetalParamsUbo);

		aoegl::GraphicId blackPaintedMetalParamsUbo;
		auto const blackPaintedMetalParams = BasicShadingParams{
			.albedo = glm::vec4{0.0f, 0.0f, 0.0f, 0.0f},
			.metallic = 0.7f,
			.roughness = 0.3f
		};
		glCreateBuffers(1, &blackPaintedMetalParamsUbo);
		glNamedBufferStorage(blackPaintedMetalParamsUbo, sizeof(blackPaintedMetalParams), &blackPaintedMetalParams, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
		auto const blackPaintedMetalMaterialIndex = materialManagerCtx.materialManager->emplaceMaterial(blackPaintedMetalParamsUbo);

		aoegl::GraphicId shinnyMetalParamsUbo;
		auto const shinnyMetalParams = BasicShadingParams{
			.albedo = glm::vec4{1.0f, 1.0f, 1.0f, 0.0f},
			.metallic = 0.75f,
			.roughness = 0.1f
		};
		glCreateBuffers(1, &shinnyMetalParamsUbo);
		glNamedBufferStorage(shinnyMetalParamsUbo, sizeof(shinnyMetalParams), &shinnyMetalParams, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
		auto const shinnyMetalMaterialIndex = materialManagerCtx.materialManager->emplaceMaterial(shinnyMetalParamsUbo);

		/*aoegl::GraphicId asphaltParamsUbo;
		auto const asphaltParams = BasicShadingParams{
			.albedo = glm::vec4{0.1f, 0.1f, 0.1f, 0.0f},
			.metallic = 0.0f,
			.roughness = 0.95f
		};
		glCreateBuffers(1, &asphaltParamsUbo);
		glNamedBufferStorage(asphaltParamsUbo, sizeof(asphaltParams), &asphaltParams, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
		auto const asphaltMaterialIndex = materialManagerCtx.materialManager->emplaceMaterial(asphaltParamsUbo);*/

		aoegl::GraphicId stepPlatformParamsUbo;
		auto const stepPlatformParams = BasicShadingParams{
			.albedo = glm::vec4{0.0f, 0.7f, 0.1f, 0.0f},
			.metallic = 0.3f,
			.roughness = 0.8f
		};
		glCreateBuffers(1, &stepPlatformParamsUbo);
		glNamedBufferStorage(stepPlatformParamsUbo, sizeof(stepPlatformParams), &stepPlatformParams, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
		auto const stepPlatformMaterialIndex = materialManagerCtx.materialManager->emplaceMaterial(stepPlatformParamsUbo);

		aoegl::GraphicId rampPlatformParamsUbo;
		auto const rampPlatformParams = BasicShadingParams{
			.albedo = glm::vec4{0.1f, 0.0f, 0.7f, 0.0f},
			.metallic = 0.7f,
			.roughness = 0.3f
		};
		glCreateBuffers(1, &rampPlatformParamsUbo);
		glNamedBufferStorage(rampPlatformParamsUbo, sizeof(rampPlatformParams), &rampPlatformParams, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
		auto const rampPlatformMaterialIndex = materialManagerCtx.materialManager->emplaceMaterial(rampPlatformParamsUbo);

		auto speedCarModelData = riggedModelDatabase.find(filesystemIndexer.get_runtime_id("data/new/models/BuggyF1.glb"));
		auto speedCarModel = aoegl::createRiggedModel(*speedCarModelData);
		{
			renderSceneContext.staticDepthProgram = aoegl::createDepthProgram(false /* use rig */);
			renderSceneContext.riggedDepthProgram = aoegl::createDepthProgram(true /* use rig */);
			renderSceneContext.lightClusteringWorkGroupSize = 128;
			renderSceneContext.lightClusteringProgram = aoegl::createLightClusteringProgram(renderSceneContext.lightClusteringWorkGroupSize);

			constexpr int32_t k_maxLightCount = 2048;
			static const glm::ivec2 k_lightClusterSize = glm::ivec2{ 16, 16 };
			constexpr int32_t k_lightClusterCountZ = 24;
			static const glm::ivec2 k_sceneFramebufferSize = a_window.getSize();
			constexpr int32_t k_maxLightCountPerCluster = 128;

			auto const lightClusterCountXY = glm::ivec2{ glm::ceil(glm::vec2{ k_sceneFramebufferSize } / glm::vec2{ k_lightClusterSize }) };
			auto const lightClusterCount = lightClusterCountXY.x * lightClusterCountXY.y * k_lightClusterCountZ;

			// Debug
			renderSceneContext.staticDebugForwardProgram = aoegl::createDebugForwardProgram(false /* use rig */);
			renderSceneContext.riggedDebugForwardProgram = aoegl::createDebugForwardProgram(true /* use rig */);
			auto const debugParams = aoegl::DebugParams{
				.mode = aoegl::DebugMode::None
			};
			glCreateBuffers(1, &renderSceneContext.debugParamsUbo);
			glNamedBufferStorage(renderSceneContext.debugParamsUbo, sizeof(debugParams), &debugParams, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);


			// Shared buffer objects
			glCreateBuffers(1, &renderSceneContext.viewParamsUbo);
			glNamedBufferStorage(renderSceneContext.viewParamsUbo, sizeof(aoegl::ViewParams), nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

			glCreateBuffers(1, &renderSceneContext.lightParamsUbo);
			glNamedBufferStorage(renderSceneContext.lightParamsUbo, sizeof(aoegl::LightParams), nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

			glCreateBuffers(1, &renderSceneContext.lightsSsbo);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, renderSceneContext.lightsSsbo);
			glNamedBufferStorage(renderSceneContext.lightsSsbo, k_maxLightCount * sizeof(aoegl::Light), nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

			glCreateBuffers(1, &renderSceneContext.lightClusterSizesSsbo);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, renderSceneContext.lightClusterSizesSsbo);
			glNamedBufferStorage(renderSceneContext.lightClusterSizesSsbo, lightClusterCount * sizeof(int32_t), nullptr, GL_DYNAMIC_STORAGE_BIT);

			glCreateBuffers(1, &renderSceneContext.lightClusterIndicesSsbo);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, renderSceneContext.lightClusterIndicesSsbo);
			glNamedBufferStorage(renderSceneContext.lightClusterIndicesSsbo, lightClusterCount * k_maxLightCountPerCluster * sizeof(int32_t), nullptr, GL_DYNAMIC_STORAGE_BIT);

		}

		entt::registry simRegistry;

		auto& simGameInputCtx = simRegistry.ctx().emplace<aoein::GameInputContext>();
		auto simForwardInputValueId = simGameInputCtx.registerValue();
		auto simBackwardInputValueId = simGameInputCtx.registerValue();
		auto simSteeringInputValueId = simGameInputCtx.registerValue();
		auto simSetRespawnStateInputEventId = simGameInputCtx.registerEvent();
		auto simRespawnInputEventId = simGameInputCtx.registerEvent();
		auto simInstantBrakeInputEventId = simGameInputCtx.registerEvent();
		auto simStepInputEventId = simGameInputCtx.registerEvent();
		auto simPlayInputEventId = simGameInputCtx.registerEvent();
		auto simRevertInputEventId = simGameInputCtx.registerEvent();
		simRegistry.ctx().emplace<PresentationExchangeContext>(presExchangeData);

		auto& simPresentationInputBindingCtx = simRegistry.ctx().emplace<aoein::PresentationInputBindingContext>();
		simPresentationInputBindingCtx.inputValueIds.emplace_back(presForwardInputValueId, simForwardInputValueId);
		simPresentationInputBindingCtx.inputValueIds.emplace_back(presBackwardInputValueId, simBackwardInputValueId);
		simPresentationInputBindingCtx.inputValueIds.emplace_back(presSteeringInputValueId, simSteeringInputValueId);
		simPresentationInputBindingCtx.inputEventIds.emplace(presSetRespawnStateInputEventId, simSetRespawnStateInputEventId);
		simPresentationInputBindingCtx.inputEventIds.emplace(presRespawnInputEventId, simRespawnInputEventId);
		simPresentationInputBindingCtx.inputEventIds.emplace(presInstantBrakeInputEventId, simInstantBrakeInputEventId);
		simPresentationInputBindingCtx.inputEventIds.emplace(presStepInputEventId, simStepInputEventId);
		simPresentationInputBindingCtx.inputEventIds.emplace(presPlayInputEventId, simPlayInputEventId);
		simPresentationInputBindingCtx.inputEventIds.emplace(presRevertInputEventId, simRevertInputEventId);

		simRegistry.ctx().emplace<aoest::TimeContext>();
		simRegistry.ctx().emplace<aoest::DebugFrameTimeContext>();
		simRegistry.ctx().emplace<SimulationExchangeContext>(simExchangeData);
		simRegistry.ctx().emplace<aoest::FixedRateTimeContext>();
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

		std::pmr::polymorphic_allocator<char> pmrCharAlloc{ std::pmr::get_default_resource() };

		// Prepare entities
		auto carEntity = preRegistry.create();
		assert(carEntity == simRegistry.create(carEntity));
		{
			// ==== GHOST CAMERA

			auto e = preRegistry.create();
			preRegistry.emplace<aoedb::DebugNameComponent>(e, "Ghost Controller");
			auto& pos = preRegistry.emplace<aoest::Position>(e, glm::vec3{ 0.0f, 1.0f, -15.0f });
			// pos = glm::vec3{ 380.0f, 0.0f, 600.0f };
			auto const dx = glm::normalize(glm::vec3{ 5.0f, 0.0f, 1.0f });
			auto const dy = glm::cross(glm::vec3{ 0.0f, 1.0f, 0.0f }, dx);
			// pos = glm::vec3{ 300.0f, 40.0f, 0.0f } + 8.0f * dx + 6.0f * dy;
			auto& rot = preRegistry.emplace<aoest::Rotation>(e, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
			// rot = glm::angleAxis(-std::numbers::pi_v<float> / 2.0f, glm::vec3{ 0.0f, 1.0f, 0.0f });
			rot = glm::angleAxis(-std::numbers::pi_v<float> / 2.0f, glm::vec3{ 1.0f, 0.0f, 0.0f });
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
			auto& cam = preRegistry.emplace<aoegl::CameraComponent>(e);
			cam.fov = glm::radians(40.0f);
			cameraDirectorContext.activeCameraEntity = e;
		}
		{
			// ==== FOLLOW CAMERA 1

			auto e = preRegistry.create();
			auto const mass = 1.0f;
			auto const elasticity = 4000.0f;
			preRegistry.emplace<aoedb::DebugNameComponent>(e, "Near Camera");
			preRegistry.emplace<aoest::Position>(e, glm::vec3{ 0.0f, 0.0f, 10.0f });
			preRegistry.emplace<aoest::Rotation>(e, glm::quat{});
			preRegistry.emplace<aoegl::CameraComponent>(e);
			preRegistry.emplace<aoest::SoftFollowComponent>(e,
				carEntity, glm::vec3{ 0.0f, 3.0f, 5.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, elasticity, mass, 2.0f * std::sqrt(elasticity * mass));
			//cameraDirectorContext.activeCameraEntity = e;
		}
		{
			// ==== FOLLOW CAMERA 1b

			auto e = preRegistry.create();
			auto const mass = 1.0f;
			auto const elasticity = 4000.0f;
			preRegistry.emplace<aoedb::DebugNameComponent>(e, "Far Camera");
			preRegistry.emplace<aoest::Position>(e, glm::vec3{ 0.0f, 0.0f, 10.0f });
			preRegistry.emplace<aoest::Rotation>(e, glm::quat{});
			preRegistry.emplace<aoegl::CameraComponent>(e);
			preRegistry.emplace<aoest::SoftFollowComponent>(e,
				carEntity, glm::vec3{ 0.0f, 6.0f, 10.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, elasticity, mass, 2.0f * std::sqrt(elasticity * mass));
			cameraDirectorContext.activeCameraEntity = e;
		}
		{
			// ==== FOLLOW CAMERA 2

			auto e = preRegistry.create();
			auto const mass = 1.0f;
			auto const elasticity = 4000.0f;
			preRegistry.emplace<aoedb::DebugNameComponent>(e, "Side Camera");
			preRegistry.emplace<aoest::Position>(e, glm::vec3{ 0.0f, 0.0f, -15.0f });
			preRegistry.emplace<aoest::Rotation>(e, glm::quat{});
			preRegistry.emplace<aoegl::CameraComponent>(e);
			preRegistry.emplace<aoest::SoftFollowComponent>(e,
				carEntity, glm::vec3{ 3.0f, 1.0f, 2.0f }, glm::vec3{ 0.0f, 0.0f, -2.0f }, elasticity, mass, 2.0f * std::sqrt(elasticity * mass));
			//cameraDirectorContext.activeCameraEntity = e;
		}
		{
			// ==== FOLLOW CAMERA 3

			auto e = preRegistry.create();
			auto const mass = 1.0f;
			auto const elasticity = 4000.0f;
			preRegistry.emplace<aoedb::DebugNameComponent>(e, "Top-Down Camera");
			preRegistry.emplace<aoest::Position>(e, glm::vec3{ 0.0f, 0.0f, -15.0f });
			preRegistry.emplace<aoest::Rotation>(e, glm::quat{});
			preRegistry.emplace<aoegl::CameraComponent>(e);
			preRegistry.emplace<aoest::SoftFollowComponent>(e,
				carEntity, glm::vec3{ 0.0f, 20.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, -0.5f }, elasticity, mass, 2.0f * std::sqrt(elasticity * mass));
			// cameraDirectorContext.activeCameraEntity = e;
		}
		{
			// ==== FOLLOW CAMERA 3

			auto e = preRegistry.create();
			auto const mass = 1.0f;
			auto const elasticity = 4000.0f;
			preRegistry.emplace<aoedb::DebugNameComponent>(e, "Micromachine Camera");
			preRegistry.emplace<aoest::Position>(e, glm::vec3{ 0.0f, 0.0f, -15.0f });
			preRegistry.emplace<aoest::Rotation>(e, glm::quat{});
			preRegistry.emplace<aoegl::CameraComponent>(e);
			preRegistry.emplace<aoest::SoftFollowComponent>(e,
				carEntity, glm::vec3{ 0.0f, 50.0f, 30.0f }, glm::vec3{ 0.0f, 0.0f, -0.5f }, elasticity, mass, 2.0f * std::sqrt(elasticity * mass));
			// cameraDirectorContext.activeCameraEntity = e;
		}
		{
			// ==== CAR

			auto e = carEntity;
			preRegistry.emplace<aoedb::DebugNameComponent>(e, "Local Player Car");
			auto& pos = simRegistry.emplace<aoest::Position>(e, glm::vec3{ 400.0f, 3.0f, 580.0f });
			pos = glm::vec3{ 00.0f, 3.0f, 0.0f };
			simRegistry.emplace<aoest::Rotation>(e, glm::angleAxis(-std::numbers::pi_v<float>, glm::vec3{ 0.0f, 1.0f, 0.0f }));
			auto& carCollider = simRegistry.emplace<aoeph::CarCollider>(e);
			// front axel
			carCollider.chassisParts.emplace_back(glm::vec3{ -0.01553f, 0.36325f, -1.75357f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.905f, 0.283f, 0.385f });
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
			carCollider.mass = 1'000.0f;
			carCollider.mass = 1'000.0f;
			carCollider.barycenterLocal = glm::vec3{ 0.0f, 0.0f, -0.288295f };
			carCollider.inertiaLocal = glm::mat3{ carCollider.mass / 12.0f };
			carCollider.inertiaLocal[0][0] *= (0.7f * 0.7f + 1.6f * 1.6f);
			carCollider.inertiaLocal[1][1] *= (1.6f * 1.6f + 0.9f * 0.9f);
			carCollider.inertiaLocal[2][2] *= (0.9f * 0.9f + 0.7f * 0.7f);

			carCollider.boundsCenterLocal = glm::vec3{ -0.00001f, 0.578155f, -0.10523f };
			carCollider.boundsHalfExtentsLocal = glm::vec3{ 1.045f, 0.589655f, 2.04086f };


			auto& carController = simRegistry.emplace<aoeph::CarControllerComponent>(e);
			carController.wheels[0].steeringFactor = 1.0f;
			carController.wheels[1].steeringFactor = 1.0f;
			carController.respawnPosition = simRegistry.get<aoest::Position>(e);
			carController.respawnRotation = simRegistry.get<aoest::Rotation>(e);
			carController.forwardInputValueId = simForwardInputValueId;
			carController.backwardInputValueId = simBackwardInputValueId;
			carController.steeringInputValueId = simSteeringInputValueId;
			carController.setRespawnStateInputEventId = simSetRespawnStateInputEventId;
			carController.respawnInputEventId = simRespawnInputEventId;
			carController.instantBrakeInputEventId = simInstantBrakeInputEventId;
			carController.stepInputEventId = simStepInputEventId;
			carController.playInputEventId = simPlayInputEventId;
			carController.revertInputEventId = simRevertInputEventId;

			preRegistry.emplace<aoest::Position>(e, simRegistry.get<aoest::Position>(e));
			preRegistry.emplace<aoest::Rotation>(e, simRegistry.get<aoest::Rotation>(e));
			preRegistry.emplace<aoeph::CarCollider>(e, simRegistry.get<aoeph::CarCollider>(e));
			preRegistry.emplace<aoeph::CarControllerComponent>(e, simRegistry.get<aoeph::CarControllerComponent>(e));
			preRegistry.emplace<aoest::InterpolatedPosition>(e).positions.fill(preRegistry.get<aoest::Position>(e));
			preRegistry.emplace<aoest::InterpolatedRotation>(e).rotations.fill(preRegistry.get<aoest::Rotation>(e));
			preRegistry.emplace<aoest::InterpolationTimeComponent>(e).times.fill(currentTime);



			auto& riggedModelCmp = preRegistry.emplace<aoegl::RiggedModelComponent>(e);
			auto meshIndex = 0;
			for (int32_t i = 0; i < mistd::isize(speedCarModel.meshes); ++i)
			{
				auto const& speedCarMeshData = speedCarModelData->meshes[i];
				auto const& speedCarMesh = speedCarModel.meshes[i];

				auto const materialUbo = [&]() {
					if (speedCarMeshData.materialName == std::string_view{ "RoughMetal" }) return blackPaintedMetalMaterialIndex;
					if (speedCarMeshData.materialName == std::string_view{ "PaintedMetal" }) return bluePaintedMetalMaterialIndex;
					if (speedCarMeshData.materialName == std::string_view{ "Metal" }) return shinnyMetalMaterialIndex;
					if (speedCarMeshData.materialName == std::string_view{ "Sticker" }) return purpleMetalMaterialIndex;
					if (speedCarMeshData.materialName == std::string_view{ "Rubber" }) return rubberMaterialIndex;
					return 0;
					}();

				riggedModelCmp.meshes.emplace_back(
					basicRiggedForwardProgram,
					materialUbo,
					speedCarMesh.vao,
					speedCarMesh.indexCount);
				++meshIndex;
			}
			glCreateBuffers(1, &riggedModelCmp.modelParamsUbo);
			auto const defaultModelParams = aoegl::ModelParams{ aoest::combine(simRegistry.get<aoest::Position>(e), simRegistry.get<aoest::Rotation>(e)) };
			glNamedBufferStorage(riggedModelCmp.modelParamsUbo, sizeof(aoegl::ModelParams), &defaultModelParams, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

			riggedModelCmp.boundingRadius = speedCarModel.boundingRadius;

			auto& carRigCmp = preRegistry.emplace<aoegl::CarRigComponent>(e);
			carRigCmp.boneTransforms.reserve(speedCarModelData->bones.size());
			carRigCmp.bones.reserve(speedCarModelData->bones.size());
			for (auto const& boneData : speedCarModelData->bones)
			{
				carRigCmp.boneTransforms.emplace_back(boneData.basePose);
				carRigCmp.bones.emplace_back(glm::mat4{ 1.0f });
			}

			glCreateBuffers(1, &riggedModelCmp.rigParamsUbo);
			glNamedBufferStorage(
				riggedModelCmp.rigParamsUbo,
				static_cast<int32_t>(carRigCmp.bones.size() * sizeof(glm::mat4)),
				carRigCmp.bones.data(),
				GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

			auto const findWheelByName = [&speedCarModelData](std::string_view const name)
				{
					return std::find_if(
						speedCarModelData->bones.begin(),
						speedCarModelData->bones.end(),
						[name](auto const& boneData) { return std::string_view{ boneData.name } == name; });
				};

			auto suspensionIndex = 0;
			for (auto const wheelName : { "Wheel.FL.Suspension", "Wheel.FR.Suspension", "Wheel.RL.Suspension", "Wheel.RR.Suspension" })
			{
				auto wheelIt = findWheelByName(wheelName);
				if (wheelIt != speedCarModelData->bones.end())
				{
					auto const parentBoneBasePose = speedCarModelData->bones[wheelIt->parentBoneIndex].basePose;
					auto const basePoseRelative = glm::inverse(parentBoneBasePose) * wheelIt->basePose;

					carRigCmp.suspensionWheels.emplace_back(
						suspensionIndex,
						static_cast<int32_t>(std::distance(speedCarModelData->bones.begin(), wheelIt)),
						wheelIt->parentBoneIndex,
						glm::inverse(wheelIt->basePose),
						basePoseRelative);
				}
				++suspensionIndex;
			}
			auto steeringIndex = 0;
			for (auto const wheelName : { "Wheel.FL.Steer", "Wheel.FR.Steer" })
			{
				auto wheelIt = findWheelByName(wheelName);
				if (wheelIt != speedCarModelData->bones.end())
				{
					auto const parentBoneBasePose = speedCarModelData->bones[wheelIt->parentBoneIndex].basePose;
					auto const basePoseRelative = glm::inverse(parentBoneBasePose) * wheelIt->basePose;

					carRigCmp.steeringWheels.emplace_back(
						steeringIndex,
						static_cast<int32_t>(std::distance(speedCarModelData->bones.begin(), wheelIt)),
						wheelIt->parentBoneIndex,
						glm::inverse(wheelIt->basePose),
						basePoseRelative);
				}
				++steeringIndex;
			}
			for (auto const wheelName : { "Wheel.FL", "Wheel.FR", "Wheel.RL", "Wheel.RR" })
			{
				auto wheelIt = findWheelByName(wheelName);
				if (wheelIt != speedCarModelData->bones.end())
				{
					auto const parentBoneBasePose = speedCarModelData->bones[wheelIt->parentBoneIndex].basePose;
					auto const basePoseRelative = glm::inverse(parentBoneBasePose) * wheelIt->basePose;

					carRigCmp.spinningWheels.emplace_back(
						0.364f /* wheel radius */,
						static_cast<int32_t>(std::distance(speedCarModelData->bones.begin(), wheelIt)),
						wheelIt->parentBoneIndex,
						glm::inverse(wheelIt->basePose),
						basePoseRelative);
				}
			}
		}
		{
			// ==== HEAD LIGHTS
			auto leftHeadLight = preRegistry.create();
			preRegistry.emplace<aoest::Position>(leftHeadLight);
			preRegistry.emplace<aoest::Rotation>(leftHeadLight);
			preRegistry.emplace<aoest::AttachmentComponent>(leftHeadLight, carEntity, glm::vec3{ -0.6f, 1.2f, -2.0f }, glm::angleAxis(-0.5f, glm::vec3{ 1.0f, 0.0f, 0.0f }));
			preRegistry.emplace<aoegl::LightComponent>(leftHeadLight,
				aoegl::LightComponent::Type::Spot,
				50.0f /* radius */,
				10.0f /* intensity */,
				glm::vec3{1.0f} /* color */,
				30.0f / 180.0f * std::numbers::pi_v<float> /* inner angle */,
				50.0f / 180.0f * std::numbers::pi_v<float> /* outer angle */);

			auto rightHeadLight = preRegistry.create();
			preRegistry.emplace<aoest::Position>(rightHeadLight);
			preRegistry.emplace<aoest::Rotation>(rightHeadLight);
			preRegistry.emplace<aoest::AttachmentComponent>(rightHeadLight, carEntity, glm::vec3{ 0.6f, 1.2f, -2.0f }, glm::angleAxis(-0.5f, glm::vec3{ 1.0f, 0.0f, 0.0f }));
			preRegistry.emplace<aoegl::LightComponent>(rightHeadLight,
				aoegl::LightComponent::Type::Spot,
				50.0f /* radius */,
				10.0f /* intensity */,
				glm::vec3{ 1.0f } /* color */,
				30.0f / 180.0f * std::numbers::pi_v<float> /* inner angle */,
				50.0f / 180.0f * std::numbers::pi_v<float> /* outer angle */);
		}

		constexpr float k_mapMinX = -120.0f;
		constexpr float k_mapMaxX = 120.0f;
		constexpr float k_mapMinZ = -1020.0f;
		constexpr float k_mapMaxZ = 20.0f;
		{
			// ==== STATIC COLLIDER

			auto e = preRegistry.create();
			assert(e == simRegistry.create(e));
			auto& pos = simRegistry.emplace<aoest::Position>(e);
			auto& rot = simRegistry.emplace<aoest::Rotation>(e, glm::quat(glm::vec3{ 0.0f }));
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

			std::vector<glm::vec3> vertices;
			std::vector<aoeph::TriangleIndices> triangles;
			vertices.emplace_back(-120.0f, 0.0f, 20.0f);
			vertices.emplace_back(120.0f, 0.0f, 20.0f);

			for (auto i = 0; i < 51; ++i)
			{
				vertices.emplace_back(-120.0f, 0.0f, -i * 20.0f);
				vertices.emplace_back(120.0f, 0.0f, -i * 20.0f);
				auto const s = mistd::isize(vertices) - 4;
				triangles.emplace_back(s + 0, s + 1, s + 2);
				triangles.emplace_back(s + 2, s + 1, s + 3);
			}

			preRegistry.emplace<aoedb::DebugNameComponent>(e, "Main Ground");
			auto& staticColliderCmp = simRegistry.emplace<aoeph::StaticColliderComponent>(e);
			auto& staticColliderPart = staticColliderCmp.parts.emplace_back(aoeph::Material{}, aoeph::BvhTriangles{ vertices, triangles, 4 });
			preRegistry.emplace<aoeph::StaticColliderComponent>(e, simRegistry.get<aoeph::StaticColliderComponent>(e));

			preRegistry.emplace<aoest::Position>(e, simRegistry.get<aoest::Position>(e));
			preRegistry.emplace<aoest::Rotation>(e, simRegistry.get<aoest::Rotation>(e));
			preRegistry.emplace<aoeph::StaticCollider>(e, col);

			auto groundModelData = staticModelDatabase.find(filesystemIndexer.get_runtime_id("data/new/models/Ground.glb"));
			auto groundModel = aoegl::createStaticModel(*groundModelData);

			auto& staticModelCmp = preRegistry.emplace<aoegl::StaticModelComponent>(e);
			for (auto const& groundMesh : groundModel.meshes)
			{
				staticModelCmp.meshes.emplace_back(
					texturedStaticForwardProgram,
					asphaltMaterialIndex,
					groundMesh.vao,
					groundMesh.indexCount);
			}
			glCreateBuffers(1, &staticModelCmp.modelParamsUbo);
			auto const defaultModelParams = aoegl::ModelParams{ aoest::combine(simRegistry.get<aoest::Position>(e), simRegistry.get<aoest::Rotation>(e)) };
			glNamedBufferStorage(staticModelCmp.modelParamsUbo, sizeof(aoegl::ModelParams), &defaultModelParams, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
			staticModelCmp.boundingRadius = groundModel.boundingRadius;

		}

		if (false)
		{
			// ==== FLOATING STEP PLATFORMS

			aoeph::StaticCollider staticCollider;
			auto const p0 = glm::vec3{ -2.0f, 0.0f, -2.0f };
			auto const p1 = glm::vec3{ -2.0f, 0.0f, 2.0f };
			auto const p2 = glm::vec3{ 2.0f, 0.0f, -2.0f };
			auto const p3 = glm::vec3{ 2.0f, 0.0f, 2.0f };
			auto& staticColliderPart = staticCollider.parts.emplace_back();
			staticColliderPart.triangles.emplace_back(p0, p1, p2);
			staticColliderPart.triangles.emplace_back(p3, p2, p1);
			glm::vec3 staticHalfExtents = glm::vec3{ 2.0f, 0.0f, 2.0f };
			aoegl::StaticModelData staticModelData = colliderToModel(staticCollider);
			auto const staticModel = aoegl::createStaticModel(staticModelData);

			constexpr int k_sectionCount = 20;
			constexpr float k_z = -100.0f;
			for (int i = 0; i < k_sectionCount; ++i)
			{
				float minX = k_mapMinX + i * (k_mapMaxX - k_mapMinX) / k_sectionCount;
				float maxX = k_mapMinX + (i+1) * (k_mapMaxX - k_mapMinX) / k_sectionCount;
				auto const y = 0.025f * (i + 1);

				auto e = preRegistry.create();
				assert(e == simRegistry.create(e));
				preRegistry.emplace<aoedb::DebugNameComponent>(e, std::pmr::string(std::format("Floating Step Platform {}", i)));
				auto& pos = simRegistry.emplace<aoest::Position>(e, glm::vec3{ 0.5f * (minX + maxX), y, k_z });
				auto& rot = simRegistry.emplace<aoest::Rotation>(e, glm::quat(glm::vec3{ 0.0f }));
				auto& col = simRegistry.emplace<aoeph::StaticCollider>(e, staticCollider);
				col.bounds = aoeph::computeBounds(pos, rot, staticHalfExtents);
				colliderToComponent(pos, rot, col, simRegistry.emplace<aoeph::StaticColliderComponent>(e));
				preRegistry.emplace<aoeph::StaticColliderComponent>(e, simRegistry.get<aoeph::StaticColliderComponent>(e));

				preRegistry.emplace<aoest::Position>(e, simRegistry.get<aoest::Position>(e));
				preRegistry.emplace<aoest::Rotation>(e, simRegistry.get<aoest::Rotation>(e));
				preRegistry.emplace<aoeph::StaticCollider>(e, col);

				auto& staticModelCmp = preRegistry.emplace<aoegl::StaticModelComponent>(e);
				for (auto const& staticMesh : staticModel.meshes)
				{
					staticModelCmp.meshes.emplace_back(
						basicStaticForwardProgram,
						stepPlatformMaterialIndex,
						staticMesh.vao,
						staticMesh.indexCount);
				}
				glCreateBuffers(1, &staticModelCmp.modelParamsUbo);
				auto const defaultModelParams = aoegl::ModelParams{ aoest::combine(pos, rot) };
				glNamedBufferStorage(staticModelCmp.modelParamsUbo, sizeof(aoegl::ModelParams), &defaultModelParams, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
				staticModelCmp.boundingRadius = staticModel.boundingRadius;
			}
		}

		{
			// ==== STEP PLATFORMS

			aoeph::StaticCollider staticCollider;
			auto const p0 = glm::vec3{ -2.0f, -2.0f, -2.0f };
			auto const p1 = glm::vec3{ -2.0f, -2.0f, 2.0f };
			auto const p2 = glm::vec3{ 2.0f, -2.0f, 2.0f };
			auto const p3 = glm::vec3{ 2.0f, -2.0f, -2.0f };
			auto const p4 = glm::vec3{ -2.0f, 0.0f, -2.0f };
			auto const p5 = glm::vec3{ -2.0f, 0.0f, 2.0f };
			auto const p6 = glm::vec3{ 2.0f, 0.0f, 2.0f };
			auto const p7 = glm::vec3{ 2.0f, 0.0f, -2.0f };
			
			auto& staticColliderPart = staticCollider.parts.emplace_back();
			staticColliderPart.triangles.emplace_back(p0, p1, p4);
			staticColliderPart.triangles.emplace_back(p4, p1, p5);
			staticColliderPart.triangles.emplace_back(p1, p2, p5);
			staticColliderPart.triangles.emplace_back(p5, p2, p6);
			staticColliderPart.triangles.emplace_back(p2, p3, p6);
			staticColliderPart.triangles.emplace_back(p6, p3, p7);
			staticColliderPart.triangles.emplace_back(p3, p0, p7);
			staticColliderPart.triangles.emplace_back(p7, p0, p4);
			staticColliderPart.triangles.emplace_back(p4, p5, p7);
			staticColliderPart.triangles.emplace_back(p7, p5, p6);
			glm::vec3 staticHalfExtents = glm::vec3{ 2.0f, 2.0f, 2.0f };
			aoegl::StaticModelData staticModelData = colliderToModel(staticCollider);
			auto const staticModel = aoegl::createStaticModel(staticModelData);

			constexpr int k_sectionCount = 20;
			constexpr float k_z = -50.0f;
			for (int i = 0; i < k_sectionCount; ++i)
			{
				float minX = k_mapMinX + i * (k_mapMaxX - k_mapMinX) / k_sectionCount;
				float maxX = k_mapMinX + (i + 1) * (k_mapMaxX - k_mapMinX) / k_sectionCount;
				auto const y = 0.025f * (i + 1);

				auto e = preRegistry.create();
				assert(e == simRegistry.create(e));
				preRegistry.emplace<aoedb::DebugNameComponent>(e, std::pmr::string(std::format("Step Platform {}", i)));
				auto& pos = simRegistry.emplace<aoest::Position>(e, glm::vec3{ 0.5f * (minX + maxX), y, k_z });
				auto& rot = simRegistry.emplace<aoest::Rotation>(e, glm::quat(glm::vec3{ 0.0f }));
				auto& col = simRegistry.emplace<aoeph::StaticCollider>(e, staticCollider);
				col.bounds = aoeph::computeBounds(pos, rot, staticHalfExtents);
				colliderToComponent(pos, rot, col, simRegistry.emplace<aoeph::StaticColliderComponent>(e));
				preRegistry.emplace<aoeph::StaticColliderComponent>(e, simRegistry.get<aoeph::StaticColliderComponent>(e));

				preRegistry.emplace<aoest::Position>(e, simRegistry.get<aoest::Position>(e));
				preRegistry.emplace<aoest::Rotation>(e, simRegistry.get<aoest::Rotation>(e));
				preRegistry.emplace<aoeph::StaticCollider>(e, col);

				auto& staticModelCmp = preRegistry.emplace<aoegl::StaticModelComponent>(e);
				for (auto const& staticMesh : staticModel.meshes)
				{
					staticModelCmp.meshes.emplace_back(
						basicStaticForwardProgram,
						stepPlatformMaterialIndex,
						staticMesh.vao,
						staticMesh.indexCount);
				}
				glCreateBuffers(1, &staticModelCmp.modelParamsUbo);
				auto const defaultModelParams = aoegl::ModelParams{ aoest::combine(pos, rot) };
				glNamedBufferStorage(staticModelCmp.modelParamsUbo, sizeof(aoegl::ModelParams), &defaultModelParams, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
				staticModelCmp.boundingRadius = staticModel.boundingRadius;
			}
		}

		struct TestStruct
		{
			enum class TestEnumA
			{
				Foo,
				Bar,
				Joh,
				Hel
			};
		};

		enum class TestEnumB : uint8_t
		{
			Foo,
			Bar,
			Joh,
			Hel
		};

		{
			// ==== RAMP PLATFORMS

			aoeph::StaticCollider staticCollider;
			auto const p0 = glm::vec3{ -2.0f, 0.0f, -8.0f };
			auto const p1 = glm::vec3{ -2.0f, 0.0f, 8.0f };
			auto const p2 = glm::vec3{ 2.0f, 0.0f, -8.0f };
			auto const p3 = glm::vec3{ 2.0f, 0.0f, 8.0f };
			auto& staticColliderPart = staticCollider.parts.emplace_back();
			staticColliderPart.triangles.emplace_back(p0, p1, p2);
			staticColliderPart.triangles.emplace_back(p3, p2, p1);
			glm::vec3 staticHalfExtents = glm::vec3{ 2.0f, 0.0f, 8.0f };
			aoegl::StaticModelData staticModelData = colliderToModel(staticCollider);
			auto const staticModel = aoegl::createStaticModel(staticModelData);

			constexpr int k_sectionCount = 20;
			constexpr float k_z = -200.0f;
			for (int i = 0; i < k_sectionCount; ++i)
			{
				float minX = k_mapMinX + i * (k_mapMaxX - k_mapMinX) / k_sectionCount;
				float maxX = k_mapMinX + (i + 1) * (k_mapMaxX - k_mapMinX) / k_sectionCount;
				auto const rx = 0.03f * (i + 1);

				auto e = preRegistry.create();
				assert(e == simRegistry.create(e));
				preRegistry.emplace<aoedb::DebugNameComponent>(e, std::pmr::string(std::format("Ramp Platform {}", i)));
				auto& pos = simRegistry.emplace<aoest::Position>(e, glm::vec3{ 0.5f * (minX + maxX), 0.0f, k_z });
				auto& rot = simRegistry.emplace<aoest::Rotation>(e, glm::angleAxis(rx, glm::vec3{ 1.0f, 0.0f, 0.0f }));
				auto& col = simRegistry.emplace<aoeph::StaticCollider>(e, staticCollider);
				col.bounds = aoeph::computeBounds(pos, rot, staticHalfExtents);
				colliderToComponent(pos, rot, col, simRegistry.emplace<aoeph::StaticColliderComponent>(e));
				preRegistry.emplace<aoeph::StaticColliderComponent>(e, simRegistry.get<aoeph::StaticColliderComponent>(e));

				preRegistry.emplace<aoest::Position>(e, simRegistry.get<aoest::Position>(e));
				preRegistry.emplace<aoest::Rotation>(e, simRegistry.get<aoest::Rotation>(e));
				preRegistry.emplace<aoeph::StaticCollider>(e, col);

				auto& staticModelCmp = preRegistry.emplace<aoegl::StaticModelComponent>(e);
				for (auto const& staticMesh : staticModel.meshes)
				{
					staticModelCmp.meshes.emplace_back(
						basicStaticForwardProgram,
						rampPlatformMaterialIndex,
						staticMesh.vao,
						staticMesh.indexCount);
				}
				glCreateBuffers(1, &staticModelCmp.modelParamsUbo);
				auto const defaultModelParams = aoegl::ModelParams{ aoest::combine(pos, rot) };
				glNamedBufferStorage(staticModelCmp.modelParamsUbo, sizeof(aoegl::ModelParams), &defaultModelParams, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
				staticModelCmp.boundingRadius = staticModel.boundingRadius;
			}
		}

		{
			// ==== CIRCUIT
			struct Point
			{
				glm::vec3 position;
				glm::vec3 weightedDirection;
				float destinationRatio = 1.0f;
				float tilt = 0.0f;
				float desiredStepLength = 2.0f;
				glm::vec3 up = glm::vec3{ 0.0f, 1.0f, 0.0f };
			};

			std::vector<Point> generators;
			generators.emplace_back(glm::vec3{ 300.0f, 1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 100.0f });
			generators.emplace_back(glm::vec3{ 400.0f, 11.0f, 200.0f }, glm::vec3{ 0.0f, 0.0f, 100.0f }, 1.0f /* destination ratio */, 0.6f /* tilt */, 0.25f /* desired step length */);
			generators.emplace_back(glm::vec3{ 100.0f, 11.0f, 300.0f }, glm::vec3{ -71.0f, 0.0f, 71.0f });
			generators.emplace_back(glm::vec3{ -300.0f, 11.0f, 100.0f }, glm::vec3{ 0.0f, 0.0f, -100.0f });
			generators.emplace_back(glm::vec3{ -400.0f, 11.0f, -100.0f }, glm::vec3{ -100.0f, 0.0f, 0.0f }, 1.0f, -0.2f, 0.25f);
			generators.emplace_back(glm::vec3{ -600.0f, 11.0f, -200.0f }, glm::vec3{ 0.0f, 0.0f, -100.0f }, 1.0f, 0.4f);
			generators.emplace_back(glm::vec3{ -100.0f, 1.0f, -300.0f }, glm::vec3{ 100.0f, 0.0f, 0.0f });
			generators.emplace_back(glm::vec3{ 100.0f, 0.1f, -400.0f }, glm::vec3{ 100.0f, 0.0f, 0.0f });
			generators.emplace_back(glm::vec3{ 300.0f, 0.1f, -200.0f }, glm::vec3{ 0.0f, 0.0f, 100.0f });

			constexpr float k_roadHalfWidth = 10.0f;
			constexpr float k_roadFenceHeight = 0.75f;
			constexpr float k_roadFenceWidth = 0.75f;
			constexpr float k_uvScale = 0.10f;

			float distance = 0.0f;
			for (int32_t i = 0; i < mistd::isize(generators); ++i)
			{
				std::vector<glm::vec3> newVertices;
				std::vector<aoeph::TriangleIndices> newTriangleIndices;

				aoegl::StaticModelData staticModelData;
				auto& staticMeshData = staticModelData.meshes.emplace_back();

				// Compute bezier points
				auto const generator0 = generators[i];
				auto const generator1 = generators[(i + 1) % generators.size()];
				auto const center = (generator0.position + generator1.position) / 2.0f;
				auto const p0 = generator0.position - center;
				auto const p1 = p0 + generator0.weightedDirection;
				auto const p3 = generator1.position - center;
				auto const p2 = p3 - generator0.destinationRatio * generator1.weightedDirection;

				// Compute bezier curve length
				float bezierLength = 0.0f;
				auto pLast = p0;
				for (int32_t j = 1; j <= 100; ++j)
				{
					auto const t = static_cast<float>(j) / 100;
					auto const t3 = t * t * t;
					auto const u = (1.0f - t);
					auto const u3 = u * u * u;
					auto const p = u3 * p0 + 3.0f * u * t * (u * p1 + t * p2) + t3 * p3;
					bezierLength += glm::length(pLast - p);
					pLast = p;
				}

				// Generate bezier curve sub-points
				auto const desiredBezierStepLength = (generator0.desiredStepLength + generator1.desiredStepLength) / 2.0f;
				auto const bezierStepCount = static_cast<int32_t>(std::ceil(bezierLength / desiredBezierStepLength));
				auto const initialForward = glm::normalize(generator0.weightedDirection);
				auto const initialRight = glm::vec3{ glm::rotate(glm::mat4(1.0f), generator0.tilt, initialForward) * glm::vec4(glm::cross(initialForward, generator0.up), 0.0f) };

				auto smoothstep = [](float t, float min, float max)
					{
						return min + (max - min) * t * t * (3.0f - 2.0f * t);
					};

				auto addMeshVertex = [&staticMeshData, &newVertices, &center](glm::vec3 const& p, glm::vec3 const& n, glm::vec2 const& uv, glm::vec3 const& t)
					{
						staticMeshData.vertices.emplace_back(p, n, uv, t);
						newVertices.push_back(p + center + glm::vec3{ 0.0f, 0.0f, 400.0f });
					};

				auto addMeshVertices = [&staticMeshData, &newVertices, &addMeshVertex](glm::vec3 p, glm::vec3 right, glm::vec3 up, float dist)
					{
						auto const uvY = -dist * k_uvScale;

						addMeshVertex(p - (k_roadHalfWidth + k_roadFenceWidth) * right + k_roadFenceHeight * up, up, glm::vec2{ -(k_roadFenceHeight + k_roadFenceWidth) * k_uvScale, uvY }, right);
						addMeshVertex(p - k_roadHalfWidth * right + k_roadFenceHeight * up, up, glm::vec2{ -k_roadFenceHeight * k_uvScale, uvY }, right);
						addMeshVertex(p - k_roadHalfWidth * right + k_roadFenceHeight * up, right, glm::vec2{ -k_roadFenceHeight * k_uvScale, uvY }, -up);
						addMeshVertex(p - k_roadHalfWidth * right, right, glm::vec2{ 0.0f, uvY }, -up);
						addMeshVertex(p - k_roadHalfWidth * right, up, glm::vec2{ 0.0f, uvY }, right);
						addMeshVertex(p + k_roadHalfWidth * right, up, glm::vec2{ 2.0f * k_roadHalfWidth * k_uvScale, uvY }, right);
						addMeshVertex(p + k_roadHalfWidth * right, -right, glm::vec2{ 2.0f * k_roadHalfWidth * k_uvScale, uvY }, up);
						addMeshVertex(p + k_roadHalfWidth * right + k_roadFenceHeight * up, -right, glm::vec2{ (2.0f * k_roadHalfWidth + k_roadFenceHeight) * k_uvScale, uvY }, up);
						addMeshVertex(p + k_roadHalfWidth * right + k_roadFenceHeight * up, up, glm::vec2{ (2.0f * k_roadHalfWidth + k_roadFenceHeight) * k_uvScale, uvY }, right);
						addMeshVertex(p + (k_roadHalfWidth + k_roadFenceWidth) * right + k_roadFenceHeight * up, up, glm::vec2{ (2.0f * k_roadHalfWidth + k_roadFenceHeight + k_roadFenceWidth) * k_uvScale, uvY }, right);
					};

				addMeshVertices(p0, initialRight, glm::cross(initialRight, initialForward), distance);

				auto lastP = p0;
				for (int32_t j = 1; j <= bezierStepCount; ++j)
				{
					auto const w0 = std::min(generator0.desiredStepLength, (generator0.desiredStepLength + generator1.desiredStepLength) / 2);
					auto const w1 = std::min(generator1.desiredStepLength, (generator0.desiredStepLength + generator1.desiredStepLength) / 2);
					auto const t = std::pow(static_cast<float>(j) / bezierStepCount, (2.0f * w1 + w0) / (2.0f * w0 + w1));
					auto const t3 = t * t * t;
					auto const u = (1.0f - t);
					auto const u3 = u * u * u;
					auto const p = u3 * p0 + 3.0f * u * t * (u * p1 + t * p2) + t3 * p3;
					auto const roughUp = u * generator0.up + t * generator1.up;
					auto const tilt = smoothstep(t, generator0.tilt, generator1.tilt);
					auto const forward = glm::normalize(3.0f * u * u * (p1 - p0) + 6.0f * u * t * (p2 - p1) + 3.0f * t * t * (p3 - p2));
					auto const right = glm::vec3{ glm::rotate(glm::mat4(1.0f), tilt, forward) * glm::vec4(glm::normalize(glm::cross(forward, roughUp)), 0.0f) };
					auto const up = glm::cross(right, forward);

					distance += glm::length(lastP - p);
					lastP = p;

					addMeshVertices(p, right, up, distance);
					auto const idx = mistd::isize(staticMeshData.vertices) - 20;
					for (int32_t k = 0; k < 5; ++k)
					{
						staticMeshData.indices.emplace_back(idx + 2 * k + 0);
						staticMeshData.indices.emplace_back(idx + 2 * k + 1);
						staticMeshData.indices.emplace_back(idx + 2 * k + 10);
						staticMeshData.indices.emplace_back(idx + 2 * k + 10);
						staticMeshData.indices.emplace_back(idx + 2 * k + 1);
						staticMeshData.indices.emplace_back(idx + 2 * k + 11);

						newTriangleIndices.emplace_back(idx + 2 * k + 0, idx + 2 * k + 1, idx + 2 * k + 10);
						newTriangleIndices.emplace_back(idx + 2 * k + 10, idx + 2 * k + 1, idx + 2 * k + 11);
					}
				}

				auto const staticModel = aoegl::createStaticModel(staticModelData);

				auto e = preRegistry.create();
				assert(e == simRegistry.create(e));
				preRegistry.emplace<aoedb::DebugNameComponent>(e, std::pmr::string(std::format("Circuit Section {}", i)));
				auto& pos = simRegistry.emplace<aoest::Position>(e, center + glm::vec3{ 0.0f, 0.0f, 400.0f });
				auto& rot = simRegistry.emplace<aoest::Rotation>(e, glm::quat(glm::vec3{ 0.0f }));
				auto& staticColliderCmp = simRegistry.emplace<aoeph::StaticColliderComponent>(e);
				staticColliderCmp.parts.emplace_back(aoeph::Material{}, aoeph::BvhTriangles{ newVertices, newTriangleIndices, 4 });
				preRegistry.emplace<aoeph::StaticColliderComponent>(e, simRegistry.get<aoeph::StaticColliderComponent>(e));

				auto& staticModelCmp = preRegistry.emplace<aoegl::StaticModelComponent>(e);
				for (auto const& staticMesh : staticModel.meshes)
				{
					staticModelCmp.meshes.emplace_back(
						texturedStaticForwardProgram,
						asphaltMaterialIndex,
						staticMesh.vao,
						staticMesh.indexCount);
				}
				glCreateBuffers(1, &staticModelCmp.modelParamsUbo);
				auto const defaultModelParams = aoegl::ModelParams{ aoest::combine(pos, rot) };
				glNamedBufferStorage(staticModelCmp.modelParamsUbo, sizeof(aoegl::ModelParams), &defaultModelParams, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
				staticModelCmp.boundingRadius = staticModel.boundingRadius;

				preRegistry.emplace<aoest::Position>(e, pos);
				preRegistry.emplace<aoest::Rotation>(e, rot);
			}
		}

		{
			// ==== CIRCUIT ACCESS RAMP

			aoeph::StaticCollider staticCollider;
			auto const p0 = glm::vec3{ -2.0f, 0.0f, -8.0f };
			auto const p1 = glm::vec3{ -2.0f, 0.0f, 8.0f };
			auto const p2 = glm::vec3{ 2.0f, 0.0f, -8.0f };
			auto const p3 = glm::vec3{ 2.0f, 0.0f, 8.0f };
			auto& staticColliderPart = staticCollider.parts.emplace_back();
			staticColliderPart.triangles.emplace_back(p0, p1, p2);
			staticColliderPart.triangles.emplace_back(p3, p2, p1);
			glm::vec3 staticHalfExtents = glm::vec3{ 2.0f, 0.0f, 8.0f };
			aoegl::StaticModelData staticModelData = colliderToModel(staticCollider);
			auto const staticModel = aoegl::createStaticModel(staticModelData);

			auto e = preRegistry.create();
			assert(e == simRegistry.create(e));
			preRegistry.emplace<aoedb::DebugNameComponent>(e, "Circuit Access Ramp");
			auto& pos = simRegistry.emplace<aoest::Position>(e, glm::vec3{ 45.0f, 0.0f, -2.5f });
			auto& rot = simRegistry.emplace<aoest::Rotation>(e, glm::angleAxis(1.0f, glm::vec3{ 0.0f, 1.0f, 0.0f }) * glm::angleAxis(-0.2f, glm::vec3{ 1.0f, 0.0f, 0.0f }));
			auto& col = simRegistry.emplace<aoeph::StaticCollider>(e, staticCollider);
			col.bounds = aoeph::computeBounds(pos, rot, staticHalfExtents);
			colliderToComponent(pos, rot, col, simRegistry.emplace<aoeph::StaticColliderComponent>(e));
			preRegistry.emplace<aoeph::StaticColliderComponent>(e, simRegistry.get<aoeph::StaticColliderComponent>(e));

			preRegistry.emplace<aoest::Position>(e, simRegistry.get<aoest::Position>(e));
			preRegistry.emplace<aoest::Rotation>(e, simRegistry.get<aoest::Rotation>(e));
			preRegistry.emplace<aoeph::StaticCollider>(e, col);

			auto& staticModelCmp = preRegistry.emplace<aoegl::StaticModelComponent>(e);
			for (auto const& staticMesh : staticModel.meshes)
			{
				staticModelCmp.meshes.emplace_back(
					basicStaticForwardProgram,
					rampPlatformMaterialIndex,
					staticMesh.vao,
					staticMesh.indexCount);
			}
			glCreateBuffers(1, &staticModelCmp.modelParamsUbo);
			auto const defaultModelParams = aoegl::ModelParams{ aoest::combine(pos, rot) };
			glNamedBufferStorage(staticModelCmp.modelParamsUbo, sizeof(aoegl::ModelParams), &defaultModelParams, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
			staticModelCmp.boundingRadius = staticModel.boundingRadius;
		}

		{
			auto streetLampModelData = staticModelDatabase.find(filesystemIndexer.get_runtime_id("data/new/models/StreetLamp.glb"));
			auto streetLampModel = aoegl::createStaticModel(*streetLampModelData);

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
					auto const x = k_mapMinX + (j + 0.5f + 0.0f * rng()) * ((k_mapMaxX - k_mapMinX) / k_cols);
					auto const y = 0.0f;
					auto const z = k_mapMinZ + (i + 0.5f + 0.0f * rng()) * ((k_mapMaxZ - k_mapMinZ) / k_rows);
					auto const r = glm::angleAxis(2.0f * std::numbers::pi_v<float> * (j % 2 == 0 ? 0.5f : 0.0f), glm::vec3{ 0.0f, 1.0f, 0.0f });

					auto const lamp = preRegistry.create();
					preRegistry.emplace<aoedb::DebugNameComponent>(lamp, std::pmr::string(std::format("Lamp {}", i)));
					auto& pos = preRegistry.emplace<aoest::Position>(lamp, glm::vec3{ x, y, z });
					auto& rot = preRegistry.emplace<aoest::Rotation>(lamp, r);
					assert(lamp == simRegistry.create(lamp));
					simRegistry.emplace<aoest::Position>(lamp, preRegistry.get<aoest::Position>(lamp));
					simRegistry.emplace<aoest::Rotation>(lamp, preRegistry.get<aoest::Rotation>(lamp));


					auto& staticModelCmp = preRegistry.emplace<aoegl::StaticModelComponent>(lamp);
					int meshIndex = 0;
					for (auto const& streetLampMesh : streetLampModel.meshes)
					{
						staticModelCmp.meshes.emplace_back(
							basicStaticForwardProgram,
							meshIndex == 0 ? blackPaintedMetalMaterialIndex : shinnyMetalMaterialIndex,
							streetLampMesh.vao,
							streetLampMesh.indexCount);
						++meshIndex;
					}
					glCreateBuffers(1, &staticModelCmp.modelParamsUbo);
					auto const defaultModelParams = aoegl::ModelParams{ aoest::combine(simRegistry.get<aoest::Position>(lamp), simRegistry.get<aoest::Rotation>(lamp)) };
					glNamedBufferStorage(staticModelCmp.modelParamsUbo, sizeof(aoegl::ModelParams), &defaultModelParams, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
					staticModelCmp.boundingRadius = streetLampModel.boundingRadius;

					auto& col = simRegistry.emplace<aoeph::StaticCollider>(lamp);
					auto& par = col.parts.emplace_back();

					static auto const rr = 0.1f;
					static auto const n = 8;
					for (auto k = 0; k < n; ++k)
					{
						auto const a0 = (2.0f * std::numbers::pi_v<float> * k) / n;
						auto const a1 = (2.0f * std::numbers::pi_v<float> *(k + 1)) / n;
						auto const c0 = rr * std::cos(a0);
						auto const c1 = rr * std::cos(a1);
						auto const s0 = rr * std::sin(a0);
						auto const s1 = rr * std::sin(a1);

						par.triangles.emplace_back(
							glm::vec3{ c1, 0.0f, s1 },
							glm::vec3{ c0, 0.0f, s0 },
							glm::vec3{ c0, 2.0f, s0 });
						par.triangles.emplace_back(
							glm::vec3{ c1, 0.0f, s1 },
							glm::vec3{ c0, 2.0f, s0 },
							glm::vec3{ c1, 2.0f, s1 });
					}
					col.bounds = { glm::vec3{x-.1f, 0.0f, z-.1f}, glm::vec3{x+.1f, 2.0f, z+.1f} };
					colliderToComponent(pos, rot, col, simRegistry.emplace<aoeph::StaticColliderComponent>(lamp));
					preRegistry.emplace<aoeph::StaticColliderComponent>(lamp, simRegistry.get<aoeph::StaticColliderComponent>(lamp));

					preRegistry.emplace<aoeph::StaticCollider>(lamp, col);

					auto const light = preRegistry.create();
					preRegistry.emplace<aoest::Position>(light, glm::vec3{ x, y, z } + r * glm::vec3{ -0.66f, 4.66f, 0.0f });
					preRegistry.emplace<aoest::Rotation>(light, r);
					preRegistry.emplace<aoegl::LightComponent>(light,
						aoegl::LightComponent::Type::Point,
						22.0f /* radius */,
						10.0f /* intensity */,
						glm::vec3{0.9f + 0.1f * rng(),0.6f + 0.2f * rng(),0.5f + 0.1f * rng()} /* color */);
				}
			}
		}

		if (false)
		{
			auto sphereModelData = staticModelDatabase.find(filesystemIndexer.get_runtime_id("data/new/models/Sphere.glb"));
			auto sphereModel = aoegl::createStaticModel(*sphereModelData);

			constexpr auto k_cols = 10;
			constexpr auto k_rows = 10;
			constexpr auto k_sets = 10;
			constexpr float k_dist = 0.02f;

			constexpr float k_minMetallic = 0.01f;
			constexpr float k_maxMetallic = 1.0f;
			constexpr float k_minRoughness = 0.01f;
			constexpr float k_maxRoughness = 1.0f;

			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
			auto rng = [&]() { return distribution(gen); };
			auto hsv2rgb = [](glm::vec3 hsv)
				{
					float kx = 1.0f;
					float ky = 2.0f / 3.0f;
					float kz = 1.0f / 3.0f;
					float kw = 3.0f;
					auto p = glm::vec3(hsv.x + kx - std::floor(hsv.x + kx), hsv.x + ky - std::floor(hsv.x + ky), hsv.x + kz - std::floor(hsv.x + kz)) * 6.0f - glm::vec3{ kw };
					auto r = hsv.y * std::clamp(p.x - kx, 0.0f, 1.0f) + (1.0f - hsv.y) * kx;
					auto g = hsv.y * std::clamp(p.y - kx, 0.0f, 1.0f) + (1.0f - hsv.y) * kx;
					auto b = hsv.y * std::clamp(p.z - kx, 0.0f, 1.0f) + (1.0f - hsv.y) * kx;
					return hsv.z * glm::vec3(r, g, b);
				};

			for (int i = 0; i < k_cols; ++i)
			{
				for (int j = 0; j < k_rows; ++j)
				{
					auto e = preRegistry.create();
					preRegistry.emplace<aoest::Position>(e, glm::vec3{ i * (k_dist + 2.0f * sphereModel.boundingRadius), sphereModel.boundingRadius + j * (k_dist + 2.0f * sphereModel.boundingRadius), 5.0f });
					preRegistry.emplace<aoest::Rotation>(e, glm::quat{});

					aoegl::GraphicId shadingParamsUbo;
					auto const shadingParams = BasicShadingParams{
						.albedo = glm::vec4{1.0f, 0.0f, 0.0f, 0.0f},
						.metallic = k_minMetallic + static_cast<float>(i) / k_cols * (k_maxMetallic - k_minMetallic),
						.roughness = k_minRoughness + static_cast<float>(j) / k_rows * (k_maxRoughness - k_minRoughness)
					};
					glCreateBuffers(1, &shadingParamsUbo);
					glNamedBufferStorage(shadingParamsUbo, sizeof(shadingParams), &shadingParams, GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT);
					auto const shadingMaterialIndex = materialManagerCtx.materialManager->emplaceMaterial(shadingParamsUbo);

					auto& staticModelCmp = preRegistry.emplace<aoegl::StaticModelComponent>(e);
					for (auto const& sphereMesh : sphereModel.meshes)
					{
						staticModelCmp.meshes.emplace_back(
							basicStaticForwardProgram,
							shadingMaterialIndex,
							sphereMesh.vao,
							sphereMesh.indexCount);
					}
					glCreateBuffers(1, &staticModelCmp.modelParamsUbo);
					auto const defaultModelParams = aoegl::ModelParams{ aoest::combine(preRegistry.get<aoest::Position>(e), preRegistry.get<aoest::Rotation>(e)) };
					glNamedBufferStorage(staticModelCmp.modelParamsUbo, sizeof(aoegl::ModelParams), &defaultModelParams, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
					staticModelCmp.boundingRadius = sphereModel.boundingRadius;
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
