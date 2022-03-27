#pragma once


namespace vob::aoein
{
	class axis_input
	{
	public:
		// ACCESSORS
		float get_value() const
		{
			return m_value;
		}

		operator float() const
		{
			return m_value;
		}

		// MANIPULATORS
		void update(float a_value)
		{
			m_value = a_value;
		}

	private:
		// DATA
		float m_value = -1.0f;
	};
}
