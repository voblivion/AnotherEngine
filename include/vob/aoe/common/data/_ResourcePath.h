#pragma once

#include <filesystem>
#include <regex>
#include <functional>

namespace vob::aoe::common
{
	/* TODO
	 *
	 *	For now I'm using filesystem for simplicity, but I dislike it because :
	 *	- no possibility for custom allocator (used default allocator afaik)
	 *	- exceptions everywhere while I only care about logical path
	 */
	namespace fs = std::filesystem;

	class ResourcePath
	{
	public:
		// Constructors
		ResourcePath() = default;

		template <typename StringType>
		ResourcePath(const StringType& a_path) noexcept
		{
			setValue(a_path);
		}

		// Accessors
		const std::string& value() const noexcept
		{
			return m_value;
		}

		// Methods
		template <typename Visitor>
		void accept(Visitor& a_visitor)
		{
			std::string path;
			a_visitor.visit(path);
			setValue(path);
		}

		// Operators
		friend bool operator==(const ResourcePath& a_lhs, const ResourcePath& a_rhs)
		{
			return a_lhs.m_value == a_rhs.m_value;
		}

		friend bool operator!=(const ResourcePath& a_lhs, const ResourcePath& a_rhs)
		{
			return a_lhs.m_value != a_rhs.m_value;
		}

		friend std::ostream& operator<<(std::ostream& a_os, const ResourcePath& a_path)
		{
			return a_os << a_path.m_value;
		}

	private:
		// Attributes
		std::string m_value;

		// Method
		template <typename StringType>
		void setValue(StringType const& a_path)
		{
			m_value = fs::relative(fs::weakly_canonical(a_path)).generic_string();
			m_value = std::regex_replace(m_value, std::regex("\\\\"), "/");
		}
	};

	inline ResourcePath resourcePathFrom(const ResourcePath& a_reference, const std::string_view a_relative)
	{
		const auto referencePath = fs::path{ a_reference.value() };
		const auto baseDirectoryPath = referencePath.parent_path();
		const auto relativePath = fs::path{ a_relative };

		return { baseDirectoryPath / relativePath };
	}

}

namespace std
{
	template <>
	struct hash<vob::aoe::common::ResourcePath>
	{
		std::size_t operator()(const vob::aoe::common::ResourcePath& a_resourcePath) const noexcept
		{
			return hash_value(a_resourcePath.value());
		}
	};
}
