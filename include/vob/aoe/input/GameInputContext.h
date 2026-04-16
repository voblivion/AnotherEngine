#pragma once

#include <vob/aoe/input/GameInput.h>
#include <vob/aoe/input/InputBinding.h>

#include <vob/misc/std/container_util.h>

#include <cinttypes>


namespace vob::aoein
{
	struct GameInputContext
	{
	public:
		GameInputValueId registerValue(float a_defaultValue = 0.0f)
		{
			m_values.emplace_back(a_defaultValue);
			return { mistd::isize(m_values) - 1 };
		}

		GameInputEventId registerEvent()
		{
			return { nextGameInputEventId++ };
		}

		void setValue(GameInputValueId a_id, float a_value)
		{
			m_values[a_id.id] = a_value;
		}

		float getValue(GameInputValueId a_id) const
		{
			return m_values.at(a_id.id);
		}

		std::vector<float> const& getValues() const
		{
			return m_values;
		}

		void addEvent(GameInputEventId a_id)
		{
			m_events.push_back(a_id);
		}

		void removeEvent(GameInputEventId a_id)
		{
			std::erase(m_events, a_id);
		}

		void flushEvents()
		{
			m_events.clear();
		}

		std::vector<GameInputEventId> const& getEvents() const
		{
			return m_events;
		}

	private:
		std::vector<float> m_values;

		int32_t nextGameInputEventId = 0;
		std::vector<GameInputEventId> m_events;
	};

}
