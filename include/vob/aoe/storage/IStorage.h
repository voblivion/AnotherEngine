#pragma once

#include <istream>
#include <memory>
#include <ostream>
#include <string_view>


namespace vob::aoesg
{
	enum class StorageDomain
	{
		Settings,
		Saves
	};

	struct IStorage
	{
		virtual ~IStorage() = default;

		virtual std::unique_ptr<std::istream> openForRead(
			StorageDomain a_domain, std::string_view a_name) const = 0;

		virtual std::unique_ptr<std::ostream> openForWrite(
			StorageDomain a_domain, std::string_view a_name) = 0;
	};
}
