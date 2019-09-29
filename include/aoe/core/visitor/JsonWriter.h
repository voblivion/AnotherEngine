#pragma once
#include <cassert>
#include <functional>
#include <istream>
#include <stack>
#include <string>
#include <type_traits>
#include "aoe/core/type/Applicator.h"

#include <aoe/core/json/JsonLoader.h>

#include <aoe/core/standard/TypeFactory.h>
#include <aoe/core/visitor/Utils.h>
#include <aoe/core/visitor/VisitorBase.h>
#include <aoe/core/visitor/Applicator.h>
namespace aoe
{
	namespace json
	{
		class JsonWriter
			: public vis::InputVisitorBase<JsonWriter>
		{
			// Aliases
			using Base = vis::InputVisitorBase<JsonWriter>;

			using Allocator = sta::Allocator<std::byte>;
			using JsonValue = json::JsonValue<>;
			using JsonValueConstRef = std::reference_wrapper<JsonValue const>;
			using JsonValueDeque = std::pmr::deque<JsonValueConstRef>;
			using JsonValueStack = std::stack<JsonValueConstRef, JsonValueDeque>;

		public:
			// Constructors
			explicit JsonWriter(sta::TypeRegistry const& a_typeRegistry
				, sta::TypeFactory const& a_typeFactory
				, Allocator const& a_allocator = {})
				: Base{ a_typeRegistry, a_typeFactory, a_allocator }
				, m_allocator{ a_allocator }
				, m_valueStack{ a_allocator }
			{}

			// Methods
			template <typename ValueType>
			void load(std::istream& a_inputStream, ValueType& a_value)
			{
				json::JsonLoader<> t_jsonLoader{};
				auto t_document = t_jsonLoader.loadFrom(a_inputStream);
				load(t_document, a_value);
			}

			template <typename ValueType>
			void load(JsonValue const& a_entryNode, ValueType& a_value)
			{
				assert(m_valueStack.empty());
				m_valueStack.emplace(a_entryNode);
				visit(a_value);
				m_valueStack.pop();
			}

			// ReSharper disable once CppMemberFunctionMayBeConst
			// ReSharper disable once CppMemberFunctionMayBeStatic
			void preVisit() {}
			// ReSharper disable once CppMemberFunctionMayBeConst
			// ReSharper disable once CppMemberFunctionMayBeStatic
			void postVisit() {}

			// ReSharper disable once CppMemberFunctionMayBeStatic
			template <typename ValueType
				, enforce(std::is_arithmetic_v<std::remove_reference_t<ValueType>>)>
				void processArithmetic(ValueType& a_value)
			{
				auto const& t_currentValue = m_valueStack.top().get();
				auto const t_number = std::get_if<json::JsonNumber<>>(&t_currentValue);
				if (t_number != nullptr)
				{
					t_number->getAs(a_value);
				}
			}

			// ReSharper disable once CppMemberFunctionMayBeStatic
			template <typename CharType, typename TraitsType, typename AllocatorType>
			void processString(std::basic_string<CharType, TraitsType
				, AllocatorType>& a_value)
			{
				auto const& t_currentValue = m_valueStack.top().get();
				if(auto const t_string = std::get_if<json::JsonString<>>(&t_currentValue))
				{
					a_value = std::basic_string<CharType, TraitsType, AllocatorType>{
						*t_string, a_value.get_allocator()
					};
				}
			}

			// ReSharper disable once CppMemberFunctionMayBeConst
			// ReSharper disable once CppMemberFunctionMayBeStatic
			void processSizeTag(vis::SizeTag& a_sizeTag)
			{
				a_sizeTag.m_size = 0;

				auto const& t_currentValue = m_valueStack.top().get();
				if (auto const t_array = std::get_if<json::JsonArray<>>(&t_currentValue))
				{
					a_sizeTag.m_size = t_array->m_values.size();
				}

			}

			// ReSharper disable once CppMemberFunctionMayBeStatic
			template <typename ValueType>
			void processKeyValuePair(
				vis::IndexValuePair<ValueType> a_indexValuePair)
			{
				// Current node is array
				auto const& t_currentValue = m_valueStack.top().get();
				auto const t_array = std::get_if<json::JsonArray<>>(&t_currentValue);
				if (t_array == nullptr)
				{
					return;
				}

				// Index is valid
				auto const t_index = a_indexValuePair.m_index;
				if (t_index >= t_array->m_values.size())
				{
					return;
				}

				m_valueStack.emplace(t_array->m_values[t_index]);
				visit(std::forward<ValueType>(a_indexValuePair.m_value));
				m_valueStack.pop();
			}

			// ReSharper disable once CppMemberFunctionMayBeStatic
			template <typename ValueType>
			void processKeyValuePair(
				vis::NameValuePair<ValueType> a_nameValuePair)
			{
				// Current node is object
				auto const& t_currentValue = m_valueStack.top().get();
				auto const t_object = std::get_if<json::JsonObject<>>(&t_currentValue);
				if (t_object == nullptr)
				{
					return;
				}

				// Key exists
				auto const t_valueIt = t_object->m_values.find(a_nameValuePair.m_name);
				if (t_valueIt == t_object->m_values.end())
				{
					return;
				}

				m_valueStack.emplace(t_valueIt->second);
				visit(a_nameValuePair.m_value);
				m_valueStack.pop();
			}

			auto getAllocator() const
			{
				return m_allocator;
			}

		private:
			// Attributes
			Allocator m_allocator;
			JsonValueStack m_valueStack;
		};
	}
}