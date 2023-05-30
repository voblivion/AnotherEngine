#pragma once

#include <vob/aoe/ecs/entity_list.h>


namespace vob::aoecs
{
	namespace detail
	{
		template <typename... TComponents>
		class entity_map_list_view;

		template <>
		class entity_map_list_view<>
		{
		public:
			explicit entity_map_list_view(entity_list const& a_entityList)
				: m_entityList{ a_entityList }
			{}

			auto size() const
			{
				return m_entityList.get().size();
			}

			static void get() {}

			auto find_index(entity_id const a_id) const
			{
				return m_entityList.get().find_index(a_id);
			}

			auto get_id(std::size_t const a_index) const
			{
				return m_entityList.get().get_id(a_index);
			}

		private:
			std::reference_wrapper<entity_list const> m_entityList;
		};

		template <typename TComponent, typename... TOtherComponents>
		class entity_map_list_view<TComponent, TOtherComponents...>
			: public entity_map_list_view<TOtherComponents...>
		{
			using base = entity_map_list_view<TOtherComponents...>;
		public:
			explicit entity_map_list_view(entity_list const& a_entityList)
				: base{ a_entityList }
				, m_componentList{ *a_entityList.find_components<TComponent>() }
			{}

			auto& get(std::type_identity<TComponent>) const
			{
				return m_componentList.get();
			}
			using base::get;

			using base::find_index;

		private:
			std::reference_wrapper<component_list<TComponent>> m_componentList;
		};

		template <typename TComponent, typename... TOtherComponents>
		class entity_map_list_view<TComponent const, TOtherComponents...>
			: public entity_map_list_view<TOtherComponents...>
		{
			using base = entity_map_list_view<TOtherComponents...>;
		public:
			explicit entity_map_list_view(entity_list const& a_entityList)
				: base{ a_entityList }
				, m_componentList{ *a_entityList.find_components<TComponent>() }
			{}

			auto const& get(std::type_identity<TComponent>) const
			{
				return m_componentList.get();
			}
			using base::get;

			using base::find_index;

		private:
			std::reference_wrapper<component_list<TComponent> const> m_componentList;
		};

		template <typename TComponent, typename... TOtherComponents>
		class entity_map_list_view<TComponent*, TOtherComponents...>
			: public entity_map_list_view<TOtherComponents...>
		{
			using base = entity_map_list_view<TOtherComponents...>;
		public:
			explicit entity_map_list_view(entity_list const& a_entityList)
				: base{ a_entityList }
				, m_componentList{ a_entityList.find_components<TComponent>() }
			{}

			auto* get(std::type_identity<TComponent>) const
			{
				return m_componentList;
			}
			using base::get;

			using base::find_index;

		private:
			component_list<TComponent>* m_componentList;
		};

		template <typename TComponent, typename... TOtherComponents>
		class entity_map_list_view<TComponent const*, TOtherComponents...>
			: public entity_map_list_view<TOtherComponents...>
		{
			using base = entity_map_list_view<TOtherComponents...>;
		public:
			explicit entity_map_list_view(entity_list const& a_entityList)
				: base{ a_entityList }
				, m_componentList{ a_entityList.find_components<TComponent>() }
			{}

			auto const* get(std::type_identity<TComponent>) const
			{
				return m_componentList;
			}
			using base::get;

			using base::find_index;

		private:
			component_list<TComponent> const* m_componentList;
		};
	
		template <typename... TComponents>
		struct entity_map_observer_list_archetype;

		template <>
		struct entity_map_observer_list_archetype<>
		{
			static auto create()
			{
				return archetype{ std::pmr::unordered_set<std::type_index>{} };
			}
		};

		template <typename TComponent, typename... TOtherComponents>
		struct entity_map_observer_list_archetype<TComponent, TOtherComponents...>
		{
			static auto create()
			{
				auto ark = entity_map_observer_list_archetype<TOtherComponents...>::create();
				ark.add<TComponent>();
				return ark;
			}
		};

		template <typename TComponent, typename... TOtherComponents>
		struct entity_map_observer_list_archetype<TComponent*, TOtherComponents...>
		{
			static auto create()
			{
				return entity_map_observer_list_archetype<TOtherComponents...>::create();
			}
		};

		class entity_map_observer_list_holder_base
		{
		public:
			virtual void on_archetype_added(entity_list const& a_entityList) = 0;
		};

		template <typename... TComponents>
		struct entity_map_observer_list_holder;
	}

	template <typename... TComponents>
	class entity_map_observer_list
	{
	public:
		class const_iterator;

		class view
		{
		public:
			template <typename TComponent>
			requires std::is_pointer_v<decltype(std::declval<detail::entity_map_list_view<
				TComponents...>>().get(std::type_identity<TComponent>{}))>
			decltype(auto) get() const
			{
				auto componentList = m_listView->get(std::type_identity<TComponent>{});
				return componentList != nullptr ? &componentList->at(m_index) : nullptr;
			}

			template <typename TComponent>
			requires (!std::is_pointer_v<decltype(std::declval<detail::entity_map_list_view<
				TComponents...>>().get(std::type_identity<TComponent>{}))>)
				decltype(auto) get() const
			{
				auto& componentList = m_listView->get(std::type_identity<TComponent>{});
				return componentList.at(m_index);
			}

			friend bool operator!=(view const& a_lhs, view const& a_rhs)
			{
				return a_lhs.m_index != a_rhs.m_index || a_lhs.m_listView != a_rhs.m_listView;
			}

			auto get_id() const
			{
				return m_listView->get_id(m_index);
			}

		private:
			friend class const_iterator;

			view(
				std::size_t a_index,
				detail::entity_map_list_view<TComponents...> const* a_listView
			)
				: m_index{ a_index }
				, m_listView{ a_listView }
			{}

			std::size_t m_index = 0;
			detail::entity_map_list_view<TComponents...> const* m_listView = nullptr;
		};

		class const_iterator
		{
		public:

			using reference = view const&;
			using pointer = view const*;

			reference operator*() const
			{
				return m_view;
			}

			pointer operator->() const
			{
				return &m_view;
			}

			auto& operator++()
			{
				++m_view.m_index;
				if (m_view.m_index == m_view.m_listView->size())
				{
					m_view.m_index = 0;
					++m_view.m_listView;
				}
				return *this;
			}

			friend bool operator!=(const_iterator const& a_lhs, const_iterator const& a_rhs)
			{
				return a_lhs.m_view != a_rhs.m_view;
			}

			friend bool operator==(const_iterator const& a_lhs, const_iterator const& a_rhs)
			{
				return !(a_lhs != a_rhs);
			}

		private:
			friend class entity_map_observer_list<TComponents...>;

			const_iterator(
				std::size_t a_index,
				detail::entity_map_list_view<TComponents...> const* a_listView
			)
				: m_view{ a_index, a_listView }
			{}

			view m_view;
		};

		const_iterator begin() const
		{
			return { 0, m_entityLists.data() };
		}

		const_iterator end() const
		{
			return{ 0, m_entityLists.data() + m_entityLists.size() };
		}

		bool empty() const
		{
			return m_entityLists.empty();
		}

		auto& get_entity_lists()
		{
			return m_entityLists;
		}

		auto find(entity_id a_id) const
		{
			for (auto it = m_entityLists.begin(); it != m_entityLists.end(); ++it)
			{
				auto entityIndex = it->find_index(a_id);
				if (entityIndex != std::nullopt)
				{
					return const_iterator(*entityIndex, &(*it));
				}
			}

			return end();
		}

	private:
		template <typename... TComponents>
		friend struct entity_map_observer_list_holder;

		std::pmr::vector<detail::entity_map_list_view<TComponents...>> m_entityLists;
	};

	namespace detail
	{
		template <typename... TComponents>
		struct entity_map_observer_list_holder final
			: public entity_map_observer_list_holder_base
		{
			entity_map_observer_list_holder()
				: m_archetype{ entity_map_observer_list_archetype<TComponents...>::create() }
			{}

			void on_archetype_added(entity_list const& a_entityList) override
			{
				if (m_archetype <= a_entityList.get_archetype())
				{
					m_list.get_entity_lists().emplace_back(a_entityList);
				}
			}

			archetype const m_archetype;
			entity_map_observer_list<TComponents...> m_list;
		};
	}

	class entity_map
	{
	public:
		explicit entity_map(component_list_factory const& a_componentListFactory)
			: m_componentListFactory{ a_componentListFactory }
		{}

		using create_callback = std::function<void(entity_list::entity_view&)>;

		entity_list::entity_view add_entity(
			entity_id a_id
			, component_set const& a_componentSet
			, create_callback const* a_createCallback
		)
		{
			auto& entityList = get_or_create_archetype_list(a_componentSet.get_archetype());

			auto entityView = entityList.add(a_id, a_componentSet);
			
			if (a_createCallback != nullptr)
			{
				(*a_createCallback)(entityView);
			}

			return entityView;
		}

		void remove_entity(entity_id a_id)
		{
			for (auto& entityListEntry : m_entityLists)
			{
				if (entityListEntry.second->contains(a_id))
				{
					entityListEntry.second->remove(a_id);
					break;
				}
			}
		}

		std::optional<entity_list::entity_view> find_entity(entity_id a_id) const
		{
			for (auto const& entityListEntry : m_entityLists)
			{
				auto entityView = entityListEntry.second->find(a_id);
				if (entityView.has_value())
				{
					return entityView;
				}
			}
			return std::nullopt;
		}

		template <typename... TComponents>
		entity_map_observer_list<TComponents...> const& observe()
		{
			auto observerHolder = vob::mistd::pmr::polymorphic_ptr_util::make<
				detail::entity_map_observer_list_holder<TComponents...>>();
			auto& listView = observerHolder->m_list;

			for (auto const& entityList : m_entityLists)
			{
				observerHolder->on_archetype_added(*entityList.second);
			}

			m_observerHolders.emplace_back(std::move(observerHolder));
			return listView;
		}

	private:
		component_list_factory const& m_componentListFactory;
		std::pmr::unordered_map<archetype, vob::mistd::polymorphic_ptr<entity_list>> m_entityLists;
		std::pmr::vector<vob::mistd::polymorphic_ptr<detail::entity_map_observer_list_holder_base>>
			m_observerHolders;

		entity_list& get_or_create_archetype_list(archetype a_archetype)
		{
			// Try get existing list
			auto const entityListIt = m_entityLists.find(a_archetype);
			if (entityListIt != m_entityLists.end())
			{
				return *(entityListIt->second);
			}

			// Else create a new one
			auto entityListPtr = vob::mistd::pmr::polymorphic_ptr_util::make<entity_list>(
				m_componentListFactory, a_archetype);
			auto& entityList = *entityListPtr;
			for (auto& observerHolder : m_observerHolders)
			{
				observerHolder->on_archetype_added(entityList);
			}
			m_entityLists.emplace(std::move(a_archetype), std::move(entityListPtr));
			return entityList;
		}
	};
}
