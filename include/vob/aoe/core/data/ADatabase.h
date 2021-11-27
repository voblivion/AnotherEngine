#pragma once

#include <memory>

#include <vob/aoe/core/data/Id.h>
#include <vob/aoe/core/type/ADynamicType.h>

#include <vob/misc/type/registry.h>


namespace vob::aoe::data
{
	class ADatabase
	{
	public:
		// Constructors
		explicit ADatabase(misty::pmr::registry& a_typeRegistry)
			: m_typeRegistry{ a_typeRegistry }
		{}

		virtual ~ADatabase() = default;

		// Methods
		template <typename DataType>
		std::shared_ptr<DataType> find(Id const a_id)
		{
			return m_typeRegistry.fast_cast<DataType>(findDynamic(a_id));
		}

		template <typename DataType>
		void find(Id const a_id, std::shared_ptr<DataType const>& a_ptr)
		{
			a_ptr = find<DataType>(a_id);
		}

	protected:
		// Methods
		virtual std::shared_ptr<type::ADynamicType> findDynamic(Id a_id) = 0;

	private:
		// Attributes
		misty::pmr::registry& m_typeRegistry;
	};
}
