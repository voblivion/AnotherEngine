#pragma once


namespace vob::aoein
{
	class switch_input
	{
	public:
		// ACCESSORS
		bool is_pressed() const
		{
			return m_isPressed;
		}

		bool changed() const
		{
			return m_changed;
		}

		operator bool() const
		{
			return m_isPressed;
		};

		// MANIPULATORS
		void update(bool a_isPressed)
		{
			m_changed = m_isPressed != a_isPressed;
			m_isPressed = a_isPressed;
		}

		void update()
		{
			m_changed = false;
		}

	private:
		// DATA
		bool m_isPressed = false;
		bool m_changed = false;

	};
}
