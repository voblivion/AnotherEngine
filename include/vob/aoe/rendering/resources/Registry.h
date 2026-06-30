#pragma once

#include "vob/aoe/rendering/RegistryControl.h"
#include "vob/aoe/rendering/resources/Handle.h"
#include "vob/aoe/rendering/resources/WeakHandle.h"

#include "vob/aoe/debug/Check.h"

#include "vob/misc/std/container_util.h"

#include <vector>


namespace vob::aoegl
{
	// Registry operations must only be called from the render thread.
	template <typename T>
	class Registry
	{
	public:
		Registry() = default;
		Registry(Registry const&) = delete;
		Registry& operator=(Registry const&) = delete;
		~Registry()
		{
			VOB_AOE_CHECK_TERMINATE_SLOW(m_entries.size() == m_freeIndices.size(), "Registry is destroyed before all resources have been released.");
		}

		Handle<T> add(T a_value)
		{
			auto index = -1;
			if (!m_freeIndices.empty())
			{
				index = m_freeIndices.back();
				m_freeIndices.pop_back();
				m_entries[index].value = std::move(a_value);
			}
			else
			{
				index = mistd::isize(m_entries);
				m_entries.emplace_back(std::move(a_value));
			}

			++m_entries[index].version;
			return Handle<T>{ index, m_entries[index].version, m_control };
		}

		template <typename... TArgs>
		Handle<T> emplace(TArgs&&... a_args)
		{
			return add(T{ std::forward<TArgs>(a_args)... });
		}

		T const& get(Handle<T> const& a_handle) const
		{
			return get(a_handle.m_index, a_handle.m_version);
		}

		T const& get(WeakHandle<T> const& a_handle) const
		{
			return get(a_handle.m_index, a_handle.m_version);
		}

		template <typename TFunc>
		void processUnusedResources(TFunc a_func)
		{
			auto const releasedIndices = m_control->pollReleasedIndices();
			m_freeIndices.reserve(m_freeIndices.size() + releasedIndices.size());
			for (auto const index : releasedIndices)
			{
				a_func(std::move(m_entries[index].value));
				m_freeIndices.push_back(index);
			}
		}

	private:
		T const& get(int32_t const a_index, int32_t const a_version) const
		{
			VOB_AOE_CHECK_BREAK(a_index < mistd::isize(m_entries), "Invalid registry index.");
			VOB_AOE_CHECK_BREAK(a_version == m_entries[a_index].version, "Invalid registry version.");
			return m_entries[a_index].value;
		}

		struct Entry
		{
			T value;
			int32_t version = -1;
		};

		std::shared_ptr<RegistryControl> m_control = std::make_shared<RegistryControl>();
		std::vector<Entry> m_entries;
		std::vector<int32_t> m_freeIndices;
	};
}
