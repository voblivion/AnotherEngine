#pragma once

#include <memory>

#include <vob/misc/std/polymorphic_ptr.h>
#include <vob/misc/std/polymorphic_ptr_util.h>

#include <vob/aoe/core/type/Traits.h>
#include <vob/aoe/core/type/Applicator.h>


namespace vob::aoe::type
{
    template <
        typename PolymorphicBaseType = ADynamicType
        , typename AllocatorType = std::pmr::polymorphic_allocator<PolymorphicBaseType>
        , typename ApplicatorAllocatorType = AllocatorType
    >
    class Cloner
    {
        using Self = Cloner<PolymorphicBaseType>;

        template <typename Type>
        struct DoClone
        {
            void operator()(
				Type const& a_source
                , mistd::polymorphic_ptr<PolymorphicBaseType>& a_target
                , AllocatorType const& a_allocator
                ) const
            {
				a_target = mistd::polymorphic_ptr_util::allocate<Type>(a_allocator, a_source);
            }
        };

    public:
		#pragma region Constructors
        explicit Cloner(AllocatorType a_allocator = {})
            : m_allocator{ a_allocator }
        {}
		#pragma endregion

		#pragma region Methods
        template <typename Type>
		mistd::polymorphic_ptr<Type> clone(mistd::polymorphic_ptr<Type> const& a_source) const
        {
            static_assert(std::is_base_of_v<PolymorphicBaseType, Type>);
            if (a_source == nullptr)
            {
				return nullptr;
            }

			mistd::polymorphic_ptr<PolymorphicBaseType> target = nullptr;
            m_applicator.apply(*a_source, target, m_allocator);
            return mistd::polymorphic_ptr_util::cast<Type>(std::move(target));
        }

        template <typename Type, typename... Args>
        auto create(Args&&... a_args) const
        {
            return mistd::polymorphic_ptr_util::allocate<Type>(m_allocator, std::forward<Args>(a_args)...);
        }

        template <typename Type>
        bool isRegistered()
        {
            static_assert(std::is_base_of_v<PolymorphicBaseType, Type>);
            return m_applicator.template isRegistered<Type>();
        }

        template <typename Type>
        void registerType()
        {
            static_assert(std::is_base_of_v<PolymorphicBaseType, Type>);
            m_applicator.template registerType<Type>();
        }
		#pragma endregion

    private:
        // Attributes
        AllocatorType m_allocator;
        Applicator<
			PolymorphicBaseType const
			, ApplicatorAllocatorType
			, DoClone
			, mistd::polymorphic_ptr<PolymorphicBaseType>&
			, AllocatorType const&
		> m_applicator;
    };

	template <
		typename Type
		, typename PolymorphicBaseType = ADynamicType
		, typename AllocatorType = std::pmr::polymorphic_allocator<PolymorphicBaseType>
		, typename ApplicatorAllocatorType = AllocatorType
	>
	class Cloneable;
}

namespace vob::aoe::vis
{
	template <
		typename VisitorType
		, typename Type
		, typename PolymorphicBaseType
		, typename AllocatorType
		, typename ApplicatorAllocatorType
	>
	void accept(VisitorType& a_visitor, type::Cloneable<Type, PolymorphicBaseType, AllocatorType, ApplicatorAllocatorType>& a_this);
}

namespace vob::aoe::type
{
	template <
		typename Type
		, typename PolymorphicBaseType
        , typename AllocatorType
        , typename ApplicatorAllocatorType
	>
	class Cloneable
	{
		using Cloner = Cloner<PolymorphicBaseType, AllocatorType, ApplicatorAllocatorType>;
	public:
		#pragma region Constructors
		explicit Cloneable(Cloner const& a_cloner)
			: m_cloner{ a_cloner }
		{}

		Cloneable(Cloneable&&) = default;
		Cloneable(Cloneable const& a_other)
			: m_cloner{ a_other.m_cloner }
			, m_ptr{ m_cloner.get().clone(a_other.m_ptr) }
        {}

		~Cloneable()
        {
            reset();
		}
		#pragma endregion

		#pragma region Methods
		Type* get() const
		{
			return m_ptr.get();
        }

		void reset()
		{
			m_ptr.reset();
		}

		template <typename OtherType>
		void reset(mistd::polymorphic_ptr<OtherType> a_ptr)
		{
			m_ptr = std::move(a_ptr);
		}

		template <typename OtherType, typename... Args>
		OtherType& init(Args&&... a_args)
		{
			reset();
			auto ptr = m_cloner.get().template create<OtherType>(std::forward<Args>(a_args)...);
			auto& object = *ptr;
			m_ptr = std::move(ptr);
			return object;
		}
		#pragma endregion

		#pragma region Operators
		Cloneable& operator=(Cloneable&&) = default;
		Cloneable& operator=(Cloneable const& a_other)
        {
            m_cloner = a_other.m_cloner;
            m_ptr = m_cloner.get().clone(a_other.m_ptr);
			return *this;
		}

		Type* operator->() const
		{
			return m_ptr.get();
		}
		Type& operator*() const
		{
			return *m_ptr;
		}
		bool operator==(std::nullptr_t) const
		{
			return m_ptr == nullptr;
		}
		bool operator!=(std::nullptr_t) const
		{
			return m_ptr != nullptr;
		}
		#pragma endregion

	private:
		std::reference_wrapper<Cloner const> m_cloner;
		mistd::polymorphic_ptr<Type> m_ptr = nullptr;


		template <
			typename VisitorType
			, typename Type
			, typename PolymorphicBaseType
			, typename AllocatorType
			, typename ApplicatorAllocatorType
		>
		friend void vis::accept(
			VisitorType& a_visitor
			, type::Cloneable<Type, PolymorphicBaseType, AllocatorType, ApplicatorAllocatorType>& a_this
		);
	};
}

namespace vob::aoe::vis
{
    template <
		typename VisitorType
		, typename Type
		, typename PolymorphicBaseType
		, typename AllocatorType
		, typename ApplicatorAllocatorType
	>
    void accept(
		VisitorType& a_visitor
		, type::Cloneable<Type, PolymorphicBaseType, AllocatorType, ApplicatorAllocatorType>& a_this
	)
    {
		accept(a_visitor, a_this.m_ptr);
    }
}
