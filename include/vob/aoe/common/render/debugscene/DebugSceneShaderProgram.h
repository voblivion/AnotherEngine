#pragma once
#include <vob/aoe/api.h>

#include <vob/aoe/common/render/resources/SceneShaderProgram.h>
#include <vob/aoe/common/render/OpenGl.h>


namespace vob::aoe::common
{
	class DebugSceneShaderProgram
		: public SceneShaderProgram
	{
	public:
		explicit DebugSceneShaderProgram(data::ADatabase& a_database)
			: SceneShaderProgram{ a_database }
		{}
	};
}
