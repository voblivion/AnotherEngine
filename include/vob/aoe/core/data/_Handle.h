#pragma once

#include <memory>
#include <cassert>

#include <vob/aoe/core/data/Id.h>
#include <vob/aoe/core/data/ADatabase.h>
#include <vob/aoe/core/visitor/JsonWriter.h>

/*namespace vob::aoe::data
{
	template <typename DataType>
	class Handle
	{
	public:
		explicit Handle(ADatabase& a_database)
			: m_database{ a_database }
		{}

		explicit Handle(ADatabase& a_database, Id const a_id)
			: m_database{ a_database }
			, m_id{ a_id }
		{}

		ADatabase& getDatabase() const
		{
			return m_database;
		}

		void setId(Id const a_id)
		{
			m_id = a_id;
			tryUnload();
		}

		void tryLoad() const
		{
			m_data = getDatabase().find<DataType>(m_id);
			m_isDirty = false;
		}

		void tryUnload() const
		{
			m_data = nullptr;
			m_isDirty = m_id != g_invalidId;
		}

		bool isLoaded() const
		{
			return m_data != nullptr;
		}

		bool isValid() const
		{
			if (m_isDirty)
			{
				tryLoad();
			}

			return isLoaded();
		}

		auto const& data() const
		{
			return m_data;
		}

		DataType const& operator*() const
		{
			assert(isLoaded());
			return *(m_data.get());
		}

		DataType const* operator->() const
		{
			return m_data.get();
		}

		bool operator==(std::nullptr_t) const
		{
			return isValid();
		}

		bool operator!=(std::nullptr_t) const
		{
			return !isValid();
		}

	private:
		mutable bool m_isDirty{ true };
		std::reference_wrapper<ADatabase> m_database;
		Id m_id{};
		mutable std::shared_ptr<DataType const> m_data{};
	};
}*/

