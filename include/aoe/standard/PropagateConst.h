#pragma once

#include <type_traits>

namespace aoe
{
	namespace sta
	{
		template <typename PtrType>
		class PropagateConst
		{
			template <typename OtherPtrType>
			friend class PropagateConst;

			using Type = std::remove_pointer_t<decltype(
				std::declval<PtrType>().get())>;

		public:
			// Constructors
			constexpr PropagateConst() = default;
			constexpr PropagateConst(PropagateConst&&) = default;
			PropagateConst(PropagateConst const&) = delete;
			~PropagateConst() = default;

			template <typename OtherPtrType>
			constexpr PropagateConst(PropagateConst<OtherPtrType>&& a_ptr)
				: m_ptr{ std::forward<OtherPtrType>(a_ptr.m_ptr) }
			{}

			template <typename OtherPtrType>
			constexpr PropagateConst(OtherPtrType&& a_ptr) // NOLINT
				: m_ptr{ std::forward<OtherPtrType>(a_ptr) }
			{}

			// Methods
			constexpr void swap(PropagateConst& a_ptr) noexcept
			{
				std::swap(m_ptr, a_ptr.m_ptr);
			}

			constexpr Type* get()
			{
				return m_ptr.get();
			}

			constexpr Type const* get() const
			{
				return m_ptr.get();
			}

			// Operators
			constexpr PropagateConst& operator=(PropagateConst&& a_ptr) = default;
			PropagateConst& operator=(PropagateConst const&) = delete;

			template <typename OtherPtrType>
			constexpr PropagateConst& operator=(PropagateConst<OtherPtrType>&& a_ptr)
			{
				m_ptr = std::move(a_ptr.m_ptr);
				return *this;
			}

			template <typename OtherPtrType>
			constexpr PropagateConst& operator=(OtherPtrType&& a_ptr)
			{
				m_ptr = std::forward(a_ptr);
				return *this;
			}

			constexpr explicit operator bool() const
			{
				return m_ptr;
			}

			constexpr Type& operator*()
			{
				return *m_ptr;
			}

			constexpr Type const& operator*() const
			{
				return *m_ptr;
			}

			constexpr Type* operator->()
			{
				return m_ptr.operator->();
			}

			constexpr Type const* operator->() const
			{
				return m_ptr.operator->();
			}

			constexpr operator Type*() // NOLINT
			{
				return operator->();
			}

			constexpr operator Type const*() const // NOLINT
			{
				return operator->();
			}

			template <typename OtherPtrType>
			friend constexpr bool operator==(
				PropagateConst<OtherPtrType> const& a_ptr, std::nullptr_t)
			{
				return a_ptr.m_ptr == nullptr;
			}

			template <typename OtherPtrType>
			friend constexpr bool operator==(
				std::nullptr_t, PropagateConst<OtherPtrType> const& a_ptr)
			{
				return a_ptr.m_ptr == nullptr;
			}

			template <typename OtherPtrType>
			friend constexpr bool operator!=(
				PropagateConst<OtherPtrType> const& a_ptr, std::nullptr_t)
			{
				return a_ptr.m_ptr != nullptr;
			}

			template <typename OtherPtrType>
			friend constexpr bool operator!=(
				std::nullptr_t, PropagateConst<OtherPtrType> const& a_ptr)
			{
				return a_ptr.m_ptr != nullptr;
			}

			template <typename PtrTypeA, typename PtrTypeB>
			friend constexpr bool operator==(PropagateConst<PtrTypeA> const& a_lhs
				, PropagateConst<PtrTypeB> const& a_rhs)
			{
				return a_lhs.m_ptr == a_rhs.m_ptr;
			}

			template <typename PtrTypeA, typename PtrTypeB>
			friend constexpr bool operator!=(PropagateConst<PtrTypeA> const& a_lhs
				, PropagateConst<PtrTypeB> const& a_rhs)
			{
				return a_lhs.m_ptr != a_rhs.m_ptr;
			}

			template <typename PtrTypeA, typename PtrTypeB>
			friend constexpr bool operator<(PropagateConst<PtrTypeA> const& a_lhs
				, PropagateConst<PtrTypeB> const& a_rhs)
			{
				return a_lhs.m_ptr < a_rhs.m_ptr;
			}

			template <typename PtrTypeA, typename PtrTypeB>
			friend constexpr bool operator>(PropagateConst<PtrTypeA> const& a_lhs
				, PropagateConst<PtrTypeB> const& a_rhs)
			{
				return a_lhs.m_ptr > a_rhs.m_ptr;
			}

			template <typename PtrTypeA, typename PtrTypeB>
			friend constexpr bool operator<=(PropagateConst<PtrTypeA> const& a_lhs
				, PropagateConst<PtrTypeB> const& a_rhs)
			{
				return a_lhs.m_ptr <= a_rhs.m_ptr;
			}

			template <typename PtrTypeA, typename PtrTypeB>
			friend constexpr bool operator>=(PropagateConst<PtrTypeA> const& a_lhs
				, PropagateConst<PtrTypeB> const& a_rhs)
			{
				return a_lhs.m_ptr >= a_rhs.m_ptr;
			}

			template <typename PtrTypeA, typename PtrTypeB>
			friend constexpr bool operator==(PropagateConst<PtrTypeA> const& a_lhs
				, PtrTypeB const& a_rhs)
			{
				return a_lhs.m_ptr == a_rhs;
			}

			template <typename PtrTypeA, typename PtrTypeB>
			friend constexpr bool operator!=(PropagateConst<PtrTypeA> const& a_lhs
				, PtrTypeB const& a_rhs)
			{
				return a_lhs.m_ptr != a_rhs;
			}

			template <typename PtrTypeA, typename PtrTypeB>
			friend constexpr bool operator==(PtrTypeA const& a_lhs
				, PropagateConst<PtrTypeB> const& a_rhs)
			{
				return a_lhs == a_rhs.m_ptr;
			}

			template <typename PtrTypeA, typename PtrTypeB>
			friend constexpr bool operator!=(PtrTypeA const& a_lhs
				, PropagateConst<PtrTypeB> const& a_rhs)
			{
				return a_lhs != a_rhs.m_ptr;
			}

			template <typename PtrTypeA, typename PtrTypeB>
			friend constexpr bool operator<(PropagateConst<PtrTypeA> const& a_lhs
				, PtrTypeB const& a_rhs)
			{
				return a_lhs.m_ptr < a_rhs;
			}

			template <typename PtrTypeA, typename PtrTypeB>
			friend constexpr bool operator>(PropagateConst<PtrTypeA> const& a_lhs
				, PtrTypeB const& a_rhs)
			{
				return a_lhs.m_ptr > a_rhs;
			}

			template <typename PtrTypeA, typename PtrTypeB>
			friend constexpr bool operator<=(PropagateConst<PtrTypeA> const& a_lhs
				, PtrTypeB const& a_rhs)
			{
				return a_lhs.m_ptr <= a_rhs;
			}

			template <typename PtrTypeA, typename PtrTypeB>
			friend constexpr bool operator>=(PropagateConst<PtrTypeA> const& a_lhs
				, PtrTypeB const& a_rhs)
			{
				return a_lhs.m_ptr >= a_rhs;
			}

			template <typename PtrTypeA, typename PtrTypeB>
			friend constexpr bool operator<(PtrTypeA const& a_lhs
				, PropagateConst<PtrTypeB> const& a_rhs)
			{
				return a_lhs < a_rhs.m_ptr;
			}

			template <typename PtrTypeA, typename PtrTypeB>
			friend constexpr bool operator>(PtrTypeA const& a_lhs
				, PropagateConst<PtrTypeB> const& a_rhs)
			{
				return a_lhs > a_rhs.m_ptr;
			}

			template <typename PtrTypeA, typename PtrTypeB>
			friend constexpr bool operator<=(PtrTypeA const& a_lhs
				, PropagateConst<PtrTypeB> const& a_rhs)
			{
				return a_lhs <= a_rhs.m_ptr;
			}

			template <typename PtrTypeA, typename PtrTypeB>
			friend constexpr bool operator>=(PtrTypeA const& a_lhs
				, PropagateConst<PtrTypeB> const& a_rhs)
			{
				return a_lhs >= a_rhs.m_ptr;
			}

		private:
			PtrType m_ptr;
		};
	}
}