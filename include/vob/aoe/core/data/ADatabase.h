#pragma once

#include <memory>

#include <vob/aoe/core/data/Id.h>
#include <vob/aoe/core/type/ADynamicType.h>
#include <vob/aoe/core/type/TypeRegistry.h>


namespace vob::aoe::data
{
	class ADatabase
	{
	public:
		// Constructors
		explicit ADatabase(type::TypeRegistry& a_typeRegistry)
			: m_typeRegistry{ a_typeRegistry }
		{}

		virtual ~ADatabase() = default;

		// Methods
		template <typename DataType>
		std::shared_ptr<DataType> find(Id const a_id)
		{
			return m_typeRegistry.fastCast<DataType>(find(a_id));
		}

	protected:
		// Methods
		virtual std::shared_ptr<type::ADynamicType> find(Id a_id) = 0;

	private:
		// Attributes
		type::TypeRegistry& m_typeRegistry;
	};
}
