#pragma once

#include <memory_resource>

namespace aoe
{
	namespace sta
	{
		namespace pmr
		{
			template <std::size_t bufferSize>
			class BufferResource final
				: public std::pmr::memory_resource
			{
			public:
				// Constructors
				explicit BufferResource(std::pmr::memory_resource* const a_resource
					= std::pmr::get_default_resource())
					: m_availableSpace{ bufferSize }
					, m_resource{ a_resource }
				{
					m_currentBuffer = m_buffer;
				}

				// Methods
				std::pmr::memory_resource* upstreamResource() const
				{
					return m_resource;
				}

			protected:
				// Methods
				virtual void* do_allocate(std::size_t a_bytes
					, std::size_t a_alignment) override
				{
					if (auto const t_ptr = std::align(a_alignment, a_bytes
						, m_currentBuffer, m_availableSpace))
					{
						m_currentBuffer = static_cast<std::byte*>(t_ptr) + a_bytes;
						m_availableSpace -= a_bytes;
						return t_ptr;
					}

					return m_resource->allocate(a_bytes, a_alignment);
				}

				virtual void do_deallocate(void* a_ptr, std::size_t a_bytes
					, std::size_t a_alignment) override
				{
					if (a_ptr < m_buffer || m_currentBuffer <= a_ptr)
					{
						m_resource->deallocate(a_ptr, a_bytes, a_alignment);
					}
				}

				virtual bool do_is_equal(
					const std::pmr::memory_resource& a_resource) const noexcept override
				{
					return &a_resource == this;
				}

			private:
				std::size_t m_availableSpace;
				alignas(alignof(std::max_align_t)) std::byte m_buffer[bufferSize];
				void* m_currentBuffer;
				std::pmr::memory_resource* m_resource;
			};

			class DynamicBufferResource final
				: public std::pmr::memory_resource
			{
			public:
				// Constructors
				explicit DynamicBufferResource(std::size_t const a_bufferSize,
					std::pmr::memory_resource* const a_resource
					= std::pmr::get_default_resource()) noexcept
					: m_bufferSize{ a_bufferSize }
					, m_availableSpace{ a_bufferSize }
					, m_resource{ a_resource }
				{}

				DynamicBufferResource(DynamicBufferResource&& a_resource) noexcept
					: m_bufferSize{ a_resource.m_bufferSize }
					, m_availableSpace{ a_resource.m_availableSpace }
					, m_buffer{ a_resource.m_buffer }
					, m_currentBuffer{ a_resource.m_currentBuffer }
					, m_resource{ a_resource.m_resource }
				{
					a_resource.m_buffer = nullptr;
				}

				DynamicBufferResource(DynamicBufferResource const&) = delete;

				~DynamicBufferResource()
				{
					if (m_buffer != nullptr)
					{
						m_resource->deallocate(m_buffer, m_bufferSize
							, alignof(std::max_align_t));
					}
				}

				// Methods
				std::pmr::memory_resource* upstreamResource() const
				{
					return m_resource;
				}

				// Operators
				DynamicBufferResource& operator=(
					DynamicBufferResource&& a_resource) noexcept
				{
					m_bufferSize = a_resource.m_bufferSize;
					m_availableSpace = a_resource.m_availableSpace;
					m_buffer = a_resource.m_buffer;
					a_resource.m_buffer = nullptr;
					m_currentBuffer = a_resource.m_currentBuffer;
					m_resource = a_resource.m_resource;
					return *this;
				}
				DynamicBufferResource& operator=(
					DynamicBufferResource const&) = delete;

			protected:
				// Methods
				virtual void* do_allocate(std::size_t a_bytes
					, std::size_t a_alignment) override
				{
					if (m_buffer == nullptr)
					{
						m_buffer = m_resource->allocate(m_bufferSize
							, alignof(std::max_align_t));
						m_currentBuffer = m_buffer;
					}

					if (auto const t_ptr = std::align(a_alignment, a_bytes
						, m_currentBuffer, m_availableSpace))
					{
						m_currentBuffer = static_cast<std::byte*>(t_ptr) + a_bytes;
						m_availableSpace -= a_bytes;
						return t_ptr;
					}

					return m_resource->allocate(a_bytes, a_alignment);
				}

				virtual void do_deallocate(void* a_ptr, std::size_t a_bytes
					, std::size_t a_alignment) override
				{
					if (a_ptr < m_buffer || m_currentBuffer <= a_ptr)
					{
						m_resource->deallocate(a_ptr, a_bytes, a_alignment);
					}
				}

				virtual bool do_is_equal(
					const std::pmr::memory_resource& a_resource) const noexcept override
				{
					return &a_resource == this;
				}

			private:
				// Attributes
				std::size_t m_bufferSize;
				std::size_t m_availableSpace;
				void* m_buffer = nullptr;
				void* m_currentBuffer = nullptr;
				std::pmr::memory_resource* m_resource;
			};
		}
	}
}