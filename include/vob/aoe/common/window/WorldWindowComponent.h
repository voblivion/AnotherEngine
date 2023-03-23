#pragma once

#include <vob/aoe/common/_render/IWindow.h>

#include <functional>
#include <vector>


namespace vob::aoe::common
{
	class WorldWindowComponent final
	{
	public:
		// Constructor
		explicit WorldWindowComponent(common::IWindow& a_window)
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
