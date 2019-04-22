#pragma once

#include <cassert>
#include <optional>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include <aoe/core/standard/Memory.h>

namespace aoe
{
	namespace sta
	{
		class TypeRegistry
		{
			struct TypeData
			{
				std::uint64_t m_id;
				std::size_t m_fastCastIndex;
			};
		public:
			// Constructors
			TypeRegistry()
			{
				m_typeData.emplace(typeid(void), TypeData{ 0, 0 });
				m_idToTypeIndex.emplace(0, typeid(void));
				m_types.emplace_back(
					std::make_pair(std::type_index{ typeid(void) }, 0));
			}

			template <typename Allocator>
			explicit TypeRegistry(Allocator const& a_allocator)
				: m_typeData{ a_allocator }
				, m_idToTypeIndex{ a_allocator }
				, m_types{ a_allocator }
			{
				m_typeData.emplace(typeid(void), TypeData{ 0, 0 });
				m_idToTypeIndex.emplace(0, typeid(void));
				m_types.emplace_back(
					std::make_pair(std::type_index{ typeid(void) }, 0));
			}

			// Methods
			bool isRegistered(std::type_index const a_type) const
			{
				return m_typeData.find(a_type) != m_typeData.end();
			}

			template <typename Type>
			bool isRegistered() const
			{
				return isRegistered(typeid(Type));
			}

			bool isUsed(std::uint64_t const a_id) const
			{
				return m_idToTypeIndex.find(a_id) != m_idToTypeIndex.end();
			}

			template <typename Type>
			void registerType(std::uint64_t const a_id)
			{
				assert(!isRegistered<Type>() && !isUsed(a_id));
				m_typeData.emplace(typeid(Type), TypeData{ a_id, m_types.size() });
				m_idToTypeIndex.emplace(a_id, typeid(Type));
				m_types.emplace_back(typeid(Type)
					, m_typeData[typeid(void)].m_fastCastIndex);
			}

			template <typename Type, typename Base>
			void registerType(std::uint64_t const a_id)
			{
				static_assert(std::is_base_of_v<Base, Type>);
				assert(!isRegistered<Type>() && isRegistered<Base>()
					&& !isUsed(a_id));
				m_typeData.emplace(typeid(Type), TypeData{ a_id, m_types.size() });
				m_idToTypeIndex.emplace(a_id, typeid(Type));
				m_types.emplace_back(typeid(Type)
					, m_typeData[typeid(Base)].m_fastCastIndex);
			}

			template <typename Base>
			bool isBaseOf(std::type_index const a_type) const
			{
				return isBaseOf(typeid(Base), a_type);
			}

			bool isBaseOf(std::type_index const a_baseType
				, std::type_index const a_type) const
			{
				assert(isRegistered(a_baseType) && isRegistered(a_type));
				auto const t_baseTypeIt = m_typeData.find(a_baseType);
				auto const t_typeIt = m_typeData.find(a_type);
				return isBaseOf(t_baseTypeIt->second.m_fastCastIndex
					, t_typeIt->second.m_fastCastIndex);
			}

			template <typename Base>
			bool isBaseOf(std::uint64_t const a_id)
			{
				assert(isUsed(a_id));
				return isBaseOf<Base>(m_idToTypeIndex.find(a_id)->second);
			}

			std::type_index getTypeIndex(std::uint64_t const a_id) const
			{
				return m_idToTypeIndex.find(a_id)->second;
			}

			std::uint64_t getId(std::type_index const a_typeIndex) const
			{
				assert(isRegistered(a_typeIndex));
				return 0;
			}

			template <typename Type>
			std::uint64_t getId() const
			{
				return getId(typeid(Type));
			}

			template <typename BaseType>
			std::uint64_t getId(BaseType* a_ptr) const
			{
				if (!a_ptr)
				{
					return getId<void>();
				}
				return getId(typeid(*a_ptr));
			}

			template <typename Type, typename Base>
			std::shared_ptr<Type> fastCast(std::shared_ptr<Base> const& a_ptr)
			{
				if (a_ptr != nullptr && isBaseOf<Type>(typeid(*a_ptr)))
				{
					return std::static_pointer_cast<Type>(a_ptr);
				}
				return nullptr;
			}

			template <typename Type, typename Base>
			PolymorphicPtr<Type> fastCast(PolymorphicPtr<Base> a_ptr)
			{
				if (a_ptr != nullptr && isBaseOf<Type>(typeid(*a_ptr)))
				{
					return staticPolymorphicCast<Type>(std::move(a_ptr));
				}
				return nullptr;
			}

		private:
			// Attributes
			std::pmr::unordered_map<std::type_index, TypeData> m_typeData;
			std::pmr::unordered_map<std::uint64_t, std::type_index> m_idToTypeIndex;
			std::pmr::vector<std::pair<std::type_index, std::size_t>> m_types;

			// Methods
			bool isBaseOf(std::size_t const a_baseTypeIndex
				, std::size_t const a_typeIndex) const
			{
				if (a_typeIndex == a_baseTypeIndex)
				{
					return true;
				}
				else if (a_typeIndex == 0)
				{
					return false;
				}
				else
				{
					return isBaseOf(a_baseTypeIndex
						, m_types[a_typeIndex].second);
				}
			}
		};
	}
}