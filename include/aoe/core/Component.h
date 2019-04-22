#pragma once
#include <memory>
#include <aoe/standard/ADynamicType.h>
#include <aoe/standard/Allocator.h>
#include <aoe/standard/Memory.h>

namespace aoe
{
	namespace core
	{
		class AComponent
			: public sta::ADynamicType
		{
		public:
			virtual std::size_t getCloneAllocationSize() const = 0;

			virtual sta::PolymorphicPtr<AComponent> clone(
				std::pmr::memory_resource* a_resource) const = 0;
		};

		template <typename ComponentType>
		class ComponentDefaultImpl
			: public AComponent
		{
		public:
			virtual std::size_t getCloneAllocationSize() const override
			{
				using Allocator = sta::Allocator<ComponentType>;
				return sta::getPolymorphicAllocationSize<ComponentType, Allocator>();
			}

			virtual sta::PolymorphicPtr<AComponent> clone(
				std::pmr::memory_resource* a_resource) const override
			{
				return sta::allocatePolymorphic<ComponentType>(
					sta::Allocator<ComponentType>{ a_resource }
				, static_cast<ComponentType const&>(*this));
			}
		};
	}
}