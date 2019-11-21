#pragma once


namespace vob::aoe::common
{
	struct Button
	{
		bool m_pressed = false;
		bool m_changed = false;

		operator bool() const
		{
			return m_pressed;
		}
	};
}