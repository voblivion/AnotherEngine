#pragma once

#include <vob/sta/memory.h>

/*
namespace vob::aoe::type
{
	template <typename BaseType>
	// ReSharper disable once CppPolymorphicClassWithNonVirtualPublicDestructor
	class ICloneable
	{
	public:
		virtual sta::polymorphic_ptr<BaseType> clone(
			std::pmr::memory_resource* a_memoryResource
			= std::pmr::get_default_resource()) const = 0;
	};

	template <typename CloneType, typename Type, typename BaseType = CloneType>
	class CloneableDefaultImpl
		: public BaseType
	{

		sta::polymorphic_ptr<CloneType> clone(
			std::pmr::memory_resource* a_resource) const override
		{
			auto t_object = static_cast<Type const&>(*this);
			return sta::allocate_polymorphic<Type>(a_resource, t_object);
		}
	};

	template <typename Type>
	class Clone
		: public sta::polymorphic_ptr<Type>
	{
		using Base = sta::polymorphic_ptr<Type>;
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
		explicit Clone(sta::polymorphic_ptr<OtherType> a_ptr)
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
		sta::polymorphic_ptr<Type> m_ptr;
	};
}
*/