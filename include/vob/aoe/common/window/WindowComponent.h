#pragma once

#include <vector>
#include <functional>

#include "vob/aoe/core/ecs/Component.h"
#include <vob/aoe/common/render/IWindow.h>

namespace vob::aoe::common
{
	class WindowComponent final
		: public ecs::AComponent
	{
	public:
		// Constructor
		explicit WindowComponent(common::IWindow& a_window)
			: m_window{ a_window }
		{}

		// Methods
		auto& getWindow()
		{
			return m_window.get();
		}
		auto const& getWindow() const
		{
			return m_window.get();
		}

	private:
		// Attributes
		std::reference_wrapper<common::IWindow> m_window;
	};
}
