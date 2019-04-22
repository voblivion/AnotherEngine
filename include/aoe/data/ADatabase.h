#pragma once

#include <memory>

#include <aoe/data/Id.h>
#include <aoe/standard/ADynamicType.h>
#include <aoe/standard/TypeRegistry.h>

namespace aoe
{
	namespace data
	{
		class ADatabase
			: public sta::ADynamicType
		{
		public:
			// Constructors
			explicit ADatabase(sta::TypeRegistry& a_typeRegistry)
				: m_typeRegistry{ a_typeRegistry }
			{}

			// Methods
			template <typename DataType>
			std::shared_ptr<DataType> find(Id const a_dataId)
			{
				return m_typeRegistry.fastCast<DataType>(find(a_dataId));
			}

		protected:
			// Methods
			virtual std::shared_ptr<sta::ADynamicType> find(Id a_dataId) = 0;

		private:
			// Attributes
			sta::TypeRegistry& m_typeRegistry;
		};
	}
}
