#pragma once

#include <vob/aoe/data/Id.h>

#include <memory>
#include <unordered_map>


namespace vob::aoedt
{
	template <typename TData>
	class Cache
	{
	public:
		std::shared_ptr<TData> findOrErase(Id const a_id)
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

		void update(Id const a_id, std::weak_ptr<TData> a_data)
		{
			m_values[a_id] = std::move(a_data);
		}

	private:
		std::pmr::unordered_map<Id, std::weak_ptr<TData>> m_values;
	};
}
