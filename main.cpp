
#include <vob/misc/std/vector_map.h>
#include <vob/_todo_/random/perlin.h>
#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include "DataHolder.h"
#include "vob/aoe/common/window/WorldWindowcomponent.h"
#include "vob/aoe/common/time/WorldTimecomponent.h"
#include "vob/aoe/common/input/WorldInputcomponent.h"
#include "vob/aoe/common/window/WorldCursorcomponent.h"
#include "vob/aoe/common/render/Directorcomponent.h"
#include "vob/aoe/common/physic/WorldPhysiccomponent.h"
#include "vob/aoe/common/physic/DefaultDynamicsWorldHolder.h"
#include "vob/aoe/ecs/world.h"
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
#include <vob/misc/hash/string_id_literals.h>
#include <vob/misc/physics/measure_literals.h>
#include <vob/misc/visitor/is_visitable.h>

#include <vob/aoe/common/editor/EditorVisitor.h>


using namespace vob;
using namespace misph::literals;
using namespace mishs::literals;

const std::uint32_t g_width = 2048u;
const std::uint32_t g_height = 1024u;

std::unique_ptr<aoecs::world> createGameWorld(aoe::DataHolder& a_data, aoe::common::IWindow& a_window)
{
	// Prepare world components
	aoecs::component_manager t_worldComponents{ a_data.componentHolderCloner };
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
	auto dynamicsWorldHolder = aoe::type::dynamic_type_clone<aoe::common::ADynamicsWorldHolder>(a_data.dynamicTypeCloner);
	dynamicsWorldHolder.init<aoe::common::DefaultDynamicsWorldHolder>();
	t_worldComponents.addComponent<aoe::common::WorldPhysicComponent>(std::move(dynamicsWorldHolder));
	t_worldComponents.addComponent<aoe::common::GuiRenderComponent>(
		a_data.database
		, a_data.guiShaderProgramResourceManager
		, a_data.guiMeshResourceManager
		, a_data.textureResourceManager
	);

	// Create world
	auto world = std::make_unique<aoecs::world>(std::move(t_worldComponents));

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
	world->setSchedule({
		mismt::thread_schedule{
			{windowInputSystemId, {timeSystemId}}
			, {windowCursorSystemId, {}}
			, {renderSystemId, {localTransformSystemId}}
		},
		mismt::thread_schedule{
			{timeSystemId,					{}}
			, {simpleControllerSystemId,	{windowCursorSystemId}}
			, {moveSystemId,				{}}
			, {testSystemId,				{}}
			, {defaultDirectorSystemId,     {}}
			, {localTransformSystemId,		{}}
			, {physicSystemId,				{renderSystemId}}
			, {lifetimeSystemId,			{}}
		//  , {guiRenderSystemId,			{}}
			, {hierarchySystemId,			{}}
		}
	});

	return std::move(world);
}

void initGameWorldGuiMap(aoe::DataHolder& a_data, aoecs::world& a_world)
{
    auto& cloner = a_data.dynamicTypeCloner;
    auto& database = a_data.database;
    auto& worldData = a_world.getData();
    auto& systemSpawnManager = worldData.m_entityManager.getSystemSpawnManager();

	aoecs::component_manager canvasArk{ a_data.componentHolderCloner };
	auto& canvasComponent = canvasArk.addComponent<aoe::common::CanvasComponent>(cloner);

	auto& guiMeshResourceManager = a_data.guiMeshResourceManager;

	auto& splitElement = canvasComponent.m_rootElement.init<aoe::common::SplitElement>(cloner);
	splitElement.m_firstSideSize = 300.0f;
	splitElement.m_firstSide = aoe::common::SplitElement::Side::Right;

	auto& textInputElement = splitElement.m_firstChildElement.init<aoe::common::TextInputElement>(guiMeshResourceManager);
	textInputElement.m_borderColor = glm::vec4{ 1.0f };
	textInputElement.m_borderWidth = glm::vec4{ 1.f, 15.f, 25.f, 125.f };
	textInputElement.setText(U"Bonjour");
	textInputElement.setSize(32);
	textInputElement.setFont(database.find<aoe::common::Font>(a_data.fileSystemIndexer.getId("data/font.fnt")));

	systemSpawnManager.spawn(canvasArk);
}

vob::aoe::vis::EditorVisitor* leakingEditorVisitor = nullptr;

void initGameWorldDefaultMap(aoe::DataHolder& a_data, aoecs::world& a_world)
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

	auto playerArk = a_data.database.find<aoecs::component_manager>(2);
	if (playerArk != nullptr)
	{
		auto& player = systemSpawnManager.spawn(*playerArk);

		auto playerNeckArk = a_data.database.find<aoecs::component_manager>(10);
		if (playerNeckArk != nullptr)
		{
			auto playerNeck = *playerNeckArk;
			auto hierarchy = playerNeck.getComponent<aoe::common::HierarchyComponent>();
			hierarchy->m_parent = player.get_id();
			auto& neck = systemSpawnManager.spawn(playerNeck);

			auto playerCameraArk = a_data.database.find<aoecs::component_manager>(3);
			if (playerCameraArk != nullptr)
			{
				auto playerCamera = *playerCameraArk;
				auto hierarchy = playerCamera.getComponent<aoe::common::HierarchyComponent>();
				hierarchy->m_parent = neck.get_id();
				systemSpawnManager.spawn(playerCamera);
			}
		}

		auto canvas = player.getComponent<aoe::common::CanvasComponent>();
		//editionInterface = leakingEditorVisitor->generateEditionInterface(canvas->m_rootElement, stupidData);
	}

	auto const groundPath = std::filesystem::path{ "data/archetypes/grass.json" };
	auto groundArk = a_data.database.find<aoecs::component_manager>(a_data.fileSystemIndexer.getId(groundPath));
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
				auto s = static_cast<std::uint32_t>(vertices.size());
				triangles.emplace_back(s, s + 1, s + 2);

				constexpr auto bs = float(64)/sizeX;
				constexpr auto sc = float(sizeX)/8;

				constexpr auto e = 4.0f;

				auto x0 = static_cast<float>(i);
				auto y0 = static_cast<float>(j);
				auto x1 = x0 + 1.0f;
				auto y1 = y0 + 1.0f;
				auto xn = x0 - 1.0f;
				auto yn = y0 - 1.0f;
				auto x2 = x1 + 1.0f;
				auto y2 = y1 + 1.0f;

				auto p = [bs, sc, e](float x, float y)
				{
					return glm::vec3{ bs * x, sc * rng::perlin(x/32, y/32), bs * y };
				};

				auto p0 = p(x0, y0);
				auto p1 = p(x1, y0);
				auto p2 = p(x0, y1);
				auto p3 = p(x1, y1);

				auto p01 = p(xn, y0);
				auto p10 = p(x2, y0);
				auto p02 = p(x0, yn);
				auto p13 = p(x1, yn);
				auto p23 = p(xn, y1);
				auto p32 = p(x2, y1);
				auto p20 = p(x0, y2);
				auto p31 = p(x1, y2);

				auto f = [](auto p0, auto p1, auto p2)
				{
					auto faceNormal = glm::cross(p1-p0, p2-p0);
					return faceNormal;
				};

				auto n = [&f](auto p0, auto p1, auto p2, auto p3, auto p4)
				{
					auto averageNormal =
						glm::normalize(f(p0, p1, p2) + f(p0, p2, p3) + f(p0, p3, p4) + f(p0, p4, p1));
					return -averageNormal;
				};
				
				auto n0 = n(p0, p1, p2, p01, p02);
				auto n1 = n(p1, p10, p3, p0, p13);
				auto n2 = n(p2, p3, p20, p23, p0);
				auto n3 = n(p3, p32, p31, p2, p1);
				/*n0 = glm::normalize(f(p1, p2, p01) + f(p01, p02, p1));
				n0 = glm::normalize(f(p10, p3, p0) + f(p0, p13, p10));
				n0 = glm::normalize(f(p3, p20, p23) + f(p23, p0, p3));
				n0 = glm::normalize(f(p32, p31, p2) + f(p2, p1, p32));

				n1 = f(p1, p3, p0);
				n2 = f(p3, p2, p1);
				n3 = f(p2, p0, p3);
				n1 = n0;
				n2 = n0;
				n3 = n0;*/

				auto tc0 = glm::vec2{ 0.0f, 0.0f };
				auto tc1 = glm::vec2{ 0.0f, 1.0f };
				auto tc2 = glm::vec2{ 1.0f, 0.0f };
				auto tc3 = glm::vec2{ 1.0f, 1.0f };

				auto t0 = glm::vec3{1.0f, 0.0f, 0.0f};
				auto t3 = glm::vec3{1.0f, 0.0f, 0.0f};

				vertices.emplace_back(p0, n0, tc0, t0);
				vertices.emplace_back(p1, n1, tc1, t0);
				vertices.emplace_back(p2, n2, tc2, t0);

				s = static_cast<std::uint32_t>(vertices.size());
				triangles.emplace_back(s, s + 1, s + 2);

				vertices.emplace_back(p3, n3, tc3, t3);
				vertices.emplace_back(p2, n2, tc2, t3);
				vertices.emplace_back(p1, n1, tc1, t3);
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
	//auto modelArk = a_data.database.find<aoecs::component_manager>(4);
	//if (modelArk != nullptr)
	//{
	//	{
	//		systemSpawnManager.spawn(*modelArk);
	//	}
	//}

	//// Load ground
	//auto groundArk = a_data.database.find<aoecs::component_manager>(5);
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

std::unique_ptr<aoecs::world> createEditorWorld(aoe::DataHolder& a_data)
{
	// Prepare world components
	aoecs::component_manager t_worldComponents{ a_data.componentHolderCloner };
	return nullptr;
}

#include <filesystem>
#include <unordered_map>

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
		auto gameWorld = createGameWorld(data, window);

		// Init with default map
		initGameWorldDefaultMap(data, *gameWorld);
		// initGameWorldGuiMap(data, *gameWorld);

		// Run game
		gameWorld->start();
	}

	// Destroy game window
	glfwTerminate();
}

/*
	Raw/Physical input -> Logical input -> Action input

	Action input:
	- dynamic level? game -> map<ActionId, LogicalInput>?


*/