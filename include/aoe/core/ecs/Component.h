#pragma once

#include <memory>

#include <aoe/core/standard/ADynamicType.h>
#include <aoe/core/standard/Allocator.h>
#include <aoe/core/standard/Memory.h>

namespace aoe
{
	namespace ecs
	{
		class AComponent
			: public sta::ADynamicType
		{
		public:
			virtual std::size_t getCloneAllocationSize() const = 0;

			virtual sta::PolymorphicPtr<AComponent> clone(
				std::pmr::memory_resource* a_resource) const = 0;

			virtual void copyFrom(AComponent const& a_component) = 0;
		};

		template <typename ComponentType>
		class ComponentDefaultImpl
			: public AComponent
		{
		public:
			// Methods
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

			virtual void copyFrom(AComponent const& a_component) override
			{
				assert(typeid(a_component) == typeid(ComponentType));
				static_cast<ComponentType&>(*this) =
					static_cast<ComponentType const&>(a_component);
			}
		};
	}
}