
#include <vob/misc/std/vector_map.h>
#include <vob/_todo_/random/perlin.h>
#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include "DataHolder.h"
#include "vob/aoe/common/window/WorldWindowcomponent.h"
#include "vob/aoe/common/time/WorldTimecomponent.h"
#include "vob/aoe/common/input/WorldInputcomponent.h"
#include "vob/aoe/common/window/WorldCursorcomponent.h"
#include "vob/aoe/common/_render/Directorcomponent.h"
#include "vob/aoe/common/physic/WorldPhysiccomponent.h"
#include "vob/aoe/common/physic/DefaultDynamicsWorldHolder.h"
#include "vob/aoe/ecs/world.h"
#include "vob/aoe/common/physic/PhysicSystem.h"
#include "vob/aoe/common/test/TestSystem.h"
#include "vob/aoe/common/_render/RenderSystem.h"
#include "vob/aoe/common/time/TimeSystem.h"
#include "vob/aoe/common/window/WindowCursorSystem.h"
#include "vob/aoe/common/window/WindowInputSystem.h"
#include "vob/aoe/common/todo/SimpleControllerSystem.h"
#include "vob/aoe/common/_render/DefaultDirectorSystem.h"
#include "vob/aoe/common/time/LifetimeSystem.h"
#include "vob/aoe/common/map/HierarchySystem.h"
#include <vob/aoe/common/_render/OpenGl.h>
#include <vob/aoe/common/_render/Window.h>
#include <vob/aoe/common/_render/SceneFramebufferInitializer.h>

#include <vob/aoe/actor/simple_actor_system.h>
#include <vob/aoe/actor/test_end_life_action_system.h>

#include <vob/aoe/debug/debug_controller.h>

#include <vob/aoe/input/mapped_inputs_world_component.h>
#include <vob/aoe/input/physical_inputs_world_component.h>
#include <vob/aoe/input/mapped_inputs_system.h>
#include <vob/aoe/input/mouse_move_axis_mapping.h>
#include <vob/aoe/input/double_switch_axis_mapping.h>
#include <vob/aoe/input/shortcut_util.h>

#include <vob/aoe/physics/physic_system.h>

#include <vob/aoe/rendering/data/model_data_resource_manager.h>
#include <vob/aoe/rendering/data/texture_data_resource_manager.h>
#include <vob/aoe/rendering/systems/bind_scene_framebuffer_system.h>
#include <vob/aoe/rendering/systems/bind_window_framebuffer_system.h>
#include <vob/aoe/rendering/systems/model_data_resource_system.h>
#include <vob/aoe/rendering/systems/render_debug_mesh_system.h>
#include <vob/aoe/rendering/systems/render_models_system.h>
#include <vob/aoe/rendering/systems/render_scene_system.h>
#include <vob/aoe/rendering/world_components/debug_mesh_world_component.h>
#include <vob/aoe/rendering/world_components/debug_render_world_component.h>
#include <vob/aoe/rendering/world_components/director_world_component.h>
#include <vob/aoe/rendering/world_components/mesh_render_world_component.h>
#include <vob/aoe/rendering/world_components/post_process_render_world_component.h>
#include <vob/aoe/rendering/world_components/scene_texture_world_component.h>

#include <vob/aoe/spacetime/presentation_time_world_component.h>
#include <vob/aoe/spacetime/presentation_time_system.h>
#include <vob/aoe/spacetime/simulation_time_world_component.h>
#include <vob/aoe/spacetime/simulation_time_system.h>

#include <vob/aoe/terrain/procedural_terrain.h>

#include <vob/aoe/window/glfw_window.h>
#include <vob/aoe/window/poll_events_system.h>
#include <vob/aoe/window/swap_buffers_system.h>
#include <vob/aoe/window/window_input_system.h>

#include <vob/misc/hash/string_id_literals.h>
#include <vob/misc/physics/measure_literals.h>
#include <vob/misc/visitor/is_visitable.h>

#include <regex>
#include <memory_resource>
#include <filesystem>
#include <utility>

#include <vob/aoe/common/editor/EditorVisitor.h>


using namespace vob;
using namespace misph::literals;
using namespace mishs::literals;

const std::uint32_t g_width = 2048u;
const std::uint32_t g_height = 1024u;
const std::uint32_t g_multiSampling = 1;

template <typename TComponentSet>
void setup_world_components(
	TComponentSet& a_componentSet,
	aoe::DataHolder& a_data,
	aoewi::glfw_window& a_window
)
{
	// OLD
	/*
	a_componentSet.add<aoe::common::WorldWindowComponent>(a_oldWindow);
	*/
	a_componentSet.add<aoe::common::SceneRenderComponent>(
		a_data.renderTextureResourceManager
		, glm::ivec2{ g_width, g_height }
	);
	a_componentSet.add<aoe::common::ModelRenderComponent>(
		a_data.database
		, a_data.textureResourceManager
		, a_data.renderTextureResourceManager
		, a_data.staticModelResourceManager
		, a_data.modelShaderProgramResourceManager
		, glm::ivec2{ g_width, g_height }
	);
	a_componentSet.add<aoe::common::DebugSceneRenderComponent>(
		a_data.database
		, a_data.debugSceneShaderProgramResourceManager
		);
	a_componentSet.add<aoe::common::PostProcessRenderComponent>(
		a_data.database
		, a_data.postProcessShaderProgramResourceManager
		);
	a_componentSet.add<aoe::common::WorldTimeComponent>();
	a_componentSet.add<aoe::common::WorldInputComponent>();
	a_componentSet.add<aoe::common::WorldCursorComponent>();
	a_componentSet.add<aoe::common::DirectorComponent>();
	auto dynamicsWorldHolder = aoe::type::dynamic_type_clone<aoe::common::ADynamicsWorldHolder>(a_data.dynamicTypeCloner);
	dynamicsWorldHolder.init<aoe::common::DefaultDynamicsWorldHolder>();
	a_componentSet.add<aoe::common::WorldPhysicComponent>(std::move(dynamicsWorldHolder));
	a_componentSet.add<aoe::common::GuiRenderComponent>(
		a_data.database
		, a_data.guiShaderProgramResourceManager
		, a_data.guiMeshResourceManager
		, a_data.textureResourceManager
		);

	// ========================================================================================= //
	// NEW
	auto loadProgram = [&a_data](auto& a_program, char const* a_programPathCStr)
	{
		auto programPath = std::filesystem::path{ a_programPathCStr };
		auto programId = a_data.filesystemIndexer.get_runtime_id(programPath);
		auto programData = a_data.shaderProgramDatabase.find(programId);
		if (programData != nullptr)
		{
			vob::aoegl::create_program(*programData, a_program);
		}
	};

	a_componentSet.add<aoest::presentation_time_world_component>();
	a_componentSet.add<aoest::simulation_time_world_component>();

	a_componentSet.add<aoewi::window_world_component>(a_window);
	a_componentSet.add<aoegl::debug_mesh_world_component>();
	auto& debugRenderWorldComponent = a_componentSet.add<aoegl::debug_render_world_component>();
	loadProgram(debugRenderWorldComponent.m_debugProgram, "data/new/shaders/debug_program.json");

	a_componentSet.add<aoegl::director_world_component>();
	auto& meshRenderWorldComponent = a_componentSet.add<aoegl::mesh_render_world_component>();
	loadProgram(meshRenderWorldComponent.m_meshProgram, "data/new/shaders/mesh_program.json");

	auto& postProcessRenderWorldComponent =
		a_componentSet.add<aoegl::post_process_render_world_component>();
	loadProgram(
		postProcessRenderWorldComponent.m_postProcessProgram
		, "data/new/shaders/post_process_program.json");

	auto& sceneTextureWorldComponent = a_componentSet.add<aoegl::scene_texture_world_component>();
	{
		auto& sceneTexture = sceneTextureWorldComponent.m_sceneTexture;

		glGenTextures(1, &sceneTexture.m_texture);

		auto const width = static_cast<GLsizei>(g_width * std::sqrt(g_multiSampling));
		auto const height = static_cast<GLsizei>(g_height * std::sqrt(g_multiSampling));
		
		glBindTexture(GL_TEXTURE_2D, sceneTexture.m_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenRenderbuffers(1, &sceneTexture.m_renderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, sceneTexture.m_renderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

		glGenFramebuffers(1, &sceneTexture.m_framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, sceneTexture.m_framebuffer);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sceneTexture.m_texture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneTexture.m_texture, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, sceneTexture.m_renderbuffer);

		glClearColor(0.3f, 0.1f, 0.1f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/*aoegl::graphic_enum drawBuffer = GL_COLOR_ATTACHMENT0;
		glDrawBuffers(1, &drawBuffer);*/

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}

	a_componentSet.add<aoein::physical_inputs_world_component>();
	aoein::mapped_inputs_world_component& mappedInputWorldComponent = a_componentSet.add<aoein::mapped_inputs_world_component>();
	aoedb::debug_controller_world_component& debugControllerWorldComponent = a_componentSet.add<aoedb::debug_controller_world_component>();
	{
		debugControllerWorldComponent.m_yawMapping = mappedInputWorldComponent.m_axes.size();
		mappedInputWorldComponent.m_axes.emplace_back(
			mistd::polymorphic_ptr_util::make<aoein::mouse_move_axis_mapping>(
			aoein::mouse_move_axis_reference{ aoein::mouse::axis::X }, 1.0f));

		debugControllerWorldComponent.m_pitchMapping = mappedInputWorldComponent.m_axes.size();
		mappedInputWorldComponent.m_axes.emplace_back(
			mistd::polymorphic_ptr_util::make<aoein::mouse_move_axis_mapping>(
				aoein::mouse_move_axis_reference{ aoein::mouse::axis::Y }, 1.0f));

		debugControllerWorldComponent.m_enableViewMapping = mappedInputWorldComponent.m_switches.size();
		mappedInputWorldComponent.m_switches.emplace_back(aoein::shortcut_util::make_switch(
			aoein::mouse::button::Right));

		debugControllerWorldComponent.m_lateralMoveMapping = mappedInputWorldComponent.m_axes.size();
		mappedInputWorldComponent.m_axes.emplace_back(
			aoein::shortcut_util::make_axis(aoein::keyboard::key::S, aoein::keyboard::key::F));

		debugControllerWorldComponent.m_longitudinalMoveMapping = mappedInputWorldComponent.m_axes.size();
		mappedInputWorldComponent.m_axes.emplace_back(
			aoein::shortcut_util::make_axis(aoein::keyboard::key::E, aoein::keyboard::key::D));
		
		debugControllerWorldComponent.m_verticalMoveMapping = mappedInputWorldComponent.m_axes.size();
		mappedInputWorldComponent.m_axes.emplace_back(
			aoein::shortcut_util::make_axis(aoein::keyboard::key::Space, aoein::keyboard::key::LBracket));

		std::vector<aoein::keyboard::key> toggleKeys = {
			aoein::keyboard::key::Num6,
			aoein::keyboard::key::Y,
			aoein::keyboard::key::H,
			aoein::keyboard::key::N };
		std::vector<aoein::keyboard::key> frequencyUpKeys = {
			aoein::keyboard::key::Num7,
			aoein::keyboard::key::U,
			aoein::keyboard::key::J,
			aoein::keyboard::key::M };
		std::vector<aoein::keyboard::key> frequencyDownKeys = {
			aoein::keyboard::key::Num8,
			aoein::keyboard::key::I,
			aoein::keyboard::key::K,
			aoein::keyboard::key::Comma };
		std::vector<aoein::keyboard::key> heightUpKeys = {
			aoein::keyboard::key::Num9,
			aoein::keyboard::key::O,
			aoein::keyboard::key::L,
			aoein::keyboard::key::Period };
		std::vector<aoein::keyboard::key> heightDownKeys = {
			aoein::keyboard::key::Num0,
			aoein::keyboard::key::P,
			aoein::keyboard::key::Semicolon,
			aoein::keyboard::key::Slash };


		debugControllerWorldComponent.m_terrainSizeUpMapping = mappedInputWorldComponent.m_switches.size();
		mappedInputWorldComponent.m_switches.emplace_back(aoein::shortcut_util::make_switch(
			aoein::keyboard::key::RShift));
		debugControllerWorldComponent.m_terrainSizeDownMapping = mappedInputWorldComponent.m_switches.size();
		mappedInputWorldComponent.m_switches.emplace_back(aoein::shortcut_util::make_switch(
			aoein::keyboard::key::Quote));
		debugControllerWorldComponent.m_terrainCellSizeUpMapping = mappedInputWorldComponent.m_switches.size();
		mappedInputWorldComponent.m_switches.emplace_back(aoein::shortcut_util::make_switch(
			aoein::keyboard::key::LShift));
		debugControllerWorldComponent.m_terrainCellSizeDownMapping = mappedInputWorldComponent.m_switches.size();
		mappedInputWorldComponent.m_switches.emplace_back(aoein::shortcut_util::make_switch(
			aoein::keyboard::key::LControl));
		debugControllerWorldComponent.m_terrainUseSmoothShadingMapping = mappedInputWorldComponent.m_switches.size();
		mappedInputWorldComponent.m_switches.emplace_back(aoein::shortcut_util::make_switch(
			aoein::keyboard::key::RBracket));

		for (int i = 0; i < 4; ++i)
		{
			auto& layer = debugControllerWorldComponent.m_terrainLayers.emplace_back();
			layer.m_isEnabled = 0;
			layer.m_frequency = 0.01f * std::powf(3.0f, 0.0f + i);
			layer.m_height = 8.0f * std::powf(0.33f, 0.0f + i);
			layer.m_offset = glm::vec2{ 1.0f } * (i * layer.m_frequency) * 256.0f;

			layer.m_toggleMapping = mappedInputWorldComponent.m_switches.size();
			mappedInputWorldComponent.m_switches.emplace_back(aoein::shortcut_util::make_switch(toggleKeys[i]));
			layer.m_frequencyUpMapping = mappedInputWorldComponent.m_switches.size();
			mappedInputWorldComponent.m_switches.emplace_back(aoein::shortcut_util::make_switch(frequencyUpKeys[i]));
			layer.m_frequencyDownMapping = mappedInputWorldComponent.m_switches.size();
			mappedInputWorldComponent.m_switches.emplace_back(aoein::shortcut_util::make_switch(frequencyDownKeys[i]));
			layer.m_heightUpMapping = mappedInputWorldComponent.m_switches.size();
			mappedInputWorldComponent.m_switches.emplace_back(aoein::shortcut_util::make_switch(heightUpKeys[i]));
			layer.m_heightDownMapping = mappedInputWorldComponent.m_switches.size();
			mappedInputWorldComponent.m_switches.emplace_back(aoein::shortcut_util::make_switch(heightDownKeys[i]));
		}
	}

	auto& textureDataResourceManager = a_componentSet.add<aoegl::texture_data_resource_manager>();
	a_componentSet.add<aoegl::model_data_resource_manager>(textureDataResourceManager);

	a_componentSet.add<aoeph::world_physic_component>(a_data.m_dynamicsWorld);
	// ----------------------------------------------------------------------------------------- //
}

std::unique_ptr<aoecs::world> createGameWorld(
	aoe::DataHolder& a_data, aoewi::glfw_window& a_window)
{
	// v2
	aoecs::component_set worldComponents;
	setup_world_components(worldComponents, a_data, a_window);

	// Create world
	auto world = std::make_unique<aoecs::world>(std::move(worldComponents), a_data.componentListFactory);

	// OLD
	/*
	// Register Systems
	auto const timeSystemId = world->add_system<aoe::common::TimeSystem>();
	auto const windowInputSystemId = world->add_system<aoe::common::WindowInputSystem>();
	auto const windowCursorSystemId = world->add_system<aoe::common::WindowCursorSystem>();
	auto const simpleControllerSystemId = world->add_system<aoe::common::SimpleControllerSystem>();
	auto const renderSystemId = world->add_system<aoe::common::GameRenderSystem>();
	auto const testSystemId = world->add_system<aoe::common::TestSystem>();
	auto const oldPhysicSystemId = world->add_system<aoe::common::PhysicSystem>();
	auto const defaultDirectorSystemId = world->add_system<aoe::common::DefaultDirectorSystem>();
	auto const lifetimeSystemId = world->add_system<aoe::common::LifetimeSystem>();
	// auto const guiRenderSystemId = world->add_system<aoe::common::gui::RenderSystem>();
	auto const hierarchySystemId = world->add_system<aoe::common::HierarchySystem>();
	auto const localTransformSystemId = world->add_system<aoe::common::LocalTransformSystem>();
	auto const mappedInputsSystemId = world->add_system<aoein::mapped_input_system>();
	auto const simpleActorSystemId = world->add_system<aoeac::simple_actor_system>();
	auto const testEndLifeActionSystemId = world->add_system<aoeac::test_end_life_action_system>();
	
	world->set_schedule({
		mismt::thread_schedule{
			{windowInputSystemId, {timeSystemId}}
			, {mappedInputsSystemId, {}}
			, {windowCursorSystemId, {}}
			, {renderSystemId, {localTransformSystemId}}
		},
		mismt::thread_schedule{
			{timeSystemId,					{physicSystemId}}
			, {simpleControllerSystemId,	{windowCursorSystemId}}
			, {simpleActorSystemId}
			, {testEndLifeActionSystemId}
			, {testSystemId,				{}}
			, {defaultDirectorSystemId,     {}}
			, {localTransformSystemId,		{}}
			, {oldPhysicSystemId,			{renderSystemId}}
			, {lifetimeSystemId,			{}}
		//  , {guiRenderSystemId,			{}}
			, {hierarchySystemId,			{}}
		}
	});
	*/

	// ========================================================================================= //
	// NEW

	auto const presentationTimeSystemId = world->add_system<aoest::presentation_time_system>();
	auto const simulationTimeSystemId = world->add_system<aoest::simulation_time_system>();

	auto const pollEventsSystemId = world->add_system<aoewi::poll_events_system>();
	auto const windowInputSystemId = world->add_system<aoewi::window_input_system>();
	auto const mappedInputsSystemId = world->add_system<aoein::mapped_input_system>();
	auto const debugControllerSystemId = world->add_system<aoedb::debug_controller_system>();
	auto const physicSystemId = world->add_system<aoeph::physic_system>();


	auto const modelDataResourceSystemId = world->add_system<aoegl::model_data_resource_system>();
	
	auto const bindSceneFramebufferSystemId = world->add_system<aoegl::bind_scene_framebuffer_system>();
	auto const renderModelsSystemId = world->add_system<aoegl::render_models_system>();
	auto const renderDebugMeshSystemId = world->add_system<aoegl::render_debug_mesh_system>();
	
	auto const bindWindowFramebufferSystemId = world->add_system<aoegl::bind_window_framebuffer_system>();
	auto const renderSceneSystemId = world->add_system<aoegl::render_scene_system>();
	
	auto const swapBuffersSystemId = world->add_system<aoewi::swap_buffers_system>();
	mismt::thread_schedule mainThread{
		{presentationTimeSystemId, {}}
		, {simulationTimeSystemId, {}}
		, {pollEventsSystemId, {}}
		, {windowInputSystemId, {}}
		, {mappedInputsSystemId, {}}
		, {debugControllerSystemId, {}}
		, {modelDataResourceSystemId, {}}
		, {bindSceneFramebufferSystemId, {}}
		, {renderModelsSystemId, {}}
		, {renderDebugMeshSystemId, {}}
		, {bindWindowFramebufferSystemId, {}}
		, {renderSceneSystemId, {}}
		, {swapBuffersSystemId, {}}
	};
	// ----------------------------------------------------------------------------------------- //

	// Set schedule
	world->set_schedule({
		// v2
		std::move(mainThread),
		mismt::thread_schedule{
			{physicSystemId, {swapBuffersSystemId}}
		}
	});

	return std::move(world);
}

/* void initGameWorldGuiMap(aoe::DataHolder& a_data, aoecs::world& a_world)
{
    auto& cloner = a_data.dynamicTypeCloner;
    auto& database = a_data.database;
    auto& worldData = a_world.get_data();
    auto& systemSpawnManager = worldData.m_oldEntityManager.get_spawn_manager();

	_aoecs::component_manager canvasArk{ a_data.componentHolderCloner };
	auto& canvasComponent = canvasArk.add<aoe::common::CanvasComponent>(cloner);

	auto& guiMeshResourceManager = a_data.guiMeshResourceManager;

	auto& splitElement = canvasComponent.m_rootElement.init<aoe::common::SplitElement>(cloner);
	splitElement.m_firstSideSize = 300.0f;
	splitElement.m_firstSide = aoe::common::SplitElement::Side::Right;

	auto& textInputElement = splitElement.m_firstChildElement.init<aoe::common::TextInputElement>(guiMeshResourceManager);
	textInputElement.m_borderColor = glm::vec4{ 1.0f };
	textInputElement.m_borderWidth = glm::vec4{ 1.f, 15.f, 25.f, 125.f };
	textInputElement.setText(U"Bonjour");
	textInputElement.setSize(32);
	textInputElement.setFont(database.find<aoe::common::Font>(a_data.fileSystemIndexer.get_id("data/font.fnt")));

	systemSpawnManager.spawn(canvasArk);
}

void initOldGameWorldDefaultMap(aoe::DataHolder& a_data, aoecs::world& a_world)
{
	auto& worldData = a_world.get_data();
	auto& oldSystemSpawnManager = worldData.m_oldEntityManager.get_spawn_manager();

	// Load player
	auto playerArk = a_data.database.find<_aoecs::component_manager>(2);
	if (playerArk != nullptr)
	{
		auto& player = oldSystemSpawnManager.spawn(*playerArk);

		auto playerNeckArk = a_data.database.find<_aoecs::component_manager>(10);
		if (playerNeckArk != nullptr)
		{
			auto playerNeck = *playerNeckArk;
			auto hierarchy = playerNeck.get_component<aoe::common::HierarchyComponent>();
			hierarchy->m_parent = player.get_id();
			auto& neck = oldSystemSpawnManager.spawn(playerNeck);

			auto playerCameraArk = a_data.database.find<_aoecs::component_manager>(3);
			if (playerCameraArk != nullptr)
			{
				auto playerCamera = *playerCameraArk;
				auto hierarchy = playerCamera.get_component<aoe::common::HierarchyComponent>();
				hierarchy->m_parent = neck.get_id();
				oldSystemSpawnManager.spawn(playerCamera);
			}
		}

		auto destroyBulletInteractionArk = a_data.database.find<_aoecs::component_manager>(11);
		if (destroyBulletInteractionArk != nullptr)
		{
			auto& destroyBulletInteraction = oldSystemSpawnManager.spawn(*destroyBulletInteractionArk);
			auto actor = player.get_component<aoeac::actor_component>();
			actor->m_actions.push_back(destroyBulletInteraction.get_id());
		}

		auto canvas = player.get_component<aoe::common::CanvasComponent>();
	}

	auto const groundPath = std::filesystem::path{ "data/archetypes/grass.json" };
	auto groundArk = a_data.database.find<_aoecs::component_manager>(a_data.fileSystemIndexer.get_id(groundPath));
	if (groundArk != nullptr)
	{
		auto ground = *groundArk;

		constexpr int sizeX = 64;
		constexpr int sizeY = 64;
		
		auto mc = ground.get_component<aoe::common::ModelComponent>();

		std::vector<aoe::common::static_vertex> vertices;
		vertices.reserve(3 * sizeX * sizeY * 2 * 2);
		std::vector<aoe::common::triangle> triangles;
		triangles.reserve(sizeX * sizeY * 2 * 2);
		for (auto i = 0; i < sizeX; ++i)
		{
			for (auto j = 0; j < sizeY; ++j)
			{
				auto s = static_cast<std::uint32_t>(vertices.size());
				triangles.emplace_back(s, s + 1, s + 2);

				constexpr auto bs = float(64)/sizeX;
				constexpr auto sc = float(sizeX)/8;

				constexpr auto e = 4.0f;

				auto x0 = static_cast<float>(i);
				auto y0 = static_cast<float>(j);
				auto x1 = x0 + 1.0f;
				auto y1 = y0 + 1.0f;
				auto xn = x0 - 1.0f;
				auto yn = y0 - 1.0f;
				auto x2 = x1 + 1.0f;
				auto y2 = y1 + 1.0f;

				auto p = [bs, sc, e](float x, float y)
				{
					return glm::vec3{ bs * x, sc * rng::perlin(x/32, y/32), bs * y };
				};

				auto p0 = p(x0, y0);
				auto p1 = p(x1, y0);
				auto p2 = p(x0, y1);
				auto p3 = p(x1, y1);

				auto p01 = p(xn, y0);
				auto p10 = p(x2, y0);
				auto p02 = p(x0, yn);
				auto p13 = p(x1, yn);
				auto p23 = p(xn, y1);
				auto p32 = p(x2, y1);
				auto p20 = p(x0, y2);
				auto p31 = p(x1, y2);

				auto f = [](auto p0, auto p1, auto p2)
				{
					auto faceNormal = glm::cross(p1-p0, p2-p0);
					return faceNormal;
				};

				auto n = [&f](auto p0, auto p1, auto p2, auto p3, auto p4)
				{
					auto averageNormal =
						glm::normalize(f(p0, p1, p2) + f(p0, p2, p3) + f(p0, p3, p4) + f(p0, p4, p1));
					return -averageNormal;
				};
				
				auto n0 = n(p0, p1, p2, p01, p02);
				auto n1 = n(p1, p10, p3, p0, p13);
				auto n2 = n(p2, p3, p20, p23, p0);
				auto n3 = n(p3, p32, p31, p2, p1);
				//n0 = glm::normalize(f(p1, p2, p01) + f(p01, p02, p1));
				//n0 = glm::normalize(f(p10, p3, p0) + f(p0, p13, p10));
				//n0 = glm::normalize(f(p3, p20, p23) + f(p23, p0, p3));
				//n0 = glm::normalize(f(p32, p31, p2) + f(p2, p1, p32));

				//n1 = f(p1, p3, p0);
				//n2 = f(p3, p2, p1);
				//n3 = f(p2, p0, p3);
				//n1 = n0;
				//n2 = n0;
				//n3 = n0;

				auto tc0 = glm::vec2{ 0.0f, 0.0f };
				auto tc1 = glm::vec2{ 0.0f, 1.0f };
				auto tc2 = glm::vec2{ 1.0f, 0.0f };
				auto tc3 = glm::vec2{ 1.0f, 1.0f };

				auto t0 = glm::vec3{1.0f, 0.0f, 0.0f};
				auto t3 = glm::vec3{1.0f, 0.0f, 0.0f};

				vertices.emplace_back(p0, n0, tc0, t0);
				vertices.emplace_back(p1, n1, tc1, t0);
				vertices.emplace_back(p2, n2, tc2, t0);

				s = static_cast<std::uint32_t>(vertices.size());
				triangles.emplace_back(s, s + 1, s + 2);

				vertices.emplace_back(p3, n3, tc3, t3);
				vertices.emplace_back(p2, n2, tc2, t3);
				vertices.emplace_back(p1, n1, tc1, t3);
			}
		}

		std::vector<aoe::common::StaticMesh> meshes;
		meshes.emplace_back(std::move(vertices), std::move(triangles), 0);
		std::vector<aoe::common::old_material> materials = (*mc->m_model)->m_materials;
		mc->m_model = std::make_shared<aoe::common::GraphicResourceHandle<aoe::common::static_model>>(a_data.staticModelResourceManager, meshes, materials);

		auto rbc = ground.get_component<aoe::common::RigidBodyComponent>();
		static_cast<aoe::common::ModelShape&>(*rbc->m_collisionShape).setModel(mc->m_model);
		rbc->m_physicMaterial = std::make_shared<aoe::common::PhysicMaterial>();

		oldSystemSpawnManager.spawn(std::move(ground));
	}
} */

void initGameWorldDefaultMap(aoe::DataHolder& a_data, aoecs::world& a_world)
{
	auto& worldData = a_world.get_data();
	auto& entityManager = worldData.m_entityManager;
	auto& systemSpawnManager = entityManager.get_spawner();

	// Debug camera
	auto debugCameraArkPath = std::filesystem::path{ "data/new/archetypes/debug_camera.json" };
	auto debugCameraArkId = a_data.filesystemIndexer.get_runtime_id(debugCameraArkPath);
	auto debugCameraArk = a_data.componentSetDatabase.find(debugCameraArkId);
	if (debugCameraArk != nullptr)
	{
		vob::aoecs::entity_map::create_callback cb = [](vob::aoecs::entity_list::entity_view&) {};
		auto const debugCameraId = systemSpawnManager.spawn(*debugCameraArk, &cb);
		entityManager.update();

		auto& directorWorldComponent = *worldData.m_worldComponents.find<aoegl::director_world_component>();
		directorWorldComponent.m_activeCamera = debugCameraId;
	}
}

#include <filesystem>
#include <unordered_map>

void glfwErrorCallback(int code, const char* description)
{
	__debugbreak();
}

char const* xinputMapping = "78696e70757401000000000000000000,XInput Gamepad (GLFW)"
	",platform:Windows,a:b0,b:b1,x:b2,y:b3,leftshoulder:b4,rightshoulder:b5,back:b6,start:b7"
	",leftstick:b8,rightstick:b9,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:a4"
	",righttrigger:a5,dpup:h0.1,dpright:h0.2,dpdown:h0.4,dpleft:h0.8,";


#include <vob/aoe/test.h>
#include <vob/aoe/ecs/entity_map.h>

int main()
{
	// Create data
	aoe::DataHolder data;

	// Create game window
	glfwSetErrorCallback(glfwErrorCallback);
	if (!glfwInit())
	{
		return EXIT_FAILURE;
	}
	// TMP: xinput mapping not there by default?
	if (!glfwUpdateGamepadMappings(xinputMapping))
	{
		return EXIT_FAILURE;
	}
	std::cout << glfwGetVersionString() << std::endl;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_REFRESH_RATE, 0);

	{
		aoewi::glfw_window window{ glm::ivec2{ g_width, g_height }, "An Other Engine" };

		glfwMakeContextCurrent(window.get_native_handle());
		glfwSwapInterval(0);
		glEnable(GL_MULTISAMPLE);

		std::cout << glGetString(GL_VERSION) << std::endl;

		// OLD
		/*
		aoe::common::Window oldWindow{ g_width, g_height, "An Other Engine", nullptr, nullptr };
		glfwMakeContextCurrent(oldWindow.getNativeHandle());
		glfwSwapInterval(0);
		glEnable(GL_MULTISAMPLE);
		*/

		// Create game world
		auto gameWorld = createGameWorld(data, window);

		// Init old game world with default map
		// initOldGameWorldDefaultMap(data, *gameWorld);
		initGameWorldDefaultMap(data, *gameWorld);
		// initGameWorldGuiMap(data, *gameWorld);

		// Run game
		gameWorld->start();
	}

	// Destroy game window
	glfwTerminate();
}

/* ACTIONS
*		Use cases		|	move	|	time	|	cancel	|	partial	|	resume	|	count	|
* ----------------------+-----------+-----------+-----------+-----------+-----------+-----------+
*	plant carrots		|	no		|	yes		|	no		|	no		|	no		|	1		|
*	recolt carrots		|	no		|	yes		|	no		|	no		|	no		|	1		|
*	recolt apples		|	yes		|	yes		|	no		|	no		|	no		|	1		|
*	simple switch		|	yes		|	no		|	no		|	no		|	no		|	inf		|
*	complex switch		|	no		|	yes		|	yes		|	maybe?	|	maybe?	|	1|inf	|
*	pump water			|	a bit	|	yes		|	yes		|	yes		|	yes		|	inf?	|
*	eat plate			|	a bit	|	no		|	no		|	yes		|	kinda	|	n		|
*	grab in hands		|	maybe?	|	maybe?	|	yes		|	no		|	no		|	1		|
*	mine ore			|	no		|	yes		|	yes		|	yes		|	yes		|	n		|
* 
* Interactable
*	- interactions[]	
*		- type
* 
* Controllers:
*	- Player
*	- Network
*	- AI
* 
* Interactors:
*	- Humanoid
*		- head | face | neck
*		- (l | r) + (shoulder arm | forearm | hand | (finger + [0-4]))
*		- chest | abdominal | back
*		- (l | r) thigh | knee | leg | ankle | foot
*	- Quadruped
*		- (f | r) + (l | r) leg
* 
* 
* What:
*	- something you can do with in hand
*	- something you can do with in world
*	- something you can put on body
* 
* Things made for human interaction:
*	- button
*	- switch
*	- water pump
* 
* Things made to be grabbed:
*	- eat plate
*	- grab object in hands
* 
* Things from nature:
*	- recolt plants
*	- recolt fruit
*	- mine ore
* 
* ------------------------------------------------------------
* 
* Interaction Component (on Humanoid / Quadruped):
*	- list of in-range interactions
*	- register to Interactable World Component
* 
* Interactable Shape Component / World Component / System:
*	- computes overlap of ghost shapes
* 
* Plant Component / System:
*	- register to Interactable World Component
*	- when overlap detected, register interaction to list of in-range interactions
* 
* Interuptor Component
* 
* Player (Humanoid?) Controller (TBD):
*	- offers promp when something in list of in-range interaction & valid
* 
* ---------------------------------------------------
* 
* Activate button / switch : -> change state
* Recolt plant / fruit : -> change state + modify inventory
* Pump water : -> change state + modify inventory
* Eat plate : -> change state + modify stats
* Mine ore : -> change state + modify inventory
* 
* ---------------------------------------------------
* Interaction Entity:
*	- transform component
*	- interaction component
*		- can interact (
*		- is interacting
*		- interactor
*		- system (to make sure it's not used by 2 systems at once)
*	- whatever needed for logic
* 
* Interactor Entity:
*	- interactor component
*		- interaction list
* 
* Interactor Pawn:
*	
* 
*/

/* DATA / GRAPHIC OBJECTS
* 
*/

/* SPAWN
* - value 
*	+ can modify easy to spawn with custom init
*	- must copy from data (and copy is dropped once really spawned_)
* - reference
*	+ not necessary to copy
*	- if need to copy, how?
*	- not clear ref will be kept?
*	- what if owner destroyed?
* - raw pointer
*	+ not necessary to copy
*	+ clearer ref
*	- can be null
*	- what if owner destroyed?
* - weak_ptr
*	+ not necessary to copy
*	- need to allocate if custom init
*	- can be null
*	- what if owner destroyed?
* - shared_ptr
*	+ not necessary to copy
*	- need to allocate if custom init
*	- can be null
* - unique_ptr
*	+ not necessary to copy
*	- need to allocate allways?
*/