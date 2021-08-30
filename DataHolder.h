#pragma once

#include <vob/sta/string_id.h>
#include <unordered_map>

#include "vob/aoe/core/ecs/ComponentManager.h"
#include "vob/aoe/core/type/TypeRegistry.h"
#include "vob/aoe/core/type/TypeFactory.h"
#include <vob/aoe/core/visitor/Standard.h>
#include <vob/aoe/core/visitor/StringId.h>

#include "vob/aoe/common/render/debugscene/DebugSceneShaderProgram.h"
#include "vob/aoe/common/render/debugscene/DebugSceneRenderComponent.h"
#include "vob/aoe/common/render/model/StaticModel.h"
#include "vob/aoe/common/render/model/ModelComponent.h"
#include "vob/aoe/common/render/model/ModelLoader.h"
#include "vob/aoe/common/render/model/ModelShaderProgram.h"
#include "vob/aoe/common/render/postprocess/PostProcessShaderProgram.h"
#include "vob/aoe/common/render/resources/RenderTexture.h"
#include "vob/aoe/common/render/resources/Texture.h"
#include "vob/aoe/common/render/TextureLoader.h"
#include <vob/aoe/common/data/filesystem/FileSystemDatabase.h>
#include "vob/aoe/common/serialization/VisitorFileSystemLoader.h"
#include "vob/aoe/common/map/HierarchyComponent.h"
#include "vob/aoe/common/space/LocalTransformSystem.h"
#include "vob/aoe/common/test/TestComponent.h"
#include "vob/aoe/common/render/CameraComponent.h"
#include "vob/aoe/common/window/SimpleControllerComponent.h"
#include "vob/aoe/common/space/VelocityComponent.h"
#include "vob/aoe/common/space/TransformComponent.h"
#include "vob/aoe/common/physic/RigidBodyComponent.h"
#include "vob/aoe/common/physic/SphereShape.h"
#include "vob/aoe/common/physic/BoxShape.h"
#include "vob/aoe/common/physic/CylinderShape.h"
#include "vob/aoe/common/time/LifetimeComponent.h"
#include "vob/aoe/common/physic/CapsuleShape.h"
#include "vob/aoe/common/physic/CharacterControllerComponent.h"
#include <vob/aoe/common/render/Manager.h>
#include <vob/aoe/common/data/filesystem/TextFileSystemLoader.h>
#include <vob/aoe/common/render/gui/GuiShaderProgram.h>
#include <vob/aoe/common/serialization/FileSystemVisitorContext.h>
#include <vob/aoe/common/render/gui/text/FontLoader.h>
#include <vob/aoe/common/render/gui/elements/TextElement.h>
#include <vob/aoe/common/render/gui/elements/TextInputElement.h>
#include <vob/aoe/common/render/gui/GuiMesh.h>
#include <vob/aoe/common/render/gui/CanvasComponent.h>
#include "vob/aoe/core/visitor/JsonWriter.h"
#include <vob/aoe/common/render/gui/elements/SplitElement.h>
using namespace vob;
using namespace sta::literals;

namespace vob::aoe
{
	namespace vis
	{
		/*template <typename DataType>
		void accept(
			vis::JsonWriter<common::FileSystemVisitorContext<vis::JsonWriter>>& a_visitor
			, data::Handle<DataType>& a_handle
		)
		{
			std::string rawPath;
			a_visitor.visit(rawPath);

			auto const& context = a_visitor.getContext();
			auto const path = common::pathFromFilePath(rawPath, context.m_loadingDataPath);

			auto& indexer = a_visitor.getContext().m_fileSystemIndexer;
			auto id = indexer.getId(path);
			a_handle.setId(id);
		}*/

		template <typename DataType>
		std::enable_if_t<std::is_base_of_v<type::ADynamicType, DataType>> accept(
			vis::JsonWriter<common::FileSystemVisitorContext<vis::JsonWriter>>& a_visitor
			, std::shared_ptr<DataType const>& a_dataPtr
		)
		{
			std::string rawPath;
			a_visitor.visit(rawPath);

			auto const& context = a_visitor.getContext();
			auto const path = common::pathFromFilePath(rawPath, context.m_loadingDataPath);

			auto& indexer = a_visitor.getContext().m_fileSystemIndexer;
			a_visitor.getContext().m_database.find(indexer.getId(path), a_dataPtr);
		}
	}

	struct DataHolder
	{
		DataHolder()
		{
			// Load indexer
			std::ifstream fileIndexerFile{ "data/indexer.json" };
			std::nullptr_t dummyContext = nullptr;
			vis::JsonWriter<std::nullptr_t> jsonWriter{ dummyContext };
			std::unordered_map<data::Id, std::filesystem::path> indexToPathMap;
			jsonWriter.load(fileIndexerFile, indexToPathMap);
			for (auto const& indexToPathPair : indexToPathMap)
			{
				fileSystemIndexer.insert(indexToPathPair.second, indexToPathPair.first);
			}

			//aoe::common::AStandardElement* element = new aoe::common::TextElement{ database, guiMeshResourceManager };
			//jsonWriter.visit(*element);


			// Register data loaders
			registerDatabaseLoaders();

			// Register dynamic types
			{
				typeRegistry.registerType<type::ADynamicType>("vob::aoe::type::ADynamicType"_id);
				registerDynamicType<ecs::AComponent>("vob::aoe::ecs::AComponent"_id);
				typeRegistry.registerType<btCollisionShape>("btCollisionShape"_id);
			}

			// Register non-visitable data types
			{
				registerDataType<common::Text>("vob::aoe::common::Text"_id);
				registerDataType<common::GraphicResourceHandle<common::StaticModel>>("vob::aoe::common::GraphicResourceHandle<vob::aoe::common::StaticModel>"_id);
				registerDataType<common::GraphicResourceHandle<common::Texture>>("vob::aoe::common::GraphicResourceHandle<vob::aoe::common::Texture>"_id);
				registerDataType<common::Font>("vob::aoe::common::Font"_id);
				registerDataType<common::AElement>("vob::aoe::common::AElement"_id);
				registerDataType<common::AStandardElement, common::AElement>("vob::aoe::common::AStandardElement"_id);
			}

			// Register visitable dynamic types
			{
				registerVisitableDynamicType<common::Material>("vob::aoe::common::Material"_id);
				registerVisitableDynamicType<
					common::GraphicResourceHandle<common::ModelShaderProgram>
					, type::ADynamicType
					, common::IGraphicResourceManager<common::ModelShaderProgram>&
				>(
					"vob::aoe::common::GraphicResourceHandle<vob::aoe::common::ModelShaderProgram>"_id
					, modelShaderProgramResourceManager
					);
				registerVisitableDynamicType<
					common::GraphicResourceHandle<common::DebugSceneShaderProgram>
					, type::ADynamicType
					, common::IGraphicResourceManager<common::DebugSceneShaderProgram>&
				>(
					"vob::aoe::common::GraphicResourceHandle<vob::aoe::common::DebugSceneShaderProgram>"_id
					, debugSceneShaderProgramResourceManager
					);
				registerVisitableDynamicType<
					common::GraphicResourceHandle<common::PostProcessShaderProgram>
					, type::ADynamicType
					, common::IGraphicResourceManager<common::PostProcessShaderProgram>&
				>(
					"vob::aoe::common::GraphicResourceHandle<vob::aoe::common::PostProcessShaderProgram>"_id
					, postProcessShaderProgramResourceManager
					);
				registerVisitableDynamicType<
					common::GraphicResourceHandle<common::GuiShaderProgram>
					, type::ADynamicType
					, common::IGraphicResourceManager<common::GuiShaderProgram>&
				>(
					"vob::aoe::common::GraphicResourceHandle<vob::aoe::common::GuiShaderProgram>"_id
					, guiShaderProgramResourceManager
					);
				registerVisitableDynamicType<
					ecs::ComponentManager
					, type::ADynamicType
					, type::Cloner<type::ADynamicType> const&
				>(
					"vob::aoe::ecs::ComponentManager"_id
					, dynamicTypeCloner
					);
				registerVisitableDynamicType<
					common::TextElement
					, common::AStandardElement
					, common::IGraphicResourceManager<common::GuiMesh>&
				>(
					"vob::aoe::common::TextElement"_id
					, guiMeshResourceManager
				);
				dynamicTypeCloner.registerType<common::TextElement>();
				registerVisitableDynamicType<
					common::TextInputElement
					, common::AStandardElement
					, common::IGraphicResourceManager<common::GuiMesh>&
				>(
					"vob::aoe::common::TextInputElement"_id
					, guiMeshResourceManager
				);
				dynamicTypeCloner.registerType<common::TextInputElement>();
				registerVisitableDynamicType<
					common::SplitElement
					, common::AStandardElement
					, type::Cloner<type::ADynamicType> const&
				>(
					"vob::aoe::common::SplitElement"_id
					, dynamicTypeCloner
				);
				dynamicTypeCloner.registerType<common::SplitElement>();

				// TODO since they will be handles, shapes should inherit from ADynamicType...
				registerVisitableBtCollisionShape<btBoxShape, btCollisionShape, btVector3>("btBoxShape"_id, btVector3{ 1.0f, 1.0f, 1.0f });
				btCollisionShapeCloner.registerType<btBoxShape>();
				registerVisitableBtCollisionShape<btCapsuleShape, btCollisionShape, btScalar, btScalar>("btCapsuleShape"_id, btScalar{ 1.0f }, btScalar{ 1.0f });
				btCollisionShapeCloner.registerType<btCapsuleShape>();
				registerVisitableBtCollisionShape<btCylinderShape, btCollisionShape, btVector3>("btCylinderShape"_id, btVector3{ 1.0f, 1.0f, 1.0f });
				btCollisionShapeCloner.registerType<btCylinderShape>();
				registerVisitableBtCollisionShape<btSphereShape, btCollisionShape, btScalar>("btSphereShape"_id, btScalar{ 1.0f });
				btCollisionShapeCloner.registerType<btSphereShape>();

				registerVisitableDynamicType<common::PhysicMaterial, type::ADynamicType>("vob::aoe::common::PhysicMaterial"_id);
			}

			// Register components
			{
				registerComponent<common::TransformComponent>("vob::aoe::common::TransformComponent"_id);
				registerComponent<common::ModelComponent>("vob::aoe::common::ModelComponent"_id);
				registerComponent<common::HierarchyComponent>("vob::aoe::common::HierarchyComponent"_id);
				registerComponent<common::LocalTransformComponent>("vob::aoe::common::LocalTransformComponent"_id);
				registerComponent<common::TestComponent>("vob::aoe::common::TestComponent"_id);
				registerComponent<common::VelocityComponent>("vob::aoe::common::VelocityComponent"_id);
				registerComponent<common::SimpleControllerComponent>("vob::aoe::common::SimpleControllerComponent"_id);
				registerComponent<common::CameraComponent>("vob::aoe::common::CameraComponent"_id);
				registerComponent<common::RigidBodyComponent, type::Cloner<btCollisionShape> const&>("vob::aoe::common::RigidBodyComponent"_id, btCollisionShapeCloner);
				registerComponent<common::CharacterControllerComponent>("vob::aoe::common::CharacterControllerComponent"_id);
				registerComponent<common::LifetimeComponent>("vob::aoe::common::LifetimeComponent"_id);
				registerComponent<common::CanvasComponent, type::Cloner<type::ADynamicType> const&>("vob::aoe::common::CanvasComponent"_id, dynamicTypeCloner);
				// registerComponent<common::gui::GuiComponent, type::Cloner const&>("gui::GuiComponent"_id, Cloner);
				// registerComponent<common::gui::ObjectComponent, type::Cloner const&>("gui::ObjectComponent"_id, Cloner);
			}
		}

		type::TypeRegistry typeRegistry;
		type::TypeFactory<type::ADynamicType> dynamicTypeFactory{ typeRegistry };
		type::TypeFactory<btCollisionShape> btCollisionShapeFactory{ typeRegistry };

		type::Cloner<type::ADynamicType> dynamicTypeCloner{};
		type::Cloner<btCollisionShape> btCollisionShapeCloner{};
		// aoe::ins::InstanceAllocationSizer instanceAllocationSizer{ typeRegistry, typeFactory, allocator };
		common::FileSystemIndexer fileSystemIndexer;
		common::FileSystemDatabase database{
			typeRegistry
			, fileSystemIndexer
		};

		common::VisitorLoader<vis::JsonWriter> jsonVisitorLoader{ typeRegistry, dynamicTypeFactory, btCollisionShapeFactory, fileSystemIndexer, database };
		common::TextFileSystemLoader textLoader{};
		common::TextureLoader textureLoader{ textureResourceManager };
		common::ModelLoader modelLoader{ database, fileSystemIndexer, staticModelResourceManager };
		common::FontLoader fontLoader{ database };

		// common resource managers
		common::SingleWindowGraphicResourceManager<common::RenderTexture> renderTextureResourceManager;
		common::SingleWindowGraphicResourceManager<common::Texture> textureResourceManager;

		common::SingleWindowGraphicResourceManager<common::StaticModel> staticModelResourceManager;
		common::SingleWindowGraphicResourceManager<common::ModelShaderProgram> modelShaderProgramResourceManager;

		common::SingleWindowGraphicResourceManager<common::DebugSceneShaderProgram> debugSceneShaderProgramResourceManager;

		common::SingleWindowGraphicResourceManager<common::PostProcessShaderProgram> postProcessShaderProgramResourceManager;

		common::SingleWindowGraphicResourceManager<common::GuiShaderProgram> guiShaderProgramResourceManager;
		common::SingleWindowGraphicResourceManager<common::GuiMesh> guiMeshResourceManager;

	private:
		void registerDatabaseLoaders()
		{
			// In order!
			database.getMultiFileSystemLoader().registerLoader(textureLoader);
			database.getMultiFileSystemLoader().registerLoader(modelLoader);
			database.getMultiFileSystemLoader().registerLoader(jsonVisitorLoader);
			database.getMultiFileSystemLoader().registerLoader(fontLoader);
			database.getMultiFileSystemLoader().registerLoader(textLoader);
		}

		template <typename DynamicType, typename BaseType = type::ADynamicType>
		void registerDynamicType(vob::sta::string_id const a_id)
		{
			typeRegistry.registerType<DynamicType, BaseType>(a_id);
		}

		template <typename DataType, typename BaseType = type::ADynamicType>
		void registerDataType(vob::sta::string_id const a_id)
		{
			registerDynamicType<DataType, BaseType>(a_id);
		}

		template <typename VisitableDynamicType, typename BaseType = type::ADynamicType
			, typename... Args>
			void registerVisitableDynamicType(vob::sta::string_id const a_id, Args&&... a_args)
		{
			registerDynamicType<VisitableDynamicType, BaseType>(a_id);

			auto& applicator = jsonVisitorLoader.getDynamicTypeApplicator();
			applicator.registerType<VisitableDynamicType>();

			// instanceAllocationSizer.registerType<VisitableDynamicType>();

			dynamicTypeFactory.addFactory<VisitableDynamicType, Args...>(std::forward<Args>(a_args)...);
		}

		template <typename VisitableBtCollisionShape, typename BaseType = btCollisionShape
			, typename... Args>
			void registerVisitableBtCollisionShape(vob::sta::string_id const a_id, Args&&... a_args)
		{
			registerDynamicType<VisitableBtCollisionShape, BaseType>(a_id);

			auto& applicator = jsonVisitorLoader.getBtCollisionShapeApplicator();
			applicator.registerType<VisitableBtCollisionShape>();

			// instanceAllocationSizer.registerType<VisitableDynamicType>();

			btCollisionShapeFactory.addFactory<VisitableBtCollisionShape, Args...>(std::forward<Args>(a_args)...);
		}

		template <typename ComponentType, typename... Args>
		void registerComponent(vob::sta::string_id const a_id, Args&&... a_args)
		{
			registerVisitableDynamicType<ComponentType, ecs::AComponent, Args...>(a_id, std::forward<Args>(a_args)...);
			dynamicTypeCloner.registerType<ComponentType>();
		}
	};
}