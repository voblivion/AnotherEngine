
#include <vob/sta/vector_map.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "DataHolder.h"
#include "vob/aoe/common/window/WindowComponent.h"
#include "vob/aoe/common/time/TimeComponent.h"
#include "vob/aoe/common/window/InputComponent.h"
#include "vob/aoe/common/window/CursorComponent.h"
#include "vob/aoe/common/render/DirectorComponent.h"
#include "vob/aoe/common/physic/WorldPhysicComponent.h"
#include "vob/aoe/common/physic/DefaultDynamicsWorldHolder.h"
#include "vob/aoe/core/ecs/World.h"
#include "vob/aoe/common/physic/PhysicSystem.h"
#include "vob/aoe/common/test/TestSystem.h"
#include "vob/aoe/common/render/RenderSystem.h"
#include "vob/aoe/common/space/MoveSystem.h"
#include "vob/aoe/common/time/TimeSystem.h"
#include "vob/aoe/common/window/InputSystem.h"
#include "vob/aoe/common/window/SimpleControllerSystem.h"
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
	t_worldComponents.addComponent<aoe::common::WindowComponent>(a_window);
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
	t_worldComponents.addComponent<aoe::common::TimeComponent>();
	t_worldComponents.addComponent<aoe::common::InputComponent>();
	t_worldComponents.addComponent<aoe::common::CursorComponent>();
	t_worldComponents.addComponent<aoe::common::DirectorComponent>();
	auto dynamicsWorldHolder = aoe::type::Cloneable<
		aoe::common::ADynamicsWorldHolder
		, vob::aoe::type::ADynamicType
	>(a_data.dynamicTypeCloner);
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
	auto const inputSystemId = world->addSystem<aoe::common::InputSystem>();
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
		, {inputSystemId,				{}}
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

	auto& splitElement = canvasComponent.m_rootElement.init<aoe::common::SplitElement>(database, cloner);
	splitElement.m_firstSideSize = 300.0f;
	splitElement.m_firstSide = aoe::common::SplitElement::Side::Right;

	auto& textInputElement = splitElement.m_firstChildElement.init<aoe::common::TextInputElement>(database, guiMeshResourceManager);
	textInputElement.m_borderColor = aoe::vec4{ 1.0f };
	textInputElement.m_borderWidth = aoe::vec4{ 1.f, 15.f, 25.f, 125.f };
	textInputElement.setText("Bonjour");
	textInputElement.setSize(32);
	textInputElement.setFont(aoe::data::Handle<aoe::common::Font>{database, a_data.fileSystemIndexer.getId("data/font.fnt")});

	systemSpawnManager.spawn(canvasArk);
}

vob::aoe::vis::EditorVisitor* leakingEditorVisitor = nullptr;

void initGameWorldDefaultMap(aoe::DataHolder& a_data, aoe::ecs::World& a_world)
{
	auto& worldData = a_world.getData();
	auto& systemSpawnManager = worldData.m_entityManager.getSystemSpawnManager();

	// Load player
	leakingEditorVisitor = new aoe::vis::EditorVisitor{
		aoe::data::Handle<aoe::common::Font>(a_data.database, 9)
		, a_data.database
		, a_data.guiMeshResourceManager
		, a_data.dynamicTypeCloner
	};

	aoe::data::Handle<aoe::ecs::ComponentManager> playerArk{ a_data.database, 2 };
	if (playerArk.isValid())
	{
		auto& player = systemSpawnManager.spawn(*playerArk);

		aoe::data::Handle<aoe::ecs::ComponentManager> playerCameraArk{ a_data.database, 3 };
		if (playerCameraArk.isValid())
		{
			auto playerCamera = *playerCameraArk;
			auto hierarchy = playerCamera.getComponent<aoe::common::HierarchyComponent>();
			hierarchy->m_parent = aoe::ecs::EntityHandle{ player };
			systemSpawnManager.spawn(playerCamera);
		}

		auto canvas = player.getComponent<aoe::common::CanvasComponent>();
		//editionInterface = leakingEditorVisitor->generateEditionInterface(canvas->m_rootElement, stupidData);
	}

	// Load model
	aoe::data::Handle<aoe::ecs::ComponentManager> t_modelArk{ a_data.database, 4 };
	if (t_modelArk.isValid())
	{
		{
			auto ark = *t_modelArk;
			systemSpawnManager.spawn(ark);
		}
// 		{
// 			auto ark = *t_modelArk;
// 			auto& matrix = ark.getComponent<aoe::common::TransformComponent>()->m_matrix;
// 			matrix = glm::translate(matrix, glm::vec3{ 0.0f, 0.5f, 0.0f });
// 			auto& t_model = systemSpawnManager.spawn(ark);
// 		}
// 		if (auto t_hierarchyComponent = t_camera.getComponent<common::HierarchyComponent>())
// 		{
// 			t_hierarchyComponent->m_children.push_back(t_model.getId());
// 		}
// 		auto& t_other = worldData.m_entityManager.getSystemSpawnManager().spawn(*t_modelArk);
	}

	// Load ground
	aoe::data::Handle<aoe::ecs::ComponentManager> t_groundArk{ a_data.database, 5 };
	if (t_groundArk.isValid())
	{
		{
			auto ark = *t_groundArk;
			auto& matrix = ark.getComponent<aoe::common::TransformComponent>()->m_matrix;
			matrix *= aoe::common::translation(glm::vec3{ 0.0f, 0.0f, 0.0f });
			systemSpawnManager.spawn(std::move(ark));
		}
		{
			auto ark = *t_groundArk;
			auto& matrix = ark.getComponent<aoe::common::TransformComponent>()->m_matrix;
			matrix *= aoe::common::translation(glm::vec3{ 8.0f, 0.0f, 0.2f });
			systemSpawnManager.spawn(std::move(ark));
		}
		{
			auto ark = *t_groundArk;
			auto& matrix = ark.getComponent<aoe::common::TransformComponent>()->m_matrix;
			matrix *= aoe::common::translation(glm::vec3{ 16.0f, 0.0f, 0.4f });
			systemSpawnManager.spawn(std::move(ark));
		}
		{
			auto ark = *t_groundArk;
			auto& matrix = ark.getComponent<aoe::common::TransformComponent>()->m_matrix;
			matrix *= aoe::common::translation(glm::vec3{ 24.0f, 0.0f, 0.6f });
			systemSpawnManager.spawn(std::move(ark));
		}
		{
			auto ark = *t_groundArk;
			auto& matrix = ark.getComponent<aoe::common::TransformComponent>()->m_matrix;
			matrix *= aoe::common::translation(glm::vec3{ -6.0f, 0.0f, 1.0f });
			systemSpawnManager.spawn(std::move(ark));
		}
	}
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

template <typename BaseType>
class DataResource : public AResourceHolder<BaseType>
{
public:
	BaseType* get() override
	{
		return m_ptr.get();
	}

private:
    std::uint64_t m_id;
	// using a shared_ptr for proper destruction and deallocation
	std::shared_ptr<BaseType> m_ptr;
};

// TO CONSIDERE IF shared_ptr TOO MEMORY HEAVY
/* struct AUniquePtrBlock
{
	virtual void destroy() = 0;
};

template <typename Type, typename Allocator>
struct PolymorphicPtrBlock : AUniquePtrBlock
{
	template <typename... Args>
	PolymorphicPtrBlock(Allocator a_allocator, Args&&... a_args)
		: m_allocator{ a_allocator }
		, m_object{ std::forward<Args>(a_args)... }
	{}

	void destroy() override
	{
		this->~PolymorphicPtrBlock();
		using Self = PolymorphicPtrBlock<Type, Allocator>;
		using BlockAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Self>;
		auto allocator = BlockAllocator{ m_allocator };
		allocator.deallocate(this, 1);
	}

    Type m_object;
    Allocator m_allocator;
};

struct PolymorphicDeleter
{
	void operator()(void* a_ptr)
	{
		auto p0 = static_cast<AUniquePtrBlock*>(a_ptr);
		auto p1 = p0 - 1;
		p1->destroy();
	}
};

template <typename Type>
using PolymorphicPtr = std::unique_ptr<Type, PolymorphicDeleter>;

template <typename Type, typename Allocator, typename... Args>
PolymorphicPtr<Type> allocatePolymorphic(Allocator a_allocator, Args&&... a_args)
{
	using Block = PolymorphicPtrBlock<Type, Allocator>;
	using BlockAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Block>;
	auto allocator = BlockAllocator{ a_allocator };

	auto block = allocator.allocate(1);
	::new (block) Block(a_allocator, std::forward<Args>(a_args)...);
	return PolymorphicPtr<Type>{ &block->m_object };
}

struct Foo
{
	~Foo()
	{
		std::cout << "deleting Foo" << std::endl;
	}

	virtual int joh() { return 0; }
};

struct Bar : public Foo
{
	Bar() = default;
	Bar(int k)
		: m_k{ k }
	{}

	~Bar()
	{
		std::cout << "deleting Bar" << std::endl;
	}

	int joh() override { return m_k; }

	int m_k = 17;
	std::array<char, 10000000> dummy;
};

struct debug_resource : std::pmr::memory_resource
{
	debug_resource(std::pmr::memory_resource* a_upstream = std::pmr::get_default_resource())
		: m_upstream{ a_upstream }
	{}

	void* do_allocate(size_t a_size, size_t a_alignment) override
    {
        std::cout << "allocating " << a_size << " bytes" << std::endl;
		return m_upstream->allocate(a_size, a_alignment);
	}

	void do_deallocate(void* a_ptr, size_t a_size, size_t a_alignment) override
	{
		std::cout << "deallocating " << a_size << " bytes" << std::endl;
		m_upstream->deallocate(a_ptr, a_size, a_alignment);
	}

	bool do_is_equal(std::pmr::memory_resource const& a_other) const noexcept override
	{
		return this == &a_other;
	}

	std::pmr::memory_resource* m_upstream;
};

void testPolymorphic()
{
    std::cout << "Foo: " << sizeof(Foo) << std::endl;
    std::cout << "PolymorphicPtr<Foo>: " << sizeof(PolymorphicPtr<Foo>) << std::endl;
    std::cout << "PolymorphicPtrBlock<Foo>: ";
    std::cout << sizeof(PolymorphicPtrBlock<Foo, std::pmr::polymorphic_allocator<Foo>>) << std::endl;
    std::cout << "SharedPtr<Foo>: " << sizeof(std::shared_ptr<Foo>) << std::endl;
    std::cout << "SharedPtrBlock<Foo>: ";
    std::cout << sizeof(std::_Ref_count_obj_alloc3<Foo, std::pmr::polymorphic_allocator<Foo>>) << std::endl;

    debug_resource resource;

    {
        auto allocator = std::pmr::polymorphic_allocator<Foo>(&resource);
        PolymorphicPtr<Foo> foo = allocatePolymorphic<Bar>(allocator, 42);
    }
}*/

int main()
{
	// Create data
	aoe::DataHolder data;

	// Create game window
	glfwInit();
	std::cout << glfwGetVersionString() << std::endl;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_REFRESH_RATE, 0);
	aoe::common::Window window{ g_width, g_height, "An Other Engine", nullptr, nullptr };
	glfwMakeContextCurrent(window.getNativeHandle());
	glfwSwapInterval(0);
	glEnable(GL_MULTISAMPLE);

	// Create game world
	auto world = createGameWorld(data, window);

	// Init with default map
	//initGameWorldDefaultMap(data, *world);
	initGameWorldGuiMap(data, *world);

	// Run game
	world->start();

	// Destroy game window
	glfwTerminate();
}

/*
void compileData(DataHolder& a_data)
{
	aoe::data::Handle<aoe::ecs::ComponentManager> m_game{ a_data.database, "../game_root.json" };

}
*/