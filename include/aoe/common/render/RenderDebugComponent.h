#pragma once
#include "aoe/core/ecs/Component.h"
#include "Model.h"
#include <vector>
#include <GL/glew.h>
#include "ShaderProgram.h"
#include "Texture.h"
#include <cassert>

// TODO
#include "aoe/core/utils/SimpleProfiler.h"


namespace aoe
{
	namespace common
	{
		struct RenderDebugComponent final
			: public ecs::ComponentDefaultImpl<RenderDebugComponent>
		{
			explicit RenderDebugComponent(data::ADatabase& a_database)
				: shader { a_database }
			{
				shader.setId(2);
				assert(shader.isValid());
			}

			data::Handle<ShaderProgram> shader;
		};
	}
}
