#pragma once

#include <vob/misc/hash/string_id.h>
#include <vob/misc/hash/string_id_literals.h>
#include <unordered_map>

#include <vob/misc/type/factory.h>
#include <vob/misc/type/registry.h>
#include <vob/misc/type/registry.h>

#include <vob/aoe/actor/action_component.h>
#include <vob/aoe/actor/actor_component.h>
#include <vob/aoe/core/type/ADynamicType.h> // ?
#include <vob/aoe/data/filesystem_database.h>
#include <vob/aoe/data/filesystem_visitor_context.h>
#include <vob/aoe/data/json_file_loader.h>
#include <vob/aoe/data/multi_database.h>
#include <vob/aoe/data/single_file_loader.h>
#include <vob/aoe/data/string_loader.h>
#include <vob/aoe/data/filesystem_util.h>
#include <vob/aoe/debug/debug_controller.h>
#include <vob/aoe/physics/material.h>
#include <vob/aoe/physics/components/rigidbody.h>
#include <vob/aoe/rendering/components/camera_component.h>
#include <vob/aoe/rendering/components/model_component.h>
#include <vob/aoe/rendering/components/model_data_component.h>
#include <vob/aoe/rendering/data/model_loader.h>
#include <vob/aoe/rendering/data/texture_file_loader.h>
#include <vob/aoe/rendering/data/program_data.h>
#include <vob/aoe/spacetime/transform.h>

#include <vob/misc/visitor/accept.h>
#include <vob/misc/visitor/json_reader.h>

#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>

using namespace vob;
using namespace vob::mishs::literals;

namespace vob::misvi
{
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
			// Register basic dynamic types
			{
				typeRegistry.register_type<type::ADynamicType>("vob::aoe::type::ADynamicType"_id);

				// v2
				typeRegistry.register_type<aoecs::detail::component_holder_base>("vob::aoecs::component_holder_base"_id);
			}

			// Register components
			{
				// register_component<common::HierarchyComponent>("vob::aoe::common::HierarchyComponent"_id);
				// register_component<common::gui::GuiComponent, type::Cloner const&>("gui::GuiComponent"_id, Cloner);
				// register_component<common::gui::ObjectComponent, type::Cloner const&>("gui::ObjectComponent"_id, Cloner);
				register_component<aoeac::action_component>("vob::aoeac::action_component"_id);
				register_component<aoeac::actor_component>("vob::newaoeac::actor_component"_id);

				// v2
				register_component<aoegl::camera_component>("vob::aoegl::camera_component"_id);
				register_component<aoegl::model_component>("vob::aoegl::model_component"_id);
				register_component<aoegl::model_data_component>("vob::aoegl::model_data_component"_id);
				// TODO: make it some comp_data -> comp pattern
				// register_component<aoeph::rigidbody>("vob::aoeph::rigidbody"_id);
				register_component<aoest::position>("vob::aoest::position"_id);
				register_component<aoest::rotation>("vob::aoest::rotation"_id);
				register_component<aoedb::debug_controller_component>("vob::aoedb::debug_controller_component"_id);
			}

			m_dynamicsWorld.setGravity(btVector3(0.0f, -25.0f, 0.0f));

			setup_multi_database();
		}

		misty::pmr::registry typeRegistry;
		misty::pmr::factory factory{ typeRegistry };

		misty::pmr::clone_copier<type::ADynamicType> dynamicTypeCloner{};
		misty::pmr::clone_copier<btCollisionShape> btCollisionShapeCloner{};

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

		// New physics
		btDefaultCollisionConfiguration m_collisionConfiguration;
		btCollisionDispatcher m_dispatcher{ &m_collisionConfiguration };
		btDbvtBroadphase m_pairCache{};
		btSequentialImpulseConstraintSolver m_constraintSolver{};
		btDiscreteDynamicsWorld m_dynamicsWorld{ &m_dispatcher, &m_pairCache
			, &m_constraintSolver, &m_collisionConfiguration };

	private:
		template <typename TDynamicType, typename TBaseType, typename... TArgs>
		void register_visitable_dynamic_type(vob::mishs::string_id const a_id, TArgs&&... a_args)
		{
			typeRegistry.register_type<TDynamicType, TBaseType>(a_id);

			jsonLoadApplicator.register_type<TDynamicType>();

			factory.add_type<TDynamicType, TArgs...>(std::forward<TArgs>(a_args)...);
		}

		template <typename TComponent, typename... TArgs>
		void register_component(vob::mishs::string_id const a_id, TArgs&&... a_args)
		{
			register_visitable_dynamic_type<
				aoecs::detail::component_holder<TComponent>, aoecs::detail::component_holder_base, TArgs...
			>(a_id, std::forward<TArgs>(a_args)...);
		}
	};
}