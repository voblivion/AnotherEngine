#pragma once
#include <cassert>
#include <functional>
#include <istream>
#include <stack>
#include <string>
#include <type_traits>

#include <vob/json/loader.h>

#include <vob/aoe/core/visitor/Utils.h>
#include <vob/aoe/core/visitor/VisitorBase.h>
#include <locale>

namespace vob::aoe::common
{
	template <typename ContextType>
	class JsonWriter
		: public vis::InputVisitorBase<JsonWriter<ContextType>, ContextType>
	{
		// Aliases
		using Base = vis::InputVisitorBase<JsonWriter<ContextType>, ContextType>;

		using JsonValue = json::value<>;
		using JsonValueConstRef = std::reference_wrapper<JsonValue const>;
		using JsonValueDeque = std::deque<JsonValueConstRef>;
		using JsonValueStack = std::stack<JsonValueConstRef, JsonValueDeque>;

	public:
		// Constructors
		explicit JsonWriter(ContextType const& a_context)
			: Base{ a_context }
		{}

		// Methods
		bool canLoad(std::istream& a_inputStream)
		{
			auto const t_startPos = a_inputStream.tellg();
			json::loader t_jsonLoader{};
			auto t_document = t_jsonLoader.load_from(a_inputStream);
			a_inputStream.seekg(t_startPos, std::ios::beg);
			return t_jsonLoader.get_error() == json::load_error::none;
		}

		template <typename ValueType>
		void load(std::istream& a_inputStream, ValueType& a_value)
		{
			json::loader t_jsonLoader{};
			auto t_document = t_jsonLoader.load_from(a_inputStream);
			load(t_document, a_value);
		}

		template <typename ValueType>
		void load(JsonValue const& a_entryNode, ValueType& a_value)
		{
			assert(m_valueStack.empty());
			m_valueStack.emplace(a_entryNode);
			Base::visit(a_value);
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
			auto const t_number = std::get_if<json::number<>>(&t_currentValue);
			if (t_number != nullptr)
			{
				t_number->get_as(a_value);
			}
		}

		// ReSharper disable once CppMemberFunctionMayBeStatic
		template <typename CharType, typename TraitsType, typename AllocatorType>
		void processString(std::basic_string<CharType, TraitsType, AllocatorType>& a_value)
		{
			auto const& t_currentValue = m_valueStack.top().get();
			if(auto const t_string = std::get_if<json::string<>>(&t_currentValue))
			{
				a_value = std::basic_string<CharType, TraitsType, AllocatorType>{
					*t_string, a_value.get_allocator()
				};
			}
		}

		void processSizeTag(vis::SizeTag& a_sizeTag)
		{
			a_sizeTag.m_size = 0;

			auto const& t_currentValue = m_valueStack.top().get();
			if (auto const t_array = std::get_if<json::array<>>(&t_currentValue))
			{
				a_sizeTag.m_size = t_array->m_values.size();
			}

		}

		template <typename ValueType>
		void processKeyValuePair(vis::IndexValuePair<ValueType> a_indexValuePair)
		{
			// Current node is array
			auto const& t_currentValue = m_valueStack.top().get();
			auto const t_array = std::get_if<json::array<>>(&t_currentValue);
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
			Base::visit(std::forward<ValueType>(a_indexValuePair.m_value));
			m_valueStack.pop();
		}

		template <typename ValueType>
		void processKeyValuePair(vis::NameValuePair<ValueType> a_nameValuePair)
		{
			// Current node is object
			auto const& t_currentValue = m_valueStack.top().get();
			auto const t_object = std::get_if<json::object<>>(&t_currentValue);
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
			Base::visit(a_nameValuePair.m_value);
			m_valueStack.pop();
		}

	private:
		// Attributes
		JsonValueStack m_valueStack;
	};
}
