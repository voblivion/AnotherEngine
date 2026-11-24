#pragma once

#include <vob/aoe/data/Database.h>
#include <vob/aoe/data/Id.h>

#include <memory>
#include <optional>


namespace vob::aoedt
{
	template <typename TData>
	class SoftPtr
	{
	public:
		SoftPtr() = default;

		explicit SoftPtr(Id const a_id)
			: m_id{ a_id }
		{}

		explicit SoftPtr(std::shared_ptr<TData const> a_data)
			: m_data{ std::move(a_data) }
		{}

		auto const& getId() const
		{
			return m_id;
		}

		std::shared_ptr<TData const> get(Database<TData> const& a_database) const
		{
			if (m_data != nullptr)
			{
				return m_data;
			}

			if (!m_id.has_value())
			{
				return nullptr;
			}

			return a_database.find(*m_id);
		}

	private:
		std::optional<Id> m_id;
		std::shared_ptr<TData const> m_data;
	};
}
