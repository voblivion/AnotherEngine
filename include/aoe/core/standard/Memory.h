#pragma once

#include <cassert>
#include <memory>
#include <memory_resource>

#include <aoe/core/standard/Allocator.h>
#include <aoe/core/standard/ADynamicType.h>

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
			};

			template <typename Type, typename Allocator>
			class PolymorphicBlock final
				: public APolymorphicBlock
			{
				// Aliases
				using Storage = std::aligned_union_t<1, Type>;
				using Block = PolymorphicBlock<Type, Allocator>;
				using BlockAllocator = RebindAlloc<Allocator, Block>;
				using AllocatorTraits = std::allocator_traits<BlockAllocator>;

			public:
				// Constructors
				template <typename... Args>
				explicit PolymorphicBlock(Allocator const& a_allocator
					, Args&&... a_args)
					: m_allocator{ std::move(a_allocator) }
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
					BlockAllocator t_allocator{ m_allocator };
					AllocatorTraits::destroy(t_allocator, this);
					AllocatorTraits::deallocate(t_allocator, this, 1);
				}

			private:
				// Attributes
				BlockAllocator m_allocator;
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

		template <typename Type, typename Allocator>
		std::size_t getPolymorphicAllocationSize()
		{
			return sizeof(detail::PolymorphicBlock<Type, Allocator>);
		}

		template <typename Type, typename Allocator, typename... Args>
		PolymorphicPtr<Type> allocatePolymorphic(Allocator const& a_allocator
			, Args&&... a_args)
		{
			using Block = detail::PolymorphicBlock<Type, Allocator>;
			using BlockAllocator = RebindAlloc<Allocator, Block>;
			using BlockAllocatorTraits = std::allocator_traits<BlockAllocator>;

			// Allocate block
			BlockAllocator t_allocator{ a_allocator };
			auto const t_block = BlockAllocatorTraits::allocate(t_allocator, 1);

			// Construct block
			try
			{
				BlockAllocatorTraits::construct(t_allocator, t_block
					, a_allocator, std::forward<Args>(a_args)...);
			}
			catch (...)
			{
				BlockAllocatorTraits::deallocate(t_allocator, t_block, 1);
				throw;
			}

			return PolymorphicPtr<Type>{t_block->getTypePtr()
				, PolymorphicDeleter{ t_block } };
		}

		template <typename TargetType, typename SourceType>
		PolymorphicPtr<TargetType> staticPolymorphicCast(PolymorphicPtr<SourceType> a_ptr)
		{
			return { static_cast<TargetType*>(a_ptr.release()), a_ptr.get_deleter() };
		}
	}
}