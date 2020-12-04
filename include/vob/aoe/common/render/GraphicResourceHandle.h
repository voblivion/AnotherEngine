#pragma once

#include <vob/aoe/core/type/ADynamicType.h>
#include <vob/aoe/common/render/Manager.h>


namespace vob::aoe::common
{
	template <typename ResourceT>
	struct GraphicResourceHandle final
		: public type::ADynamicType
	{
#pragma region Constructors
		template <typename... Args>
		explicit GraphicResourceHandle(
			IGraphicResourceManager<ResourceT>& a_manager
			, Args&&... a_args
		)
			: m_manager{ a_manager }
			, m_resource{ std::make_shared<ResourceT>(std::forward<Args>(a_args)...) }
		{
			if (m_resource != nullptr)
			{
				m_manager.requestCreate(m_resource);
			}
		}

		GraphicResourceHandle(GraphicResourceHandle&&) = default;
		// TODO : could allow copy but reset m_resource ?
		GraphicResourceHandle(GraphicResourceHandle const&) = delete;

		~GraphicResourceHandle()
		{
			if (m_resource != nullptr && m_resource->isReady())
			{
				m_manager.requestDestroy(std::move(m_resource));
			}
		}
#pragma endregion
#pragma region Operators
		GraphicResourceHandle& operator=(GraphicResourceHandle&&) = default;
		// TODO : could allow copy but reset m_resource ?
		GraphicResourceHandle& operator=(GraphicResourceHandle const&) = delete;
		auto const& operator*() const
		{
			return *resource();
		}
		auto& operator*()
		{
			return *resource();
		}
		auto operator->() const
		{
			return resource();
		}
		auto operator->()
		{
			return resource();
		}
#pragma endregion
#pragma region Methods
		ResourceT const* resource() const
		{
			return m_resource.get();
		}
		ResourceT* resource()
		{
			return m_resource.get();
		}

		template <typename VisitorT>
		void accept(VisitorT& a_visitor)
		{
			a_visitor.visit(*m_resource);
		}

		template <typename VisitorT>
		void accept(VisitorT& a_visitor) const
		{
			a_visitor.visit(*m_resource);
		}
#pragma endregion
	private:
#pragma region Attributes
		IGraphicResourceManager<ResourceT>& m_manager;
		std::shared_ptr<ResourceT> m_resource;
#pragma endregion
	};
}
