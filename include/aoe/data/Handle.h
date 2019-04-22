#pragma once

#include <memory>
#include <cassert>

#include <aoe/data/Id.h>
#include <aoe/data/ADatabase.h>

namespace aoe
{
	namespace data
	{
		template <typename DataType>
		class Handle
		{
		public:
			explicit Handle(ADatabase& a_database)
				: m_database{ a_database }
			{}

			template <typename Visitor>
			void accept(Visitor& a_visitor)
			{
				a_visitor.visit("id", m_id);
				tryUnload();
			}

			void setId(Id const a_id)
			{
				m_id = a_id;
				tryUnload();
			}

			void tryLoad() const
			{
				m_data = m_database.find<DataType>(m_id);
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

			DataType const& operator*() const
			{
				assert(isLoaded());
				return *(m_data.get());
			}

			DataType const* operator->() const
			{
				return m_data.get();
			}

		private:
			mutable bool m_isDirty{ true };
			ADatabase& m_database;
			Id m_id{};
			mutable std::shared_ptr<DataType> m_data{};
		};
	}
}
