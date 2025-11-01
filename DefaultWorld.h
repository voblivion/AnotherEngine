#pragma once

#include <vob/aoe/engine/Game.h>
#include <vob/aoe/window/Window.h>

#include <memory>


namespace vob
{
	std::shared_ptr<aoeng::IWorld> createDefaultWorld(aoewi::IWindow& a_window);
}
