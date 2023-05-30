#pragma once

#include <vob/aoe/ecs/component_list_factory.h>

#include <vob/misc/std/ignorable_assert.h>

#include <cassert>
#include <optional>


namespace vob::aoecs
{
	using entity_id = std::uint64_t;

	constexpr entity_id k_invalidEntityId = -1;

	class entity_list
	{
	public:
		explicit entity_list(
			component_list_factory const& a_componentListFactory, archetype const& a_archetype)
		{
			m_componentLists.reserve(a_archetype.size());
			for (auto const& type : a_archetype)
			{
				auto componentList = a_componentListFactory.create(type);
				assert(componentList != nullptr);
				m_componentLists.emplace(type, std::move(componentList));
			}
		}

		auto get_id(std::size_t const a_index) const
		{
			return m_ids[a_index];
		}

		class entity_view
		{
		public:
			entity_view(entity_list const& a_list, std::size_t a_index)
				: m_list{ a_list }
				, m_index{ a_index }
			{}

			template <typename TComponent>
			TComponent* get_component() const
			{
				auto const componentList = m_list.get().find_components<
					std::remove_cvref_t<TComponent>>();
				if (componentList == nullptr)
				{
					return nullptr;
				}

				return &(componentList->at(m_index));
			}

			auto get_id() const
			{
				return m_list.get().get_id(m_index);
			}

		private:
			std::reference_wrapper<entity_list const> m_list;
			std::size_t m_index;
		};

		entity_view add(entity_id a_id, component_set const& a_componentSet)
		{
			assert(get_archetype() == a_componentSet.get_archetype());
			auto const index = m_ids.size();
			auto [it, added] = m_indices.emplace(a_id, index);
			assert(added);
			m_ids.emplace_back(a_id);
			for (auto& componentList : m_componentLists)
			{
				componentList.second->add_from(a_componentSet);
			}
			return { *this, index };
		}

		auto size() const
		{
			return m_ids.size();
		}

		void remove(entity_id a_id)
		{
			auto const it = m_indices.find(a_id);
			if (it == m_indices.end())
			{
				return;
			}

			auto const index = it->second;
			std::swap(m_ids[index], m_ids.back());
			m_indices[m_ids[index]] = index;
			m_ids.pop_back();
			m_indices.erase(it);

			for (auto& componentList : m_componentLists)
			{
				componentList.second->swap_remove_at(index);
			}
		}

		bool contains(entity_id a_id) const
		{
			return m_indices.contains(a_id);
		}

		std::optional<std::size_t> find_index(entity_id const a_id) const
		{
			auto const it = m_indices.find(a_id);
			if (it == m_indices.end())
			{
				return std::nullopt;
			}

			return it->second;
		}

		std::optional<entity_view> find(entity_id a_id)
		{
			auto const it = m_indices.find(a_id);
			if (it == m_indices.end())
			{
				return std::nullopt;
			}

			return entity_view{ *this, it->second };
		}

		template <typename TComponent>
		bool has_component() const
		{
			return m_componentLists.contains(typeid(TComponent));
		}

		archetype get_archetype() const
		{
			return archetype{ m_componentLists };
		}

		template <typename TComponent>
		component_list<TComponent>* find_components() const
		{
			auto const it = m_componentLists.find(typeid(TComponent));
			if (it == m_componentLists.end())
			{
				return nullptr;
			}

			return static_cast<component_list<TComponent>*>(it->second.get());
		}

	private:
		std::pmr::unordered_map<entity_id, std::size_t> m_indices;
		std::pmr::vector<entity_id> m_ids;
		std::pmr::unordered_map<std::type_index, vob::mistd::polymorphic_ptr<component_list_base>>
			m_componentLists;
	};
}
