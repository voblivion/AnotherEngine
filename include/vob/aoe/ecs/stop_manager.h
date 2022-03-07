#pragma once

#include <vob/aoe/api.h>


namespace vob::aoecs
{
	class stop_manager
	{
	public:
		// ACCESSORS
		bool should_stop() const
		{
			return m_shouldStop;
		}

		// MANIPULATORS
		void set_should_stop(bool a_shouldStop)
		{
			m_shouldStop = a_shouldStop;
		}

	private:
		// Attributes
		bool m_shouldStop = false;
	};
}

