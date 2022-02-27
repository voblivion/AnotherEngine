#pragma once
#include <cassert>
#include <functional>
#include <istream>
#include <stack>
#include <string>
#include <type_traits>

#include <vob/misc/std/json.h>

#include <vob/misc/visitor/name_value_pair.h>


namespace vob::aoe::vis
{
	template <typename ContextType>
	class JsonWriter
	{
		// Aliases
		using Self = JsonWriter<ContextType>;
		using JsonValue = mistd::pmr::json_value;
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
			auto t_document = JsonValue{};
			a_inputStream >> t_document;
			a_inputStream.seekg(t_startPos, std::ios::beg);
			auto const isValidJson = !a_inputStream.fail();
			a_inputStream.clear(std::ios_base::failbit);
			return isValidJson;
		}

		template <typename ValueType>
		void load(std::istream& a_inputStream, ValueType& a_value)
		{
			auto t_document = JsonValue{};
			a_inputStream >> t_document;
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
		requires FreeAcceptVisitable<Self, ValueType>
			&& (!std::is_arithmetic_v<ValueType>)
		void visit(ValueType& a_object)
        {
            accept(*this, a_object);
        }

		template <typename ValueType>
		requires MemberAcceptVisitable<Self, ValueType>
		void visit(ValueType& a_object)
		{
			a_object.accept(*this);
		}

        template <typename ValueType>
        requires StaticAcceptVisitable<Self, ValueType>
        void visit(ValueType& a_object)
        {
            ValueType::accept(*this, a_object);
        }

		template <typename ValueType>
		requires std::is_arithmetic_v<ValueType>
		void visit(ValueType& a_number)
		{
			auto const& currentValue = m_valueStack.top().get();
			if (auto const number = currentValue.get<mistd::pmr::json_number>())
			{
				std::visit(
					[&a_number](auto const a_value){ a_number = static_cast<ValueType>(a_value); }, number->value);
			}
		}

		void visit(bool& a_boolean)
		{
			auto const& currentValue = m_valueStack.top().get();
			if (auto const boolean = currentValue.get<mistd::pmr::json_boolean>())
			{
				a_boolean = boolean->value;
			}
		}

		template <typename CharType, typename TraitsType, typename AllocatorType>
		void visit(std::basic_string<CharType, TraitsType, AllocatorType>& a_string)
		{
			auto const& currentValue = m_valueStack.top().get();
			if(auto const string = currentValue.get<mistd::pmr::json_string>())
			{
				a_string.assign(string->value);
			}
		}

        template <typename ValueType>
        requires std::is_same_v<vis::SizeTag, ValueType>
		void visit(ValueType& a_sizeTag)
		{
			a_sizeTag.m_size = 0;

			auto const& currentValue = m_valueStack.top().get();
			if (auto const array = currentValue.get<mistd::pmr::json_array>())
			{
				a_sizeTag.m_size = array->data.size();
			}

		}
		
		template <typename ValueType>
		void visit(vis::IndexValuePair<ValueType> a_indexValuePair)
		{
			// Current node is array
			auto const& currentValue = m_valueStack.top().get();
			auto const array = currentValue.get<mistd::pmr::json_array>();
			if (array == nullptr)
			{
				return;
			}

			// Index is valid
			auto const index = a_indexValuePair.m_index;
			if (index >= array->data.size())
			{
				return;
			}

			m_valueStack.emplace(array->data[index]);
			visit(std::forward<ValueType>(a_indexValuePair.m_value));
			m_valueStack.pop();
		}

		template <typename ValueType>
		void visit(vis::NameValuePair<ValueType> a_nameValuePair)
		{
			// Current node is object
			auto const& currentValue = m_valueStack.top().get();
			auto const object = currentValue.get<mistd::pmr::json_object>();
			if (object == nullptr)
			{
				return;
			}

			// Key exists
			auto const valueIt = object->data.find(a_nameValuePair.m_name);
			if (valueIt == object->data.end())
			{
				return;
			}

			m_valueStack.emplace(valueIt->second);
			visit(a_nameValuePair.m_value);
			m_valueStack.pop();
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
