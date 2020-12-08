#pragma once
#include <cassert>
#include <functional>
#include <istream>
#include <stack>
#include <string>
#include <type_traits>

#include <vob/json/loader.h>

#include <vob/aoe/core/type/Traits.h>
#include <vob/aoe/core/visitor/Traits.h>
#include <vob/aoe/core/visitor/Utils.h>

namespace vob::aoe::vis
{
	template <typename ContextType>
	class JsonWriter
	{
		// Aliases
		using JsonValue = json::value<>;
		using JsonValueConstRef = std::reference_wrapper<JsonValue const>;
		using JsonValueDeque = std::deque<JsonValueConstRef>;
		using JsonValueStack = std::stack<JsonValueConstRef, JsonValueDeque>;

	public:
		// Constructors
		explicit JsonWriter(ContextType const& a_context)
			: m_context{ a_context }
		{}

		// Methods
		ContextType const& getContext()
		{
			return m_context;
		}

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
			visit(a_value);
			m_valueStack.pop();
		}

		template <typename ValueType>
		std::enable_if_t<std::is_arithmetic_v<ValueType>> visit(ValueType& a_number)
		{
			auto const& currentValue = m_valueStack.top().get();
			if (auto const number = std::get_if<json::number<>>(&currentValue))
			{
				number->get_as(a_number);
			}
		}

		void visit(bool& a_boolean)
		{
			auto const& currentValue = m_valueStack.top().get();
			if (auto const boolean = std::get_if<json::boolean>(&currentValue))
			{
				a_boolean = *boolean;
			}
		}

		template <typename CharType, typename TraitsType, typename AllocatorType>
		void visit(std::basic_string<CharType, TraitsType, AllocatorType>& a_string)
		{
			auto const& currentValue = m_valueStack.top().get();
			if(auto const string = std::get_if<json::string<>>(&currentValue))
			{
				a_string = std::basic_string<CharType, TraitsType, AllocatorType>{
					*string, a_string.get_allocator()
				};
			}
		}

		void visit(vis::SizeTag& a_sizeTag)
		{
			a_sizeTag.m_size = 0;

			auto const& currentValue = m_valueStack.top().get();
			if (auto const array = std::get_if<json::array<>>(&currentValue))
			{
				a_sizeTag.m_size = array->m_values.size();
			}

		}

		template <typename ValueType>
		void visit(vis::IndexValuePair<ValueType> a_indexValuePair)
		{
			// Current node is array
			auto const& currentValue = m_valueStack.top().get();
			auto const array = std::get_if<json::array<>>(&currentValue);
			if (array == nullptr)
			{
				return;
			}

			// Index is valid
			auto const index = a_indexValuePair.m_index;
			if (index >= array->m_values.size())
			{
				return;
			}

			m_valueStack.emplace(array->m_values[index]);
			visit(std::forward<ValueType>(a_indexValuePair.m_value));
			m_valueStack.pop();
		}

		template <typename ValueType>
		void visit(vis::NameValuePair<ValueType> a_nameValuePair)
		{
			// Current node is object
			auto const& currentValue = m_valueStack.top().get();
			auto const object = std::get_if<json::object<>>(&currentValue);
			if (object == nullptr)
			{
				return;
			}

			// Key exists
			auto const valueIt = object->m_values.find(a_nameValuePair.m_name);
			if (valueIt == object->m_values.end())
			{
				return;
			}

			m_valueStack.emplace(valueIt->second);
			visit(a_nameValuePair.m_value);
			m_valueStack.pop();
		}

		template <typename ValueType>
		std::enable_if_t<
			!std::is_arithmetic_v<ValueType>
			&& vis::hasAcceptValue<JsonWriter<ContextType>, ValueType>
		> visit(ValueType& a_object)
		{
			accept(*this, a_object);
		}

		template <typename ContainerType, typename FactoryType>
		void visit(ContainerHolder<ContainerType, FactoryType> const& a_containerHolder)
		{
			accept(*this, a_containerHolder);
		}

	private:
		// Attributes
		JsonValueStack m_valueStack;
		ContextType const& m_context;
	};
}
