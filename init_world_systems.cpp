#include "init_world_systems.h"

#include <vob/aoe/debug/debug_game_mode_system.h>
#include <vob/aoe/debug/debug_ghost_controller_system.h>
#include <vob/aoe/engine/world.h>
#include <vob/aoe/input/binding_system.h>
#include <vob/aoe/physics/systems/physics_debug_system.h>
#include <vob/aoe/physics/systems/physics_system.h>
#include <vob/aoe/physics/systems/rigidbody_system.h>
#include <vob/aoe/spacetime/lifetime_system.h>
#include <vob/aoe/spacetime/pause_system.h>
#include <vob/aoe/spacetime/time_system.h>
#include <vob/aoe/window/poll_events_system.h>
#include <vob/aoe/window/swap_buffers_system.h>
#include <vob/aoe/window/window_input_system.h>
#include <vob/aoe/rendering/systems/bind_scene_framebuffer_system.h>
#include <vob/aoe/rendering/systems/bind_window_framebuffer_system.h>
#include <vob/aoe/rendering/systems/debug_render_camera_system.h>
#include <vob/aoe/rendering/systems/model_data_resource_system.h>
#include <vob/aoe/rendering/systems/render_debug_mesh_system.h>
#include <vob/aoe/rendering/systems/render_imgui_system.h>
#include <vob/aoe/rendering/systems/render_models_system.h>
#include <vob/aoe/rendering/systems/render_scene_system.h>

#include <vob/aoe/debug/debug_controller.h>
#include <vob/aoe/spacetime/attachment_system.h>
#include <vob/aoe/spacetime/soft_follow_system.h>



using namespace vob;

void init_world_systems(vob::aoeng::world& a_world, vob::mismt::pmr::schedule& a_schedule)
{
	auto const presentationTimeSystemId = a_world.add_system<aoest::presentation_time_system>();
	auto const simulationTimeSystemId = a_world.add_system<aoest::simulation_time_system>();
	auto const simulationPauseSystemId = a_world.add_system<aoest::simulation_pause_system>();

	auto const pollEventsSystemId = a_world.add_system<aoewi::poll_events_system>();
	auto const windowInputSystemId = a_world.add_system<aoewi::window_input_system>();
	auto const bindingSystemId = a_world.add_system<aoein::binding_system>();
	auto const debugGameModeSystemId = a_world.add_system<aoedb::debug_game_mode_system>();
	auto const debugGhostControllerSystemId = a_world.add_system<aoedb::debug_ghost_controller_system>();
	auto const debugControllerSystemId = a_world.add_system<aoedb::debug_controller_system>();
	auto const physicsSystemId = a_world.add_system<aoeph::physics_system>();
	auto const rigidbodySystemId = a_world.add_system<aoeph::rigidbody_system>();
	auto const physicsDebugSystemId = a_world.add_system<aoeph::physics_debug_system>();
	auto const attachmentSystemId = a_world.add_system<aoest::attachment_system>();
	auto const softFollowSystemId = a_world.add_system<aoest::soft_follow_system>();
	auto const lifetimeSystemId = a_world.add_system<aoest::lifetime_system>();

	// Rendering
	auto const debugRenderCameraSystemId = a_world.add_system<aoegl::debug_render_camera_system>();
	auto const modelDataResourceSystemId = a_world.add_system<aoegl::model_data_resource_system>();

	auto const bindSceneFramebufferSystemId = a_world.add_system<aoegl::bind_scene_framebuffer_system>();
	auto const renderModelsSystemId = a_world.add_system<aoegl::render_models_system>();
	auto const renderDebugMeshSystemId = a_world.add_system<aoegl::render_debug_mesh_system>();

	auto const bindWindowFramebufferSystemId = a_world.add_system<aoegl::bind_window_framebuffer_system>();
	auto const renderSceneSystemId = a_world.add_system<aoegl::render_scene_system>();

	auto const renderImguiSystemId = a_world.add_system<aoegl::render_imgui_system>();

	auto const swapBuffersSystemId = a_world.add_system<aoewi::swap_buffers_system>();
	// ^Rendering

	a_schedule.clear();
	a_schedule.emplace_back(mismt::pmr::thread_schedule{
		{presentationTimeSystemId, {}},
		{simulationTimeSystemId, {}},
		{simulationPauseSystemId, {}},
		{pollEventsSystemId, {}},
		{windowInputSystemId, {}},
		{bindingSystemId, {}},
		{debugGameModeSystemId, {}},
		{debugGhostControllerSystemId, {}},
		{debugControllerSystemId, {}},
		{lifetimeSystemId, {}},
		{debugRenderCameraSystemId, {attachmentSystemId}},
		{modelDataResourceSystemId, {}},
		{bindSceneFramebufferSystemId, {}},
		{renderModelsSystemId, {}},
		{renderDebugMeshSystemId, {}},
		{bindWindowFramebufferSystemId, {}},
		{renderSceneSystemId, {}},
		{renderImguiSystemId, {}},
		{swapBuffersSystemId, {}} });

	a_schedule.emplace_back(mismt::pmr::thread_schedule{
		{physicsSystemId, {debugControllerSystemId}},
		{rigidbodySystemId},
		{physicsDebugSystemId},
		{softFollowSystemId, {}},
		{attachmentSystemId, {}} });
}