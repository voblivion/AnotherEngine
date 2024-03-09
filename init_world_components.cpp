#include "init_world_components.h"

#include <vob/aoe/engine/world.h>
#include <vob/aoe/debug/debug_game_mode_world_component.h>
#include <vob/aoe/input/binding_util.h>
#include <vob/aoe/physics/components/car_controller.h>
#include <vob/aoe/physics/world_components/physics_debug_world_component.h>
#include <vob/aoe/physics/world_components/physics_world_component.h>
#include <vob/aoe/rendering/data/model_data_resource_manager.h>
#include <vob/aoe/rendering/data/texture_data_resource_manager.h>
#include <vob/aoe/rendering/world_components/debug_render_world_component.h>
#include <vob/aoe/rendering/world_components/director_world_component.h>
#include <vob/aoe/rendering/world_components/mesh_render_world_component.h>
#include <vob/aoe/rendering/world_components/post_process_render_world_component.h>
#include <vob/aoe/rendering/world_components/scene_texture_world_component.h>
#include <vob/aoe/window/glfw_window.h>
#include <vob/aoe/window/window_input_world_component.h>
#include <vob/aoe/window/window_world_component.h>

#include "DataHolder.h"


const std::uint32_t g_multiSampling = 1;

using namespace aoe;

void init_world_components(
	vob::aoeng::world& a_world,
	vob::aoe::DataHolder& a_data,
	vob::aoewi::glfw_window& a_window)
{
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
	a_world.add_world_component<aoest::simulation_pause_world_component>();

	a_world.add_world_component<aoewi::window_input_world_component>();
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

		auto const width = static_cast<GLsizei>(a_window.get_size().x * std::sqrt(g_multiSampling));
		auto const height = static_cast<GLsizei>(a_window.get_size().y * std::sqrt(g_multiSampling));

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
	auto& debugGameModeWorldComponent = a_world.add_world_component<aoedb::debug_game_mode_world_component>();
	{
		debugGameModeWorldComponent.m_switchActiveCamera = bindings.switches.add(
			aoein::binding_util::make_switch(aoein::switch_reference{ 0, aoein::gamepad::button::LB }));
		debugGameModeWorldComponent.m_switchActiveController = bindings.switches.add(
			aoein::binding_util::make_switch(aoein::switch_reference{ 0, aoein::gamepad::button::RB }));
	}
	aoedb::debug_controller_world_component& debugControllerWorldComponent = a_world.add_world_component<aoedb::debug_controller_world_component>();
	{
		debugControllerWorldComponent.m_enableViewMapping = bindings.switches.add(
			aoein::binding_util::make_switch(aoein::mouse::button::Right));

		debugControllerWorldComponent.m_spawnItem = bindings.switches.add(
			aoein::binding_util::make_switch(aoein::keyboard::key::Q));

		debugControllerWorldComponent.m_playSim = bindings.switches.add(
			aoein::binding_util::make_switch(aoein::keyboard::key::A));

		debugControllerWorldComponent.m_stepSim = bindings.switches.add(
			aoein::binding_util::make_switch(aoein::keyboard::key::Z));

		debugControllerWorldComponent.m_yawMapping = bindings.axes.add(
			aoein::binding_util::make_derived_axis(aoein::mouse::axis::X, 0.001f));

		debugControllerWorldComponent.m_pitchMapping = bindings.axes.add(
			aoein::binding_util::make_derived_axis(aoein::mouse::axis::Y, 0.001f));

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

		// TODO: toggled off for work on car
		/*debugControllerWorldComponent.m_terrainSubdivisionCountUpMapping = bindings.switches.add(
			aoein::binding_util::make_switch(aoein::keyboard::key::LShift));

		debugControllerWorldComponent.m_terrainSubdivisionCountDownMapping = bindings.switches.add(
			aoein::binding_util::make_switch(aoein::keyboard::key::LControl));*/

		debugControllerWorldComponent.m_terrainUseSmoothShadingMapping = bindings.switches.add(
			aoein::binding_util::make_switch(aoein::keyboard::key::RBracket));

		for (int i = 0; i < 4; ++i)
		{
			auto& layer = debugControllerWorldComponent.m_terrainLayers.emplace_back();
			layer.m_isEnabled = 0;
			layer.m_frequency = 0.01f * std::powf(3.0f, 0.0f + i);
			layer.m_height = 8.0f * std::powf(0.33f, 0.0f + i);
			layer.m_offset = glm::vec2{ 1.0f } * (i * layer.m_frequency) * 256.0f;

			layer.m_toggleMapping = bindings.switches.add(aoein::binding_util::make_switch(toggleKeys[i]));
			layer.m_frequencyUpMapping = bindings.switches.add(aoein::binding_util::make_switch(frequencyUpKeys[i]));
			layer.m_frequencyDownMapping = bindings.switches.add(aoein::binding_util::make_switch(frequencyDownKeys[i]));
			layer.m_heightUpMapping = bindings.switches.add(aoein::binding_util::make_switch(heightUpKeys[i]));
			layer.m_heightDownMapping = bindings.switches.add(aoein::binding_util::make_switch(heightDownKeys[i]));
		}

		auto itemModelId = a_data.filesystemIndexer.get_runtime_id("data/new/models/sphere.gltf");
		debugControllerWorldComponent.m_itemModel = a_data.modelDatabase.find(itemModelId);
	}
	auto& carControllerWorldComponent = a_world.add_world_component<aoeph::car_controller_world_component>();
	{
		carControllerWorldComponent.m_turn = bindings.axes.add(
			aoein::binding_util::make_axis(aoein::axis_reference{ 0, aoein::gamepad::axis::LeftX }));
		carControllerWorldComponent.m_forward = bindings.switches.add(
			aoein::binding_util::make_switch(aoein::switch_reference{ 0, aoein::gamepad::button::A }));
		carControllerWorldComponent.m_reverse = bindings.switches.add(
			aoein::binding_util::make_switch(aoein::switch_reference{ 0, aoein::gamepad::button::X }));
		//carControllerWorldComponent.m_engine = bindings.axes.add(
		//	aoein::binding_util::make_axis(
		//		aoein::switch_reference{ 0, aoein::gamepad::button::A },
		//		aoein::switch_reference{ 0, aoein::gamepad::button::X }));
		carControllerWorldComponent.m_respawn = bindings.switches.add(
			aoein::binding_util::make_switch(aoein::switch_reference{ 0, aoein::gamepad::button::LS }));
	}

	auto& textureDataResourceManager = a_world.add_world_component<aoegl::texture_data_resource_manager>();
	a_world.add_world_component<aoegl::model_data_resource_manager>(textureDataResourceManager);

	auto& physicsWorldComponent = a_world.add_world_component<aoeph::physics_world_component>(a_data.m_dynamicsWorld);
	auto& physicsDebugWorldComponent = a_world.add_world_component<aoeph::physics_debug_world_component>();
	{
		physicsDebugWorldComponent.m_cycleDebugDrawModeBinding = bindings.switches.add(
			aoein::binding_util::make_switch(aoein::keyboard::key::T));
	}
}
