#pragma once

#include <vob/aoe/data/id.h>

#include <memory>
#include <unordered_map>


namespace vob::aoedt
{
	template <typename TData>
	class cache
	{
	public:
		std::shared_ptr<TData> find_or_erase(id const a_id)
		{
			auto const it = m_values.find(a_id);
			if (it == m_values.end())
			{
				return nullptr;
			}

			auto const value = it->second.lock();
			if (value == nullptr)
			{
				m_values.erase(it);
			}

			return value;
		}

		void update(id const a_id, std::weak_ptr<TData> a_data)
		{
			m_values[a_id] = std::move(a_data);
		}

	private:
		std::pmr::unordered_map<id, std::weak_ptr<TData>> m_values;
	};
}
