#pragma once

#include <memory>
#include <unordered_map>

#include <vob/aoe/core/data/Id.h>
#include <vob/aoe/core/type/ADynamicType.h>


namespace vob::aoe::data
{
	class Cache
	{
	public:
		// Aliases
		using KeyType = Id;
		using ValueType = std::weak_ptr<type::ADynamicType> const;
		using MapType = std::pmr::unordered_map<KeyType, ValueType>;
		using AllocatorType = MapType::allocator_type;

		// Constructors
		Cache() = default;

		explicit Cache(AllocatorType const& a_allocator)
			: m_data{ a_allocator }
		{}

		// Methods
		void set(
			Id a_dataId
			, std::shared_ptr<type::ADynamicType> const& a_data
		)
		{
			m_data.emplace(a_dataId, a_data);
		}

		auto find(Id const a_dataId)
		{
			// Recover data from cache
			auto const t_dataIt = m_data.find(a_dataId);
			if (t_dataIt == m_data.end())
			{
				return std::shared_ptr<type::ADynamicType>{ nullptr };
			}

			// Ensure data is still in memory
			auto const t_data = t_dataIt->second.lock();
			if (t_data == nullptr)
			{
				m_data.erase(t_dataIt);
			}

			return t_data;
		}

		auto getAllocator() const
		{
			return m_data.get_allocator();
		}

	private:
		// Attributes
		MapType m_data;
	};
}

