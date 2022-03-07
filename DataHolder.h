#pragma once

#include <vob/misc/hash/string_id.h>
#include <vob/misc/hash/string_id_literals.h>
#include <unordered_map>

#include "vob/aoe/ecs/component_manager.h"

#include <vob/misc/type/factory.h>
#include <vob/misc/type/registry.h>

#include "vob/aoe/common/render/debugscene/DebugSceneShaderProgram.h"
#include "vob/aoe/common/render/debugscene/DebugSceneRendercomponent.h"
#include "vob/aoe/common/render/model/StaticModel.h"
#include "vob/aoe/common/render/model/Modelcomponent.h"
#include "vob/aoe/common/render/model/ModelLoader.h"
#include "vob/aoe/common/render/model/ModelShaderProgram.h"
#include "vob/aoe/common/render/postprocess/PostProcessShaderProgram.h"
#include "vob/aoe/common/render/resources/RenderTexture.h"
#include "vob/aoe/common/render/resources/Texture.h"
#include "vob/aoe/common/render/TextureLoader.h"
#include <vob/aoe/common/data/filesystem/FileSystemDatabase.h>
#include "vob/aoe/common/serialization/VisitorFileSystemLoader.h"
#include "vob/aoe/common/map/Hierarchycomponent.h"
#include "vob/aoe/common/space/LocalTransformSystem.h"
#include "vob/aoe/common/test/Testcomponent.h"
#include "vob/aoe/common/render/Cameracomponent.h"
#include "vob/aoe/common/todo/SimpleControllercomponent.h"
#include "vob/aoe/common/space/Velocitycomponent.h"
#include "vob/aoe/common/space/Transformcomponent.h"
#include "vob/aoe/common/physic/RigidBodycomponent.h"
#include "vob/aoe/common/physic/SphereShape.h"
#include "vob/aoe/common/physic/CompoundShape.h"
#include "vob/aoe/common/physic/BoxShape.h"
#include "vob/aoe/common/physic/CylinderShape.h"
#include "vob/aoe/common/physic/ModelShape.h"
#include "vob/aoe/common/time/Lifetimecomponent.h"
#include "vob/aoe/common/physic/CapsuleShape.h"
#include "vob/aoe/common/physic/CharacterControllercomponent.h"
#include <vob/aoe/common/render/Manager.h>
#include <vob/aoe/common/data/filesystem/TextFileSystemLoader.h>
#include <vob/aoe/common/render/gui/GuiShaderProgram.h>
#include <vob/aoe/common/serialization/FileSystemVisitorContext.h>
#include <vob/aoe/common/render/gui/text/FontLoader.h>
#include <vob/aoe/common/render/gui/elements/TextElement.h>
#include <vob/aoe/common/render/gui/elements/TextInputElement.h>
#include <vob/aoe/common/render/gui/GuiMesh.h>
#include <vob/aoe/common/render/gui/Canvascomponent.h>
#include <vob/aoe/common/render/gui/elements/SplitElement.h>

#include <vob/aoe/ecs/component_holder.h>

#include <vob/misc/visitor/accept.h>
#include <vob/misc/visitor/json_reader.h>

using namespace vob;
using namespace vob::mishs::literals;

namespace vob::misvi
{
	template <typename DataType>
	requires std::is_base_of_v<aoe::type::ADynamicType, DataType>
	bool accept(
			misvi::pmr::json_reader<aoe::common::FileSystemVisitorContext>& a_visitor
			, std::shared_ptr<DataType const>& a_dataPtr
		)
	{
		std::string rawPath;
		a_visitor.visit(rawPath);

		auto const& context = a_visitor.get_context();
		auto const path = aoe::common::pathFromFilePath(rawPath, context.m_loadingDataPath);

		auto& indexer = context.m_fileSystemIndexer;
		context.m_database.find(indexer.get_id(path), a_dataPtr);
		return true;
	}
}

namespace vob::aoe
{
	struct is_json_file
	{
		bool operator()(std::filesystem::path const& a_path) const
		{
			return a_path.extension().generic_string() == ".json";
		}
	};

	template <typename TContext>
	struct json_visitor_loader
	{
		using allocator = std::pmr::polymorphic_allocator<char>;
		using applicator = misvi::pmr::applicator<false, misvi::pmr::json_reader<TContext>>;

		json_visitor_loader(
			misty::pmr::factory const& a_factory,
			applicator const& a_applicator,
			TContext a_context,
			allocator a_allocator = {})
			: m_jsonVisitor{ a_factory, a_applicator, std::move(a_context), a_allocator }
		{}

		template <typename TValue>
		void load(std::istream& a_inputStream, TValue& a_value)
		{
			mistd::pmr::json_value json;
			a_inputStream >> json;
			m_jsonVisitor.read(json, a_value);
		}

		misvi::pmr::json_reader<TContext> m_jsonVisitor;
	};

	struct DataHolder
	{
		DataHolder()
		{
			// Load indexer
			{
				std::ifstream fileIndexerFile{ "data/indexer.json" };
				std::nullptr_t dummyContext = nullptr;
				misty::pmr::registry dummyRegistry{};
				misty::pmr::factory dummyFactory{ dummyRegistry };
				misvi::pmr::applicator<false, misvi::pmr::json_reader<std::nullptr_t>> dummyApplicator;
				json_visitor_loader<std::nullptr_t> jsonVisitorLoader{
					dummyFactory,
					dummyApplicator,
					dummyContext
				};
				std::unordered_map<data::Id, std::filesystem::path> indexToPathMap;

				jsonVisitorLoader.load(fileIndexerFile, indexToPathMap);
				for (auto const& indexToPathPair : indexToPathMap)
				{
					fileSystemIndexer.insert(indexToPathPair.second, indexToPathPair.first);
				}
			}

			//aoe::common::AStandardElement* element = new aoe::common::TextElement{ database, guiMeshResourceManager };
			//jsonWriter.visit(*element);


			// Register data loaders
			registerDatabaseLoaders();

			// Register basic dynamic types
			{
				typeRegistry.register_type<type::ADynamicType>("vob::aoe::type::ADynamicType"_id);

				typeRegistry.register_type<aoecs::basic_component_holder>("vob::aoecs::basic_component_holder"_id);
				// registerDynamicType<aoecs::AComponent>("vob::aoecs::AComponent"_id);
				registerDynamicType<common::AElement>("vob::aoe::common::AElement"_id);
				registerDynamicType<common::AStandardElement, common::AElement>("vob::aoe::common::AStandardElement"_id);
				registerDynamicType<common::ACollisionShape>("vob::aoe::common::ACollisionShape"_id);
			}

			// Register non-visitable data types
			{
				registerDataType<common::Text>("vob::aoe::common::Text"_id);
				registerDataType<common::GraphicResourceHandle<common::StaticModel>>("vob::aoe::common::GraphicResourceHandle<vob::aoe::common::StaticModel>"_id);
				registerDataType<common::GraphicResourceHandle<common::Texture>>("vob::aoe::common::GraphicResourceHandle<vob::aoe::common::Texture>"_id);
				registerDataType<common::Font>("vob::aoe::common::Font"_id);
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
					aoecs::component_manager
					, type::ADynamicType
					, aoecs::component_holder_cloner const&
				>(
					"vob::aoecs::component_manager"_id
					, componentHolderCloner
					);
				registerVisitableDynamicType<
					common::TextElement
					, common::AStandardElement
					, common::IGraphicResourceManager<common::GuiMesh>&
				>(
					"vob::aoe::common::TextElement"_id
					, guiMeshResourceManager
				);
				dynamicTypeCloner.register_type<common::TextElement>();
				registerVisitableDynamicType<
					common::TextInputElement
					, common::AStandardElement
					, common::IGraphicResourceManager<common::GuiMesh>&
				>(
					"vob::aoe::common::TextInputElement"_id
					, guiMeshResourceManager
				);
				dynamicTypeCloner.register_type<common::TextInputElement>();
				registerVisitableDynamicType<
					common::SplitElement
					, common::AStandardElement
					, type::dynamic_type_clone_copier const&
				>(
					"vob::aoe::common::SplitElement"_id
					, dynamicTypeCloner
				);
				dynamicTypeCloner.register_type<common::SplitElement>();

				// TODO since they will be handles, shapes should inherit from ADynamicType...
				registerVisitableDynamicType<common::BoxShape, common::ACollisionShape>("vob::aoe::common::BoxShape"_id);
				dynamicTypeCloner.register_type<common::BoxShape>();
				registerVisitableDynamicType<common::CapsuleShape, common::ACollisionShape>("vob::aoe::common::CapsuleShape"_id);
				dynamicTypeCloner.register_type<common::CapsuleShape>();
				registerVisitableDynamicType<common::CylinderShape, common::ACollisionShape>("vob::aoe::common::CylinderShape"_id);
				dynamicTypeCloner.register_type<common::CylinderShape>();
				registerVisitableDynamicType<common::SphereShape, common::ACollisionShape>("vob::aoe::common::SphereShape"_id);
				dynamicTypeCloner.register_type<common::SphereShape>();
				registerVisitableDynamicType<common::ModelShape, common::ACollisionShape>("vob::aoe::common::ModelShape"_id);
				dynamicTypeCloner.register_type<common::ModelShape>();
				registerVisitableDynamicType<
					common::CompoundShape
					, common::ACollisionShape
					, type::dynamic_type_clone_copier const&
				>("vob::aoe::common::CompoundShape"_id, dynamicTypeCloner);
				dynamicTypeCloner.register_type<common::CompoundShape>();

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
				registerComponent<common::RigidBodyComponent, type::dynamic_type_clone_copier const&>("vob::aoe::common::RigidBodyComponent"_id, dynamicTypeCloner);
				registerComponent<common::CharacterControllerComponent>("vob::aoe::common::CharacterControllerComponent"_id);
				registerComponent<common::LifetimeComponent>("vob::aoe::common::LifetimeComponent"_id);
				registerComponent<common::CanvasComponent, type::dynamic_type_clone_copier const&>("vob::aoe::common::CanvasComponent"_id, dynamicTypeCloner);
				// registerComponent<common::gui::GuiComponent, type::Cloner const&>("gui::GuiComponent"_id, Cloner);
				// registerComponent<common::gui::ObjectComponent, type::Cloner const&>("gui::ObjectComponent"_id, Cloner);
			}
		}

		misty::pmr::registry typeRegistry;
		misty::pmr::factory factory{ typeRegistry };

		misty::pmr::clone_copier<type::ADynamicType> dynamicTypeCloner{};
		aoecs::component_holder_cloner componentHolderCloner{};
		misty::pmr::clone_copier<btCollisionShape> btCollisionShapeCloner{};
		// aoe::ins::InstanceAllocationSizer instanceAllocationSizer{ typeRegistry, typeFactory, allocator };
		common::FileSystemIndexer fileSystemIndexer;
		common::FileSystemDatabase database{
			typeRegistry
			, fileSystemIndexer
		};

		common::VisitorLoader<json_visitor_loader<common::FileSystemVisitorContext>, is_json_file> jsonVisitorLoader{
			factory,
			json_visitor_loader<common::FileSystemVisitorContext>::applicator{},
			fileSystemIndexer,
			database
		};
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
		void registerDynamicType(vob::mishs::string_id const a_id)
		{
			typeRegistry.register_type<DynamicType, BaseType>(a_id);
		}

		template <typename DataType, typename BaseType = type::ADynamicType>
		void registerDataType(vob::mishs::string_id const a_id)
		{
			registerDynamicType<DataType, BaseType>(a_id);
		}

		template <typename VisitableDynamicType, typename BaseType = type::ADynamicType, typename... Args>
		void registerVisitableDynamicType(vob::mishs::string_id const a_id, Args&&... a_args)
		{
			registerDynamicType<VisitableDynamicType, BaseType>(a_id);

			auto& applicator = jsonVisitorLoader.get_applicator();
			applicator.register_type<VisitableDynamicType>();

			// instanceAllocationSizer.register_type<VisitableDynamicType>();

			factory.add_type<VisitableDynamicType, Args...>(std::forward<Args>(a_args)...);
		}

		template <typename VisitableBtCollisionShape, typename BaseType = btCollisionShape, typename... Args>
		void registerVisitableBtCollisionShape(vob::mishs::string_id const a_id, Args&&... a_args)
		{
			registerDynamicType<VisitableBtCollisionShape, BaseType>(a_id);

			auto& applicator = jsonVisitorLoader.get_applicator();
			applicator.register_type<VisitableBtCollisionShape>();

			// instanceAllocationSizer.register_type<VisitableDynamicType>();

			factory.add_type<VisitableBtCollisionShape, Args...>(std::forward<Args>(a_args)...);
		}

		template <typename TComponent, typename... Args>
		void registerComponent(vob::mishs::string_id const a_id, Args&&... a_args)
		{
			registerVisitableDynamicType<
				aoecs::component_holder<TComponent>, aoecs::basic_component_holder, Args...
			>(a_id, std::forward<Args>(a_args)...);
			componentHolderCloner.register_type<aoecs::component_holder<TComponent>>();
		}
	};
}