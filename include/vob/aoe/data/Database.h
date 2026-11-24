#pragma once

#include <vob/aoe/data/Id.h>

#include <memory>


namespace vob::aoedt
{
	template <typename TData>
	class Database
	{
	public:
		virtual std::shared_ptr<TData const> find(Id const a_id) const = 0;
	};
}
