
#include <vob/misc/std/vector_map.h>
#include <vob/_todo_/random/perlin.h>
#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include "DataHolder.h"

#include <vob/aoe/actor/simple_actor_system.h>



#include <vob/aoe/engine/world.h>

#include <vob/aoe/input/bindings.h>
#include <vob/aoe/input/binding_util.h>
#include <vob/aoe/input/inputs.h>

#include <vob/aoe/terrain/procedural_terrain.h>

#include <vob/aoe/window/glfw_window.h>

#include <vob/misc/hash/string_id_literals.h>
#include <vob/misc/physics/measure_literals.h>
#include <vob/misc/visitor/is_visitable.h>

#include "init_default_map.h"
#include "init_world_components.h"
#include "init_world_systems.h"

#include <regex>
#include <memory_resource>
#include <filesystem>
#include <fstream>
#include <utility>

using namespace vob;
using namespace misph::literals;
using namespace mishs::literals;

const std::uint32_t g_width = 2048u;
const std::uint32_t g_height = 1024u;

#include <filesystem>
#include <unordered_map>

void glfwErrorCallback(int code, const char* description)
{
	__debugbreak();
}

char const* xinputMapping = "78696e70757401000000000000000000,Xbox Controller,platform:Windows,a:b0,b:b1,x:b2,y:b3,leftshoulder:b4,rightshoulder:b5,back:b6,start:b7,guide:b8,leftstick:b9,rightstick:b10,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:a4,righttrigger:a5,dpup:h0.1,dpright:h0.2,dpdown:h0.4,dpleft:h0.8,";

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

	auto q = glfwGetJoystickGUID(0);
	auto r = glfwGetJoystickName(0);

	std::cout << glfwGetVersionString() << std::endl;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_REFRESH_RATE, 0);

	{
		aoewi::glfw_window window{ glm::ivec2{ g_width, g_height }, "An Other Engine" };

		glfwMakeContextCurrent(window.get_native_handle());
		glfwSwapInterval(0);
		glEnable(GL_MULTISAMPLE);

		std::cout << glGetString(GL_VERSION) << std::endl;

		aoeng::world world;
		mismt::pmr::schedule schedule;
		init_world_components(world, data, window);
		init_world_systems(world, schedule);
		init_default_map(world, data);

		// Run game
		world.start(schedule);
	}

	// Destroy game window
	glfwTerminate();
}

/* ACTIONS
*		Use cases		|	move	|	time	|	cancel	|	partial	|	resume	|	count	|
* ----------------------+-----------+-----------+-----------+-----------+-----------+-----------+
*	plant carrots		|	no		|	yes		|	no		|	no		|	no		|	1		|
*	recolt carrots		|	no		|	yes		|	no		|	no		|	no		|	1		|
*	recolt apples		|	yes		|	yes		|	no		|	no		|	no		|	1		|
*	simple switch		|	yes		|	no		|	no		|	no		|	no		|	inf		|
*	complex switch		|	no		|	yes		|	yes		|	maybe?	|	maybe?	|	1|inf	|
*	pump water			|	a bit	|	yes		|	yes		|	yes		|	yes		|	inf?	|
*	eat plate			|	a bit	|	no		|	no		|	yes		|	kinda	|	n		|
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

/* DATA / GRAPHIC OBJECTS
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