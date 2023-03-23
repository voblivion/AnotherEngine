#pragma once

#include <vob/aoe/api.h>

#include <vob/misc/type/clone.h>
#include <vob/misc/visitor/is_visitable.h>


namespace vob::_aoecs
{
	class basic_component_holder
	{
	public:
		virtual ~basic_component_holder() = default;
	};

	using component_holder_cloner = misty::pmr::clone_copier<basic_component_holder>;

	template <typename TComponent>
	class component_holder
		: public basic_component_holder
	{
	public:
		template <typename... TArgs>
		component_holder(TArgs&&... a_args)
			: m_component{ std::forward<TArgs>(a_args)... }
		{}

		TComponent m_component;
	};

	using basic_component_holder_clone = misty::pmr::clone<basic_component_holder, component_holder_cloner>;

	template <typename TComponent>
	using component_holder_clone = misty::pmr::clone<component_holder<TComponent>, component_holder_cloner>;
}

namespace vob::misvi
{
	template <typename TVisitor, typename TComponent>
	requires is_visitable_free<TVisitor, TComponent>
	bool accept(TVisitor& a_visitor, _aoecs::component_holder<TComponent>& a_componentHolder)
	{
		return accept(a_visitor, a_componentHolder.m_component);
	}

	template <typename TVisitor, typename TComponent>
	requires is_visitable_member<TVisitor, TComponent>
	bool accept(TVisitor& a_visitor, _aoecs::component_holder<TComponent>& a_componentHolder)
	{
		return a_componentHolder.m_component.accept(a_visitor);
	}

	template <typename TVisitor, typename TComponent>
	requires is_visitable_static<TVisitor, TComponent>
	bool accept(TVisitor& a_visitor, _aoecs::component_holder<TComponent>& a_componentHolder)
	{
		return TComponent::accept(a_visitor, a_componentHolder.m_component);
	}
}

