#include <vob/aoe/common/window/WindowInputSystem.h>

#include <vob/aoe/common/window/WorldWindowcomponent.h>
#include <vob/aoe/common/input/WorldInputcomponent.h>
#include <vob/aoe/common/input/KeyboardUtil.h>

#include <vob/aoe/ecs/WorldDataProvider.h>

#include <vob/misc/std/enum_traits.h>
#include <iostream>

using namespace vob;
using namespace aoe::common;

namespace
{

	inline aoein::keyboard::key glfw_to_keyboard_key(int a_glfwKeyId)
	{
		constexpr std::array<aoein::keyboard::key, GLFW_KEY_MENU + 1 - GLFW_KEY_SPACE> s_source{
			aoein::keyboard::key::Space // 32 GLFW_KEY_SPACE
			, aoein::keyboard::key::unknown // 33
			, aoein::keyboard::key::unknown // 34
			, aoein::keyboard::key::unknown // 35
			, aoein::keyboard::key::unknown // 36
			, aoein::keyboard::key::unknown // 37
			, aoein::keyboard::key::unknown // 38
			, aoein::keyboard::key::Quote // 39 GLFW_KEY_APOSTROPHE
			, aoein::keyboard::key::unknown // 40
			, aoein::keyboard::key::unknown // 41
			, aoein::keyboard::key::unknown // 42
			, aoein::keyboard::key::unknown // 43
			, aoein::keyboard::key::Comma // 44 GLFW_KEY_COMMA
			, aoein::keyboard::key::Subtract // 45 GLFW_KEY_MINUS
			, aoein::keyboard::key::Period // 46 GLFW_KEY_PERIOD
			, aoein::keyboard::key::Slash // 47 GLFW_KEY_SLASH
			, aoein::keyboard::key::Num0 // 48 GLFW_KEY_0
			, aoein::keyboard::key::Num1 // 49 GLFW_KEY_1
			, aoein::keyboard::key::Num2 // 50 GLFW_KEY_2
			, aoein::keyboard::key::Num3 // 51 GLFW_KEY_3
			, aoein::keyboard::key::Num4 // 52 GLFW_KEY_4
			, aoein::keyboard::key::Num5 // 53 GLFW_KEY_5
			, aoein::keyboard::key::Num6 // 54 GLFW_KEY_6
			, aoein::keyboard::key::Num7 // 55 GLFW_KEY_7
			, aoein::keyboard::key::Num8 // 56 GLFW_KEY_8
			, aoein::keyboard::key::Num9 // 57 GLFW_KEY_9
			, aoein::keyboard::key::unknown // 58
			, aoein::keyboard::key::Semicolon // 59 GLFW_KEY_SEMICOLON
			, aoein::keyboard::key::unknown // 60
			, aoein::keyboard::key::Equal // 61 GLFW_KEY_EQUAL
			, aoein::keyboard::key::unknown // 62
			, aoein::keyboard::key::unknown // 63
			, aoein::keyboard::key::unknown // 64
			, aoein::keyboard::key::A // 65 GLFW_KEY_A
			, aoein::keyboard::key::B // 66 GLFW_KEY_B
			, aoein::keyboard::key::C // 67 GLFW_KEY_C
			, aoein::keyboard::key::D // 68 GLFW_KEY_D
			, aoein::keyboard::key::E // 69 GLFW_KEY_E
			, aoein::keyboard::key::F // 70 GLFW_KEY_F
			, aoein::keyboard::key::G // 71 GLFW_KEY_G
			, aoein::keyboard::key::H // 72 GLFW_KEY_H
			, aoein::keyboard::key::I // 73 GLFW_KEY_I
			, aoein::keyboard::key::J // 74 GLFW_KEY_J
			, aoein::keyboard::key::K // 75 GLFW_KEY_K
			, aoein::keyboard::key::L // 76 GLFW_KEY_L
			, aoein::keyboard::key::M // 77 GLFW_KEY_M
			, aoein::keyboard::key::N // 78 GLFW_KEY_N
			, aoein::keyboard::key::O // 79 GLFW_KEY_O
			, aoein::keyboard::key::P // 80 GLFW_KEY_P
			, aoein::keyboard::key::Q // 81 GLFW_KEY_Q
			, aoein::keyboard::key::R // 82 GLFW_KEY_R
			, aoein::keyboard::key::S // 83 GLFW_KEY_S
			, aoein::keyboard::key::T // 84 GLFW_KEY_T
			, aoein::keyboard::key::U // 85 GLFW_KEY_U
			, aoein::keyboard::key::V // 86 GLFW_KEY_V
			, aoein::keyboard::key::W // 87 GLFW_KEY_W
			, aoein::keyboard::key::X // 88 GLFW_KEY_X
			, aoein::keyboard::key::Y // 89 GLFW_KEY_Y
			, aoein::keyboard::key::Z // 90 GLFW_KEY_Z
			, aoein::keyboard::key::LBracket // 91 GLFW_KEY_LEFT_BRACKET
			, aoein::keyboard::key::Backslash // 92 GLFW_KEY_BACKSLASH
			, aoein::keyboard::key::RBracket // 93 GLFW_KEY_RIGHT_BRACKET
			, aoein::keyboard::key::unknown // 94
			, aoein::keyboard::key::unknown // 95
			, aoein::keyboard::key::unknown // 96 GLFW_KEY_GRAVE_ACCENT
			, aoein::keyboard::key::unknown // 97
			, aoein::keyboard::key::unknown // 98
			, aoein::keyboard::key::unknown // 99
			, aoein::keyboard::key::unknown // 100
			, aoein::keyboard::key::unknown // 101
			, aoein::keyboard::key::unknown // 102
			, aoein::keyboard::key::unknown // 103
			, aoein::keyboard::key::unknown // 104
			, aoein::keyboard::key::unknown // 105
			, aoein::keyboard::key::unknown // 106
			, aoein::keyboard::key::unknown // 107
			, aoein::keyboard::key::unknown // 108
			, aoein::keyboard::key::unknown // 109
			, aoein::keyboard::key::unknown // 110
			, aoein::keyboard::key::unknown // 111
			, aoein::keyboard::key::unknown // 112
			, aoein::keyboard::key::unknown // 113
			, aoein::keyboard::key::unknown // 114
			, aoein::keyboard::key::unknown // 115
			, aoein::keyboard::key::unknown // 116
			, aoein::keyboard::key::unknown // 117
			, aoein::keyboard::key::unknown // 118
			, aoein::keyboard::key::unknown // 119
			, aoein::keyboard::key::unknown // 120
			, aoein::keyboard::key::unknown // 121
			, aoein::keyboard::key::unknown // 122
			, aoein::keyboard::key::unknown // 123
			, aoein::keyboard::key::unknown // 124
			, aoein::keyboard::key::unknown // 125
			, aoein::keyboard::key::unknown // 126
			, aoein::keyboard::key::unknown // 127
			, aoein::keyboard::key::unknown // 128
			, aoein::keyboard::key::unknown // 129
			, aoein::keyboard::key::unknown // 130
			, aoein::keyboard::key::unknown // 131
			, aoein::keyboard::key::unknown // 132
			, aoein::keyboard::key::unknown // 133
			, aoein::keyboard::key::unknown // 134
			, aoein::keyboard::key::unknown // 135
			, aoein::keyboard::key::unknown // 136
			, aoein::keyboard::key::unknown // 137
			, aoein::keyboard::key::unknown // 138
			, aoein::keyboard::key::unknown // 139
			, aoein::keyboard::key::unknown // 140
			, aoein::keyboard::key::unknown // 141
			, aoein::keyboard::key::unknown // 142
			, aoein::keyboard::key::unknown // 143
			, aoein::keyboard::key::unknown // 144
			, aoein::keyboard::key::unknown // 145
			, aoein::keyboard::key::unknown // 146
			, aoein::keyboard::key::unknown // 147
			, aoein::keyboard::key::unknown // 148
			, aoein::keyboard::key::unknown // 149
			, aoein::keyboard::key::unknown // 150
			, aoein::keyboard::key::unknown // 151
			, aoein::keyboard::key::unknown // 152
			, aoein::keyboard::key::unknown // 153
			, aoein::keyboard::key::unknown // 154
			, aoein::keyboard::key::unknown // 155
			, aoein::keyboard::key::unknown // 156
			, aoein::keyboard::key::unknown // 157
			, aoein::keyboard::key::unknown // 158
			, aoein::keyboard::key::unknown // 159
			, aoein::keyboard::key::unknown // 160
			, aoein::keyboard::key::unknown // 161 GLFW_KEY_WORLD_1
			, aoein::keyboard::key::unknown // 162 GLFW_KEY_WORLD_2
			, aoein::keyboard::key::unknown // 163
			, aoein::keyboard::key::unknown // 164
			, aoein::keyboard::key::unknown // 165
			, aoein::keyboard::key::unknown // 166
			, aoein::keyboard::key::unknown // 167
			, aoein::keyboard::key::unknown // 168
			, aoein::keyboard::key::unknown // 169
			, aoein::keyboard::key::unknown // 170
			, aoein::keyboard::key::unknown // 171
			, aoein::keyboard::key::unknown // 172
			, aoein::keyboard::key::unknown // 173
			, aoein::keyboard::key::unknown // 174
			, aoein::keyboard::key::unknown // 175
			, aoein::keyboard::key::unknown // 176
			, aoein::keyboard::key::unknown // 177
			, aoein::keyboard::key::unknown // 178
			, aoein::keyboard::key::unknown // 179
			, aoein::keyboard::key::unknown // 180
			, aoein::keyboard::key::unknown // 181
			, aoein::keyboard::key::unknown // 182
			, aoein::keyboard::key::unknown // 183
			, aoein::keyboard::key::unknown // 184
			, aoein::keyboard::key::unknown // 185
			, aoein::keyboard::key::unknown // 186
			, aoein::keyboard::key::unknown // 187
			, aoein::keyboard::key::unknown // 188
			, aoein::keyboard::key::unknown // 190
			, aoein::keyboard::key::unknown // 191
			, aoein::keyboard::key::unknown // 192
			, aoein::keyboard::key::unknown // 193
			, aoein::keyboard::key::unknown // 194
			, aoein::keyboard::key::unknown // 195
			, aoein::keyboard::key::unknown // 196
			, aoein::keyboard::key::unknown // 197
			, aoein::keyboard::key::unknown // 198
			, aoein::keyboard::key::unknown // 199
			, aoein::keyboard::key::unknown // 200
			, aoein::keyboard::key::unknown // 201
			, aoein::keyboard::key::unknown // 202
			, aoein::keyboard::key::unknown // 203
			, aoein::keyboard::key::unknown // 204
			, aoein::keyboard::key::unknown // 205
			, aoein::keyboard::key::unknown // 206
			, aoein::keyboard::key::unknown // 207
			, aoein::keyboard::key::unknown // 208
			, aoein::keyboard::key::unknown // 209
			, aoein::keyboard::key::unknown // 210
			, aoein::keyboard::key::unknown // 211
			, aoein::keyboard::key::unknown // 212
			, aoein::keyboard::key::unknown // 213
			, aoein::keyboard::key::unknown // 214
			, aoein::keyboard::key::unknown // 215
			, aoein::keyboard::key::unknown // 216
			, aoein::keyboard::key::unknown // 217
			, aoein::keyboard::key::unknown // 218
			, aoein::keyboard::key::unknown // 219
			, aoein::keyboard::key::unknown // 220
			, aoein::keyboard::key::unknown // 221
			, aoein::keyboard::key::unknown // 222
			, aoein::keyboard::key::unknown // 223
			, aoein::keyboard::key::unknown // 224
			, aoein::keyboard::key::unknown // 225
			, aoein::keyboard::key::unknown // 226
			, aoein::keyboard::key::unknown // 227
			, aoein::keyboard::key::unknown // 228
			, aoein::keyboard::key::unknown // 229
			, aoein::keyboard::key::unknown // 230
			, aoein::keyboard::key::unknown // 231
			, aoein::keyboard::key::unknown // 232
			, aoein::keyboard::key::unknown // 233
			, aoein::keyboard::key::unknown // 234
			, aoein::keyboard::key::unknown // 235
			, aoein::keyboard::key::unknown // 236
			, aoein::keyboard::key::unknown // 237
			, aoein::keyboard::key::unknown // 238
			, aoein::keyboard::key::unknown // 239
			, aoein::keyboard::key::unknown // 240
			, aoein::keyboard::key::unknown // 241
			, aoein::keyboard::key::unknown // 242
			, aoein::keyboard::key::unknown // 243
			, aoein::keyboard::key::unknown // 244
			, aoein::keyboard::key::unknown // 245
			, aoein::keyboard::key::unknown // 246
			, aoein::keyboard::key::unknown // 247
			, aoein::keyboard::key::unknown // 248
			, aoein::keyboard::key::unknown // 249
			, aoein::keyboard::key::unknown // 250
			, aoein::keyboard::key::unknown // 251
			, aoein::keyboard::key::unknown // 252
			, aoein::keyboard::key::unknown // 253
			, aoein::keyboard::key::unknown // 254
			, aoein::keyboard::key::unknown // 255
			, aoein::keyboard::key::Escape // 256 GLFW_KEY_ESCAPE
			, aoein::keyboard::key::Enter // 257 GLFW_KEY_ENTER
			, aoein::keyboard::key::Tab // 258 GLFW_KEY_TAB
			, aoein::keyboard::key::Backspace // 259 GLFW_KEY_BACKSPACE
			, aoein::keyboard::key::Insert // 260 GLFW_KEY_INSERT
			, aoein::keyboard::key::Delete // 261 GLFW_KEY_DELETE
			, aoein::keyboard::key::Right // 262 GLFW_KEY_RIGHT
			, aoein::keyboard::key::Left // 263 GLFW_KEY_LEFT
			, aoein::keyboard::key::Down // 264 GLFW_KEY_DOWN
			, aoein::keyboard::key::Up // 265 GLFW_KEY_UP
			, aoein::keyboard::key::PageUp // 266 GLFW_KEY_PAGE_UP
			, aoein::keyboard::key::PageDown // 267 GLFW_KEY_PAGE_DOWN
			, aoein::keyboard::key::Home // 268 GLFW_KEY_HOME
			, aoein::keyboard::key::End // 269 GLFW_KEY_END
			, aoein::keyboard::key::unknown // 270
			, aoein::keyboard::key::unknown // 271
			, aoein::keyboard::key::unknown // 272
			, aoein::keyboard::key::unknown // 273
			, aoein::keyboard::key::unknown // 274
			, aoein::keyboard::key::unknown // 275
			, aoein::keyboard::key::unknown // 276
			, aoein::keyboard::key::unknown // 277
			, aoein::keyboard::key::unknown // 278
			, aoein::keyboard::key::unknown // 279
			, aoein::keyboard::key::unknown // 280 GLFW_KEY_CAPS_LOCK
			, aoein::keyboard::key::unknown // 281 GLFW_KEY_SCROLL_LOCK
			, aoein::keyboard::key::unknown // 282 GLFW_KEY_NUM_LOCK
			, aoein::keyboard::key::unknown // 283 GLFW_KEY_PRINT_SCREEN
			, aoein::keyboard::key::Pause // 284 GLFW_KEY_PAUSE
			, aoein::keyboard::key::unknown // 285
			, aoein::keyboard::key::unknown // 286
			, aoein::keyboard::key::unknown // 287
			, aoein::keyboard::key::unknown // 288
			, aoein::keyboard::key::unknown // 289
			, aoein::keyboard::key::F1 // 290 GLFW_KEY_F1
			, aoein::keyboard::key::F2 // 291 GLFW_KEY_F2
			, aoein::keyboard::key::F3 // 292 GLFW_KEY_F3
			, aoein::keyboard::key::F4 // 293 GLFW_KEY_F4
			, aoein::keyboard::key::F5 // 294 GLFW_KEY_F5
			, aoein::keyboard::key::F6 // 295 GLFW_KEY_F6
			, aoein::keyboard::key::F7 // 296 GLFW_KEY_F7
			, aoein::keyboard::key::F8 // 297 GLFW_KEY_F8
			, aoein::keyboard::key::F9 // 298 GLFW_KEY_F9
			, aoein::keyboard::key::F10 // 299 GLFW_KEY_F10
			, aoein::keyboard::key::F11 // 300 GLFW_KEY_F11
			, aoein::keyboard::key::F12 // 301 GLFW_KEY_F12
			, aoein::keyboard::key::F13 // 302 GLFW_KEY_F13
			, aoein::keyboard::key::F14 // 303 GLFW_KEY_F14
			, aoein::keyboard::key::F15 // 304 GLFW_KEY_F15
			, aoein::keyboard::key::unknown // 305 GLFW_KEY_F16
			, aoein::keyboard::key::unknown // 306 GLFW_KEY_F17
			, aoein::keyboard::key::unknown // 307 GLFW_KEY_F18
			, aoein::keyboard::key::unknown // 308 GLFW_KEY_F19
			, aoein::keyboard::key::unknown // 309 GLFW_KEY_F20
			, aoein::keyboard::key::unknown // 310 GLFW_KEY_F21
			, aoein::keyboard::key::unknown // 311 GLFW_KEY_F22
			, aoein::keyboard::key::unknown // 312 GLFW_KEY_F23
			, aoein::keyboard::key::unknown // 313 GLFW_KEY_F24
			, aoein::keyboard::key::unknown // 314 GLFW_KEY_F25
			, aoein::keyboard::key::unknown // 315
			, aoein::keyboard::key::unknown // 316
			, aoein::keyboard::key::unknown // 317
			, aoein::keyboard::key::unknown // 318
			, aoein::keyboard::key::unknown // 319
			, aoein::keyboard::key::Numpad0 // 320 GLFW_KEY_KP_0
			, aoein::keyboard::key::Numpad1 // 321 GLFW_KEY_KP_1
			, aoein::keyboard::key::Numpad2 // 322 GLFW_KEY_KP_2
			, aoein::keyboard::key::Numpad3 // 323 GLFW_KEY_KP_3
			, aoein::keyboard::key::Numpad4 // 324 GLFW_KEY_KP_4
			, aoein::keyboard::key::Numpad5 // 325 GLFW_KEY_KP_5
			, aoein::keyboard::key::Numpad6 // 326 GLFW_KEY_KP_6
			, aoein::keyboard::key::Numpad7 // 327 GLFW_KEY_KP_7
			, aoein::keyboard::key::Numpad8 // 328 GLFW_KEY_KP_8
			, aoein::keyboard::key::Numpad9 // 329 GLFW_KEY_KP_9
			, aoein::keyboard::key::unknown // 330 GLFW_KEY_KP_DECIMAL
			, aoein::keyboard::key::unknown // 331 GLFW_KEY_KP_DIVIDE
			, aoein::keyboard::key::unknown // 332 GLFW_KEY_KP_MULTIPLY
			, aoein::keyboard::key::unknown // 333 GLFW_KEY_KP_SUBTRACT
			, aoein::keyboard::key::unknown // 334 GLFW_KEY_KP_ADD
			, aoein::keyboard::key::unknown // 335 GLFW_KEY_KP_ENTER
			, aoein::keyboard::key::unknown // 336 GLFW_KEY_KP_EQUAL
			, aoein::keyboard::key::unknown // 337
			, aoein::keyboard::key::unknown // 338
			, aoein::keyboard::key::unknown // 339
			, aoein::keyboard::key::LShift // 340 GLFW_KEY_LEFT_SHIFT
			, aoein::keyboard::key::LControl // 341 GLFW_KEY_LEFT_CONTROL
			, aoein::keyboard::key::LAlt // 342 GLFW_KEY_LEFT_ALT
			, aoein::keyboard::key::LSystem // 343 GLFW_KEY_LEFT_SUPER
			, aoein::keyboard::key::RShift // 344 GLFW_KEY_RIGHT_SHIFT
			, aoein::keyboard::key::RControl // 345 GLFW_KEY_RIGHT_CONTROL
			, aoein::keyboard::key::RAlt // 346 GLFW_KEY_RIGHT_ALT
			, aoein::keyboard::key::RSystem // 347 GLFW_KEY_RIGHT_SUPPER
			, aoein::keyboard::key::unknown // 348 GLFW_KEY_MENU
		};

		if (a_glfwKeyId < GLFW_KEY_SPACE || a_glfwKeyId > GLFW_KEY_MENU)
		{
			return aoein::keyboard::key::unknown;
		}

		return s_source[static_cast<std::size_t>(a_glfwKeyId) - GLFW_KEY_SPACE];
	}

	inline aoein::gamepad::button glfw_to_gamepad_button(int a_glfwGamepadButtonId)
	{
		if (a_glfwGamepadButtonId < GLFW_GAMEPAD_BUTTON_A || a_glfwGamepadButtonId > GLFW_GAMEPAD_BUTTON_LAST)
		{
			return aoein::gamepad::button::unknown;
		}

		return static_cast<aoein::gamepad::button>(a_glfwGamepadButtonId);
	}

	inline int gamepad_button_to_glfw(aoein::gamepad::button a_button)
	{
		return static_cast<int>(a_button);
	}

	inline aoein::gamepad::axis glfw_to_gamepad_axis(int a_glfwGamepadAxisId)
	{
		if (a_glfwGamepadAxisId < GLFW_GAMEPAD_AXIS_LEFT_X || a_glfwGamepadAxisId > GLFW_GAMEPAD_AXIS_LAST)
		{
			return aoein::gamepad::axis::unknown;
		}

		return static_cast<aoein::gamepad::axis>(a_glfwGamepadAxisId);
	}

	inline int gamepad_axis_to_glfw(aoein::gamepad::axis a_axis)
	{
		return static_cast<int>(a_axis);
	}

	inline void processEvent(
		KeyEvent const& a_keyEvent,
		WorldInputComponent& a_worldInput,
		aoein::physical_inputs_world_component& a_physicalInputsComponent)
	{
		if (a_keyEvent.m_action == KeyEvent::Action::Repeat)
		{
			return;
		}

		{
			auto const key = KeyboardUtil::keyFromGlfw(a_keyEvent.m_keyCode);
			if (key != Keyboard::Key::Unknown)
			{
				auto& keyState = a_worldInput.m_keyboard.m_keys[key];
				keyState.m_changed = true;
				keyState.m_isActive = a_keyEvent.m_action == KeyEvent::Action::Press;
			}
		}
		auto const key = glfw_to_keyboard_key(a_keyEvent.m_keyCode);
		if (key != aoein::keyboard::key::unknown)
		{
			auto& keyState = a_physicalInputsComponent.m_inputs.m_keyboard.m_keys[key];
			keyState.update(a_keyEvent.m_action == KeyEvent::Action::Press);
		}
	}

	inline void processEvent(
		MouseMoveEvent const& a_mouseMoveEvent,
		WorldInputComponent& a_worldInput,
		aoein::physical_inputs_world_component& a_physicalInputsComponent)
	{
		a_worldInput.m_mouse.m_move +=
			glm::vec2{ a_mouseMoveEvent.m_position } - a_worldInput.m_mouse.m_position;
		a_worldInput.m_mouse.m_position = a_mouseMoveEvent.m_position;
	}

	inline void processEvent(
		MouseButtonEvent const& a_mouseButtonEvent,
		WorldInputComponent& a_worldInput,
		aoein::physical_inputs_world_component& a_physicalInputsComponent)
	{
		auto const button = mouseButtonFromGlfw(a_mouseButtonEvent.m_button);
		if (button != Mouse::Button::Unknown)
		{
			auto& buttonState = a_worldInput.m_mouse.m_buttons[button];
			buttonState.m_changed = true;
			buttonState.m_isActive = a_mouseButtonEvent.m_pressed;
		}
	}

	inline void processEvent(
		MouseScrollEvent const& a_mouseScrollEvent,
		WorldInputComponent& a_worldInput,
		aoein::physical_inputs_world_component& a_physicalInputsComponent)
	{
		// TODO
	}

	inline void processEvent(
		TextEvent const& a_textEvent,
		WorldInputComponent& a_worldInput,
		aoein::physical_inputs_world_component& a_physicalInputsComponent)
	{
		// TODO?
	}

	inline void processEvent(
		MouseEnterEvent const& a_mouseEnterEvent,
		WorldInputComponent& a_worldInput,
		aoein::physical_inputs_world_component& a_physicalInputsComponent)
	{}

	inline void resetFrameState(
		WorldInputComponent& a_worldInput,
		aoein::physical_inputs_world_component& a_physicalInputsComponent)
	{
		a_worldInput.m_mouse.m_move = {};

		for (auto& key : a_worldInput.m_keyboard.m_keys)
		{
			key.m_changed = false;
		}

		for (auto& button : a_worldInput.m_mouse.m_buttons)
		{
			button.m_changed = false;
		}

		a_physicalInputsComponent.m_inputs.update();
	}

	inline void processEvents(
		IWindow& a_window,
		WorldInputComponent& a_worldInput,
		aoein::physical_inputs_world_component& a_physicalInputsComponent)
	{
		a_window.pollEvents();
		for (auto const& polledEvent : a_window.getPolledEvents())
		{
			std::visit([&a_worldInput, &a_physicalInputsComponent](auto const& a_event) {
				processEvent(a_event, a_worldInput, a_physicalInputsComponent);
			}, polledEvent);
		}
	}

	inline void updateHoverState(IWindow& a_window, WorldInputComponent& a_worldInput,
		aoein::physical_inputs_world_component& a_physicalInputsComponent)
	{
		auto isActive = a_window.isHovered();
		auto wasActive = a_worldInput.m_mouse.m_hover.m_isActive;
		a_worldInput.m_mouse.m_hover.m_changed = isActive != wasActive;
		a_worldInput.m_mouse.m_hover.m_isActive = isActive;

		// TODO -> new aoein system
	}

	inline void updateGamepad(std::size_t a_gamepadIndex, Gamepad& a_oldGamepad, aoein::gamepad& a_gamepad)
	{
		GLFWgamepadstate state;
		if (!glfwJoystickPresent(static_cast<int>(a_gamepadIndex))
			|| !glfwJoystickIsGamepad(static_cast<int>(a_gamepadIndex))
			|| glfwGetGamepadState(static_cast<int>(a_gamepadIndex), &state) == GLFW_FALSE)
		{
			if (a_gamepad.m_isConnected.is_pressed())
			{
				a_gamepad.m_name = {};
				a_oldGamepad.m_name = {};
			}
			a_gamepad.m_isConnected.update(false);
			a_oldGamepad.m_state.update(false);
			return;
		}

		if (!a_gamepad.m_isConnected)
		{
			a_gamepad.m_name = glfwGetGamepadName(static_cast<int>(a_gamepadIndex));
			a_oldGamepad.m_name = glfwGetGamepadName(static_cast<int>(a_gamepadIndex));
		}
		a_gamepad.m_isConnected.update(true);

		for (auto i = 0; i != a_gamepad.m_buttons.size(); ++i)
		{
			{
				auto button = Gamepad::Button(a_oldGamepad.m_buttons.begin_value + i);
				auto isActive = state.buttons[gamepadButtonToGlfw(button)] == GLFW_PRESS;
				auto& buttonState = a_oldGamepad.m_buttons[i];
				buttonState.m_changed = buttonState.m_isActive != isActive;
				buttonState.m_isActive = isActive;
			}
			auto button = aoein::gamepad::button{ a_gamepad.m_buttons.begin_value + i };
			auto& buttonState = a_gamepad.m_buttons[button];
			buttonState.update(state.buttons[gamepad_button_to_glfw(button)] == GLFW_PRESS);
		}

		for (auto i = 0; i != a_oldGamepad.m_axes.size(); ++i)
		{
			{
				auto axis = Gamepad::Axis{ a_oldGamepad.m_axes.begin_value + i };
				auto& axisState = a_oldGamepad.m_axes[i];
				axisState = state.axes[gamepadAxisToGlfw(axis)];
			}
			auto axis = aoein::gamepad::axis{ a_gamepad.m_axes.begin_value + i };
			auto& axisState = a_gamepad.m_axes[axis];
			axisState.update(state.axes[gamepad_axis_to_glfw(axis)]);
		}
	}

	inline void updateGamepads(
		WorldInputComponent& a_worldInput,
		aoein::physical_inputs_world_component& a_physicalInputsComponent)
	{
		for (auto i = 0; i < a_worldInput.m_gamepads.size(); ++i)
		{
			updateGamepad(i, a_worldInput.m_gamepads[i], a_physicalInputsComponent.m_inputs.m_gamepads[i]);
		}
	}
}

WindowInputSystem::WindowInputSystem(aoecs::WorldDataProvider& a_wdp)
	: m_worldWindowComponent{ *a_wdp.getWorldComponent<WorldWindowComponent>() }
	, m_worldInputComponent{ *a_wdp.getWorldComponent<WorldInputComponent>() }
	, m_physicalInputsComponent{ *a_wdp.getWorldComponent<aoein::physical_inputs_world_component>() }
	, m_stopManager{ a_wdp.getStopManager() }
{}

void WindowInputSystem::update() const
{
	auto& window = m_worldWindowComponent.getWindow();

	resetFrameState(m_worldInputComponent, m_physicalInputsComponent);

	if (window.shouldClose())
	{
		m_stopManager.set_should_stop(true);
	}

	processEvents(window, m_worldInputComponent, m_physicalInputsComponent);

	updateHoverState(window, m_worldInputComponent, m_physicalInputsComponent);

	updateGamepads(m_worldInputComponent, m_physicalInputsComponent);
}
