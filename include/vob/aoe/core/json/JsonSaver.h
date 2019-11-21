#pragma once
#include <iomanip>

#include <vob/aoe/core/json/JsonValue.h>

namespace vob::aoe::json
{
	template <template <typename> typename AllocatorType = std::pmr::polymorphic_allocator>
	class JsonSaver
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

		void setIndentation(CharString a_charString)
		{
			m_indentation = std::move(a_charString);
		}

		void saveTo(std::ostream& a_os, JsonValue const& a_value) const
		{
			m_indentationLevel = 0;
			save(a_os, a_value);
		}

	private:
		mutable std::size_t m_indentationLevel{ 0 };
		CharString m_indentation{ '\t' };

		void save(std::ostream& a_os, JsonValue const& a_value) const
		{
			if (auto t_null = std::get_if<JsonNull>(&a_value))
			{
				save(a_os, *t_null);
			}
			else if (auto t_boolean = std::get_if<JsonBoolean>(&a_value))
			{
				save(a_os, *t_boolean);
			}
			else if (auto t_string = std::get_if<JsonString>(&a_value))
			{
				save(a_os, *t_string);
			}
			else if (auto t_number = std::get_if<JsonNumber>(&a_value))
			{
				save(a_os, *t_number);
			}
			else if (auto t_array = std::get_if<JsonArray>(&a_value))
			{
				save(a_os, *t_array);
			}
			else if (auto t_object = std::get_if<JsonObject>(&a_value))
			{
				save(a_os, *t_object);
			}
		}

		static void save(std::ostream& a_os, JsonNull const& a_null)
		{
			a_os << "null";
		}

		static void save(std::ostream& a_os, JsonBoolean const& a_boolean)
		{
			a_os << (a_boolean ? "true" : "false");
		}

		static void save(std::ostream& a_os, JsonString const& a_string)
		{
			a_os << std::quoted(a_string);
		}

		static void save(std::ostream& a_os, JsonNumber const& a_number)
		{
			a_os << a_number.m_representation;
		}

		void save(std::ostream& a_os, JsonArray const& a_array) const
		{
			if (a_array.m_values.empty())
			{
				a_os << "[]";
				return;
			}
			a_os <<	'[';
			++m_indentationLevel;
			auto t_firstItem = true;
			for(auto const& t_item : a_array.m_values)
			{
				if(t_firstItem)
				{
					a_os << '\n';
					t_firstItem = false;
				}
				else
				{
					a_os << ",\n";
				}
				saveIndentation(a_os);
				save(a_os, t_item);
			}
			a_os << '\n';
			--m_indentationLevel;
			saveIndentation(a_os);
			a_os << ']';
		}

		void save(std::ostream& a_os, JsonObject const& a_object) const
		{
			if (a_object.m_values.empty())
			{
				a_os << "{}";
				return;
			}
			a_os << '{';
			++m_indentationLevel;
			auto t_firstItem = true;
			for (auto const& t_item : a_object.m_values)
			{
				if (t_firstItem)
				{
					a_os << '\n';
					t_firstItem = false;
				}
				else
				{
					a_os << ",\n";
				}
				saveIndentation(a_os);

				if(auto const t_key = std::get_if<CharString>(&t_item.first))
				{
					a_os << std::quoted(*t_key);
				}
				else
				{
					a_os << std::quoted(std::get<std::string_view>(t_item.first));
				}
				a_os << ": ";
				save(a_os, t_item.second);
			}
			a_os << '\n';
			--m_indentationLevel;
			saveIndentation(a_os);
			a_os << '}';
		}

		void saveIndentation(std::ostream& a_os) const
		{
			for (auto t_k = 0; t_k < m_indentationLevel; ++t_k)
				a_os << m_indentation;
		}
	};

}
