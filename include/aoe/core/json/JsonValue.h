#pragma once
#include <variant>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include <memory_resource>

namespace aoe::json
{
	template <template <typename> typename AllocatorType = std::pmr::polymorphic_allocator>
	using CharString = std::basic_string<char, std::char_traits<char>, AllocatorType<char>>;

	template <template <typename> typename AllocatorType = std::pmr::polymorphic_allocator>
	using JsonString = CharString<AllocatorType>;

	template <template <typename> typename AllocatorType = std::pmr::polymorphic_allocator>
	struct JsonNumber
	{
		void getAs(bool& a_value) const
		{
			char const* t_cStr = m_representation.c_str();
			char* t_end;
			a_value = (std::strtod(t_cStr, &t_end) != 0.0);
		}

		void getAs(std::uint8_t& a_value) const
		{
			char const* t_cStr = m_representation.c_str();
			char* t_end;
			a_value = static_cast<std::uint8_t>(
				std::strtoul(t_cStr, &t_end, 10));
		}

		void getAs(std::int8_t& a_value) const
		{
			char const* t_cStr = m_representation.c_str();
			char* t_end;
			a_value = static_cast<std::int8_t>(
				std::strtol(t_cStr, &t_end, 10));
		}

		void getAs(std::uint16_t& a_value) const
		{
			char const* t_cStr = m_representation.c_str();
			char* t_end;
			a_value = static_cast<std::uint16_t>(
				std::strtoul(t_cStr, &t_end, 10));
		}

		void getAs(std::int16_t& a_value) const
		{
			char const* t_cStr = m_representation.c_str();
			char* t_end;
			a_value = static_cast<std::int16_t>(
				std::strtol(t_cStr, &t_end, 10));
		}

		void getAs(std::uint32_t& a_value) const
		{
			char const* t_cStr = m_representation.c_str();
			char* t_end;
			a_value = static_cast<std::uint32_t>(
				std::strtoul(t_cStr, &t_end, 10));
		}

		void getAs(std::int32_t& a_value) const
		{
			char const* t_cStr = m_representation.c_str();
			char* t_end;
			a_value = static_cast<std::int32_t>(
				std::strtol(t_cStr, &t_end, 10));
		}

		void getAs(std::uint64_t& a_value) const
		{
			char const* t_cStr = m_representation.c_str();
			char* t_end;
			a_value = static_cast<std::uint64_t>(
				std::strtoull(t_cStr, &t_end, 10));

		}

		void getAs(std::int64_t& a_value) const
		{
			char const* t_cStr = m_representation.c_str();
			char* t_end;
			a_value = static_cast<std::int64_t>(
				std::strtoll(t_cStr, &t_end, 10));
		}

		void getAs(float& a_value) const
		{
			char const* t_cStr = m_representation.c_str();
			char* t_end;
			a_value = std::strtof(t_cStr, &t_end);
		}

		void getAs(double& a_value) const
		{
			char const* t_cStr = m_representation.c_str();
			char* t_end;
			a_value = std::strtod(t_cStr, &t_end);
		}

		template <typename ValueType>
		void setFrom(ValueType const& a_value)
		{
			m_value = a_value;
			m_representation = std::to_string(a_value); // TODO
		}

		std::variant<bool, std::uint8_t, std::int8_t, std::uint16_t, std::int16_t, std::uint32_t, std::int32_t, std::uint64_t, std::int64_t, float, double> m_value;
		CharString<AllocatorType> m_representation;
	};

	using JsonBoolean = bool;
	using JsonNull = std::nullptr_t;

	template <template <typename> typename AllocatorType = std::pmr::polymorphic_allocator>
	struct JsonArray;

	template <template <typename> typename AllocatorType = std::pmr::polymorphic_allocator>
	struct JsonObject
	{
		using CharString = CharString<AllocatorType>;
		using JsonKey = std::variant<CharString, std::string_view>;
		using JsonValue = std::variant<JsonNull, JsonString<AllocatorType>
			, JsonNumber<AllocatorType>, JsonObject<AllocatorType>
			, JsonArray<AllocatorType>, JsonBoolean>;

		struct JsonKeyHash
		{
			std::size_t operator()(JsonKey const& a_key) const
			{
				if (auto t_stringKey = std::get_if<CharString>(&a_key))
				{
					return std::hash<CharString>{}(*t_stringKey);
				}
				else
				{
					auto t_stringViewKey = std::get<std::string_view>(a_key);
					return std::hash<std::string_view>{}(t_stringViewKey);
				}
			}
		};

		struct JsonKeyEqual
		{
			bool operator()(JsonKey const& a_lhs, JsonKey const& a_rhs) const
			{
				if (CharString const* t_stringLhs = std::get_if<CharString>(&a_lhs))
				{
					if (CharString const* t_stringRhs = std::get_if<CharString>(&a_rhs))
					{
						return *t_stringLhs == *t_stringRhs;
					}
					else
					{
						auto t_stringViewRhs = std::get<std::string_view>(a_rhs);
						return *t_stringLhs == t_stringViewRhs;
					}
				}
				else
				{
					auto t_stringViewLhs = std::get<std::string_view>(a_lhs);
					if (CharString const* t_stringRhs = std::get_if<CharString>(&a_rhs))
					{
						return t_stringViewLhs == *t_stringRhs;
					}
					else
					{
						auto t_stringViewRhs = std::get<std::string_view>(a_rhs);
						return t_stringViewLhs == t_stringViewRhs;
					}
				}
			}
		};

		std::unordered_map<JsonKey, JsonValue, JsonKeyHash
			, JsonKeyEqual, AllocatorType<std::pair<JsonKey const, JsonValue>>> m_values;
	};

	template <template <typename> typename AllocatorType>
	struct JsonArray
	{
		using JsonValue = std::variant<JsonNull, JsonString<AllocatorType>
			, JsonNumber<AllocatorType>, JsonObject<AllocatorType>
			, JsonArray<AllocatorType>, JsonBoolean>;

		std::vector<JsonValue, AllocatorType<JsonValue>> m_values;
	};

	template <template <typename> typename AllocatorType = std::pmr::polymorphic_allocator>
	using JsonValue = std::variant<JsonNull, JsonString<AllocatorType>
		, JsonNumber<AllocatorType>, JsonObject<AllocatorType>
		, JsonArray<AllocatorType>, JsonBoolean>;
}
