#pragma once

#include <vob/aoe/physics/math_util.h>

#include <vob/aoe/rendering/color.h>
#include <vob/aoe/rendering/world_components/debug_mesh_world_component.h>

#include <bullet/LinearMath/btIDebugDraw.h>


namespace vob::aoeph
{
	class debug_drawer final
		: public btIDebugDraw
	{
	public:
		explicit debug_drawer(vob::aoegl::debug_mesh_world_component& a_debugMeshWorldComponent)
			: m_debugMeshWorldComponent{ a_debugMeshWorldComponent }
		{}

		void drawLine(
			btVector3 const& a_source
			, btVector3 const& a_target
			, btVector3 const& a_color) override
		{
			m_debugMeshWorldComponent.add_line(
				vob::aoegl::debug_vertex{ to_glm(a_source), vob::aoegl::to_rgba(to_glm(a_color)) },
				vob::aoegl::debug_vertex{ to_glm(a_target), vob::aoegl::to_rgba(to_glm(a_color)) });
		}

		void drawContactPoint(
			btVector3 const& a_position
			, btVector3 const& a_normal
			, btScalar a_distance
			, int a_lifetime
			, btVector3 const& a_color) override
		{

		}

		void reportErrorWarning(char const* a_warningStr) override
		{

		}

		void draw3dText(btVector3 const& a_position, char const* a_textStr) override
		{

		}

		void setDebugMode(int a_debugMode) override
		{
			m_debugModes = static_cast<DebugDrawModes>(a_debugMode);
		}

		int getDebugMode() const override
		{
			return m_debugModes;
		}

	private:
		vob::aoegl::debug_mesh_world_component& m_debugMeshWorldComponent;
		DebugDrawModes m_debugModes = DBG_NoDebug;
	};
}
