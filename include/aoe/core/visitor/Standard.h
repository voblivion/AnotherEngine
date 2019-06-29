#pragma once

#include <utility>
#include <unordered_map>
#include <vector>
#include <aoe/core/visitor/Utils.h>
#include <aoe/core/standard/VectorMap.h>
#include <aoe/core/type/Factory.h>

namespace aoe
{
	namespace visitor
	{

		template <typename VisitorType, typename FirstType, typename SecondType>
		void makeVisit(VisitorType& a_visitor
			, std::pair<FirstType, SecondType>& a_value)
		{
			a_visitor.visit("first", a_value.first);
			a_visitor.visit("second", a_value.second);
		}

		template <typename VisitorType, typename KeyType, typename ValueType
			, typename HashType, typename KeyEqType, typename AllocatorType
			, ReaderType<VisitorType>* = nullptr>
			void makeVisit(VisitorType& a_visitor, std::unordered_map<KeyType
				, ValueType, HashType, KeyEqType, AllocatorType>& a_map)
		{
			SizeTag t_size{ a_map.size() };
			a_visitor.visit(t_size);
			std::size_t t_index{ 0 };
			for (auto t_pair : a_map)
			{
				a_visitor.visit(t_index++, t_pair);
			}
		}

		template <typename VisitorType, typename KeyType, typename ValueType
			, typename HashType, typename KeyEqType, typename AllocatorType
			, WriterType<VisitorType>* = nullptr>
			void makeVisit(VisitorType& a_visitor, std::unordered_map<KeyType
				, ValueType, HashType, KeyEqType, AllocatorType>& a_map
				, type::Factory<std::pair<KeyType, ValueType>> a_defaultFactory = {})
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

		template <typename VisitorType, typename KeyType, typename ValueType
			, typename AllocatorType
			, ReaderType<VisitorType>* = nullptr>
			void makeVisit(VisitorType& a_visitor, sta::VectorMap<KeyType
				, ValueType, AllocatorType>& a_map)
		{
			SizeTag t_size{ a_map.size() };
			a_visitor.visit(t_size);
			std::size_t t_index{ 0 };
			for (auto t_pair : a_map)
			{
				a_visitor.visit(t_index++, t_pair);
			}
		}

		template <typename VisitorType, typename KeyType, typename ValueType
			, typename AllocatorType
			, WriterType<VisitorType>* = nullptr>
			void makeVisit(VisitorType& a_visitor, sta::VectorMap<KeyType
				, ValueType, AllocatorType>& a_map
				, type::Factory<std::pair<KeyType, ValueType>> a_defaultFactory = {})
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

		template <typename VisitorType, typename ValueType
			, typename AllocatorType
			, WriterType<VisitorType>* = nullptr>
		void makeVisit(VisitorType& a_visitor
			, std::vector<ValueType, AllocatorType>& a_container
			, type::Factory<ValueType> a_defaultFactory = {})
		{
			SizeTag t_size{};
			a_visitor.visit(t_size);
			a_container.reserve(t_size.m_size + a_container.size());
			for(auto t_index = 0u; t_index < t_size.m_size; ++t_index)
			{
				auto t_value = a_defaultFactory();
				a_visitor.visit(t_index, t_value);
				a_container.emplace_back(std::move(t_value));
			}
		}

		template <typename VisitorType, typename ContainerType
			, typename ConstructorType
			, WriterType<VisitorType>* = nullptr>
		void makeVisit(VisitorType& a_visitor
				, std::pair<ContainerType&
				, ConstructorType> a_pair)
		{
			makeVisit(a_visitor, a_pair.first, a_pair.second);
		}

		template <typename VisitorType, typename ContainerType
			, typename ConstructorType
			, ReaderType<VisitorType>* = nullptr>
			void makeVisit(VisitorType& a_visitor
				, std::pair<ContainerType&
				, ConstructorType> a_pair)
		{
			makeVisit(a_visitor, a_pair.first);
		}
	}
}