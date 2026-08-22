#pragma once

#include "vob/aoe/rendering/resources/Handle.h"

#include <compare>


namespace vob::aoegl
{
	template <typename T>
	class Registry;

	template <typename T>
	class WeakHandle
	{
	public:
		WeakHandle() = default;
		WeakHandle(WeakHandle<T> const& a_handle) = default;
		WeakHandle(Handle<T> const& a_handle)
			: m_index{ a_handle.m_index }
			, m_version{ a_handle.m_version }
		{

		}

		WeakHandle& operator=(WeakHandle<T> const& a_handle)
		{
			m_index = a_handle.m_index;
			m_version = a_handle.m_version;
			return *this;
		}

		WeakHandle& operator=(Handle<T> const& a_handle)
		{
			m_index = a_handle.m_index;
			m_version = a_handle.m_version;
			return *this;
		}

		friend auto operator<=>(WeakHandle const&, WeakHandle const&) = default;

		bool isValid() const
		{
			return m_index != -1;
		}

	private:
		friend class Registry<T>;

		int32_t m_index = -1;
		int32_t m_version = -1;
	};
}
