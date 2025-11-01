#pragma once

#include <numbers>


namespace vob::aoegl
{
	struct CameraComponent
	{
		float fov = 70.0f / 180.0f * std::numbers::pi_v<float>;
		float nearClip = 0.1f;
		float farClip = 1000.0f;
	};
}
