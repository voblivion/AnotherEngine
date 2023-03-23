#pragma once

#include <cinttypes>
#include <limits>

namespace vob::_aoecs
{
	class entity_id
	{
	public:
		using value_type = std::uint64_t;

		static constexpr value_type k_invalid_entity_id_value = std::numeric_limits<value_type>::max();
		
#pragma region CREATORS
		entity_id() = default;

		explicit entity_id(value_type a_value)
			: m_value{ a_value }
		{}
#pragma endregion

#pragma region ACCESSORS
		bool is_valid() const
		{
			return m_value != k_invalid_entity_id_value;
		}

		auto operator<=>(entity_id const&) const = default;

		value_type get_value() const
		{
			return m_value;
		}
#pragma endregion

#pragma region MANIPULATORS
		void reset()
		{
			m_value = k_invalid_entity_id_value;
		}

		void set_value(value_type a_value)
		{
			m_value = a_value;
		}
#pragma endregion

	private:
#pragma region DATA
		value_type m_value = k_invalid_entity_id_value;
#pragma endregion
	};

	struct entity_id_hash
	{
		std::size_t operator()(entity_id const& a_entityId) const
		{
			return a_entityId.get_value();
		}
	};
}

namespace vob::misvi
{
	template <typename TVisitor, typename TValue>
	requires std::is_same_v<_aoecs::entity_id, std::remove_const_t<TValue>>
	bool accept(TVisitor& a_visitor, TValue& a_value)
	{
		return a_visitor.visit(nvp("value", a_value));
	}
}

