#pragma once
#include <cassert>
#include <functional>
#include <istream>
#include <stack>
#include <string>
#include <type_traits>

#define RAPIDJSON_HAS_STDSTRING 1
#define RAPIDJSON_NO_SIZETYPEDEFINE 1
namespace rapidjson
{
	typedef std::size_t SizeType;
}
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include <aoe/core/standard/TypeFactory.h>
#include <aoe/core/visitor/Utils.h>
#include <aoe/core/visitor/TypeApplicator.h>

namespace aoe
{
	namespace visitor
	{
		namespace rjs = rapidjson;
		class JsonWriter
		{
			// Aliases
			using JsonValue = rjs::Document::ValueType;
			using JsonValueConstRef = std::reference_wrapper<JsonValue const>;
			using TypeVisitorApplicator = TypeApplicator<JsonWriter>;
			using JsonValueDeque = std::pmr::deque<JsonValueConstRef>;
			using JsonValueStack = std::stack<JsonValueConstRef, JsonValueDeque>;
			using Allocator = sta::Allocator<std::byte>;

		public:
			// Using
			static const AccessType accessType = AccessType::Writer;
			static const VisitType visitType = VisitType::Random;

			// Constructors
			explicit JsonWriter(sta::TypeFactory const& a_typeFactory)
				: m_typeFactory{ a_typeFactory }
			{}

			explicit JsonWriter(sta::TypeFactory const& a_typeFactory
				, Allocator const& a_allocator)
				: m_typeFactory{ a_typeFactory }
				, m_valueStack{ a_allocator }
			{}

			// Methods
			template <typename ValueType>
			void load(std::istream& a_inputStream, ValueType& a_value)
			{
				assert(m_valueStack.empty());
				rjs::Document t_document;
				rjs::IStreamWrapper t_inputStream{ a_inputStream };
				t_document.ParseStream(t_inputStream);
				m_valueStack.emplace(t_document);
				processVisit(a_value);
				m_valueStack.pop();
			}

			template <typename ValueType>
			bool visit(ValueType&& a_value)
			{
				processVisit(std::forward<ValueType>(a_value));

				return true;
			}

			template <typename ValueType>
			bool visit(std::string_view const a_name, ValueType&& a_value)
			{
				// Current node is object
				auto const& t_currentValue = m_valueStack.top().get();
				if (!t_currentValue.IsObject())
				{
					return false;
				}

				// Key exists
				auto const t_valueIt = t_currentValue.FindMember(a_name.data());
				if (t_valueIt == t_currentValue.MemberEnd())
				{
					return false;
				}

				m_valueStack.emplace(t_valueIt->value);
				processVisit(std::forward<ValueType>(a_value));
				m_valueStack.pop();

				return true;
			}

			bool visit(SizeTag& a_sizeTag)
			{
				auto const& t_currentValue = m_valueStack.top().get();
				if (!t_currentValue.IsArray())
				{
					return false;
				}

				a_sizeTag.m_size = t_currentValue.Size();
				return true;
			}

			template <typename ValueType>
			bool visit(std::size_t const a_index, ValueType&& a_value)
			{
				// Current node is array
				auto const& t_currentValue = m_valueStack.top().get();
				if (!t_currentValue.IsArray())
				{
					return false;
				}

				// Index is valid
				if (a_index >= t_currentValue.Size())
				{
					return false;
				}

				m_valueStack.emplace(t_currentValue[rjs::SizeType{ a_index }]);
				processVisit(std::forward<ValueType>(a_value));
				m_valueStack.pop();

				return true;
			}

			auto getAllocator()
			{
				return m_typeVisitorApplicator.getAllocator();
			}

			sta::TypeFactory const& getTypeFactory() const
			{
				return m_typeFactory;
			}

			TypeVisitorApplicator& getApplicator()
			{
				return m_typeVisitorApplicator;
			}

		private:
			// Attributes
			sta::TypeFactory const& m_typeFactory;
			TypeVisitorApplicator m_typeVisitorApplicator;
			JsonValueStack m_valueStack;

			// Methods
			template <typename ValueType
				, std::enable_if_t<!std::is_arithmetic_v<ValueType>>* = nullptr>
				void processVisit(ValueType&& a_value)
			{
				makeVisit(*this, std::forward<ValueType>(a_value));
			}

			template <typename ValueType
				, std::enable_if_t<std::is_arithmetic_v<ValueType>>* = nullptr>
				void processVisit(ValueType& a_value)
			{
				auto const& t_currentValue = m_valueStack.top().get();
				if (t_currentValue.Is<ValueType>())
				{
					a_value = t_currentValue.Get<ValueType>();
				}
			}

			template <typename CharType, typename TraitsType
				, typename AllocatorType>
			void processVisit(std::basic_string<CharType, TraitsType
				, AllocatorType>& a_value)
			{
				auto const& t_currentValue = m_valueStack.top().get();
				if (t_currentValue.Is<std::basic_string<CharType>>())
				{
					a_value = t_currentValue.Get<std::basic_string<CharType>>();
				}
			}
		};
	}
}