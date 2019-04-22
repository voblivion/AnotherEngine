#pragma once

#include <cassert>
#include <type_traits>

namespace aoe
{
	namespace sta
	{
		template <typename PointerType>
		class NotNull
		{
		public:
			static_assert(std::is_assignable_v<PointerType&, std::nullptr_t>
				, "PointerType cannot be assigned nullptr.");

			// Aliases
			using Type = std::remove_reference_t<decltype(
				*std::declval<PointerType>())>;

			// Constructors
			template <typename OtherPointerType, typename = std::enable_if_t<
				std::is_convertible_v<OtherPointerType, PointerType>>>
			constexpr NotNull(OtherPointerType&& a_pointer)
				: m_pointer{ std::forward<OtherPointerType>(a_pointer) }
			{
				assert(m_pointer != nullptr);
			}

			template <typename = std::enable_if_t<
				std::is_same_v<std::nullptr_t, PointerType>>>
				constexpr NotNull(PointerType a_pointer)
				: m_pointer{ a_pointer }
			{
				assert(m_pointer != nullptr);
			}

			template <typename OtherPointerType, typename = std::enable_if_t<
				std::is_convertible_v<OtherPointerType, PointerType>>>
				constexpr NotNull(NotNull<OtherPointerType> const& a_pointer)
				: m_pointer{ a_pointer.get() }
			{
				assert(m_pointer != nullptr);
			}

			NotNull(NotNull&&) = default;
			// NotNull(NotNull const&) = default;
			explicit NotNull(std::nullptr_t) = delete;
			~NotNull() = default;

			// Methods
			PointerType get() const
			{
				return m_pointer;
			}

			// Operators
			NotNull& operator=(NotNull&&) = default;
			NotNull& operator=(NotNull const&) = default;
			NotNull& operator=(std::nullptr_t) = delete;

			Type& operator*()
			{
				return *m_pointer;
			}

			Type* operator->()
			{
				return &(*m_pointer);
			}

			constexpr operator PointerType() const
			{
				return get();
			}

		private:
			// Attributes
			PointerType m_pointer;
		};
	}
}
