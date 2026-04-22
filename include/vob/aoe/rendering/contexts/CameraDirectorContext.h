#pragma once

#include <entt/entt.hpp>


namespace vob::aoegl
{
	struct CameraDirectorContext
	{
		entt::entity activeCameraEntity = entt::null;
		entt::entity debugCameraEntity = entt::null;
	};
}
