#include "DefaultWorld.h"

#include <vob/aoe/engine/EcsWorld.h>

#include <vob/aoe/spacetime/TimeSystem.h>
#include <vob/aoe/spacetime/FixedRateTimeSystem.h>
#include <vob/aoe/window/PollEventsSystem.h>
#include <vob/aoe/rendering/PrepareImGuiFrameSystem.h>
#include <vob/aoe/window/WindowInputSystem.h>
#include <vob/aoe/input/InputBindingSystem.h>
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

#pragma optimize("", off)

namespace vob
{
	template <typename TSystem>
	int32_t addSystem(std::vector<std::shared_ptr<aoeng::IEcsSystem>>& a_ecsSystems)
	{
		auto const id = static_cast<int32_t>(a_ecsSystems.size());
		a_ecsSystems.push_back(std::make_shared<aoeng::BasicEcsSystem<TSystem>>());
		return id;
	}

	std::pair<std::vector<std::shared_ptr<aoeng::IEcsSystem>>, aoeng::EcsSchedule> createSimulationEcsSystems()
	{
		std::vector<std::shared_ptr<aoeng::IEcsSystem>> ecsSystems;
		auto timeId = addSystem<aoest::TimeSystem>(ecsSystems);
		auto fixedRateTimeId = addSystem<aoest::FixedRateTimeSystem>(ecsSystems);
		auto pollEventsId = addSystem<aoewi::PollEventsSystem>(ecsSystems);
		auto prepareImGuiFrameId = addSystem<aoegl::PrepareImGuiFrameSystem>(ecsSystems);
		auto windowInputId = addSystem<aoewi::WindowInputSystem>(ecsSystems);
		auto inputBindingId = addSystem<aoein::InputBindingSystem>(ecsSystems);
		auto ghostControllerId = addSystem<aoedb::GhostControllerSystem>(ecsSystems);
		auto carControllerId = addSystem<aoeph::CarControllerSystem>(ecsSystems);
		auto collisionId = addSystem<aoeph::CollisionSystem>(ecsSystems);
		auto debugRenderCollidersId = addSystem<aoeph::DebugRenderCollidersSystem>(ecsSystems);
		auto softFollowId = addSystem<aoest::SoftFollowSystem>(ecsSystems);
		auto bindSceneFramebufferId = addSystem<aoegl::BindSceneFramebufferSystem>(ecsSystems);
		auto renderDebugMeshId = addSystem<aoegl::RenderDebugMeshSystem>(ecsSystems);
		auto bindWindowFramebufferId = addSystem<aoegl::BindWindowFramebufferSystem>(ecsSystems);
		auto renderSceneId = addSystem<aoegl::RenderSceneSystem>(ecsSystems);
		auto renderImGuiFrameId = addSystem<aoegl::RenderImGuiFrameSystem>(ecsSystems);
		auto swapBuffersId = addSystem<aoewi::SwapBuffersSystem>(ecsSystems);
		auto fixedRateLimitingId = addSystem<aoest::FixedRateLimitingSystem>(ecsSystems);

		aoeng::EcsSchedule ecsSchedule({ { "Main", {
			{ pollEventsId },
			{ prepareImGuiFrameId },
			{ timeId },
			{ fixedRateTimeId },
			{ windowInputId },
			{ inputBindingId },
			{ ghostControllerId },
			{ carControllerId },
			{ collisionId },
			{ debugRenderCollidersId },
			{ softFollowId },
			{ bindSceneFramebufferId },
			{ renderDebugMeshId },
			{ bindWindowFramebufferId },
			{ renderSceneId },
			{ renderImGuiFrameId },
			{ swapBuffersId },
			{ fixedRateLimitingId }
		} } });

		return { ecsSystems, ecsSchedule };
	}

	entt::registry createSimulationEcsRegistry(aoewi::IWindow& a_window)
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

		// Prepare contexts
		entt::registry ecsRegistry;
		ecsRegistry.ctx().emplace<aoest::TimeContext>();
		ecsRegistry.ctx().emplace<aoest::FixedRateTimeContext>();
		ecsRegistry.ctx().emplace<aoewi::WindowInputContext>();
		ecsRegistry.ctx().emplace<aoewi::WindowContext>(a_window);
		ecsRegistry.ctx().emplace<aoegl::DebugMeshContext>();
		ecsRegistry.ctx().emplace<aoein::Inputs>();
		auto& inputBindings = ecsRegistry.ctx().emplace<aoein::InputBindings>();
		ecsRegistry.ctx().emplace<aoeph::CollisionContext>();
		auto& cameraDirectorContext = ecsRegistry.ctx().emplace<aoegl::CameraDirectorContext>();
		auto& sceneTextureContext = ecsRegistry.ctx().emplace<aoegl::SceneTextureContext>();
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
		auto& debugRenderContext = ecsRegistry.ctx().emplace<aoegl::DebugRenderContext>();
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
					offsetof(aoegl::DebugVertex , position),
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
		auto& postProcessRenderContext = ecsRegistry.ctx().emplace<aoegl::PostProcessRenderContext>();
		{
			auto const postProcessProgramData = shaderProgramDatabase.find(filesystemIndexer.get_runtime_id("data/new/shaders/post_process_program.json"));
			if (postProcessProgramData != nullptr)
			{
				aoegl::createProgram(*postProcessProgramData, postProcessRenderContext.postProcessProgram);
			}
		}

		// Prepare entities
		{
			auto ent = ecsRegistry.create();
			auto& pos = ecsRegistry.emplace<aoest::Position>(ent);
			auto& rot = ecsRegistry.emplace<aoest::Rotation>(ent, glm::quat());
			auto& col = ecsRegistry.emplace<aoeph::StaticCollider>(ent);
			auto& par = col.parts.emplace_back();
			par.triangles.emplace_back(
				glm::vec3{ -32.0f, 0.0f, -32.0f },
				glm::vec3{ -32.0f, 0.0f, 48.0f },
				glm::vec3{ 64.0f, 0.0f, -32.0f });
			col.bounds = { glm::vec3{-32.0f, 0.0f, -32.0f}, glm::vec3{64.0f, 0.0f, 64.0f} };
		}
		entt::entity carEntity = ecsRegistry.create();
		{
			auto e = ecsRegistry.create();
			ecsRegistry.emplace<aoest::Position>(e, glm::vec3{ 0.0f, 0.0f, 10.0f });
			ecsRegistry.emplace<aoest::Rotation>(e, glm::quat{});
			auto& gcc = ecsRegistry.emplace<aoedb::GhostControllerComponent>(e);
			
			gcc.lateralMoveBinding = inputBindings.axes.add(aoein::InputBindingUtils::makeAxis(aoein::Keyboard::Key::S, aoein::Keyboard::Key::F));
			gcc.longitudinalMoveBinding = inputBindings.axes.add(aoein::InputBindingUtils::makeAxis(aoein::Keyboard::Key::E, aoein::Keyboard::Key::D));
			gcc.verticalMoveMapping = inputBindings.axes.add(aoein::InputBindingUtils::makeAxis(aoein::Keyboard::Key::W, aoein::Keyboard::Key::R));
			gcc.pitchBinding = inputBindings.axes.add(aoein::InputBindingUtils::makeDerivedAxis(aoein::Mouse::Axis::Y, 0.001f));
			gcc.yawBinding = inputBindings.axes.add(aoein::InputBindingUtils::makeDerivedAxis(aoein::Mouse::Axis::X, 0.001f));
			gcc.enableRotationBinding = inputBindings.switches.add(aoein::InputBindingUtils::makeSwitch(aoein::Mouse::Button::Right));
			gcc.decreaseSpeedBinding = inputBindings.switches.add(aoein::InputBindingUtils::makeSwitch(aoein::Mouse::Button::ScrollDown));
			gcc.increaseSpeedBinding = inputBindings.switches.add(aoein::InputBindingUtils::makeSwitch(aoein::Mouse::Button::ScrollUp));

			ecsRegistry.emplace<aoedb::IsControlledTag>(e);
			ecsRegistry.emplace<aoegl::CameraComponent>(e);
			cameraDirectorContext.activeCameraEntity = e;
		}
		{
			auto e = ecsRegistry.create();
			ecsRegistry.emplace<aoest::Position>(e, glm::vec3{ 0.0f, 2.0f, 0.0f });
			ecsRegistry.emplace<aoest::Rotation>(e, glm::quat{});
			auto& carCollider = ecsRegistry.emplace<aoeph::CarCollider>(e);
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


			auto& carController = ecsRegistry.emplace<aoeph::CarControllerComponent>(e);
			carController.wheels[0].steeringFactor = 1.0f;
			carController.wheels[1].steeringFactor = 1.0f;
			carController.forwardInputId = inputBindings.switches.add(aoein::InputBindingUtils::makeSwitch(aoein::SwitchReference{ 0, aoein::Gamepad::Button::A }));
			carController.backwardInputId = inputBindings.switches.add(aoein::InputBindingUtils::makeSwitch(aoein::SwitchReference{ 0, aoein::Gamepad::Button::B }));
			carController.steeringInputId = inputBindings.axes.add(aoein::InputBindingUtils::makeAxis(aoein::AxisReference{ 0, aoein::Gamepad::Axis::LeftX }));
		}

		return ecsRegistry;
	}

	std::shared_ptr<aoeng::IWorld> createDefaultWorld(aoewi::IWindow& a_window)
	{
		auto [simulationSystems, simulationSchedule] = createSimulationEcsSystems();

		auto simulationRegistry = createSimulationEcsRegistry(a_window);
		

		return std::make_shared<aoeng::EcsWorld>(simulationSystems, simulationSchedule, std::move(simulationRegistry));
	}
}
