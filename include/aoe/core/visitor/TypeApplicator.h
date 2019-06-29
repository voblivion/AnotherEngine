#pragma once

#include <typeindex>

#include <aoe/core/standard/ADynamicType.h>

namespace aoe
{
	namespace visitor
	{
		namespace detail
		{
			template <typename Visitor>
			class IVisitorApplicator
				: public sta::ADynamicType
			{
			public:
				virtual void apply(Visitor& a_visitor, void* a_ptr) const = 0;
			};

			template <typename Visitor, typename Type>
			class VisitorApplicator final
				: public IVisitorApplicator<Visitor>
			{
			public:
				void apply(Visitor& a_visitor, void* a_ptr) const final override
				{
					static_cast<Type*>(a_ptr)->accept(a_visitor);
				}
			};
		}

		template <typename Visitor>
		class TypeApplicator
		{
		public:
			// Constructor
			TypeApplicator() = default;

			explicit TypeApplicator(sta::Allocator<std::byte> const& a_allocator)
				: m_applicators{ a_allocator }
			{}

			// Methods
			bool isRegistered(std::type_index a_typeIndex) const
			{
				return m_applicators.find(a_typeIndex) != m_applicators.end();
			}

			template <typename Type>
			bool isRegistered() const
			{
				return isRegistered(typeid(Type));
			}

			template <typename Type>
			void registerType()
			{
				assert(!isRegistered<Type>());
				m_applicators.emplace(typeid(Type)
					, sta::allocatePolymorphic<
					detail::VisitorApplicator<Visitor, Type>>(
						m_applicators.get_allocator()));
			}

			template <typename BaseType>
			void apply(Visitor& a_visitor, BaseType& a_value) const
			{
				assert(isRegistered(typeid(a_value)));
				auto it = m_applicators.find(typeid(a_value));
				it->second->apply(a_visitor, &a_value);
			}

			auto getAllocator()
			{
				return m_applicators.get_allocator();
			}

		private:
			// Attributes
			std::pmr::unordered_map<std::type_index
				, sta::PolymorphicPtr<
				detail::IVisitorApplicator<Visitor>>> m_applicators;
		};
	}
}