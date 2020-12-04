#pragma once

#include <filesystem>
#include <utility>
#include <unordered_map>
#include <vector>
#include <variant>

#include <vob/sta/vector_map.h>
#include <vob/sta/vector_set.h>
#include <vob/sta/ignorable_assert.h>

#include <vob/aoe/core/visitor/Utils.h>
#include <vob/aoe/core/type/Factory.h>
#include <vob/aoe/core/type/Variant.h>
#include <vob/aoe/core/type/TypeRegistry.h>
#include <vob/aoe/core/type/Traits.h>
#include <vob/aoe/core/type/TypeFactory.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>

namespace vob::aoe::vis
{
#define BLOCK_VISIT_PAIR
#ifdef BLOCK_VISIT_PAIR
	template <typename VisitorType, typename FirstType, typename SecondType>
	void accept(VisitorType& a_visitor
		, std::pair<FirstType, SecondType>& a_value)
	{
		a_visitor.visit(makeNameValuePair("first", a_value.first));
		a_visitor.visit(makeNameValuePair("second", a_value.second));
	}

	template <typename VisitorType, typename FirstType, typename SecondType>
	void accept(VisitorType& a_visitor
		, std::pair<FirstType, SecondType> const& a_value)
	{
		a_visitor.visit(makeNameValuePair("first", a_value.first));
		a_visitor.visit(makeNameValuePair("second", a_value.second));
	}
#endif

#define BLOCK_VISIT_UNORDERED_MAP
#ifdef BLOCK_VISIT_UNORDERED_MAP
	template <typename VisitorType, typename KeyType, typename ValueType
		, typename HashType, typename KeyEqType, typename AllocatorType
		
		, typename Factory = type::Factory<std::pair<KeyType, ValueType>>>
	void accept(
		VisitorType& a_visitor
		, std::unordered_map<KeyType, ValueType, HashType, KeyEqType, AllocatorType>& a_map
		, Factory a_defaultFactory = {}
	)
	{
		SizeTag t_size{};
		a_visitor.visit(t_size);
		a_map.reserve(t_size.m_size + a_map.size());
		for (auto t_index = 0u; t_index < t_size.m_size; ++t_index)
		{
			auto t_pair = a_defaultFactory();
			auto t_indexedPair = makeIndexValuePair(t_index, t_pair);
			a_visitor.visit(t_indexedPair);
			a_map.emplace(std::move(t_pair.first), std::move(t_pair.second));
		}
	}

	template <typename VisitorType, typename KeyType, typename ValueType
		, typename HashType, typename KeyEqType, typename AllocatorType
		
	>
	void accept(
		VisitorType& a_visitor
		, std::unordered_map<KeyType, ValueType, HashType, KeyEqType, AllocatorType> const& a_map
	)
	{
		SizeTag t_size{ a_map.size() };
		a_visitor.visit(t_size);
		std::size_t t_index{ 0 };
		for (auto t_pair : a_map)
		{
			// TODO ?
			a_visitor.visit(t_index++, t_pair);
		}
	}
#endif

#define BLOCK_VISIT_VECTOR_MAP
#ifdef BLOCK_VISIT_VECTOR_MAP
	template <
		typename VisitorType, typename KeyType, typename ValueType, typename AllocatorType
		
	>
	void accept(
		VisitorType& a_visitor
		, sta::vector_map<KeyType, ValueType, AllocatorType> const& a_map
	)
	{
		SizeTag t_size{ a_map.size() };
		a_visitor.visit(t_size);
		std::size_t t_index{ 0 };
		for (auto t_pair : a_map)
		{
			a_visitor.visit(t_index++, t_pair);
		}
	}

	template <
		typename VisitorType, typename KeyType, typename ValueType, typename AllocatorType
		
		, typename Factory = type::Factory<std::pair<KeyType, ValueType>>
	>
	void accept(
		VisitorType& a_visitor
		, sta::vector_map<KeyType, ValueType, AllocatorType>& a_map
		, Factory a_defaultFactory = {}
	)
	{
		SizeTag t_size{};
		a_visitor.visit(t_size);
		a_map.reserve(t_size.m_size + a_map.size());
		for (auto t_index = 0u; t_index < t_size.m_size; ++t_index)
		{
			auto t_pair = a_defaultFactory();
			a_visitor.visit(t_index, t_pair);
			a_map.emplace(std::move(t_pair.first), std::move(t_pair.second));
		}
	}
#endif

#define BLOCK_VISIT_VECTOR_SET
#ifdef BLOCK_VISIT_VECTOR_SET
	template <
		typename VisitorType, typename KeyType, typename EqualType, typename AllocatorType
		
	>
	void accept(
		VisitorType& a_visitor
		, sta::vector_set<KeyType, EqualType, AllocatorType> const& a_set
	)
	{
		SizeTag t_size{ a_set.size() };
		a_visitor.visit(t_size);
		std::size_t t_index{ 0 };
		for (auto t_item : a_set)
		{
			a_visitor.visit(makeIndexValuePair(t_index++, t_item));
		}
	}

	template <
		typename VisitorType, typename KeyType, typename EqualType, typename AllocatorType
		
		, typename Factory = type::Factory<KeyType>
	>
	void accept(
		VisitorType& a_visitor
		, sta::vector_set<KeyType, EqualType, AllocatorType>& a_map
		, Factory a_defaultFactory = {})
	{
		SizeTag t_size{};
		a_visitor.visit(t_size);
		a_map.reserve(t_size.m_size + a_map.size());
		for (auto t_index = 0u; t_index < t_size.m_size; ++t_index)
		{
			auto t_item = a_defaultFactory();
			a_visitor.visit(makeIndexValuePair(t_index, t_item));
			a_map.emplace(std::move(t_item));
		}
	}
#endif

#define BLOCK_VISIT_VECTOR
#ifdef BLOCK_VISIT_VECTOR
	template <
		typename VisitorType, typename ValueType, typename AllocatorType
		, typename Factory = type::Factory<ValueType>
	>
	void accept(VisitorType& a_visitor
			, std::vector<ValueType, AllocatorType>& a_container
			, Factory a_defaultFactory = {})
	{
		SizeTag t_size{};
		a_visitor.visit(t_size);
		a_container.reserve(t_size.m_size + a_container.size());
		for (auto t_index = 0u; t_index < t_size.m_size; ++t_index)
		{
			auto t_value = a_defaultFactory();
			a_visitor.visit(makeIndexValuePair(t_index, t_value));
			a_container.emplace_back(std::move(t_value));
		}
	}
#endif

#define BLOCK_VISIT_VARIANT
#ifdef BLOCK_VISIT_VARIANT
	template <typename VisitorType, typename... Types>
	void accept(
		VisitorType& a_visitor
		, std::variant<Types...> const& a_variant
	)
	{
		std::size_t t_variantIndex{ a_variant.index() };
		a_visitor.visit(nvp("variant_index", t_variantIndex));
			
		std::visit([&a_visitor](auto&& a_value)
		{
			a_visitor.visit(nvp("data", a_value));
		}, a_variant);
	}

	template <typename VisitorType, typename FactoryType, typename... Types>
	void accept(VisitorType& a_visitor, std::variant<Types...>& a_variant
		, FactoryType const& a_variantFactory)
	{
		std::size_t t_variantIndex{ a_variant.index() };
		a_visitor.visit(nvp("variant_index", t_variantIndex));

		a_variant = a_variantFactory(t_variantIndex);
		std::visit([&a_visitor](auto&& a_value)
		{
			a_visitor.visit(nvp("data", a_value));
		}, a_variant);
	}

	template <typename VisitorType, typename... Types>
	void accept(VisitorType& a_visitor, std::variant<Types...>& a_variant)
	{
		accept(a_visitor, a_variant, type::VariantFactory<Types...>{});
	}
#endif

#define BLOCK_VISIT_CONTAINER_ITEM
#ifdef BLOCK_VISIT_CONTAINER_ITEM
	template <typename VisitorType, typename ContainerType
		, typename ConstructorType, enforce(!std::is_const_v<ContainerType>)>
	void accept(VisitorType& a_visitor
		, ContainerHolder<ContainerType, ConstructorType> a_pair)
	{
		accept(a_visitor, a_pair.m_container, a_pair.m_factory);
	}

	template <typename VisitorType, typename ContainerType
		, typename ConstructorType, enforce(std::is_const_v<ContainerType>)>
	void accept(VisitorType& a_visitor
		, ContainerHolder<ContainerType&, ConstructorType> a_pair)
	{
		accept(a_visitor, a_pair.m_container);
	}
#endif

#define BLOCK_VISIT_POINTER
#ifdef BLOCK_VISIT_POINTER
	template <typename VisitorType>
	sta::string_id readTypeId(
		VisitorType& a_visitor
		, sta::string_id const a_defaultId
	)
	{
		auto t_id = a_defaultId;
		a_visitor.visit(makeNameValuePair("type_id", t_id));
		return t_id;
	}

	template <typename VisitorType>
	void writeTypeId(VisitorType& a_visitor, sta::string_id const a_id)
	{
		auto const& t_typeRegistry = a_visitor.getContext().m_typeRegistry;

		a_visitor.visit(makeNameValuePair("type_id", a_id));
	}

	template <typename VisitorType, typename PointerType>
	void visitData(VisitorType& a_visitor, PointerType& a_ptr)
	{
		if (a_ptr != nullptr)
		{
			auto t_dynamicValue = makeDynamicValue(*a_ptr);
			a_visitor.visit(makeNameValuePair("data", t_dynamicValue));
		}
	}


	template <typename VisitorType, typename BaseType>
	void accept(
		VisitorType& a_visitor
		, std::unique_ptr<BaseType> const& a_ptr
	)
	{
		// TODO ?
		auto const& t_typeRegistry = a_visitor.getContext().m_typeRegistry;
		
		writeTypeId(
			a_visitor
			, t_typeRegistry.getId(std::type_index{ a_ptr != nullptr ? typeid(*a_ptr) : typeid(void) })
		);

		visitData(a_visitor, a_ptr);
	}

	template <typename PolymorphicBaseType, typename VisitorType, typename BaseType>
	void visitUnique(
		VisitorType& a_visitor
		, std::unique_ptr<BaseType>& a_ptr
		, type::TypeFactory<PolymorphicBaseType> const& a_typeFactory
	)
	{
		auto const& t_typeRegistry = a_visitor.getContext().m_typeRegistry;
		auto t_voidTypeId = t_typeRegistry.template getId<void>();
		auto t_id = readTypeId(a_visitor, t_voidTypeId);

		a_ptr = a_typeFactory.template create<BaseType>(t_id);

		ignorable_assert(a_ptr != nullptr || t_id == t_voidTypeId);
		visitData(a_visitor, a_ptr);
	}

	template <typename PolymorphicBaseType, typename VisitorType, typename BaseType>
	void visitShared(
		VisitorType& a_visitor
		, std::shared_ptr<BaseType>& a_ptr
		, type::TypeFactory<PolymorphicBaseType> const& a_typeFactory
	)
	{
		auto const& t_typeRegistry = a_visitor.getContext().m_typeRegistry;
		auto t_voidTypeId = t_typeRegistry.template getId<void>();
		auto t_id = readTypeId(a_visitor, t_voidTypeId);

		a_ptr = a_typeFactory.template createShared<BaseType>(t_id);

		ignorable_assert(a_ptr != nullptr || t_id == t_voidTypeId);
		visitData(a_visitor, a_ptr);
	}



#define VOB_AOE_VIS_PTR(PolymorphicBaseType, polymorphicBaseTypeName) \
	template <typename VisitorType, typename BaseType> \
	std::enable_if_t<std::is_base_of_v<PolymorphicBaseType, BaseType>> accept( \
		VisitorType& a_visitor \
		, std::unique_ptr<BaseType>& a_ptr \
	) \
	{ \
		auto const& t_typeFactory = a_visitor.getContext().m_##polymorphicBaseTypeName##Factory; \
		visitUnique<PolymorphicBaseType>(a_visitor, a_ptr, t_typeFactory); \
	} \
	\
	template <typename VisitorType, typename BaseType> \
	std::enable_if_t<std::is_base_of_v<PolymorphicBaseType, BaseType>> accept( \
		VisitorType& a_visitor \
		, std::shared_ptr<BaseType>& a_ptr \
	) \
	{ \
		auto const& t_typeFactory = a_visitor.getContext().m_##polymorphicBaseTypeName##Factory; \
		visitShared<PolymorphicBaseType>(a_visitor, a_ptr, t_typeFactory); \
	} \
	\
	template <typename VisitorType, typename BaseType> \
	std::enable_if_t<std::is_base_of_v<PolymorphicBaseType, BaseType>> accept( \
		VisitorType& a_visitor \
		, DynamicValue<BaseType> const& a_value \
	) \
	{ \
		auto& t_typeVisitorApplicator = a_visitor.getContext().m_##polymorphicBaseTypeName##Applicator; \
		t_typeVisitorApplicator.apply(a_value.m_value, a_visitor); \
	}

	VOB_AOE_VIS_PTR(type::ADynamicType, dynamicType)
	VOB_AOE_VIS_PTR(btCollisionShape, btCollisionShape)

	/*template <typename VisitorType, typename BaseType>
	void accept(
		VisitorType& a_visitor
		, std::shared_ptr<BaseType> const& a_ptr
	)
	{
		// Todo
		auto const& t_dynamicTypeFactory = a_visitor.getContext().m_dynamicTypeFactory;
		auto const& t_typeRegistry = a_visitor.getContext().m_typeRegistry;

		writeTypeId(
			a_visitor
			, t_typeRegistry.getId(std::type_index{ a_ptr != nullptr ? typeid(*a_ptr) : typeid(void) })
		);

		visitData(a_visitor, a_ptr);
	}*/
#endif
#pragma region Filesystem
	template <typename VisitorType>
	void accept(
		VisitorType& a_visitor
		, std::filesystem::path& a_path
	)
	{
		std::string rawPath;
		a_visitor.visit(rawPath);
		a_path = std::filesystem::path{ rawPath };
	}
#pragma endregion
}
