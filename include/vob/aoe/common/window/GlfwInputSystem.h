#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/core/ecs/WorldDataProvider.h>

#include <vob/aoe/common/window/InputComponent.h>
#include <vob/aoe/common/window/WindowComponent.h>

#include <iomanip>



namespace vob::aoe::common
{
	class VOB_AOE_API GlfwInputSystem
	{
	public:
		explicit GlfwInputSystem(ecs::WorldDataProvider& a_wdp);

		void update() const;

	private:
		WindowComponent& m_worldWindowComponent;
		InputComponent& m_worldInput;
		bool& m_worldStop;
	};
}
