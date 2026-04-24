#pragma once

#include <entt/entt.hpp>


namespace vob::aoegl
{
	struct CameraDirectorContext
	{
		entt::entity activeCameraEntity = entt::null;
		entt::entity debugCameraEntity = entt::null;
		entt::entity focusEntity = entt::null;
	};
}
