#pragma once

#include <fstream>
#include <memory>

#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>
#include <vob/aoe/core/data/Handle.h>
#include <vob/aoe/common/data/filesystem/AFileSystemLoader.h>
#include <vob/aoe/common/serialization/FileSystemVisitorContext.h>

namespace vob::aoe::common
{
	template <template <typename> typename VisitorWriterType>
	class VisitorLoader final
		: public AFileSystemLoader
	{
	public:
		// Aliases
		using Context = FileSystemVisitorContext<VisitorWriterType>;
		using Visitor = VisitorWriterType<Context>;

		// Constructors
		explicit VisitorLoader(
			type::TypeRegistry const& a_typeRegistry
			, type::TypeFactory<type::ADynamicType> const& a_dynamicTypeFactory
			, type::TypeFactory<btCollisionShape> const& a_btCollisionShapeFactory
			, FileSystemIndexer& a_fileSystemIndexer
		)
			: m_typeRegistry{ a_typeRegistry }
			, m_dynamicTypeFactory{ a_dynamicTypeFactory }
			, m_btCollisionShapeFactory{ a_btCollisionShapeFactory }
			, m_fileSystemIndexer{ a_fileSystemIndexer }
		{}

		// Methods
		bool canLoad(std::filesystem::path const& a_path) const override
		{
			// TODO should set extensions that are supported ?
			std::ifstream file;
			open(file, a_path);

			const auto context = createContext(a_path);
			auto visitor = Visitor{ context };

			return visitor.canLoad(file);
		}

		std::shared_ptr<ADynamicType> load(std::filesystem::path const& a_path) const override
		{
			std::ifstream file;
			open(file, a_path);

			const auto context = createContext(a_path);
			auto visitor = Visitor{ context };

			std::shared_ptr<ADynamicType> t_data;
			visitor.load(file, t_data);
			return t_data;
		}

		auto& getDynamicTypeApplicator()
		{
			return m_dynamicTypeApplicator;
		}

		auto& getBtCollisionShapeApplicator()
		{
			return m_btCollisionShapeApplicator;
		}

	private:
		// Attributes
		type::TypeRegistry const& m_typeRegistry;
		type::TypeFactory<type::ADynamicType> const& m_dynamicTypeFactory;
		type::TypeFactory<btCollisionShape> const& m_btCollisionShapeFactory;
		FileSystemIndexer& m_fileSystemIndexer;
		vis::Applicator<type::ADynamicType, Visitor> m_dynamicTypeApplicator;
		vis::Applicator<btCollisionShape, Visitor> m_btCollisionShapeApplicator;

		// Methods
		static void open(std::ifstream& a_file, std::filesystem::path const& a_path)
		{
			a_file.open(a_path, std::ios::binary | std::ios::in);
		}

		Context createContext(std::filesystem::path const& a_path) const
		{
			return {
				m_typeRegistry
				, m_dynamicTypeFactory
				, m_btCollisionShapeFactory
				, m_fileSystemIndexer
				, m_dynamicTypeApplicator
				, m_btCollisionShapeApplicator
				, a_path
			};
		}
	};
}

