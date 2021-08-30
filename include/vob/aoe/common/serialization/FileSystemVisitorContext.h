#pragma once

#include <vob/aoe/core/type/TypeRegistry.h>
#include <vob/aoe/core/type/TypeFactory.h>
#include <vob/aoe/core/visitor/Applicator.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>

#include <vob/aoe/common/data/filesystem/FileSystemIndexer.h>

namespace vob::aoe::common
{
	template <template <typename> typename VisitorType>
	struct FileSystemVisitorContext
	{
		using Self = FileSystemVisitorContext<VisitorType>;
		using DynamicTypeApplicator = vis::Applicator<type::ADynamicType, VisitorType<Self>>;
		using btCollisionShapeApplicator = vis::Applicator<btCollisionShape, VisitorType<Self>>;

		FileSystemVisitorContext(
			type::TypeRegistry const& a_typeRegistry
			, type::TypeFactory<type::ADynamicType> const& a_dynamicTypeFactory
			, type::TypeFactory<btCollisionShape> const& a_btCollisionShapeFactory
			, FileSystemIndexer& a_fileSystemIndexer
			, DynamicTypeApplicator const& a_dynamicTypeApplicator
			, btCollisionShapeApplicator const& a_btCollisionShapeApplicator
			, std::filesystem::path const& a_loadingDataPath
			, data::ADatabase& a_database
		)
			: m_typeRegistry{ a_typeRegistry }
			, m_dynamicTypeFactory{ a_dynamicTypeFactory }
			, m_btCollisionShapeFactory{ a_btCollisionShapeFactory }
			, m_fileSystemIndexer{ a_fileSystemIndexer }
			, m_dynamicTypeApplicator{ a_dynamicTypeApplicator }
			, m_btCollisionShapeApplicator{ a_btCollisionShapeApplicator }
			, m_loadingDataPath{ a_loadingDataPath }
			, m_database{ a_database }
		{}

		type::TypeRegistry const& m_typeRegistry;
		type::TypeFactory<type::ADynamicType> const& m_dynamicTypeFactory;
		type::TypeFactory<btCollisionShape> const& m_btCollisionShapeFactory;
		FileSystemIndexer& m_fileSystemIndexer;
		DynamicTypeApplicator const& m_dynamicTypeApplicator;
		btCollisionShapeApplicator const& m_btCollisionShapeApplicator;
		std::filesystem::path const& m_loadingDataPath;
		data::ADatabase& m_database;
	};
}
