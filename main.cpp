
#include <vob/misc/std/vector_map.h>

// This include must come before GL/glew.h and GLFW/glfw3.h to avoid some warning.
#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#pragma comment(lib, "winmm.lib")
#endif

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include "DataHolder.h"

#include <vob/aoe/actor/simple_actor_system.h>

#include <vob/aoe/engine/world.h>

#include <vob/aoe/engine/game.h>
#include <vob/aoe/engine/MultiWorld.h>
#include <vob/aoe/engine/EcsWorld.h>


#include <vob/misc/hash/string_id_literals.h>
#include <vob/misc/physics/measure_literals.h>
#include <vob/misc/visitor/is_visitable.h>

#include <regex>
#include <memory_resource>
#include <filesystem>
#include <fstream>
#include <utility>

using namespace vob;
using namespace misph::literals;
using namespace mishs::literals;

const std::uint32_t g_width = static_cast<uint32_t>(2560u / 1.05);
const std::uint32_t g_height = static_cast<uint32_t>(1440u / 1.05);

#include <filesystem>
#include <unordered_map>

void glfwErrorCallback(int code, const char* description)
{
	__debugbreak();
}

#include "DefaultWorld.h"
#include <vob/aoe/window/GlfwWindow.h>
#include <vob/aoe/rendering/ImGuiUtils.h>

#include <tracy/Tracy.hpp>

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
	std::ifstream gamecontrollerdb("data/gamecontrollerdb.txt");
	gamecontrollerdb.seekg(0, std::ios::end);
	std::string gamepadMappings(gamecontrollerdb.tellg(), '0');
	gamecontrollerdb.seekg(0, std::ios::beg);
	gamecontrollerdb.read(gamepadMappings.data(), gamepadMappings.size());
	auto const result = glfwUpdateGamepadMappings(gamepadMappings.data());
	if (result != GLFW_TRUE)
	{
		return EXIT_FAILURE;
	}

	std::cout << glfwGetVersionString() << std::endl;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_REFRESH_RATE, 0);

	// ver 2.0
	{
		// Init window
		aoewi::GlfwWindow window{ glm::ivec2{ g_width, g_height }, "An Other Engine v2" };
		glfwMakeContextCurrent(window.getNativeHandle());
		glfwSwapInterval(0);
		glEnable(GL_MULTISAMPLE);
		std::cout << glGetString(GL_VERSION) << std::endl;

		// Init ImGui
		aoegl::initializeImGui(window);

#if defined(_WIN32)
		timeBeginPeriod(1);
#endif

		aoeng::Game myGame;
		std::shared_ptr<aoeng::IWorld> world = createDefaultWorld(window);
		myGame.run(world);

#if defined(_WIN32)
		timeEndPeriod(1);
#endif

		// Finalize ImGui
		aoegl::terminateImGui();
	}

	// Destroy game window
	glfwTerminate();
}

/* ACTIONS
*		Use cases		|	move	|	time	|	cancel	|	partial	|	resume	|	count	|
* ----------------------+-----------+-----------+-----------+-----------+-----------+-----------+
*	plant carrots		|	no		|	yes		|	no		|	no		|	no		|	1		|
*	collect carrots		|	no		|	yes		|	no		|	no		|	no		|	1		|
*	collect apples		|	yes		|	yes		|	no		|	no		|	no		|	1		|
*	simple switch		|	yes		|	no		|	no		|	no		|	no		|	inf		|
*	complex switch		|	no		|	yes		|	yes		|	maybe?	|	maybe?	|	1|inf	|
*	pump water			|	a bit	|	yes		|	yes		|	yes		|	yes		|	inf?	|
*	eat plate			|	a bit	|	no		|	no		|	yes		|	sort of	|	n		|
*	grab in hands		|	maybe?	|	maybe?	|	yes		|	no		|	no		|	1		|
*	mine ore			|	no		|	yes		|	yes		|	yes		|	yes		|	n		|
* 
* Interactable
*	- interactions[]	
*		- type
* 
* Controllers:
*	- Player
*	- Network
*	- AI
* 
* Interactors:
*	- Humanoid
*		- head | face | neck
*		- (l | r) + (shoulder arm | forearm | hand | (finger + [0-4]))
*		- chest | abdominal | back
*		- (l | r) thigh | knee | leg | ankle | foot
*	- Quadruped
*		- (f | r) + (l | r) leg
* 
* 
* What:
*	- something you can do with in hand
*	- something you can do with in world
*	- something you can put on body
* 
* Things made for human interaction:
*	- button
*	- switch
*	- water pump
* 
* Things made to be grabbed:
*	- eat plate
*	- grab object in hands
* 
* Things from nature:
*	- recolt plants
*	- recolt fruit
*	- mine ore
* 
* ------------------------------------------------------------
* 
* Interaction Component (on Humanoid / Quadruped):
*	- list of in-range interactions
*	- register to Interactable World Component
* 
* Interactable Shape Component / World Component / System:
*	- computes overlap of ghost shapes
* 
* Plant Component / System:
*	- register to Interactable World Component
*	- when overlap detected, register interaction to list of in-range interactions
* 
* Interuptor Component
* 
* Player (Humanoid?) Controller (TBD):
*	- offers promp when something in list of in-range interaction & valid
* 
* ---------------------------------------------------
* 
* Activate button / switch : -> change state
* Recolt plant / fruit : -> change state + modify inventory
* Pump water : -> change state + modify inventory
* Eat plate : -> change state + modify stats
* Mine ore : -> change state + modify inventory
* 
* ---------------------------------------------------
* Interaction Entity:
*	- transform component
*	- interaction component
*		- can interact (
*		- is interacting
*		- interactor
*		- system (to make sure it's not used by 2 systems at once)
*	- whatever needed for logic
* 
* Interactor Entity:
*	- interactor component
*		- interaction list
* 
* Interactor Pawn:
*	
* 
*/

/* SPAWN
* - value 
*	+ can modify easy to spawn with custom init
*	- must copy from data (and copy is dropped once really spawned_)
* - reference
*	+ not necessary to copy
*	- if need to copy, how?
*	- not clear ref will be kept?
*	- what if owner destroyed?
* - raw pointer
*	+ not necessary to copy
*	+ clearer ref
*	- can be null
*	- what if owner destroyed?
* - weak_ptr
*	+ not necessary to copy
*	- need to allocate if custom init
*	- can be null
*	- what if owner destroyed?
* - shared_ptr
*	+ not necessary to copy
*	- need to allocate if custom init
*	- can be null
* - unique_ptr
*	+ not necessary to copy
*	- need to allocate allways?
*/


/* DRIVING MECHANICS
*
* - instant grip loss button
* - down force change button
*/