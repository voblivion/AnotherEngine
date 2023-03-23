#pragma once

#include <vob/aoe/physics/scalar.h>

#include <LinearMath/btIDebugDraw.h>

namespace vob::aoeph
{
	class debug_drawer final
		: public btIDebugDraw
	{
	public:
		void drawLine(
			btVector3 const& a_source
			, btVector3 const& a_target
			, btVector3 const& a_color) override
		{

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
		DebugDrawModes m_debugModes = DBG_NoDebug;
	};
}
