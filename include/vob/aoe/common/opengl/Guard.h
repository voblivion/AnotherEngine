#pragma once

#include <utility>


namespace vob::aoe::ogl
{
	template <typename ResourceT>
	class Guard
	{
	public:
		template <typename... Args>
		explicit Guard(Args&&... a_args)
			: m_resource{ std::forward<Args>(a_args)... }
		{
			m_resource.create();
		}

		Guard(Guard&&) = delete;
		Guard(Guard const&) = delete;

		~Guard()
		{
			m_resource.destroy();
		}

		Guard& operator=(Guard&&) = delete;
		Guard& operator=(Guard const&) = delete;

		ResourceT const& resource() const
		{
			return m_resource;
		}

	private:
		ResourceT m_resource;
	};
}