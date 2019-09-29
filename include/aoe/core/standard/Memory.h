#pragma once

#include <cassert>
#include <memory>
#include <memory_resource>

#include <aoe/core/standard/Allocator.h>
#include <aoe/core/standard/ADynamicType.h>
#include "Traits.h"

namespace aoe
{
	namespace sta
	{
		namespace detail
		{
			class APolymorphicBlock
				: public ADynamicType
			{
			public:
				// Methods
				virtual void* getPtr() noexcept = 0;
				virtual void destroy() noexcept = 0;
				virtual std::pmr::memory_resource* getResource() = 0;
			};

			template <typename Type>
			class PolymorphicBlock final
				: public APolymorphicBlock
			{
				// Aliases
				using Storage = std::aligned_union_t<1, Type>;
				using Block = PolymorphicBlock<Type>;
				using Allocator = Allocator<Block>;
				using AllocatorTraits = std::allocator_traits<Allocator>;

			public:
				// Constructors
				template <typename... Args>
				explicit PolymorphicBlock(std::pmr::memory_resource* a_resource
					, Args&&... a_args)
					: m_allocator{ a_resource }
				{
					// ReSharper disable All
					::new (static_cast<void*>(&m_storage))
						Type(std::forward<Args>(a_args)...);
					// ReSharper restore All
				}

				// Methods
				virtual void* getPtr() noexcept override
				{
					return getTypePtr();
				}

				Type* getTypePtr() noexcept
				{
					return reinterpret_cast<Type*>(&m_storage);
				}

				virtual void destroy() noexcept override
				{
					getTypePtr()->~Type();
					AllocatorTraits::destroy(m_allocator, this);
					AllocatorTraits::deallocate(m_allocator, this, 1);
				}

				virtual std::pmr::memory_resource* getResource() override
				{
					return m_allocator.resource();
				}

			private:
				// Attributes
				Allocator m_allocator;
				Storage m_storage;
			};
		}

		class PolymorphicDeleter
		{
		public:
			// Constructors
			explicit PolymorphicDeleter() = default;
			explicit PolymorphicDeleter(detail::APolymorphicBlock* a_polymorphicBlock)
				: m_polymorphicBlock{ a_polymorphicBlock }
			{}

			// Methods
			std::pmr::memory_resource* getResource() const
			{
				if (m_polymorphicBlock != nullptr)
				{
					return m_polymorphicBlock->getResource();
				}
				return nullptr;
			}

			// Operators
			void operator()(void* a_ptr)
			{
				assert(m_polymorphicBlock != nullptr
					&& m_polymorphicBlock->getPtr() == a_ptr);
				m_polymorphicBlock->destroy();
				m_polymorphicBlock = nullptr;
			}

		private:
			// Attributes
			detail::APolymorphicBlock* m_polymorphicBlock;
		};

		template <typename Type>
		using PolymorphicPtr = std::unique_ptr<Type, PolymorphicDeleter>;

		template <typename Type, typename... Args>
		PolymorphicPtr<Type> allocatePolymorphicWith(
			std::pmr::memory_resource* a_resource
			, Args&&... a_args)
		{
			using Block = detail::PolymorphicBlock<Type>;
			using Allocator = Allocator<Block>;
			using AllocatorTraits = std::allocator_traits<Allocator>;

			// Allocate block
			Allocator t_allocator{ a_resource };
			auto const t_block = AllocatorTraits::allocate(t_allocator, 1);

			// Construct block
			try
			{
				AllocatorTraits::construct(t_allocator, t_block
					, a_resource, std::forward<Args>(a_args)...);
			}
			catch (...)
			{
				AllocatorTraits::deallocate(t_allocator, t_block, 1);
				throw;
			}

			return PolymorphicPtr<Type>{t_block->getTypePtr()
				, PolymorphicDeleter{ t_block } };
		}

		template <typename Type, typename... Args>
		auto allocatePolymorphic(Args&&... a_args)
		{
			return allocatePolymorphicWith<Type, Args...>(
				std::pmr::get_default_resource()
				, std::forward<Args>(a_args)...);
		}

		template <typename TargetType, typename SourceType>
		PolymorphicPtr<TargetType> staticPolymorphicCast(PolymorphicPtr<SourceType> a_ptr)
		{
			return { static_cast<TargetType*>(a_ptr.release()), a_ptr.get_deleter() };
		}
	}
}
