#pragma once
#include <cassert>
#include <vector>
#include <mutex>
#include <vob/aoe/common/render/OpenGl.h>


namespace vob::aoe::common
{
	template <typename TGraphicResource>
	class IGraphicResourceManager
	{
	public:
		virtual ~IGraphicResourceManager() = default;
		virtual void requestCreate(std::weak_ptr<TGraphicResource const> a_resource) = 0;
		virtual void requestDestroy(std::shared_ptr<TGraphicResource const> a_resource) = 0;
		virtual void update() = 0;
	};

	template <typename TGraphicResource>
	class SingleWindowGraphicResourceManager
		: public IGraphicResourceManager<TGraphicResource>
	{
	public:
		void requestCreate(std::weak_ptr<TGraphicResource const> a_resource) override
		{
			auto lock = std::lock_guard{ m_resourcesToCreateMutex };
			m_resourcesToCreate.emplace_back(std::move(a_resource));
		}

		void requestDestroy(std::shared_ptr<TGraphicResource const> a_resource) override
		{
			auto lock = std::lock_guard{ m_resourcesToDestroyMutex };
			m_resourcesToDestroy.emplace_back(std::move(a_resource));
		}

		void update() override
		{
			processCreateStates();
			processDestroyStates();
		}

	private:

		void processCreateStates()
		{
			auto lock = std::lock_guard{ m_resourcesToCreateMutex };
			for (auto& someResource : m_resourcesToCreate)
			{
				if (auto resource = someResource.lock())
				{
					assert(!resource->isReady());
					resource->create();
				}
			}
			m_resourcesToCreate.clear();
		}
		void processDestroyStates()
		{
			auto lock = std::lock_guard{ m_resourcesToDestroyMutex };
			for (auto& resource : m_resourcesToDestroy)
			{
				assert(resource->isReady());
				resource->destroy();
			}
			m_resourcesToDestroy.clear();
		}

		// TODO : double buffer queue ? lock-free queue ?
		std::vector<std::weak_ptr<TGraphicResource const>> m_resourcesToCreate{};
		std::mutex m_resourcesToCreateMutex{};
		std::vector<std::shared_ptr<TGraphicResource const>> m_resourcesToDestroy{};
		std::mutex m_resourcesToDestroyMutex{};
	};

	template <typename TGraphicResource>
	class MultiWindowGraphicResourceManager
		: public SingleWindowGraphicResourceManager<TGraphicResource>
	{
	public:
		void requestCreate(std::weak_ptr<TGraphicResource const> a_resource) override
		{
			
		}

		void requestDestroy(std::shared_ptr<TGraphicResource const> a_resource) override
		{
			
		}

		void changeWindow(GLFWwindow* a_windowNativeHandle)
		{
			
		}

	private:
		GLFWwindow* m_windowNativeHandle = nullptr;
	};
}