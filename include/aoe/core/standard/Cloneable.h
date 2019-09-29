#pragma once

#include <aoe/core/standard/Memory.h>


namespace aoe
{
	namespace sta
	{
		template <typename BaseType>
		// ReSharper disable once CppPolymorphicClassWithNonVirtualPublicDestructor
		class ICloneable
		{
		public:
			virtual PolymorphicPtr<BaseType> clone(
				std::pmr::memory_resource* a_memoryResource
				= std::pmr::get_default_resource()) const = 0;
		};

		template <typename CloneType, typename Type, typename BaseType = CloneType>
		class CloneableDefaultImpl
			: public BaseType
		{

			sta::PolymorphicPtr<CloneType> clone(
				std::pmr::memory_resource* a_resource) const override
			{
				auto t_object = static_cast<Type const&>(*this);
				return sta::allocatePolymorphicWith<Type>(a_resource, t_object);
			}
		};

		template <typename Type>
		class Clone
			: public PolymorphicPtr<Type>
		{
			using Base = PolymorphicPtr<Type>;
		public:
			// Constructors
			Clone() = default;
			Clone(Clone&&) = default;

			Clone(Clone const& a_other)
				: Base{ a_other != nullptr
					? a_other->clone(a_other.get_deleter().getResource())
					: nullptr }
			{}

			template <typename OtherType = Type>
			explicit Clone(PolymorphicPtr<OtherType> a_ptr)
				: Base{ std::move(a_ptr) }
			{}

			~Clone() = default;

			// Operators
			Clone& operator=(Clone&&) = default;
			Clone& operator=(Clone const& a_other)
			{
				static_cast<Base&>(*this) = a_other != nullptr
					? a_other->clone(a_other.get_deleter().getResource())
					: nullptr;
				return *this;
			}

		private:
			// Attributes
			PolymorphicPtr<Type> m_ptr;
		};
	}
}
