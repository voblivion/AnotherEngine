#pragma once

#include "vob/aoe/rendering/RegistryControl.h"

#include <memory>


namespace vob::aoegl
{
	template <typename T>
	class Registry;
	template <typename T>
	class WeakHandle;

	template <typename T>
	class Handle
	{
	public:
		Handle() = default;
		Handle(Handle&& a_other)
		{
			std::swap(m_index, a_other.m_index);
			std::swap(m_version, a_other.m_version);
			std::swap(m_control, a_other.m_control);
		}

		Handle(Handle const& a_other)
			: m_index{ a_other.m_index }
			, m_version{ a_other.m_version }
			, m_control{ a_other.m_control }
		{
			if (m_index != -1)
			{
				if (auto const control = m_control.lock())
				{
					control->addReference(m_index);
				}
			}
		}

		~Handle()
		{
			if (m_index != -1)
			{
				if (auto const control = m_control.lock())
				{
					control->removeReference(m_index);
				}
			}
		}

		friend bool operator==(Handle const& a_lhs, Handle const& a_rhs)
		{
			return (a_lhs.m_index == a_rhs.m_index) && (a_lhs.m_version == a_rhs.m_version);
		}

		Handle& operator=(Handle&& a_other)
		{
			std::swap(m_index, a_other.m_index);
			std::swap(m_version, a_other.m_version);
			std::swap(m_control, a_other.m_control);
			return *this;
		}

		Handle& operator=(Handle const& a_other)
		{
			if (*this == a_other)
			{
				return *this;
			}

			if (m_index != -1)
			{
				if (auto const control = m_control.lock())
				{
					control->removeReference(m_index);
				}
			}

			m_index = a_other.m_index;
			m_version = a_other.m_version;
			m_control = a_other.m_control;

			if (m_index != -1)
			{
				if (auto const control = m_control.lock())
				{
					control->addReference(m_index);
				}
			}

			return *this;
		}

		bool isValid() const
		{
			return m_index != -1;
		}

	private:
		friend class Registry<T>;
		friend class WeakHandle<T>;

		Handle(int32_t a_index, int32_t a_version, std::shared_ptr<RegistryControl> const& a_control)
			: m_index{ a_index }
			, m_version{ a_version }
			, m_control{ a_control }
		{
			a_control->addReference(m_index);
		}

		int32_t m_index = -1;
		int32_t m_version = -1;
		std::weak_ptr<RegistryControl> m_control;
	};
}
