#pragma once

/*#include <fstream>
#include <string>
#include <unordered_map>
#include <vob/aoe/core/data/Id.h>
#include <vob/aoe/core/data/FormattedInputStream.h>
#include <vob/aoe/core/type/TypeFactory.h>
#include <vob/aoe/core/visitor/Standard.h>

namespace vob::aoe::data
{
	class FormattedFile
	{
	public:
		// Methods
		template <typename Visitor>
		void accept(Visitor& a_visitor)
		{
			a_visitor.visit("format_id", m_format);
			a_visitor.visit("binary", m_binary);
			a_visitor.visit("file_name", m_name);
		}

		auto getFormat() const
		{
			return m_format;
		}

		bool isBinary() const
		{
			return m_binary;
		}

		std::string const& getName() const
		{
			return m_name;
		}

	private:
		// Attributes
		FormatId m_format = 0;
		bool m_binary = true;
		std::string m_name;
	};

	class FileIndexer
	{
	public:
		// Aliases
		using FormattedFileMap = std::unordered_map<Id, FormattedFile>;
		using AllocatorType = FormattedFileMap::allocator_type;

		// Constructors
		FileIndexer() = default;

		explicit FileIndexer(AllocatorType const& a_allocator)
			: m_formattedFiles{ a_allocator }
		{}

		// Methods
		template <typename Visitor>
		void accept(Visitor& a_visitor)
		{
			a_visitor.visit("data", m_formattedFiles);
		}

		sta::PolymorphicPtr<data::AFormattedInputStream> find(
			data::DataId const a_dataId) const
		{
			auto const t_it = m_formattedFiles.find(a_dataId);
			if (t_it != m_formattedFiles.end())
			{
				auto const t_openMode = t_it->second.isBinary()
					? std::ios::binary | std::ios::in : std::ios::in;
				return sta::allocatePolymorphic<
					data::FormattedInputStream<std::ifstream>>(
					m_formattedFiles.get_allocator()
					, t_it->second.getFormat()
					, std::ifstream{ t_it->second.getName().data()
						, t_openMode });
			}
			return nullptr;
		}

	private:
		FormattedFileMap m_formattedFiles;
	};
}
*/