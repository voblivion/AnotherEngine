#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/resources/program.h>

#include <vob/misc/visitor/macros.h>

#include <string>
#include <memory>


namespace vob::aoegl
{
	constexpr char const* k_viewPosition = "u_viewPosition";
	constexpr char const* k_viewProjectionTransform = "u_viewProjectionTransform";
	constexpr char const* k_ambientColor = "u_ambientColor";
	constexpr char const* k_sunColor = "u_sunColor";
	constexpr char const* k_sunDirection = "u_sunDirection";
	constexpr char const* k_meshTransform = "u_meshTransform";
	constexpr char const* k_meshNormalTransform = "u_meshNormalTransform";
	constexpr char const* k_isRigged = "u_isRigged";
	constexpr char const* k_rigPose = "u_rigPose";
	constexpr char const* k_windowSize = "u_windowSize";

	struct program_data
	{
		std::shared_ptr<std::pmr::string const> m_vertexShaderSource;
		std::shared_ptr<std::pmr::string const> m_fragmentShaderSource;
	};

	VOB_AOE_API void create_program(program_data const& a_programData, program& a_program);

	VOB_AOE_API void create_program(program_data const& a_programData, scene_program& a_program);

	VOB_AOE_API void create_program(program_data const& a_programData, mesh_program& a_program);

	VOB_AOE_API void create_program(program_data const& a_programData, post_process_program& a_program);
}
// component manager
// physic material
namespace vob::misvi
{
	VOB_MISVI_ACCEPT(aoegl::program_data)
	{
		VOB_MISVI_NVP("Vertex Shader", vertexShaderSource);
		VOB_MISVI_NVP("Fragment Shader", fragmentShaderSource);
		return true;
	}
}
