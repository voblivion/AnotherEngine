#pragma once


namespace aoe
{
	namespace common
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
}