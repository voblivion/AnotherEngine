
#include <vob/misc/std/vector_map.h>
#include <vob/_todo_/random/perlin.h>
#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include "DataHolder.h"
#include "vob/aoe/common/window/WorldWindowcomponent.h"
#include "vob/aoe/common/time/WorldTimecomponent.h"
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


#include <vob/aoe/engine/world.h>

#include <vob/aoe/input/bindings.h>
#include <vob/aoe/input/binding_system.h>
#include <vob/aoe/input/binding_util.h>
#include <vob/aoe/input/inputs.h>

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

#include <vob/aoe/spacetime/time_world_component.h>
#include <vob/aoe/spacetime/time_system.h>

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

void init_world_and_schedule(aoeng::world& a_world, mismt::pmr::schedule& a_schedule, aoe::DataHolder& a_data, aoewi::glfw_window& a_window)
{
	// world components
	{
		a_world.add_world_component<aoe::common::SceneRenderComponent>(
			a_data.renderTextureResourceManager,
			glm::ivec2{ g_width, g_height });

		a_world.add_world_component<aoe::common::ModelRenderComponent>(
			a_data.database,
			a_data.textureResourceManager,
			a_data.renderTextureResourceManager,
			a_data.staticModelResourceManager,
			a_data.modelShaderProgramResourceManager,
			glm::ivec2{ g_width, g_height });
		a_world.add_world_component<aoe::common::DebugSceneRenderComponent>(
			a_data.database,
			a_data.debugSceneShaderProgramResourceManager);
		a_world.add_world_component<aoe::common::PostProcessRenderComponent>(
			a_data.database,
			a_data.postProcessShaderProgramResourceManager);
		a_world.add_world_component<aoe::common::WorldTimeComponent>();
		a_world.add_world_component<aoe::common::WorldInputComponent>();
		a_world.add_world_component<aoe::common::WorldCursorComponent>();
		a_world.add_world_component<aoe::common::DirectorComponent>();
		auto dynamicsWorldHolder = aoe::type::dynamic_type_clone<aoe::common::ADynamicsWorldHolder>(a_data.dynamicTypeCloner);
		dynamicsWorldHolder.init<aoe::common::DefaultDynamicsWorldHolder>();
		a_world.add_world_component<aoe::common::WorldPhysicComponent>(std::move(dynamicsWorldHolder));
		a_world.add_world_component<aoe::common::GuiRenderComponent>(
			a_data.database,
			a_data.guiShaderProgramResourceManager,
			a_data.guiMeshResourceManager,
			a_data.textureResourceManager);
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

		a_world.add_world_component<aoest::presentation_time_world_component>();
		a_world.add_world_component<aoest::simulation_time_world_component>();

		a_world.add_world_component<aoewi::window_world_component>(a_window);
		a_world.add_world_component<aoegl::debug_mesh_world_component>();
		auto& debugRenderWorldComponent = a_world.add_world_component<aoegl::debug_render_world_component>();
		loadProgram(debugRenderWorldComponent.m_debugProgram, "data/new/shaders/debug_program.json");

		a_world.add_world_component<aoegl::director_world_component>();
		auto& meshRenderWorldComponent = a_world.add_world_component<aoegl::mesh_render_world_component>();
		loadProgram(meshRenderWorldComponent.m_meshProgram, "data/new/shaders/mesh_program.json");

		auto& postProcessRenderWorldComponent =
			a_world.add_world_component<aoegl::post_process_render_world_component>();
		loadProgram(
			postProcessRenderWorldComponent.m_postProcessProgram
			, "data/new/shaders/post_process_program.json");

		auto& sceneTextureWorldComponent = a_world.add_world_component<aoegl::scene_texture_world_component>();
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

		a_world.add_world_component<aoein::inputs>();
		aoein::bindings& bindings = a_world.add_world_component<aoein::bindings>();
		aoedb::debug_controller_world_component& debugControllerWorldComponent = a_world.add_world_component<aoedb::debug_controller_world_component>();
		{
			debugControllerWorldComponent.m_yawMapping = bindings.axes.add(
				aoein::binding_util::make_derived_axis(aoein::mouse::axis::X, 0.01f));

			debugControllerWorldComponent.m_pitchMapping = bindings.axes.add(
				aoein::binding_util::make_derived_axis(aoein::mouse::axis::Y, 0.01f));

			debugControllerWorldComponent.m_enableViewMapping = bindings.switches.add(
				aoein::binding_util::make_switch(aoein::mouse::button::Right));

			debugControllerWorldComponent.m_lateralMoveMapping = bindings.axes.add(
				aoein::binding_util::make_axis(aoein::keyboard::key::S, aoein::keyboard::key::F));

			debugControllerWorldComponent.m_longitudinalMoveMapping = bindings.axes.add(
				aoein::binding_util::make_axis(aoein::keyboard::key::E, aoein::keyboard::key::D));

			debugControllerWorldComponent.m_verticalMoveMapping = bindings.axes.add(
				aoein::binding_util::make_axis(aoein::keyboard::key::Space, aoein::keyboard::key::LBracket));

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


			debugControllerWorldComponent.m_terrainSizeUpMapping = bindings.switches.add(
				aoein::binding_util::make_switch(aoein::keyboard::key::RShift));

			debugControllerWorldComponent.m_terrainSizeDownMapping = bindings.switches.add(
				aoein::binding_util::make_switch(aoein::keyboard::key::Quote));

			debugControllerWorldComponent.m_terrainCellSizeUpMapping = bindings.switches.add(
				aoein::binding_util::make_switch(aoein::keyboard::key::LShift));

			debugControllerWorldComponent.m_terrainCellSizeDownMapping = bindings.switches.add(
				aoein::binding_util::make_switch(aoein::keyboard::key::LControl));

			debugControllerWorldComponent.m_terrainUseSmoothShadingMapping = bindings.switches.add(
				aoein::binding_util::make_switch(aoein::keyboard::key::RBracket));

			for (int i = 0; i < 4; ++i)
			{
				auto& layer = debugControllerWorldComponent.m_terrainLayers.emplace_back();
				layer.m_isEnabled = 0;
				layer.m_frequency = 0.01f * std::powf(3.0f, 0.0f + i);
				layer.m_height = 8.0f * std::powf(0.33f, 0.0f + i);
				layer.m_offset = glm::vec2{ 1.0f } *(i * layer.m_frequency) * 256.0f;

				layer.m_toggleMapping = bindings.switches.add(aoein::binding_util::make_switch(toggleKeys[i]));
				layer.m_frequencyUpMapping = bindings.switches.add(aoein::binding_util::make_switch(frequencyUpKeys[i]));
				layer.m_frequencyDownMapping = bindings.switches.add(aoein::binding_util::make_switch(frequencyDownKeys[i]));
				layer.m_heightUpMapping = bindings.switches.add(aoein::binding_util::make_switch(heightUpKeys[i]));
				layer.m_heightDownMapping = bindings.switches.add(aoein::binding_util::make_switch(heightDownKeys[i]));
			}
		}

		auto& textureDataResourceManager = a_world.add_world_component<aoegl::texture_data_resource_manager>();
		a_world.add_world_component<aoegl::model_data_resource_manager>(textureDataResourceManager);

		a_world.add_world_component<aoeph::world_physic_component>(a_data.m_dynamicsWorld);
	}

	// systems
	{
		auto const presentationTimeSystemId = a_world.add_system<aoest::presentation_time_system>();
		auto const simulationTimeSystemId = a_world.add_system<aoest::simulation_time_system>();

		auto const pollEventsSystemId = a_world.add_system<aoewi::poll_events_system>();
		auto const windowInputSystemId = a_world.add_system<aoewi::window_input_system>();
		auto const bindingSystemId = a_world.add_system<aoein::binding_system>();
		auto const debugControllerSystemId = a_world.add_system<aoedb::debug_controller_system>();
		auto const physicSystemId = a_world.add_system<aoeph::physic_system>();


		auto const modelDataResourceSystemId = a_world.add_system<aoegl::model_data_resource_system>();

		auto const bindSceneFramebufferSystemId = a_world.add_system<aoegl::bind_scene_framebuffer_system>();
		auto const renderModelsSystemId = a_world.add_system<aoegl::render_models_system>();
		auto const renderDebugMeshSystemId = a_world.add_system<aoegl::render_debug_mesh_system>();

		auto const bindWindowFramebufferSystemId = a_world.add_system<aoegl::bind_window_framebuffer_system>();
		auto const renderSceneSystemId = a_world.add_system<aoegl::render_scene_system>();

		auto const swapBuffersSystemId = a_world.add_system<aoewi::swap_buffers_system>();
		
		a_schedule.clear();
		a_schedule.emplace_back(mismt::pmr::thread_schedule{
			{presentationTimeSystemId, {}},
			{simulationTimeSystemId, {}},
			{pollEventsSystemId, {}},
			{windowInputSystemId, {}},
			{bindingSystemId, {}},
			{debugControllerSystemId, {}},
			{modelDataResourceSystemId, {}},
			{bindSceneFramebufferSystemId, {}},
			{renderModelsSystemId, {}},
			{renderDebugMeshSystemId, {}},
			{bindWindowFramebufferSystemId, {}},
			{renderSceneSystemId, {}},
			{swapBuffersSystemId, {}} });

		a_schedule.emplace_back(mismt::pmr::thread_schedule{
			{physicSystemId, {swapBuffersSystemId}} });
	}
}

void init_default_map(aoeng::world& a_world, aoe::DataHolder& a_data)
{
	auto& worldData = a_world.get_data();
	auto& entityRegistry = worldData.get_entity_registry();

	auto debugCameraArkPath = std::filesystem::path{ "data/new/archetypes/debug_camera.json" };
	auto debugCameraArkId = a_data.filesystemIndexer.get_runtime_id(debugCameraArkPath);
	auto debugCameraArk = a_data.componentSetDatabase.find(debugCameraArkId);
	if (debugCameraArk != nullptr)
	{
		auto const debugCamera = entityRegistry.create();
		entityRegistry.emplace<vob::aoedb::debug_controller_component>(debugCamera, *debugCameraArk->find<vob::aoedb::debug_controller_component>());
		entityRegistry.emplace<vob::aoest::transform_component>(debugCamera, *debugCameraArk->find<vob::aoest::transform_component>());
		entityRegistry.emplace<vob::aoegl::camera_component>(debugCamera, *debugCameraArk->find<vob::aoegl::camera_component>());

		auto& directorWorldComponent = entityRegistry.get<aoegl::director_world_component>(worldData.get_world_entity());
		directorWorldComponent.m_activeCamera = debugCamera;
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

		aoeng::world world;
		mismt::pmr::schedule schedule;
		init_world_and_schedule(world, schedule, data, window);
		init_default_map(world, data);

		// Run game
		world.start(schedule);
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