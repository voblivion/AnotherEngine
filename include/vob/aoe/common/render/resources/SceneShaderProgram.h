#pragma once
#include <vob/aoe/api.h>

#include <vob/aoe/common/render/resources/ShaderProgram.h>


namespace vob::aoe::common
{
	class SceneShaderProgram
		: public ShaderProgram
	{
	public:
		// Constructor
		explicit SceneShaderProgram(data::ADatabase& a_database)
			: ShaderProgram{ a_database }
		{}

		// Methods
		auto getViewUniformLocation() const
		{
			return m_view;
		}
		auto getViewPositionUniformLocation() const
		{
			return m_viewPosition;
		}
		auto getProjectionUniformLocation() const
		{
			return m_projection;
		}
		void create() const
		{
			ShaderProgram::create();
			if (!isReady())
			{
				return;
			}

			m_view = getUniformLocation("u_view");
			m_viewPosition = getUniformLocation("u_viewPosition");
			m_projection = getUniformLocation("u_projection");
		}

	private:
		// Attributes
		mutable UniformLocation m_view = 0;
		mutable UniformLocation m_viewPosition = 0;
		mutable UniformLocation m_projection = 0;
	};
}
