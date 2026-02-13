#include <vob/aoe/window/GlfwWindow.h>

#include <GL/glew.h>

#include <vob/misc/std/ignorable_assert.h>

#ifndef NDEBUG
#include <iostream>
#include <vob/misc/std/enum_traits.h>
#endif


namespace vob::aoewi
{
	GlfwCreateWindowError::GlfwCreateWindowError()
		: std::runtime_error{ "Failed to create GLFW window." }
	{}

	GlewInitializeError::GlewInitializeError()
		: std::runtime_error{ "Failed to initialize GLEW context." }
	{}

	GlfwWindow::GlfwWindow(
		glm::ivec2 a_size
		, char const* const a_title
		, GLFWmonitor* const a_monitor
		, GLFWwindow* const a_share)
	{
		m_nativeHandle = glfwCreateWindow(a_size.x, a_size.y, a_title, a_monitor, a_share);
		if (m_nativeHandle == nullptr)
		{
			throw GlfwCreateWindowError{};
		}

		glfwMakeContextCurrent(m_nativeHandle);
		if (glewInit() != GLEW_OK)
		{
			throw GlewInitializeError{};
		}

		glfwSetWindowUserPointer(m_nativeHandle, this);
		glfwSetKeyCallback(m_nativeHandle, keyEventCallback);
		glfwSetCharCallback(m_nativeHandle, textEventCallback);
		glfwSetCursorPosCallback(m_nativeHandle, mouseMoveEventCallback);
		glfwSetCursorEnterCallback(m_nativeHandle, mouseEnterEventCallback);
		glfwSetMouseButtonCallback(m_nativeHandle, mouseButtonEventCallback);
		glfwSetScrollCallback(m_nativeHandle, mouseScrollEventCallback);

		glfwSetFramebufferSizeCallback(m_nativeHandle, frameBufferSizeCallback);

#ifndef NDEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(debugMessageCallback, this);
		GLuint ids = 0;
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &ids, true);

#endif
	}

	GlfwWindow::~GlfwWindow()
	{
		glfwDestroyWindow(m_nativeHandle);
	}

	glm::ivec2 GlfwWindow::getSize() const
	{
		auto size = glm::ivec2{};
		glfwGetWindowSize(m_nativeHandle, &size.x, &size.y);
		return size;
	}

	void GlfwWindow::swapBuffers()
	{
		glfwSwapBuffers(m_nativeHandle);
	}

	void GlfwWindow::pollEvents()
	{
		m_events.clear();
		glfwPollEvents();
	}

	std::span<WindowEvent const> GlfwWindow::getPolledEvents() const
	{
		return m_events;
	}

	bool GlfwWindow::shouldClose() const
	{
		return glfwWindowShouldClose(m_nativeHandle) == GLFW_TRUE;
	}

	unsigned int GlfwWindow::getDefaultFramebufferId() const
	{
		return 0; // 0 is default framebuffer id, created with any new window
	}

	bool GlfwWindow::isHovered() const
	{
		return glfwGetWindowAttrib(m_nativeHandle, GLFW_HOVERED) == GLFW_TRUE;
	}

	void GlfwWindow::setCursorState(CursorState a_cursorState)
	{
		glfwSetInputMode(m_nativeHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL + static_cast<GLenum>(a_cursorState));
	}

	bool GlfwWindow::isGamepadPresent(int a_gamepadIndex) const
	{
		return glfwJoystickPresent(GLFW_JOYSTICK_1 + a_gamepadIndex) != 0;
	}

	bool GlfwWindow::isGamepadButtonPressed(int a_gamepadIndex, aoein::Gamepad::Button a_button) const
	{
		int joystickButtonCount;
		auto const joystickButtons = glfwGetJoystickButtons(GLFW_JOYSTICK_1 + a_gamepadIndex, &joystickButtonCount);

		if (static_cast<int>(a_button) >= joystickButtonCount)
		{
			return false;
		}

		auto const result = joystickButtons[static_cast<int>(a_button)] == GLFW_PRESS;
#ifndef NDEBUG
		if (result)
		{
			std::cout << *vob::mistd::enum_traits<aoein::Gamepad::Button>::cast(a_button) << std::endl;
		}
#endif

		return result;
	}

	float GlfwWindow::getGamepadAxisValue(int a_gamepadIndex, aoein::Gamepad::Axis a_axis) const
	{
		int joystickAxisCount;
		auto const joystickAxes = glfwGetJoystickAxes(GLFW_JOYSTICK_1 + a_gamepadIndex, &joystickAxisCount);

		if (static_cast<int>(a_axis) >= joystickAxisCount)
		{
			return 0.0f;
		}

		return joystickAxes[static_cast<int>(a_axis)];
	}

	GLFWwindow* GlfwWindow::getNativeHandle() const
	{
		return m_nativeHandle;
	}

	void GlfwWindow::keyEventCallback(
		GLFWwindow* a_nativeHandle
		, GLint a_keyCode
		, GLint a_scanCode
		, GLint a_action
		, GLint a_modifierMask
	)
	{
		auto const window = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(a_nativeHandle));

		constexpr std::array<aoein::Keyboard::Key, GLFW_KEY_MENU + 1 - GLFW_KEY_SPACE> k_keyCodeToKey{
			aoein::Keyboard::Key::Space // 32 GLFW_KEY_SPACE
			, aoein::Keyboard::Key::Unknown // 33
			, aoein::Keyboard::Key::Unknown // 34
			, aoein::Keyboard::Key::Unknown // 35
			, aoein::Keyboard::Key::Unknown // 36
			, aoein::Keyboard::Key::Unknown // 37
			, aoein::Keyboard::Key::Unknown // 38
			, aoein::Keyboard::Key::Quote // 39 GLFW_KEY_APOSTROPHE
			, aoein::Keyboard::Key::Unknown // 40
			, aoein::Keyboard::Key::Unknown // 41
			, aoein::Keyboard::Key::Unknown // 42
			, aoein::Keyboard::Key::Unknown // 43
			, aoein::Keyboard::Key::Comma // 44 GLFW_KEY_COMMA
			, aoein::Keyboard::Key::Subtract // 45 GLFW_KEY_MINUS
			, aoein::Keyboard::Key::Period // 46 GLFW_KEY_PERIOD
			, aoein::Keyboard::Key::Slash // 47 GLFW_KEY_SLASH
			, aoein::Keyboard::Key::Num0 // 48 GLFW_KEY_0
			, aoein::Keyboard::Key::Num1 // 49 GLFW_KEY_1
			, aoein::Keyboard::Key::Num2 // 50 GLFW_KEY_2
			, aoein::Keyboard::Key::Num3 // 51 GLFW_KEY_3
			, aoein::Keyboard::Key::Num4 // 52 GLFW_KEY_4
			, aoein::Keyboard::Key::Num5 // 53 GLFW_KEY_5
			, aoein::Keyboard::Key::Num6 // 54 GLFW_KEY_6
			, aoein::Keyboard::Key::Num7 // 55 GLFW_KEY_7
			, aoein::Keyboard::Key::Num8 // 56 GLFW_KEY_8
			, aoein::Keyboard::Key::Num9 // 57 GLFW_KEY_9
			, aoein::Keyboard::Key::Unknown // 58
			, aoein::Keyboard::Key::Semicolon // 59 GLFW_KEY_SEMICOLON
			, aoein::Keyboard::Key::Unknown // 60
			, aoein::Keyboard::Key::Equal // 61 GLFW_KEY_EQUAL
			, aoein::Keyboard::Key::Unknown // 62
			, aoein::Keyboard::Key::Unknown // 63
			, aoein::Keyboard::Key::Unknown // 64
			, aoein::Keyboard::Key::A // 65 GLFW_KEY_A
			, aoein::Keyboard::Key::B // 66 GLFW_KEY_B
			, aoein::Keyboard::Key::C // 67 GLFW_KEY_C
			, aoein::Keyboard::Key::D // 68 GLFW_KEY_D
			, aoein::Keyboard::Key::E // 69 GLFW_KEY_E
			, aoein::Keyboard::Key::F // 70 GLFW_KEY_F
			, aoein::Keyboard::Key::G // 71 GLFW_KEY_G
			, aoein::Keyboard::Key::H // 72 GLFW_KEY_H
			, aoein::Keyboard::Key::I // 73 GLFW_KEY_I
			, aoein::Keyboard::Key::J // 74 GLFW_KEY_J
			, aoein::Keyboard::Key::K // 75 GLFW_KEY_K
			, aoein::Keyboard::Key::L // 76 GLFW_KEY_L
			, aoein::Keyboard::Key::M // 77 GLFW_KEY_M
			, aoein::Keyboard::Key::N // 78 GLFW_KEY_N
			, aoein::Keyboard::Key::O // 79 GLFW_KEY_O
			, aoein::Keyboard::Key::P // 80 GLFW_KEY_P
			, aoein::Keyboard::Key::Q // 81 GLFW_KEY_Q
			, aoein::Keyboard::Key::R // 82 GLFW_KEY_R
			, aoein::Keyboard::Key::S // 83 GLFW_KEY_S
			, aoein::Keyboard::Key::T // 84 GLFW_KEY_T
			, aoein::Keyboard::Key::U // 85 GLFW_KEY_U
			, aoein::Keyboard::Key::V // 86 GLFW_KEY_V
			, aoein::Keyboard::Key::W // 87 GLFW_KEY_W
			, aoein::Keyboard::Key::X // 88 GLFW_KEY_X
			, aoein::Keyboard::Key::Y // 89 GLFW_KEY_Y
			, aoein::Keyboard::Key::Z // 90 GLFW_KEY_Z
			, aoein::Keyboard::Key::LBracket // 91 GLFW_KEY_LEFT_BRACKET
			, aoein::Keyboard::Key::Backslash // 92 GLFW_KEY_BACKSLASH
			, aoein::Keyboard::Key::RBracket // 93 GLFW_KEY_RIGHT_BRACKET
			, aoein::Keyboard::Key::Unknown // 94
			, aoein::Keyboard::Key::Unknown // 95
			, aoein::Keyboard::Key::GraveAccent // 96 GLFW_KEY_GRAVE_ACCENT
			, aoein::Keyboard::Key::Unknown // 97
			, aoein::Keyboard::Key::Unknown // 98
			, aoein::Keyboard::Key::Unknown // 99
			, aoein::Keyboard::Key::Unknown // 100
			, aoein::Keyboard::Key::Unknown // 101
			, aoein::Keyboard::Key::Unknown // 102
			, aoein::Keyboard::Key::Unknown // 103
			, aoein::Keyboard::Key::Unknown // 104
			, aoein::Keyboard::Key::Unknown // 105
			, aoein::Keyboard::Key::Unknown // 106
			, aoein::Keyboard::Key::Unknown // 107
			, aoein::Keyboard::Key::Unknown // 108
			, aoein::Keyboard::Key::Unknown // 109
			, aoein::Keyboard::Key::Unknown // 110
			, aoein::Keyboard::Key::Unknown // 111
			, aoein::Keyboard::Key::Unknown // 112
			, aoein::Keyboard::Key::Unknown // 113
			, aoein::Keyboard::Key::Unknown // 114
			, aoein::Keyboard::Key::Unknown // 115
			, aoein::Keyboard::Key::Unknown // 116
			, aoein::Keyboard::Key::Unknown // 117
			, aoein::Keyboard::Key::Unknown // 118
			, aoein::Keyboard::Key::Unknown // 119
			, aoein::Keyboard::Key::Unknown // 120
			, aoein::Keyboard::Key::Unknown // 121
			, aoein::Keyboard::Key::Unknown // 122
			, aoein::Keyboard::Key::Unknown // 123
			, aoein::Keyboard::Key::Unknown // 124
			, aoein::Keyboard::Key::Unknown // 125
			, aoein::Keyboard::Key::Unknown // 126
			, aoein::Keyboard::Key::Unknown // 127
			, aoein::Keyboard::Key::Unknown // 128
			, aoein::Keyboard::Key::Unknown // 129
			, aoein::Keyboard::Key::Unknown // 130
			, aoein::Keyboard::Key::Unknown // 131
			, aoein::Keyboard::Key::Unknown // 132
			, aoein::Keyboard::Key::Unknown // 133
			, aoein::Keyboard::Key::Unknown // 134
			, aoein::Keyboard::Key::Unknown // 135
			, aoein::Keyboard::Key::Unknown // 136
			, aoein::Keyboard::Key::Unknown // 137
			, aoein::Keyboard::Key::Unknown // 138
			, aoein::Keyboard::Key::Unknown // 139
			, aoein::Keyboard::Key::Unknown // 140
			, aoein::Keyboard::Key::Unknown // 141
			, aoein::Keyboard::Key::Unknown // 142
			, aoein::Keyboard::Key::Unknown // 143
			, aoein::Keyboard::Key::Unknown // 144
			, aoein::Keyboard::Key::Unknown // 145
			, aoein::Keyboard::Key::Unknown // 146
			, aoein::Keyboard::Key::Unknown // 147
			, aoein::Keyboard::Key::Unknown // 148
			, aoein::Keyboard::Key::Unknown // 149
			, aoein::Keyboard::Key::Unknown // 150
			, aoein::Keyboard::Key::Unknown // 151
			, aoein::Keyboard::Key::Unknown // 152
			, aoein::Keyboard::Key::Unknown // 153
			, aoein::Keyboard::Key::Unknown // 154
			, aoein::Keyboard::Key::Unknown // 155
			, aoein::Keyboard::Key::Unknown // 156
			, aoein::Keyboard::Key::Unknown // 157
			, aoein::Keyboard::Key::Unknown // 158
			, aoein::Keyboard::Key::Unknown // 159
			, aoein::Keyboard::Key::Unknown // 160
			, aoein::Keyboard::Key::Unknown // 161 GLFW_KEY_WORLD_1
			, aoein::Keyboard::Key::Unknown // 162 GLFW_KEY_WORLD_2
			, aoein::Keyboard::Key::Unknown // 163
			, aoein::Keyboard::Key::Unknown // 164
			, aoein::Keyboard::Key::Unknown // 165
			, aoein::Keyboard::Key::Unknown // 166
			, aoein::Keyboard::Key::Unknown // 167
			, aoein::Keyboard::Key::Unknown // 168
			, aoein::Keyboard::Key::Unknown // 169
			, aoein::Keyboard::Key::Unknown // 170
			, aoein::Keyboard::Key::Unknown // 171
			, aoein::Keyboard::Key::Unknown // 172
			, aoein::Keyboard::Key::Unknown // 173
			, aoein::Keyboard::Key::Unknown // 174
			, aoein::Keyboard::Key::Unknown // 175
			, aoein::Keyboard::Key::Unknown // 176
			, aoein::Keyboard::Key::Unknown // 177
			, aoein::Keyboard::Key::Unknown // 178
			, aoein::Keyboard::Key::Unknown // 179
			, aoein::Keyboard::Key::Unknown // 180
			, aoein::Keyboard::Key::Unknown // 181
			, aoein::Keyboard::Key::Unknown // 182
			, aoein::Keyboard::Key::Unknown // 183
			, aoein::Keyboard::Key::Unknown // 184
			, aoein::Keyboard::Key::Unknown // 185
			, aoein::Keyboard::Key::Unknown // 186
			, aoein::Keyboard::Key::Unknown // 187
			, aoein::Keyboard::Key::Unknown // 188
			, aoein::Keyboard::Key::Unknown // 189
			, aoein::Keyboard::Key::Unknown // 190
			, aoein::Keyboard::Key::Unknown // 191
			, aoein::Keyboard::Key::Unknown // 192
			, aoein::Keyboard::Key::Unknown // 193
			, aoein::Keyboard::Key::Unknown // 194
			, aoein::Keyboard::Key::Unknown // 195
			, aoein::Keyboard::Key::Unknown // 196
			, aoein::Keyboard::Key::Unknown // 197
			, aoein::Keyboard::Key::Unknown // 198
			, aoein::Keyboard::Key::Unknown // 199
			, aoein::Keyboard::Key::Unknown // 200
			, aoein::Keyboard::Key::Unknown // 201
			, aoein::Keyboard::Key::Unknown // 202
			, aoein::Keyboard::Key::Unknown // 203
			, aoein::Keyboard::Key::Unknown // 204
			, aoein::Keyboard::Key::Unknown // 205
			, aoein::Keyboard::Key::Unknown // 206
			, aoein::Keyboard::Key::Unknown // 207
			, aoein::Keyboard::Key::Unknown // 208
			, aoein::Keyboard::Key::Unknown // 209
			, aoein::Keyboard::Key::Unknown // 210
			, aoein::Keyboard::Key::Unknown // 211
			, aoein::Keyboard::Key::Unknown // 212
			, aoein::Keyboard::Key::Unknown // 213
			, aoein::Keyboard::Key::Unknown // 214
			, aoein::Keyboard::Key::Unknown // 215
			, aoein::Keyboard::Key::Unknown // 216
			, aoein::Keyboard::Key::Unknown // 217
			, aoein::Keyboard::Key::Unknown // 218
			, aoein::Keyboard::Key::Unknown // 219
			, aoein::Keyboard::Key::Unknown // 220
			, aoein::Keyboard::Key::Unknown // 221
			, aoein::Keyboard::Key::Unknown // 222
			, aoein::Keyboard::Key::Unknown // 223
			, aoein::Keyboard::Key::Unknown // 224
			, aoein::Keyboard::Key::Unknown // 225
			, aoein::Keyboard::Key::Unknown // 226
			, aoein::Keyboard::Key::Unknown // 227
			, aoein::Keyboard::Key::Unknown // 228
			, aoein::Keyboard::Key::Unknown // 229
			, aoein::Keyboard::Key::Unknown // 230
			, aoein::Keyboard::Key::Unknown // 231
			, aoein::Keyboard::Key::Unknown // 232
			, aoein::Keyboard::Key::Unknown // 233
			, aoein::Keyboard::Key::Unknown // 234
			, aoein::Keyboard::Key::Unknown // 235
			, aoein::Keyboard::Key::Unknown // 236
			, aoein::Keyboard::Key::Unknown // 237
			, aoein::Keyboard::Key::Unknown // 238
			, aoein::Keyboard::Key::Unknown // 239
			, aoein::Keyboard::Key::Unknown // 240
			, aoein::Keyboard::Key::Unknown // 241
			, aoein::Keyboard::Key::Unknown // 242
			, aoein::Keyboard::Key::Unknown // 243
			, aoein::Keyboard::Key::Unknown // 244
			, aoein::Keyboard::Key::Unknown // 245
			, aoein::Keyboard::Key::Unknown // 246
			, aoein::Keyboard::Key::Unknown // 247
			, aoein::Keyboard::Key::Unknown // 248
			, aoein::Keyboard::Key::Unknown // 249
			, aoein::Keyboard::Key::Unknown // 250
			, aoein::Keyboard::Key::Unknown // 251
			, aoein::Keyboard::Key::Unknown // 252
			, aoein::Keyboard::Key::Unknown // 253
			, aoein::Keyboard::Key::Unknown // 254
			, aoein::Keyboard::Key::Unknown // 255
			, aoein::Keyboard::Key::Escape // 256 GLFW_KEY_ESCAPE
			, aoein::Keyboard::Key::Enter // 257 GLFW_KEY_ENTER
			, aoein::Keyboard::Key::Tab // 258 GLFW_KEY_TAB
			, aoein::Keyboard::Key::Backspace // 259 GLFW_KEY_BACKSPACE
			, aoein::Keyboard::Key::Insert // 260 GLFW_KEY_INSERT
			, aoein::Keyboard::Key::Delete // 261 GLFW_KEY_DELETE
			, aoein::Keyboard::Key::Right // 262 GLFW_KEY_RIGHT
			, aoein::Keyboard::Key::Left // 263 GLFW_KEY_LEFT
			, aoein::Keyboard::Key::Down // 264 GLFW_KEY_DOWN
			, aoein::Keyboard::Key::Up // 265 GLFW_KEY_UP
			, aoein::Keyboard::Key::PageUp // 266 GLFW_KEY_PAGE_UP
			, aoein::Keyboard::Key::PageDown // 267 GLFW_KEY_PAGE_DOWN
			, aoein::Keyboard::Key::Home // 268 GLFW_KEY_HOME
			, aoein::Keyboard::Key::End // 269 GLFW_KEY_END
			, aoein::Keyboard::Key::Unknown // 270
			, aoein::Keyboard::Key::Unknown // 271
			, aoein::Keyboard::Key::Unknown // 272
			, aoein::Keyboard::Key::Unknown // 273
			, aoein::Keyboard::Key::Unknown // 274
			, aoein::Keyboard::Key::Unknown // 275
			, aoein::Keyboard::Key::Unknown // 276
			, aoein::Keyboard::Key::Unknown // 277
			, aoein::Keyboard::Key::Unknown // 278
			, aoein::Keyboard::Key::Unknown // 279
			, aoein::Keyboard::Key::CapsLock // 280 GLFW_KEY_CAPS_LOCK
			, aoein::Keyboard::Key::ScrollLock // 281 GLFW_KEY_SCROLL_LOCK
			, aoein::Keyboard::Key::NumLock // 282 GLFW_KEY_NUM_LOCK
			, aoein::Keyboard::Key::PrintScreen // 283 GLFW_KEY_PRINT_SCREEN
			, aoein::Keyboard::Key::Pause // 284 GLFW_KEY_PAUSE
			, aoein::Keyboard::Key::Unknown // 285
			, aoein::Keyboard::Key::Unknown // 286
			, aoein::Keyboard::Key::Unknown // 287
			, aoein::Keyboard::Key::Unknown // 288
			, aoein::Keyboard::Key::Unknown // 289
			, aoein::Keyboard::Key::F1 // 290 GLFW_KEY_F1
			, aoein::Keyboard::Key::F2 // 291 GLFW_KEY_F2
			, aoein::Keyboard::Key::F3 // 292 GLFW_KEY_F3
			, aoein::Keyboard::Key::F4 // 293 GLFW_KEY_F4
			, aoein::Keyboard::Key::F5 // 294 GLFW_KEY_F5
			, aoein::Keyboard::Key::F6 // 295 GLFW_KEY_F6
			, aoein::Keyboard::Key::F7 // 296 GLFW_KEY_F7
			, aoein::Keyboard::Key::F8 // 297 GLFW_KEY_F8
			, aoein::Keyboard::Key::F9 // 298 GLFW_KEY_F9
			, aoein::Keyboard::Key::F10 // 299 GLFW_KEY_F10
			, aoein::Keyboard::Key::F11 // 300 GLFW_KEY_F11
			, aoein::Keyboard::Key::F12 // 301 GLFW_KEY_F12
			, aoein::Keyboard::Key::F13 // 302 GLFW_KEY_F13
			, aoein::Keyboard::Key::F14 // 303 GLFW_KEY_F14
			, aoein::Keyboard::Key::F15 // 304 GLFW_KEY_F15
			, aoein::Keyboard::Key::F16 // 305 GLFW_KEY_F16
			, aoein::Keyboard::Key::F17 // 306 GLFW_KEY_F17
			, aoein::Keyboard::Key::F18 // 307 GLFW_KEY_F18
			, aoein::Keyboard::Key::F19 // 308 GLFW_KEY_F19
			, aoein::Keyboard::Key::F20 // 309 GLFW_KEY_F20
			, aoein::Keyboard::Key::F21 // 310 GLFW_KEY_F21
			, aoein::Keyboard::Key::F22 // 311 GLFW_KEY_F22
			, aoein::Keyboard::Key::F23 // 312 GLFW_KEY_F23
			, aoein::Keyboard::Key::F24 // 313 GLFW_KEY_F24
			, aoein::Keyboard::Key::F25 // 314 GLFW_KEY_F25
			, aoein::Keyboard::Key::Unknown // 315
			, aoein::Keyboard::Key::Unknown // 316
			, aoein::Keyboard::Key::Unknown // 317
			, aoein::Keyboard::Key::Unknown // 318
			, aoein::Keyboard::Key::Unknown // 319
			, aoein::Keyboard::Key::Numpad0 // 320 GLFW_KEY_KP_0
			, aoein::Keyboard::Key::Numpad1 // 321 GLFW_KEY_KP_1
			, aoein::Keyboard::Key::Numpad2 // 322 GLFW_KEY_KP_2
			, aoein::Keyboard::Key::Numpad3 // 323 GLFW_KEY_KP_3
			, aoein::Keyboard::Key::Numpad4 // 324 GLFW_KEY_KP_4
			, aoein::Keyboard::Key::Numpad5 // 325 GLFW_KEY_KP_5
			, aoein::Keyboard::Key::Numpad6 // 326 GLFW_KEY_KP_6
			, aoein::Keyboard::Key::Numpad7 // 327 GLFW_KEY_KP_7
			, aoein::Keyboard::Key::Numpad8 // 328 GLFW_KEY_KP_8
			, aoein::Keyboard::Key::Numpad9 // 329 GLFW_KEY_KP_9
			, aoein::Keyboard::Key::NumpadDecimal // 330 GLFW_KEY_KP_DECIMAL
			, aoein::Keyboard::Key::NumpadDivide // 331 GLFW_KEY_KP_DIVIDE
			, aoein::Keyboard::Key::NumpadMultiply // 332 GLFW_KEY_KP_MULTIPLY
			, aoein::Keyboard::Key::NumpadSubstract // 333 GLFW_KEY_KP_SUBTRACT
			, aoein::Keyboard::Key::NumpadAdd // 334 GLFW_KEY_KP_ADD
			, aoein::Keyboard::Key::NumpadEnter // 335 GLFW_KEY_KP_ENTER
			, aoein::Keyboard::Key::NumpadEqual // 336 GLFW_KEY_KP_EQUAL
			, aoein::Keyboard::Key::Unknown // 337
			, aoein::Keyboard::Key::Unknown // 338
			, aoein::Keyboard::Key::Unknown // 339
			, aoein::Keyboard::Key::LShift // 340 GLFW_KEY_LEFT_SHIFT
			, aoein::Keyboard::Key::LControl // 341 GLFW_KEY_LEFT_CONTROL
			, aoein::Keyboard::Key::LAlt // 342 GLFW_KEY_LEFT_ALT
			, aoein::Keyboard::Key::LSystem // 343 GLFW_KEY_LEFT_SUPER
			, aoein::Keyboard::Key::RShift // 344 GLFW_KEY_RIGHT_SHIFT
			, aoein::Keyboard::Key::RControl // 345 GLFW_KEY_RIGHT_CONTROL
			, aoein::Keyboard::Key::RAlt // 346 GLFW_KEY_RIGHT_ALT
			, aoein::Keyboard::Key::RSystem // 347 GLFW_KEY_RIGHT_SUPPER
			, aoein::Keyboard::Key::Menu // 348 GLFW_KEY_MENU
		};
		if (a_keyCode - GLFW_KEY_SPACE < 0 || a_keyCode - GLFW_KEY_SPACE >= k_keyCodeToKey.size())
		{
			// ignorable_assert(false && "Key code not implemented.");
			return;
		}

		auto const key = k_keyCodeToKey[a_keyCode - GLFW_KEY_SPACE];
		if (key == aoein::Keyboard::Key::Unknown)
		{
			ignorable_assert(false && "Key code not implemented.");
			return;
		}


		window->pushEvent(KeyEvent{
			key,
			static_cast<std::uint32_t>(a_scanCode),
			static_cast<KeyEvent::Action>(a_action),
			KeyboardModifierMask{ static_cast<std::uint8_t>(a_modifierMask) } });
	}

	void GlfwWindow::textEventCallback(
		GLFWwindow* a_nativeHandle
		, GLuint a_unicode
	)
	{
		auto const window = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(a_nativeHandle));
		window->pushEvent(TextEvent{ a_unicode });
	}

	void GlfwWindow::mouseMoveEventCallback(
		GLFWwindow* a_nativeHandle
		, GLdouble a_x
		, GLdouble a_y
	)
	{
		auto const window = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(a_nativeHandle));
		window->pushEvent(MouseMoveEvent{ { a_x, a_y } });
	}

	void GlfwWindow::mouseEnterEventCallback(
		GLFWwindow* a_nativeHandle
		, GLint a_entered
	)
	{
		auto const window = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(a_nativeHandle));
		window->pushEvent(MouseHoverEvent{ a_entered == GL_TRUE });
	}

	void GlfwWindow::mouseButtonEventCallback(
		GLFWwindow* a_nativeHandle
		, GLint a_button
		, GLint a_action
		, GLint a_modifierMask
	)
	{
		auto const window = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(a_nativeHandle));
		window->pushEvent(MouseButtonEvent{
			static_cast<aoein::Mouse::Button>(a_button)
			, a_action == GLFW_PRESS
			, KeyboardModifierMask{ static_cast<std::uint8_t>(a_modifierMask) }
			});

	}

	void GlfwWindow::mouseScrollEventCallback(
		GLFWwindow* a_nativeHandle
		, GLdouble a_x
		, GLdouble a_y
	)
	{
		auto const window = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(a_nativeHandle));
		window->pushEvent(MouseScrollEvent{ { a_x, a_y } });
	}

	void GlfwWindow::frameBufferSizeCallback(
		GLFWwindow* a_nativeHandle
		, GLint a_width
		, GLint a_height
	)
	{
	}

#ifndef NDEBUG
	void GlfwWindow::debugMessageCallback(
		GLenum a_source
		, GLenum a_type
		, GLuint a_id
		, GLenum a_severity
		, GLsizei a_length
		, GLchar const* a_message
		, void const* a_userParam)
	{
		// ignore non-significant messages
		if (a_type == GL_DEBUG_TYPE_OTHER)
		{
			return;
		}

		switch (a_type)
		{
		case GL_DEBUG_TYPE_ERROR:
			std::cerr << "Error";
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			std::cerr << "Deprecated Behaviour";
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			std::cerr << "Undefined Behaviour";
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
			std::cerr << "Portability";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			std::cerr << "Performance";
			break;
		case GL_DEBUG_TYPE_MARKER:
			std::cerr << "Marker";
			break;
		case GL_DEBUG_TYPE_PUSH_GROUP:
			std::cerr << "Push Group";
			break;
		case GL_DEBUG_TYPE_POP_GROUP:
			std::cerr << "Pop Group";
			break;
		case GL_DEBUG_TYPE_OTHER:
			std::cerr << "Other";
			break;
		default:
			std::cerr << "?";
			break;
		}
		std::cerr << "][";
		switch (a_severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:
			std::cerr << "HIGH";
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			std::cerr << "MEDIUM";
			break;
		case GL_DEBUG_SEVERITY_LOW:
			std::cerr << "LOW";
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			std::cerr << "NOTIFICATION";
			break;
		default:
			std::cerr << "?";
			break;
		}
		std::cerr << "] ";
		switch (a_source)
		{
		case GL_DEBUG_SOURCE_API:
			std::cerr << "API";
			break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			std::cerr << "Window System";
			break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
			std::cerr << "Shader Compiler";
			break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:
			std::cerr << "Third Party";
			break;
		case GL_DEBUG_SOURCE_APPLICATION:
			std::cerr << "Application";
			break;
		case GL_DEBUG_SOURCE_OTHER:
			std::cerr << "Other";
			break;
		default:
			std::cerr << "?";
			break;
		}
		std::cerr << " (" << a_id << "): " << a_message << std::endl;

		ignorable_assert(a_type != GL_DEBUG_TYPE_ERROR);
	}
#endif

	void GlfwWindow::pushEvent(WindowEvent aEvent)
	{
		m_events.push_back(aEvent);
	}
}
