#pragma once

#include <optional>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vob/aoe/core/data/Id.h>
#include <vob/aoe/core/data/FormattedInputStream.h>
#include <vob/aoe/core/visitor/Standard.h>

namespace vob::aoe::common
{
	class FormattedFile
	{
	public:
		// Methods
		template <typename Visitor>
		void accept(Visitor& a_visitor)
		{
			a_visitor.visit(vis::makeNameValuePair("format_id", m_format));
			a_visitor.visit(vis::makeNameValuePair("binary", m_binary));
			a_visitor.visit(vis::makeNameValuePair("file_name", m_name));
		}

		data::FormatId getFormat() const
		{
			return m_format;
		}

		bool isBinary() const
		{
			return m_binary;
		}

		std::pmr::string const& getName() const
		{
			return m_name;
		}

	private:
		// Attributes
		data::FormatId m_format = 0;
		bool m_binary = true;
		std::pmr::string m_name;
	};

	class FileIndexer
	{
	public:
		// Aliases
		using FormattedFileMap = std::pmr::unordered_map<data::Id, FormattedFile>;
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
			a_visitor.visit(vis::makeNameValuePair("data", m_formattedFiles));
		}

		FormattedFile const* find(data::Id const a_dataId) const
		{
			auto const t_it = m_formattedFiles.find(a_dataId);
			return t_it != m_formattedFiles.end() ? &t_it->second : nullptr;
		}

		void set(data::Id const a_dataId, FormattedFile a_formattedFile)
		{
			m_formattedFiles.emplace(a_dataId, std::move(a_formattedFile));
		}

		void set(data::Id const a_dataId
			, FormattedFile const& a_formattedFile)
		{
			m_formattedFiles.emplace(a_dataId, a_formattedFile);
		}

	private:
		FormattedFileMap m_formattedFiles;
	};
}
