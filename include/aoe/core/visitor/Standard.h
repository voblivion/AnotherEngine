#pragma once

#include <utility>
#include <unordered_map>
#include <aoe/core/visitor/Utils.h>

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
			, std::enable_if_t<VisitorType::accessType == AccessType::Reader>* = nullptr>
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
			, std::enable_if_t<VisitorType::accessType == AccessType::Writer>* = nullptr>
			void makeVisit(VisitorType& a_visitor, std::unordered_map<KeyType
				, ValueType, HashType, KeyEqType, AllocatorType>& a_map)
		{
			SizeTag t_size{};
			a_visitor.visit(t_size);
			for (std::size_t t_index = 0; t_index < t_size.m_size; ++t_index)
			{
				std::pair<KeyType, ValueType> t_pair;
				a_visitor.visit(t_index, t_pair);
				a_map.emplace(t_pair);
			}
		}
	}
}