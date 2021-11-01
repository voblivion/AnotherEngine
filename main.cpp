
#include <vob/sta/vector_map.h>
#include <vob/random/perlin.h>
#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include "DataHolder.h"
#include "vob/aoe/common/window/WorldWindowComponent.h"
#include "vob/aoe/common/time/WorldTimeComponent.h"
#include "vob/aoe/common/input/WorldInputComponent.h"
#include "vob/aoe/common/window/WorldCursorComponent.h"
#include "vob/aoe/common/render/DirectorComponent.h"
#include "vob/aoe/common/physic/WorldPhysicComponent.h"
#include "vob/aoe/common/physic/DefaultDynamicsWorldHolder.h"
#include "vob/aoe/core/ecs/World.h"
#include "vob/aoe/common/physic/PhysicSystem.h"
#include "vob/aoe/common/test/TestSystem.h"
#include "vob/aoe/common/render/RenderSystem.h"
#include "vob/aoe/common/space/MoveSystem.h"
#include "vob/aoe/common/time/TimeSystem.h"
#include "vob/aoe/common/window/WindowCursorSystem.h"
#include "vob/aoe/common/window/WindowInputSystem.h"
#include "vob/aoe/common/todo/SimpleControllerSystem.h"
#include "vob/aoe/common/render/DefaultDirectorSystem.h"
#include "vob/aoe/common/time/LifetimeSystem.h"
#include "vob/aoe/common/map/HierarchySystem.h"
#include <vob/aoe/common/render/OpenGl.h>
#include <vob/aoe/common/render/Window.h>
#include <vob/aoe/common/render/SceneFramebufferInitializer.h>
#include <regex>
#include <memory_resource>

#include <vob/aoe/common/editor/EditorVisitor.h>

using namespace vob;
using namespace sta::literals;

const std::uint32_t g_width = 2048u;
const std::uint32_t g_height = 1024u;

std::unique_ptr<aoe::ecs::World> createGameWorld(aoe::DataHolder& a_data, aoe::common::IWindow& a_window)
{
	// Prepare world components
	aoe::ecs::ComponentManager t_worldComponents{ a_data.dynamicTypeCloner };
	t_worldComponents.addComponent<aoe::common::WorldWindowComponent>(a_window);
	t_worldComponents.addComponent<aoe::common::SceneRenderComponent>(
		a_data.renderTextureResourceManager
		, glm::ivec2{ g_width, g_height }
	);
	t_worldComponents.addComponent<aoe::common::ModelRenderComponent>(
		a_data.database
		, a_data.textureResourceManager
		, a_data.renderTextureResourceManager
		, a_data.staticModelResourceManager
		, a_data.modelShaderProgramResourceManager
		, glm::ivec2{ g_width, g_height }
	);
	t_worldComponents.addComponent<aoe::common::DebugSceneRenderComponent>(
		a_data.database
		, a_data.debugSceneShaderProgramResourceManager
	);
	t_worldComponents.addComponent<aoe::common::PostProcessRenderComponent>(
		a_data.database
		, a_data.postProcessShaderProgramResourceManager
	);
	t_worldComponents.addComponent<aoe::common::WorldTimeComponent>();
	t_worldComponents.addComponent<aoe::common::WorldInputComponent>();
	t_worldComponents.addComponent<aoe::common::WorldCursorComponent>();
	t_worldComponents.addComponent<aoe::common::DirectorComponent>();
	auto dynamicsWorldHolder = aoe::type::Cloneable<aoe::common::ADynamicsWorldHolder>(a_data.dynamicTypeCloner);
	dynamicsWorldHolder.init<aoe::common::DefaultDynamicsWorldHolder>();
	t_worldComponents.addComponent<aoe::common::WorldPhysicComponent>(std::move(dynamicsWorldHolder));
	t_worldComponents.addComponent<aoe::common::GuiRenderComponent>(
		a_data.database
		, a_data.guiShaderProgramResourceManager
		, a_data.guiMeshResourceManager
		, a_data.textureResourceManager
	);

	// Create world
	auto world = std::make_unique<aoe::ecs::World>(std::move(t_worldComponents));

	// Register Systems
	auto const timeSystemId = world->addSystem<aoe::common::TimeSystem>();
	auto const moveSystemId = world->addSystem<aoe::common::MoveSystem>();
	auto const windowInputSystemId = world->addSystem<aoe::common::WindowInputSystem>();
	auto const windowCursorSystemId = world->addSystem<aoe::common::WindowCursorSystem>();
	auto const simpleControllerSystemId = world->addSystem<aoe::common::SimpleControllerSystem>();
	auto const renderSystemId = world->addSystem<aoe::common::GameRenderSystem>();
	auto const testSystemId = world->addSystem<aoe::common::TestSystem>();
	auto const physicSystemId = world->addSystem<aoe::common::PhysicSystem>();
	auto const defaultDirectorSystemId = world->addSystem<aoe::common::DefaultDirectorSystem>();
	auto const lifetimeSystemId = world->addSystem<aoe::common::LifetimeSystem>();
	// auto const guiRenderSystemId = world->addSystem<aoe::common::gui::RenderSystem>();
	auto const hierarchySystemId = world->addSystem<aoe::common::HierarchySystem>();
	auto const localTransformSystemId = world->addSystem<aoe::common::LocalTransformSystem>();

	// Set schedule
	world->setSchedule({ {
		{timeSystemId,					{}}
		, {windowInputSystemId,			{}}
		, {windowCursorSystemId,		{}}
		, {simpleControllerSystemId,	{}}
		, {moveSystemId,				{}}
		, {testSystemId,				{}}
		, {defaultDirectorSystemId,     {}}
		, {localTransformSystemId,		{}}
		, {renderSystemId,			    {}}
		, {physicSystemId,				{}}
		, {lifetimeSystemId,			{}}
	//  , {guiRenderSystemId,			{}}
		, {hierarchySystemId,			{}}
	} });

	return std::move(world);
}

void initGameWorldGuiMap(aoe::DataHolder& a_data, aoe::ecs::World& a_world)
{
    auto& cloner = a_data.dynamicTypeCloner;
    auto& database = a_data.database;
    auto& worldData = a_world.getData();
    auto& systemSpawnManager = worldData.m_entityManager.getSystemSpawnManager();

	aoe::ecs::ComponentManager canvasArk{ cloner };
	auto& canvasComponent = canvasArk.addComponent<aoe::common::CanvasComponent>(cloner);

	auto& guiMeshResourceManager = a_data.guiMeshResourceManager;

	auto& splitElement = canvasComponent.m_rootElement.init<aoe::common::SplitElement>(cloner);
	splitElement.m_firstSideSize = 300.0f;
	splitElement.m_firstSide = aoe::common::SplitElement::Side::Right;

	auto& textInputElement = splitElement.m_firstChildElement.init<aoe::common::TextInputElement>(guiMeshResourceManager);
	textInputElement.m_borderColor = aoe::vec4{ 1.0f };
	textInputElement.m_borderWidth = aoe::vec4{ 1.f, 15.f, 25.f, 125.f };
	textInputElement.setText("Bonjour");
	textInputElement.setSize(32);
	textInputElement.setFont(database.find<aoe::common::Font>(a_data.fileSystemIndexer.getId("data/font.fnt")));

	systemSpawnManager.spawn(canvasArk);
}

vob::aoe::vis::EditorVisitor* leakingEditorVisitor = nullptr;

void initGameWorldDefaultMap(aoe::DataHolder& a_data, aoe::ecs::World& a_world)
{
	auto& worldData = a_world.getData();
	auto& systemSpawnManager = worldData.m_entityManager.getSystemSpawnManager();

	// Load player
	leakingEditorVisitor = new aoe::vis::EditorVisitor{
		a_data.database.find<aoe::common::Font>(9)
		, a_data.database
		, a_data.guiMeshResourceManager
		, a_data.dynamicTypeCloner
	};

	auto playerArk = a_data.database.find<aoe::ecs::ComponentManager>(2);
	if (playerArk != nullptr)
	{
		auto& player = systemSpawnManager.spawn(*playerArk);

		auto playerNeckArk = a_data.database.find<aoe::ecs::ComponentManager>(10);
		if (playerNeckArk != nullptr)
		{
			auto playerNeck = *playerNeckArk;
			auto hierarchy = playerNeck.getComponent<aoe::common::HierarchyComponent>();
			hierarchy->m_parent = aoe::ecs::EntityHandle{ player };
			auto& neck = systemSpawnManager.spawn(playerNeck);

			auto playerCameraArk = a_data.database.find<aoe::ecs::ComponentManager>(3);
			if (playerCameraArk != nullptr)
			{
				auto playerCamera = *playerCameraArk;
				auto hierarchy = playerCamera.getComponent<aoe::common::HierarchyComponent>();
				hierarchy->m_parent = aoe::ecs::EntityHandle{ neck };
				systemSpawnManager.spawn(playerCamera);
			}
		}

		auto canvas = player.getComponent<aoe::common::CanvasComponent>();
		//editionInterface = leakingEditorVisitor->generateEditionInterface(canvas->m_rootElement, stupidData);
	}

	auto const groundPath = std::filesystem::path{ "data/archetypes/grass.json" };
	auto groundArk = a_data.database.find<aoe::ecs::ComponentManager>(a_data.fileSystemIndexer.getId(groundPath));
	if (groundArk != nullptr)
	{
		auto ground = *groundArk;

		constexpr int sizeX = 64;
		constexpr int sizeY = 64;
		
		auto mc = ground.getComponent<aoe::common::ModelComponent>();

		std::vector<aoe::common::Vertex> vertices;
		vertices.reserve(3 * sizeX * sizeY * 2 * 2);
		std::vector<aoe::common::Triangle> triangles;
		triangles.reserve(sizeX * sizeY * 2 * 2);
		for (auto i = 0; i < sizeX; ++i)
		{
			for (auto j = 0; j < sizeY; ++j)
			{
				auto n = glm::vec3{ 0.0f, 1.0f, 0.0f };

				auto s = static_cast<std::uint32_t>(vertices.size());
				triangles.emplace_back(s, s + 1, s + 2);

				constexpr auto sc = 2.0f;

				constexpr auto e = 4.0f;

				auto a = static_cast<float>(i);
				auto b = static_cast<float>(j);
				auto c = a + 1.0f;
				auto d = b + 1.0f;

				vertices.emplace_back(glm::vec3{ a, sc *rng::perlin(a/e, b/e), b }, n, glm::vec2{ 0.0f, 0.0f });
				vertices.emplace_back(glm::vec3{ a, sc *rng::perlin(a/e, d/e), d }, n, glm::vec2{ 0.0f, 1.0f });
				vertices.emplace_back(glm::vec3{ c, sc *rng::perlin(c/e, b/e), b }, n, glm::vec2{ 1.0f, 0.0f });

				s = static_cast<std::uint32_t>(vertices.size());
				triangles.emplace_back(s, s + 1, s + 2);

				vertices.emplace_back(glm::vec3{ c, sc *rng::perlin(c/e, d/e), d }, n, glm::vec2{ 1.0f, 1.0f });
				vertices.emplace_back(glm::vec3{ c, sc *rng::perlin(c/e, b/e), b }, n, glm::vec2{ 1.0f, 0.0f });
				vertices.emplace_back(glm::vec3{ a, sc *rng::perlin(a/e, d/e), d }, n, glm::vec2{ 0.0f, 1.0f });
			}
		}

		std::vector<aoe::common::StaticMesh> meshes;
		meshes.emplace_back(std::move(vertices), std::move(triangles), 0);
		std::vector<aoe::common::Material> materials = (*mc->m_model)->m_materials;
		mc->m_model = std::make_shared<aoe::common::GraphicResourceHandle<aoe::common::StaticModel>>(a_data.staticModelResourceManager, meshes, materials);

		auto rbc = ground.getComponent<aoe::common::RigidBodyComponent>();
		static_cast<aoe::common::ModelShape&>(*rbc->m_collisionShape).setModel(mc->m_model);
		rbc->m_physicMaterial = std::make_shared<aoe::common::PhysicMaterial>();

		systemSpawnManager.spawn(std::move(ground));
	}

	//// Load model
	//auto modelArk = a_data.database.find<aoe::ecs::ComponentManager>(4);
	//if (modelArk != nullptr)
	//{
	//	{
	//		systemSpawnManager.spawn(*modelArk);
	//	}
	//}

	//// Load ground
	//auto groundArk = a_data.database.find<aoe::ecs::ComponentManager>(5);
	//if (groundArk != nullptr)
	//{
	//	{
	//		auto ark = *groundArk;
	//		auto& matrix = ark.getComponent<aoe::common::TransformComponent>()->m_matrix;
	//		matrix *= aoe::common::translation(glm::vec3{ 0.0f, 0.0f, 0.0f });
	//		systemSpawnManager.spawn(std::move(ark));
	//	}
	//	{
	//		auto ark = *groundArk;
	//		auto& matrix = ark.getComponent<aoe::common::TransformComponent>()->m_matrix;
	//		matrix *= aoe::common::translation(glm::vec3{ 8.0f, 0.0f, 0.2f });
	//		systemSpawnManager.spawn(std::move(ark));
	//	}
	//	{
	//		auto ark = *groundArk;
	//		auto& matrix = ark.getComponent<aoe::common::TransformComponent>()->m_matrix;
	//		matrix *= aoe::common::translation(glm::vec3{ 16.0f, 0.0f, 0.4f });
	//		systemSpawnManager.spawn(std::move(ark));
	//	}
	//	{
	//		auto ark = *groundArk;
	//		auto& matrix = ark.getComponent<aoe::common::TransformComponent>()->m_matrix;
	//		matrix *= aoe::common::translation(glm::vec3{ 24.0f, 0.0f, 0.6f });
	//		systemSpawnManager.spawn(std::move(ark));
	//	}
	//	{
	//		auto ark = *groundArk;
	//		auto& matrix = ark.getComponent<aoe::common::TransformComponent>()->m_matrix;
	//		matrix *= aoe::common::translation(glm::vec3{ -6.0f, 0.0f, 1.0f });
	//		systemSpawnManager.spawn(std::move(ark));
	//	}
	//}
}

std::unique_ptr<aoe::ecs::World> createEditorWorld(aoe::DataHolder& a_data)
{
	// Prepare world components
	aoe::ecs::ComponentManager t_worldComponents{ a_data.dynamicTypeCloner };
	return nullptr;
}

#include <filesystem>
#include <unordered_map>

static_assert(aoe::vis::FreeAcceptVisitable<aoe::vis::EditorVisitor, int>);


void glfwErrorCallback(int code, const char* description)
{
	__debugbreak();
}

char const* xinputMapping = "78696e70757401000000000000000000,XInput Gamepad (GLFW)"
	",platform:Windows,a:b0,b:b1,x:b2,y:b3,leftshoulder:b4,rightshoulder:b5,back:b6,start:b7"
	",leftstick:b8,rightstick:b9,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:a4"
	",righttrigger:a5,dpup:h0.1,dpright:h0.2,dpdown:h0.4,dpleft:h0.8,";

int main()
{
	// Create data
	aoe::DataHolder data;

	// Create game window
	glfwSetErrorCallback(glfwErrorCallback);
	if (!glfwInit())
	{
		return EXIT_FAILURE;
	}
	// TMP: xinput mapping not there by default?
	if (!glfwUpdateGamepadMappings(xinputMapping))
	{
		return EXIT_FAILURE;
	}
	std::cout << glfwGetVersionString() << std::endl;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_REFRESH_RATE, 0);

	{
		aoe::common::Window window{ g_width, g_height, "An Other Engine", nullptr, nullptr };
		glfwMakeContextCurrent(window.getNativeHandle());
		glfwSwapInterval(0);
		glEnable(GL_MULTISAMPLE);

		// Create game world
		auto world = createGameWorld(data, window);

		// Init with default map
		initGameWorldDefaultMap(data, *world);
		// initGameWorldGuiMap(data, *world);

		// Run game
		world->start();
	}

	// Destroy game window
	glfwTerminate();
}
