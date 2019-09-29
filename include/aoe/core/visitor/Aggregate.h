#pragma once

#include <utility>

namespace aoe
{
	namespace vis
	{
		namespace detail
		{
			struct Empty{};
		}

		template <typename Type, typename BaseType = detail::Empty>
		class Aggregate
			: public BaseType
		{
		public:
			// Constructor
			template <typename... Args>
			explicit Aggregate(Args&&... a_args)
				: BaseType{ std::forward<Args>(a_args)... }
			{}

			// Methods
			void preAccept() {}
			void preAccept() const {}
			void postAccept() {}
			void postAccept() const {}

			template <typename VisitorType>
			void accept(VisitorType& a_visitor) const
			{
				static_cast<Type const&>(*this).preAccept();
				Type::makeVisit(a_visitor, static_cast<Type const&>(*this));
				static_cast<Type const&>(*this).postAccept();
			}

			template <typename VisitorType>
			void accept(VisitorType& a_visitor)
			{
				static_cast<Type&>(*this).preAccept();
				Type::makeVisit(a_visitor, static_cast<Type&>(*this));
				static_cast<Type&>(*this).postAccept();
			}
		};
	}
}