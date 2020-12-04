#pragma once
#include <vob/aoe/api.h>

#include <vob/aoe/common/render/resources/ShaderProgram.h>
#include <vob/aoe/common/render/OpenGl.h>


namespace vob::aoe::common
{
	class PostProcessShaderProgram
		: public ShaderProgram
	{
	public:
		explicit PostProcessShaderProgram(data::ADatabase& a_database)
			: ShaderProgram{ a_database }
		{}
	};
}
