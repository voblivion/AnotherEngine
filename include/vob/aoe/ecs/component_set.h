#pragma once

#include <vob/aoe/ecs/archetype.h>

#include <vob/misc/std/polymorphic_ptr_util.h>

#include <vob/misc/visitor/container.h>
#include <vob/misc/visitor/accept.h>


namespace vob::aoecs
{
	class component_set;
}

namespace vob::misvi
{
	template <typename TVisitor>
	bool accept(TVisitor& a_visitor, aoecs::component_set& a_componentSet);

	template <typename TVisitor>
	bool accept(TVisitor& a_visitor, aoecs::component_set const& a_componentSet);
}

namespace vob::aoecs
{
	namespace detail
	{
		struct component_holder_base
		{
			virtual ~component_holder_base() = default;

			virtual const std::type_info& get_type_id() const = 0;
		};

		template <typename TComponent>
		struct component_holder
			: public component_holder_base
		{
			template <typename... TArgs>
			explicit component_holder(TArgs&&... a_args)
				: m_component{ std::forward<TArgs>(a_args)... }
			{}

			const std::type_info& get_type_id() const override
			{
				return typeid(TComponent);
			}

			TComponent m_component;
		};
	}

	class component_set
	{
	public:
		component_set() = default;
		~component_set() = default;
		component_set(component_set const&) = delete;
		component_set(component_set&&) = default;
		component_set& operator=(component_set const&) = delete;
		component_set& operator=(component_set&&) = default;

		auto size() const
		{
			return m_components.size();
		}

		template <typename TComponent, typename... TArgs>
		TComponent& add(TArgs&&... a_args)
		{
			auto componentHolder = vob::mistd::pmr::polymorphic_ptr_util::make<
				detail::component_holder<TComponent>>(std::forward<TArgs>(a_args)...);
			auto& component = componentHolder->m_component;
			m_components[typeid(TComponent)] = std::move(componentHolder);
			return component;
		}

		template <typename TComponent>
		TComponent const* find() const
		{
			auto const it = m_components.find(typeid(std::remove_const_t<TComponent>));
			if (it == m_components.end())
			{
				return nullptr;
			}

			using holder_type = detail::component_holder<TComponent>;
			auto holder = static_cast<holder_type const*>(it->second.get());
			return &holder->m_component;
		}

		template <typename TComponent>
		TComponent* find()
		{
			auto const it = m_components.find(typeid(std::remove_const_t<TComponent>));
			if (it == m_components.end())
			{
				return nullptr;
			}

			using holder_type = detail::component_holder<TComponent>;
			auto holder = static_cast<holder_type*>(it->second.get());
			return &holder->m_component;
		}

		auto get_archetype() const
		{
			return archetype{ m_components };
		}

	private:
		template <typename TVisitor>
		friend bool vob::misvi::accept(TVisitor& a_visitor, component_set& a_componentSet);
		template <typename TVisitor>
		friend bool vob::misvi::accept(TVisitor& a_visitor, component_set const& a_componentSet);

		std::pmr::unordered_map<
			std::type_index,
			vob::mistd::polymorphic_ptr<detail::component_holder_base>
		> m_components;
	};
}

namespace vob::misvi
{
	template <typename TVisitor>
	bool accept(TVisitor& a_visitor, aoecs::component_set& a_componentSet)
	{
		std::pmr::vector<mistd::polymorphic_ptr<aoecs::detail::component_holder_base>> components;
		accept(a_visitor, components);
		for (auto& component : components)
		{
			a_componentSet.m_components.emplace(component->get_type_id(), std::move(component));
		}
		return true;
	}

	template <typename TVisitor, typename TComponent>
	bool accept(
		TVisitor& a_visitor, aoecs::detail::component_holder<TComponent>& a_componentHolder)
	{
		return accept(a_visitor, a_componentHolder.m_component);
	}
}
