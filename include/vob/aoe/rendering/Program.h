#pragma once

#include <vob/aoe/rendering/GraphicTypes.h>


namespace vob::aoegl
{
	struct Program
	{
		GraphicId id = 0;
	};

	struct SceneProgram : public Program
	{
		GraphicUniformLocation viewPositionLocation = k_invalidUniformLocation;
		GraphicUniformLocation viewProjectionTransformLocation = k_invalidUniformLocation;
	};

	struct DebugProgram : public SceneProgram
	{

	};

	struct PostProcessProgram : public Program
	{
		GraphicUniformLocation windowSize = k_invalidUniformLocation;
	};
}
