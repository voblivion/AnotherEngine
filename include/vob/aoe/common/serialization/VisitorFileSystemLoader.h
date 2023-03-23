#pragma once

#include <fstream>
#include <memory>

#include <vob/aoe/common/data/filesystem/AFileSystemLoader.h>
#include <vob/aoe/common/serialization/FileSystemVisitorContext.h>

#include <vob/misc/visitor/applicator.h>

#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>

namespace vob::aoe::common
{
#pragma message("TODO Should be removed, probably no need for VisitorLoader<json_visitor_loader<context>>")
	template <typename TVisitor, typename TCanLoad>
	class VisitorLoader final
		: public AFileSystemLoader
	{
	public:
		// Constructors
		explicit VisitorLoader(
			misty::pmr::factory const& a_factory,
			TVisitor::applicator a_applicator,
			FileSystemIndexer& a_fileSystemIndexer,
			data::ADatabase& a_database,
			TVisitor::allocator a_allocator = {})
			: m_factory{ a_factory }
			, m_applicator{ a_applicator }
			, m_fileSystemIndexer{ a_fileSystemIndexer }
			, m_database{ a_database }
			, m_allocator{ a_allocator }
		{}

		// Methods
		bool canLoad(std::filesystem::path const& a_path) const override
		{
			return TCanLoad{}(a_path);
		}

		std::shared_ptr<ADynamicType> load(std::filesystem::path const& a_path) const override
		{
			std::ifstream file{ a_path, std::ios::binary | std::ios::in };

			const auto context = createContext(a_path);
			auto visitor = TVisitor{ m_applicator, createContext(a_path), m_allocator };

			std::shared_ptr<ADynamicType> t_data;
			visitor.load(file, t_data);
			return t_data;
		}

		auto& get_applicator()
		{
			return m_applicator;
		}

	private:
		// Attributes
		misty::pmr::factory const& m_factory;
		TVisitor::applicator m_applicator;
		FileSystemIndexer& m_fileSystemIndexer;
		data::ADatabase& m_database;
		TVisitor::allocator m_allocator;

		// Methods
		FileSystemVisitorContext createContext(std::filesystem::path const& a_path) const
		{
			return {
				m_factory
				, m_fileSystemIndexer
				, a_path
				, m_database
			};
		}
	};
}

