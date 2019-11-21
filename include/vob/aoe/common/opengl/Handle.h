#pragma once

#include <vob/aoe/core/type/ADynamicType.h>
#include <vob/aoe/common/opengl/Manager.h>


namespace vob::aoe::ogl
{
	template <typename ResourceT>
	struct Handle final
		: public type::ADynamicType
	{
#pragma region Constructors
		template <typename... Args>
		explicit Handle(
			Manager<ResourceT>& a_manager
			, std::pmr::memory_resource* a_memoryResource
			, Args&&... a_args
		)
			: m_manager{ a_manager }
			, m_resource{
				std::allocate_shared<ResourceT>(
					std::pmr::polymorphic_allocator<ResourceT>(a_memoryResource)
					, std::forward<Args>(a_args)...
				)
			}
		{
			if (m_resource != nullptr)
			{
				m_manager.requestCreate(m_resource);
			}
		}

		Handle(Handle&&) = default;
		Handle(Handle const&) = delete;

		~Handle()
		{
			if (m_resource != nullptr && m_resource->isReady())
			{
				m_manager.requestDestroy(std::move(m_resource));
			}
		}
#pragma endregion
#pragma region Methods
		ResourceT& resource() const
		{
			return *m_resource;
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
#pragma region Operators
		Handle& operator=(Handle&&) = default;
		Handle& operator=(Handle const&) = delete;
#pragma endregion
	private:
#pragma region Attributes
		Manager<ResourceT>& m_manager;
		std::shared_ptr<ResourceT> m_resource;
#pragma endregion
	};
}