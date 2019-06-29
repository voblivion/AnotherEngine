#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <aoe/core/data/Id.h>
#include <aoe/core/data/FormattedInputStream.h>
#include <aoe/core/visitor/Standard.h>

namespace aoe
{
	namespace common
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
				a_visitor.visit("data", m_formattedFiles);
			}

			sta::PolymorphicPtr<data::AFormattedInputStream> find(
				data::Id const a_dataId) const
			{
				using InputStream = data::FormattedInputStream<std::ifstream>;

				auto const t_it = m_formattedFiles.find(a_dataId);
				if (t_it != m_formattedFiles.end())
				{
					auto const t_openMode = t_it->second.isBinary()
						? std::ios::binary | std::ios::in : std::ios::in;
					return sta::allocatePolymorphic<InputStream>(
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
}
