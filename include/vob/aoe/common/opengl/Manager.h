#pragma once
#include <vector>
#include <mutex>


namespace vob::aoe::ogl
{
	template <typename ResourceT>
	class Manager
	{
	public:
		void requestCreate(std::weak_ptr<ResourceT const> a_resource)
		{
			auto lock = std::lock_guard{ m_resourcesToCreateMutex };
			m_resourcesToCreate.emplace_back(std::move(a_resource));
		}

		void requestDestroy(std::shared_ptr<ResourceT const> a_resource)
		{
			auto lock = std::lock_guard{ m_resourcesToDestroyMutex };
			m_resourcesToDestroy.emplace_back(std::move(a_resource));
		}

		void update()
		{
			processCreates();
			processDestroys();
		}

	private:

		void processCreates()
		{
			auto lock = std::lock_guard{ m_resourcesToCreateMutex };
			for (auto& someResource : m_resourcesToCreate)
			{
				if (auto resource = someResource.lock())
				{
					resource->create();
				}
			}
			m_resourcesToCreate.clear();
		}
		void processDestroys()
		{
			auto lock = std::lock_guard{ m_resourcesToDestroyMutex };
			for (auto& resource : m_resourcesToDestroy)
			{
				resource->destroy();
			}
			m_resourcesToDestroy.clear();
		}

		// TODO : double buffer queue ? lock-free queue ?
		std::pmr::vector<std::weak_ptr<ResourceT const>> m_resourcesToCreate{};
		std::mutex m_resourcesToCreateMutex{};
		std::pmr::vector<std::shared_ptr<ResourceT const>> m_resourcesToDestroy{};
		std::mutex m_resourcesToDestroyMutex{};
	};
}