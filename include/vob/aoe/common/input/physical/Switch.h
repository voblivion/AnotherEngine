#pragma once


namespace vob::aoe::common
{
	struct Switch
	{
		bool m_isActive = false;
		bool m_changed = false;

		operator bool() const
		{
			return m_isActive;
		}

		void update(bool isActive)
		{
			m_changed = m_isActive != isActive;
			m_isActive = isActive;
		}
	};
}