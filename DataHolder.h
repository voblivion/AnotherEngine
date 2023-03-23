#pragma once

#include <vob/misc/hash/string_id.h>
#include <vob/misc/hash/string_id_literals.h>
#include <unordered_map>

#include "vob/aoe/ecs/_component_manager.h"

#include <vob/misc/type/factory.h>
#include <vob/misc/type/registry.h>

#include "vob/aoe/common/_render/debugscene/DebugSceneShaderProgram.h"
#include "vob/aoe/common/_render/debugscene/DebugSceneRendercomponent.h"
#include "vob/aoe/common/_render/model/static_model.h"
#include "vob/aoe/common/_render/model/Modelcomponent.h"
#include "vob/aoe/common/_render/model/static_model_loader.h"
#include "vob/aoe/common/_render/model/model_shader_program.h"
#include "vob/aoe/common/_render/postprocess/PostProcessShaderProgram.h"
#include "vob/aoe/common/_render/resources/RenderTexture.h"
#include "vob/aoe/common/_render/resources/Texture.h"
#include "vob/aoe/common/_render/TextureLoader.h"
#include <vob/aoe/common/data/filesystem/FileSystemDatabase.h>
#include "vob/aoe/common/serialization/VisitorFileSystemLoader.h"
#include "vob/aoe/common/map/Hierarchycomponent.h"
#include "vob/aoe/common/space/LocalTransformSystem.h"
#include "vob/aoe/common/test/Testcomponent.h"
#include "vob/aoe/common/_render/Cameracomponent.h"
#include "vob/aoe/common/todo/SimpleControllercomponent.h"
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
#include <vob/aoe/common/_render/Manager.h>
#include <vob/aoe/common/data/filesystem/TextFileSystemLoader.h>
#include <vob/aoe/common/_render/gui/GuiShaderProgram.h>
#include <vob/aoe/common/serialization/FileSystemVisitorContext.h>
#include <vob/aoe/common/_render/gui/text/FontLoader.h>
#include <vob/aoe/common/_render/gui/elements/TextElement.h>
#include <vob/aoe/common/_render/gui/elements/TextInputElement.h>
#include <vob/aoe/common/_render/gui/GuiMesh.h>
#include <vob/aoe/common/_render/gui/Canvascomponent.h>
#include <vob/aoe/common/_render/gui/elements/SplitElement.h>

#include <vob/aoe/actor/action_component.h>
#include <vob/aoe/actor/actor_component.h>
#include <vob/aoe/data/filesystem_database.h>
#include <vob/aoe/data/filesystem_visitor_context.h>
#include <vob/aoe/data/json_file_loader.h>
#include <vob/aoe/data/multi_database.h>
#include <vob/aoe/data/single_file_loader.h>
#include <vob/aoe/data/string_loader.h>
#include <vob/aoe/data/filesystem_util.h>
#include <vob/aoe/debug/debug_controller.h>
#include <vob/aoe/ecs/_component_holder.h>
#include <vob/aoe/ecs/component_list_factory.h>
#include <vob/aoe/physics/material.h>
#include <vob/aoe/physics/rigidbody_component.h>
#include <vob/aoe/rendering/components/camera_component.h>
#include <vob/aoe/rendering/components/model_component.h>
#include <vob/aoe/rendering/components/model_data_component.h>
#include <vob/aoe/rendering/data/model_loader.h>
#include <vob/aoe/rendering/data/texture_file_loader.h>
#include <vob/aoe/rendering/data/program_data.h>
#include <vob/aoe/spacetime/transform_component.h>

#include <vob/misc/visitor/accept.h>
#include <vob/misc/visitor/json_reader.h>

#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

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

	template <typename TData>
	bool accept(
		pmr::json_reader<aoedt::filesystem_visitor_context>& a_visitor
		, std::shared_ptr<TData const>& a_data)
	{
		std::string rawPath;
		a_visitor.visit(rawPath);

		auto const& context = a_visitor.get_context();
		auto const path = aoedt::filesystem_util::normalize(rawPath, context.get_base_path());
		auto const& indexer = context.get_indexer();
		a_data = context.get_multi_database().find<TData>(indexer.get_runtime_id(path));
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
			applicator const& a_applicator,
			TContext a_context,
			allocator a_allocator = {})
			: m_jsonVisitor{ a_applicator, std::move(a_context), a_allocator }
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

				typeRegistry.register_type<_aoecs::basic_component_holder>("vob::aoecs::basic_component_holder"_id);
				// registerDynamicType<aoecs::AComponent>("vob::aoecs::AComponent"_id);
				registerDynamicType<common::AElement>("vob::aoe::common::AElement"_id);
				registerDynamicType<common::AStandardElement, common::AElement>("vob::aoe::common::AStandardElement"_id);
				registerDynamicType<common::ACollisionShape>("vob::aoe::common::ACollisionShape"_id);

				// v2
				typeRegistry.register_type<aoecs::detail::component_holder_base>("vob::aoecs::component_holder_base"_id);
			}

			// Register non-visitable data types
			{
				registerDataType<common::Text>("vob::aoe::common::Text"_id);
				registerDataType<common::GraphicResourceHandle<common::static_model>>("vob::aoe::common::GraphicResourceHandle<vob::aoe::common::StaticModel>"_id);
				registerDataType<common::GraphicResourceHandle<common::Texture>>("vob::aoe::common::GraphicResourceHandle<vob::aoe::common::Texture>"_id);
				registerDataType<common::Font>("vob::aoe::common::Font"_id);
			}

			// Register visitable dynamic types
			{
				registerVisitableDynamicType<common::old_material>("vob::aoe::common::Material"_id);
				registerVisitableDynamicType<
					common::GraphicResourceHandle<aoegl::model_shader_program>
					, type::ADynamicType
					, common::IGraphicResourceManager<aoegl::model_shader_program>&
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
					_aoecs::component_manager
					, type::ADynamicType
					, _aoecs::component_holder_cloner const&
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
				registerComponent<common::SimpleControllerComponent>("vob::aoe::common::SimpleControllerComponent"_id);
				registerComponent<common::CameraComponent>("vob::aoe::common::CameraComponent"_id);
				registerComponent<common::RigidBodyComponent, type::dynamic_type_clone_copier const&>("vob::aoe::common::RigidBodyComponent"_id, dynamicTypeCloner);
				registerComponent<common::CharacterControllerComponent>("vob::aoe::common::CharacterControllerComponent"_id);
				registerComponent<common::LifetimeComponent>("vob::aoe::common::LifetimeComponent"_id);
				registerComponent<common::CanvasComponent, type::dynamic_type_clone_copier const&>("vob::aoe::common::CanvasComponent"_id, dynamicTypeCloner);
				// registerComponent<common::gui::GuiComponent, type::Cloner const&>("gui::GuiComponent"_id, Cloner);
				// registerComponent<common::gui::ObjectComponent, type::Cloner const&>("gui::ObjectComponent"_id, Cloner);
				registerComponent<aoeac::action_component>("vob::aoeac::action_component"_id);
				registerComponent<aoeac::actor_component>("vob::newaoeac::actor_component"_id);

				// v2
				register_component<aoegl::camera_component>("vob::aoegl::camera_component"_id);
				register_component<aoegl::model_component>("vob::aoegl::model_component"_id);
				register_component<aoegl::model_data_component>("vob::aoegl::model_data_component"_id);
				register_component<aoeac::actor_component>("vob::aoeac::actor_component"_id);
				register_component<aoeph::rigidbody_component>("vob::aoeph::rigidbody_component"_id);
				register_component<aoest::transform_component>("vob::aoest::transform_component"_id);
				register_component<aoedb::debug_controller_component>("vob::aoedb::debug_controller_component"_id);
				
				// old v2
				register_component<common::HierarchyComponent>("vob::oldaoe::common::HierarchyComponent"_id);
				register_component<common::TransformComponent>("vob::oldaoe::common::TransformComponent"_id);
				register_component<common::ModelComponent>("vob::oldaoe::common::ModelComponent"_id);
				register_component<common::SimpleControllerComponent>("vob::oldaoe::common::SimpleControllerComponent"_id);
				//register_component<common::RigidBodyComponent>("vob::oldaoe::common::RigidBodyComponent"_id);
			}

			setup_multi_database();
		}

		vob::aoecs::component_list_factory componentListFactory;

		misty::pmr::registry typeRegistry;
		misty::pmr::factory factory{ typeRegistry };

		misty::pmr::clone_copier<type::ADynamicType> dynamicTypeCloner{};
		_aoecs::component_holder_cloner componentHolderCloner{};
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
		common::static_model_loader modelLoader{ database, fileSystemIndexer, staticModelResourceManager };
		common::FontLoader fontLoader{ database };

		// common resource managers
		common::SingleWindowGraphicResourceManager<common::RenderTexture> renderTextureResourceManager;
		common::SingleWindowGraphicResourceManager<common::Texture> textureResourceManager;

		common::SingleWindowGraphicResourceManager<common::static_model> staticModelResourceManager;
		common::SingleWindowGraphicResourceManager<aoegl::model_shader_program> modelShaderProgramResourceManager;

		common::SingleWindowGraphicResourceManager<common::DebugSceneShaderProgram> debugSceneShaderProgramResourceManager;

		common::SingleWindowGraphicResourceManager<common::PostProcessShaderProgram> postProcessShaderProgramResourceManager;

		common::SingleWindowGraphicResourceManager<common::GuiShaderProgram> guiShaderProgramResourceManager;
		common::SingleWindowGraphicResourceManager<common::GuiMesh> guiMeshResourceManager;


		// v2
		aoedt::filesystem_indexer filesystemIndexer;
		aoedt::multi_database multiDatabase;
		using context = aoedt::filesystem_visitor_context;
		using context_factory = aoedt::filesystem_visitor_context_factory;
		context_factory contextFactory{ factory, filesystemIndexer, multiDatabase };

		aoedt::filesystem_database<aoedt::single_file_loader<aoedt::string_loader>> stringDatabase{
			filesystemIndexer };
		aoedt::filesystem_database<aoegl::texture_file_loader> textureDatabase{
			filesystemIndexer };
		aoedt::filesystem_database<aoegl::model_loader> modelDatabase{
			filesystemIndexer, textureDatabase, filesystemIndexer };
		
		misvi::pmr::applicator<false, misvi::pmr::json_reader<context>> jsonLoadApplicator;
		aoedt::filesystem_database<aoedt::json_file_loader<aoegl::program_data, context_factory>>
			shaderProgramDatabase{ filesystemIndexer, jsonLoadApplicator, contextFactory };
		aoedt::filesystem_database<aoedt::json_file_loader<aoeph::material, context_factory>>
			physicMaterialDatabase{ filesystemIndexer, jsonLoadApplicator, contextFactory };
		aoedt::filesystem_database<aoedt::json_file_loader<aoecs::component_set, context_factory>>
			componentSetDatabase{ filesystemIndexer, jsonLoadApplicator, contextFactory };

		void setup_multi_database()
		{
			multiDatabase.register_database(stringDatabase);
			multiDatabase.register_database(textureDatabase);
			multiDatabase.register_database(modelDatabase);

			multiDatabase.register_database(shaderProgramDatabase);
			multiDatabase.register_database(physicMaterialDatabase);
			multiDatabase.register_database(componentSetDatabase);

			auto foo = shaderProgramDatabase.find(
				filesystemIndexer.get_runtime_id("data/shaders/shader_new_db_test.json"));
			auto bar = physicMaterialDatabase.find(
				filesystemIndexer.get_runtime_id("data/physics/new_material.json"));
			auto joh = componentSetDatabase.find(
				filesystemIndexer.get_runtime_id("data/player_v2.json"));
		}
		/*{
			aoedt::filesystem_indexer indexer;

			aoedt::filesystem_database stringDatabase{
				indexer, aoedt::single_file_loader{ aoedt::string_loader{} } };

			misvi::pmr::applicator<false, misvi::pmr::json_reader<std::nullptr_t>> applicator;
			aoedt::json_loader<int, std::nullptr_t> intLoader{ applicator, nullptr };

		}*/

		// New physics
		btDefaultCollisionConfiguration m_collisionConfiguration;
		btCollisionDispatcher m_dispatcher{ &m_collisionConfiguration };
		btDbvtBroadphase m_pairCache{};
		btSequentialImpulseConstraintSolver m_constraintSolver{};
		btDiscreteDynamicsWorld m_dynamicsWorld{ &m_dispatcher, &m_pairCache
			, &m_constraintSolver, &m_collisionConfiguration };

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

		template <typename TDynamicType, typename TBaseType, typename... TArgs>
		void register_visitable_dynamic_type(vob::mishs::string_id const a_id, TArgs&&... a_args)
		{
			typeRegistry.register_type<TDynamicType, TBaseType>(a_id);

			jsonLoadApplicator.register_type<TDynamicType>();

			factory.add_type<TDynamicType, TArgs...>(std::forward<TArgs>(a_args)...);
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

		template <typename TComponent, typename... TArgs>
		void registerComponent(vob::mishs::string_id const a_id, TArgs&&... a_args)
		{
			registerVisitableDynamicType<
				_aoecs::component_holder<TComponent>, _aoecs::basic_component_holder, TArgs...
			>(a_id, std::forward<TArgs>(a_args)...);
			componentHolderCloner.register_type<_aoecs::component_holder<TComponent>>();
		}

		template <typename TComponent, typename... TArgs>
		void register_component(vob::mishs::string_id const a_id, TArgs&&... a_args)
		{
			register_visitable_dynamic_type<
				aoecs::detail::component_holder<TComponent>, aoecs::detail::component_holder_base, TArgs...
			>(a_id, std::forward<TArgs>(a_args)...);

			componentListFactory.register_component<TComponent>();
		}
	};
}