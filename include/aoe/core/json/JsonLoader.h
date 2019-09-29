#pragma once
#include <iomanip>

#include <aoe/core/json/JsonValue.h>

namespace aoe::json
{
	enum class JsonError
	{
		None = 0, // No error
		ExtraCharacterFound, // A json value can be read but there are extra chars
		InvalidCharacterFound, // Character read was an expected Json Value
		ExpectObjectKey, // Json Object elements must be indexed by "key"
		ExpectColonSeparator, // Json Object key must be followed by a colon
		ExpectCommaSeparator, // Between Object/Array elements, a comma is expected
		BadBooleanFormat, // Read t*** or f**** but not true/false
		BadNullFormat, // Read n*** but not null
		MissingLeadingDigit, // A number must start by a (signed) digit
		MissingDecimal, // In number after "." must be a digit
		MissingExponent, // In number after "e"/"E" must be a digit
	};

	template <template <typename> typename AllocatorType = std::pmr::polymorphic_allocator>
	class JsonLoader
	{
	public:
		using CharString = CharString<AllocatorType>;
		using JsonNull = JsonNull;
		using JsonBoolean = JsonBoolean;
		using JsonString = JsonString<AllocatorType>;
		using JsonNumber = JsonNumber<AllocatorType>;
		using JsonArray = JsonArray<AllocatorType>;
		using JsonObject = JsonObject<AllocatorType>;
		using JsonValue = JsonValue<AllocatorType>;

		JsonValue loadFrom(std::istream& a_is)
		{
			m_error = JsonError::None;

			JsonValue t_value;
			load(a_is, t_value);

			a_is >> std::ws;
			if (m_error == JsonError::None && !a_is.eof())
			{
				setError(a_is, JsonError::ExtraCharacterFound);
			}

			return t_value;
		}

		JsonError getError() const { return m_error; }
		std::size_t getErrorPosition() const { return m_errorPosition; }

	private:
		JsonError m_error{ JsonError::None };
		std::size_t m_errorPosition{ 0 };

		void load(std::istream& a_is, JsonValue& a_value)
		{
			a_is >> std::ws;
			switch (a_is.peek())
			{
			case '"': // String
				a_value = JsonString{};
				load(a_is, std::get<JsonString>(a_value));
				break;
			case '{': // Object
				a_value = JsonObject{};
				load(a_is, std::get<JsonObject>(a_value));
				break;
			case '[': // Array
				a_value = JsonArray{};
				load(a_is, std::get<JsonArray>(a_value));
				break;
			case 't': // Boolean
			case 'f':
				a_value = JsonBoolean{};
				load(a_is, std::get<JsonBoolean>(a_value));
				break;
			case 'n': // Null
				a_value = JsonNull{};
				load(a_is, std::get<JsonNull>(a_value));
				break;
			default: // Number
				a_value = JsonNumber{};
				load(a_is, std::get<JsonNumber>(a_value));
				break;
			}
		}

		void load(std::istream& a_is, JsonString& a_string)
		{
			if (a_is.peek() != '"') { setError(a_is, JsonError::ExpectObjectKey); return; }
			a_is >> std::quoted(a_string);
			// Todo detect missing '\"'
		}

		void load(std::istream& a_is, JsonObject& a_object)
		{
			auto c = a_is.get();
			a_is >> std::ws;
			c = a_is.peek();
			while (c != '}')
			{
				CharString t_key;
				load(a_is, t_key);
				if (m_error != JsonError::None) { return; }
				a_is >> std::ws;
				c = a_is.get();
				if (c != ':') { setError(a_is, JsonError::ExpectColonSeparator); return; }
				JsonValue t_value;
				load(a_is, t_value);
				if (m_error != JsonError::None) { return; }
				a_object.m_values.emplace(t_key, t_value);
				a_is >> std::ws;
				c = a_is.peek();
				if (c == ',')
				{
					c = a_is.get();
					a_is >> std::ws;
					c = a_is.peek();
				}
				else if (c != '}') { setError(a_is, JsonError::ExpectCommaSeparator); return; }
			}
			c = a_is.get();
		}

		void load(std::istream& a_is, JsonArray& a_array)
		{
			auto c = a_is.get();
			a_is >> std::ws;
			c = a_is.peek();
			while (c != ']')
			{
				JsonValue t_value;
				load(a_is, t_value);
				if (m_error != JsonError::None) { return; }
				a_array.m_values.emplace_back(t_value);
				a_is >> std::ws;
				c = a_is.peek();
				if (c == ',')
				{
					c = a_is.get();
					a_is >> std::ws;
					c = a_is.peek();
				}
				else if (c != ']') { setError(a_is, JsonError::ExpectCommaSeparator); return; }
			}
			c = a_is.get();
		}

		void load(std::istream& a_is, JsonBoolean& a_boolean)
		{
			if (a_is.get() == 't')
			{
				if (a_is.get() != 'r') { setError(a_is, JsonError::BadBooleanFormat); return; }
				if (a_is.get() != 'u') { setError(a_is, JsonError::BadBooleanFormat); return; }
				if (a_is.get() != 'e') { setError(a_is, JsonError::BadBooleanFormat); return; }
			}
			else
			{
				if (a_is.get() != 'a') { setError(a_is, JsonError::BadBooleanFormat); return; }
				if (a_is.get() != 'l') { setError(a_is, JsonError::BadBooleanFormat); return; }
				if (a_is.get() != 's') { setError(a_is, JsonError::BadBooleanFormat); return; }
				if (a_is.get() != 'e') { setError(a_is, JsonError::BadBooleanFormat); return; }
			}
		}

		void load(std::istream& a_is, JsonNull& a_null)
		{
			a_is.get();
			if (a_is.get() != 'u') { setError(a_is, JsonError::BadBooleanFormat); return; }
			if (a_is.get() != 'l') { setError(a_is, JsonError::BadBooleanFormat); return; }
			if (a_is.get() != 'l') { setError(a_is, JsonError::BadBooleanFormat); return; }
		}

		void load(std::istream& a_is, JsonNumber& a_number)
		{
			a_is >> std::ws;
			auto tryReadRange = [&a_is, &a_number](char a_min = '0', char a_max = '9') {
				auto c = a_is.peek();
				if (c >= a_min && c <= a_max)
				{
					a_number.m_representation += static_cast<char>(a_is.get());
					return true;
				}
				return false;
			};

			auto const t_isNegative = tryReadRange('-', '-');
			if (tryReadRange('0', '0')) {}
			else
			{
				if (!tryReadRange('1'))
				{
					if(t_isNegative) setError(a_is, JsonError::MissingLeadingDigit);
					else setError(a_is, JsonError::InvalidCharacterFound);
					return;
				}
				while (tryReadRange()) {}
			}

			if (tryReadRange('.', '.'))
			{
				if (!tryReadRange()) { setError(a_is, JsonError::MissingDecimal); return; }
				while (tryReadRange()) {}
			}

			if (tryReadRange('e', 'e') || tryReadRange('E', 'E'))
			{
				if (!tryReadRange('-'))
				{
					tryReadRange('+');
				}
				if (!tryReadRange()) { setError(a_is, JsonError::MissingExponent); return; }
				while (tryReadRange()) {}
			}

		}

		void setError(std::istream& a_is, JsonError a_error)
		{
			m_error = a_error;
			m_errorPosition = a_is.tellg();
		}
	};
}
