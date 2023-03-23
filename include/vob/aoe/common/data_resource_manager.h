#pragma once

#include <cassert>
#include <unordered_map>


namespace vob::aoeco
{
	template <typename TData, typename TResourceHandle>
	class data_resource_manager
	{
	public:
		template <typename TResourceHandleAllocator>
		TResourceHandle const& add_reference(
			TData const& a_data, TResourceHandleAllocator resourceHandleAllocator = {})
		{
			auto const result = m_resources.try_emplace(
				a_data, std::pair<TResourceHandle, std::size_t>{});
			if (result.second)
			{
				result.first->second.first = resourceHandleAllocator(a_data);
			}

			++result.first->second.second;
			return result.first->second.first;
		}

		template <typename TResourceHandleDeallocator>
		void remove_reference(
			TData const& a_data, TResourceHandleDeallocator resourceHandleDeallocator = {})
		{
			auto const it = m_resources.find(a_data);
			assert(it != m_resources.end() && it->second.second > 0);

			--it->second.second;
			if (it->second.second == 0)
			{
				resourceHandleDeallocator(a_data, it->second.first);
				m_resources.erase(it);
			}
		}

	private:
		std::unordered_map<TData, std::pair<TResourceHandle, std::size_t>> m_resources;
	};
}
