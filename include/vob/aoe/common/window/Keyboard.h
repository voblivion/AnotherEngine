#pragma once

#include <array>

#include <vob/aoe/common/window/Switch.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace vob::aoe::common
{
	struct Keyboard
	{
		enum Key
		{
			Unknown = -1
			, A = 0
			, B
			, C
			, D
			, E
			, F
			, G
			, H
			, I
			, J
			, K
			, L
			, M
			, N
			, O
			, P
			, Q
			, R
			, S
			, T
			, U
			, V
			, W
			, X
			, Y
			, Z
			, Num0
			, Num1
			, Num2
			, Num3
			, Num4
			, Num5
			, Num6
			, Num7
			, Num8
			, Num9
			, Escape
			, LControl
			, LShift
			, LAlt
			, LSystem
			, RControl
			, RShift
			, RAlt
			, RSystem
			, Menu
			, LBracket
			, RBracket
			, Semicolon
			, Comma
			, Period
			, Quote
			, Slash
			, Backslash
			, Tilde
			, Equal
			, Hyphen
			, Space
			, Enter
			, Backspace
			, Tab
			, PageUp
			, PageDown
			, End
			, Home
			, Insert
			, Delete
			, Add
			, Subtract
			, Multiply
			, Divide
			, Left
			, Right
			, Up
			, Down
			, Numpad0
			, Numpad1
			, Numpad2
			, Numpad3
			, Numpad4
			, Numpad5
			, Numpad6
			, Numpad7
			, Numpad8
			, Numpad9
			, F1
			, F2
			, F3
			, F4
			, F5
			, F6
			, F7
			, F8
			, F9
			, F10
			, F11
			, F12
			, F13
			, F14
			, F15
			, Pause
			, Count
		};

		std::array<Switch, static_cast<std::underlying_type_t<Key>>(Key::Count)> m_keys;
	};

	inline Keyboard::Key toKey(int a_glfwKeyId)
	{
		constexpr std::array<Keyboard::Key, GLFW_KEY_MENU + 1 - GLFW_KEY_SPACE> s_source{
			Keyboard::Key::Space // 32 GLFW_KEY_SPACE
			, Keyboard::Key::Unknown // 33
			, Keyboard::Key::Unknown // 34
			, Keyboard::Key::Unknown // 35
			, Keyboard::Key::Unknown // 36
			, Keyboard::Key::Unknown // 37
			, Keyboard::Key::Unknown // 38
			, Keyboard::Key::Quote // 39 GLFW_KEY_APOSTROPHE
			, Keyboard::Key::Unknown // 40
			, Keyboard::Key::Unknown // 41
			, Keyboard::Key::Unknown // 42
			, Keyboard::Key::Unknown // 43
			, Keyboard::Key::Comma // 44 GLFW_KEY_COMMA
			, Keyboard::Key::Subtract // 45 GLFW_KEY_MINUS
			, Keyboard::Key::Period // 46 GLFW_KEY_PERIOD
			, Keyboard::Key::Slash // 47 GLFW_KEY_SLASH
			, Keyboard::Key::Num0 // 48 GLFW_KEY_0
			, Keyboard::Key::Num1 // 49 GLFW_KEY_1
			, Keyboard::Key::Num2 // 50 GLFW_KEY_2
			, Keyboard::Key::Num3 // 51 GLFW_KEY_3
			, Keyboard::Key::Num4 // 52 GLFW_KEY_4
			, Keyboard::Key::Num5 // 53 GLFW_KEY_5
			, Keyboard::Key::Num6 // 54 GLFW_KEY_6
			, Keyboard::Key::Num7 // 55 GLFW_KEY_7
			, Keyboard::Key::Num8 // 56 GLFW_KEY_8
			, Keyboard::Key::Num9 // 57 GLFW_KEY_9
			, Keyboard::Key::Unknown // 58
			, Keyboard::Key::Semicolon // 59 GLFW_KEY_SEMICOLON
			, Keyboard::Key::Unknown // 60
			, Keyboard::Key::Equal // 61 GLFW_KEY_EQUAL
			, Keyboard::Key::Unknown // 62
			, Keyboard::Key::Unknown // 63
			, Keyboard::Key::Unknown // 64
			, Keyboard::Key::A // 65 GLFW_KEY_A
			, Keyboard::Key::B // 66 GLFW_KEY_B
			, Keyboard::Key::C // 67 GLFW_KEY_C
			, Keyboard::Key::D // 68 GLFW_KEY_D
			, Keyboard::Key::E // 69 GLFW_KEY_E
			, Keyboard::Key::F // 70 GLFW_KEY_F
			, Keyboard::Key::G // 71 GLFW_KEY_G
			, Keyboard::Key::H // 72 GLFW_KEY_H
			, Keyboard::Key::I // 73 GLFW_KEY_I
			, Keyboard::Key::J // 74 GLFW_KEY_J
			, Keyboard::Key::K // 75 GLFW_KEY_K
			, Keyboard::Key::L // 76 GLFW_KEY_L
			, Keyboard::Key::M // 77 GLFW_KEY_M
			, Keyboard::Key::N // 78 GLFW_KEY_N
			, Keyboard::Key::O // 79 GLFW_KEY_O
			, Keyboard::Key::P // 80 GLFW_KEY_P
			, Keyboard::Key::Q // 81 GLFW_KEY_Q
			, Keyboard::Key::R // 82 GLFW_KEY_R
			, Keyboard::Key::S // 83 GLFW_KEY_S
			, Keyboard::Key::T // 84 GLFW_KEY_T
			, Keyboard::Key::U // 85 GLFW_KEY_U
			, Keyboard::Key::V // 86 GLFW_KEY_V
			, Keyboard::Key::W // 87 GLFW_KEY_W
			, Keyboard::Key::X // 88 GLFW_KEY_X
			, Keyboard::Key::Y // 89 GLFW_KEY_Y
			, Keyboard::Key::Z // 90 GLFW_KEY_Z
			, Keyboard::Key::LBracket // 91 GLFW_KEY_LEFT_BRACKET
			, Keyboard::Key::Backslash // 92 GLFW_KEY_BACKSLASH
			, Keyboard::Key::RBracket // 93 GLFW_KEY_RIGHT_BRACKET
			, Keyboard::Key::Unknown // 94
			, Keyboard::Key::Unknown // 95
			, Keyboard::Key::Unknown // 96 GLFW_KEY_GRAVE_ACCENT
			, Keyboard::Key::Unknown // 97
			, Keyboard::Key::Unknown // 98
			, Keyboard::Key::Unknown // 99
			, Keyboard::Key::Unknown // 100
			, Keyboard::Key::Unknown // 101
			, Keyboard::Key::Unknown // 102
			, Keyboard::Key::Unknown // 103
			, Keyboard::Key::Unknown // 104
			, Keyboard::Key::Unknown // 105
			, Keyboard::Key::Unknown // 106
			, Keyboard::Key::Unknown // 107
			, Keyboard::Key::Unknown // 108
			, Keyboard::Key::Unknown // 109
			, Keyboard::Key::Unknown // 110
			, Keyboard::Key::Unknown // 111
			, Keyboard::Key::Unknown // 112
			, Keyboard::Key::Unknown // 113
			, Keyboard::Key::Unknown // 114
			, Keyboard::Key::Unknown // 115
			, Keyboard::Key::Unknown // 116
			, Keyboard::Key::Unknown // 117
			, Keyboard::Key::Unknown // 118
			, Keyboard::Key::Unknown // 119
			, Keyboard::Key::Unknown // 120
			, Keyboard::Key::Unknown // 121
			, Keyboard::Key::Unknown // 122
			, Keyboard::Key::Unknown // 123
			, Keyboard::Key::Unknown // 124
			, Keyboard::Key::Unknown // 125
			, Keyboard::Key::Unknown // 126
			, Keyboard::Key::Unknown // 127
			, Keyboard::Key::Unknown // 128
			, Keyboard::Key::Unknown // 129
			, Keyboard::Key::Unknown // 130
			, Keyboard::Key::Unknown // 131
			, Keyboard::Key::Unknown // 132
			, Keyboard::Key::Unknown // 133
			, Keyboard::Key::Unknown // 134
			, Keyboard::Key::Unknown // 135
			, Keyboard::Key::Unknown // 136
			, Keyboard::Key::Unknown // 137
			, Keyboard::Key::Unknown // 138
			, Keyboard::Key::Unknown // 139
			, Keyboard::Key::Unknown // 140
			, Keyboard::Key::Unknown // 141
			, Keyboard::Key::Unknown // 142
			, Keyboard::Key::Unknown // 143
			, Keyboard::Key::Unknown // 144
			, Keyboard::Key::Unknown // 145
			, Keyboard::Key::Unknown // 146
			, Keyboard::Key::Unknown // 147
			, Keyboard::Key::Unknown // 148
			, Keyboard::Key::Unknown // 149
			, Keyboard::Key::Unknown // 150
			, Keyboard::Key::Unknown // 151
			, Keyboard::Key::Unknown // 152
			, Keyboard::Key::Unknown // 153
			, Keyboard::Key::Unknown // 154
			, Keyboard::Key::Unknown // 155
			, Keyboard::Key::Unknown // 156
			, Keyboard::Key::Unknown // 157
			, Keyboard::Key::Unknown // 158
			, Keyboard::Key::Unknown // 159
			, Keyboard::Key::Unknown // 160
			, Keyboard::Key::Unknown // 161 GLFW_KEY_WORLD_1
			, Keyboard::Key::Unknown // 162 GLFW_KEY_WORLD_2
			, Keyboard::Key::Unknown // 163
			, Keyboard::Key::Unknown // 164
			, Keyboard::Key::Unknown // 165
			, Keyboard::Key::Unknown // 166
			, Keyboard::Key::Unknown // 167
			, Keyboard::Key::Unknown // 168
			, Keyboard::Key::Unknown // 169
			, Keyboard::Key::Unknown // 170
			, Keyboard::Key::Unknown // 171
			, Keyboard::Key::Unknown // 172
			, Keyboard::Key::Unknown // 173
			, Keyboard::Key::Unknown // 174
			, Keyboard::Key::Unknown // 175
			, Keyboard::Key::Unknown // 176
			, Keyboard::Key::Unknown // 177
			, Keyboard::Key::Unknown // 178
			, Keyboard::Key::Unknown // 179
			, Keyboard::Key::Unknown // 180
			, Keyboard::Key::Unknown // 181
			, Keyboard::Key::Unknown // 182
			, Keyboard::Key::Unknown // 183
			, Keyboard::Key::Unknown // 184
			, Keyboard::Key::Unknown // 185
			, Keyboard::Key::Unknown // 186
			, Keyboard::Key::Unknown // 187
			, Keyboard::Key::Unknown // 188
			, Keyboard::Key::Unknown // 190
			, Keyboard::Key::Unknown // 191
			, Keyboard::Key::Unknown // 192
			, Keyboard::Key::Unknown // 193
			, Keyboard::Key::Unknown // 194
			, Keyboard::Key::Unknown // 195
			, Keyboard::Key::Unknown // 196
			, Keyboard::Key::Unknown // 197
			, Keyboard::Key::Unknown // 198
			, Keyboard::Key::Unknown // 199
			, Keyboard::Key::Unknown // 200
			, Keyboard::Key::Unknown // 201
			, Keyboard::Key::Unknown // 202
			, Keyboard::Key::Unknown // 203
			, Keyboard::Key::Unknown // 204
			, Keyboard::Key::Unknown // 205
			, Keyboard::Key::Unknown // 206
			, Keyboard::Key::Unknown // 207
			, Keyboard::Key::Unknown // 208
			, Keyboard::Key::Unknown // 209
			, Keyboard::Key::Unknown // 210
			, Keyboard::Key::Unknown // 211
			, Keyboard::Key::Unknown // 212
			, Keyboard::Key::Unknown // 213
			, Keyboard::Key::Unknown // 214
			, Keyboard::Key::Unknown // 215
			, Keyboard::Key::Unknown // 216
			, Keyboard::Key::Unknown // 217
			, Keyboard::Key::Unknown // 218
			, Keyboard::Key::Unknown // 219
			, Keyboard::Key::Unknown // 220
			, Keyboard::Key::Unknown // 221
			, Keyboard::Key::Unknown // 222
			, Keyboard::Key::Unknown // 223
			, Keyboard::Key::Unknown // 224
			, Keyboard::Key::Unknown // 225
			, Keyboard::Key::Unknown // 226
			, Keyboard::Key::Unknown // 227
			, Keyboard::Key::Unknown // 228
			, Keyboard::Key::Unknown // 229
			, Keyboard::Key::Unknown // 230
			, Keyboard::Key::Unknown // 231
			, Keyboard::Key::Unknown // 232
			, Keyboard::Key::Unknown // 233
			, Keyboard::Key::Unknown // 234
			, Keyboard::Key::Unknown // 235
			, Keyboard::Key::Unknown // 236
			, Keyboard::Key::Unknown // 237
			, Keyboard::Key::Unknown // 238
			, Keyboard::Key::Unknown // 239
			, Keyboard::Key::Unknown // 240
			, Keyboard::Key::Unknown // 241
			, Keyboard::Key::Unknown // 242
			, Keyboard::Key::Unknown // 243
			, Keyboard::Key::Unknown // 244
			, Keyboard::Key::Unknown // 245
			, Keyboard::Key::Unknown // 246
			, Keyboard::Key::Unknown // 247
			, Keyboard::Key::Unknown // 248
			, Keyboard::Key::Unknown // 249
			, Keyboard::Key::Unknown // 250
			, Keyboard::Key::Unknown // 251
			, Keyboard::Key::Unknown // 252
			, Keyboard::Key::Unknown // 253
			, Keyboard::Key::Unknown // 254
			, Keyboard::Key::Unknown // 255
			, Keyboard::Key::Escape // 256 GLFW_KEY_ESCAPE
			, Keyboard::Key::Enter // 257 GLFW_KEY_ENTER
			, Keyboard::Key::Tab // 258 GLFW_KEY_TAB
			, Keyboard::Key::Backspace // 259 GLFW_KEY_BACKSPACE
			, Keyboard::Key::Insert // 260 GLFW_KEY_INSERT
			, Keyboard::Key::Delete // 261 GLFW_KEY_DELETE
			, Keyboard::Key::Right // 262 GLFW_KEY_RIGHT
			, Keyboard::Key::Left // 263 GLFW_KEY_LEFT
			, Keyboard::Key::Down // 264 GLFW_KEY_DOWN
			, Keyboard::Key::Up // 265 GLFW_KEY_UP
			, Keyboard::Key::PageUp // 266 GLFW_KEY_PAGE_UP
			, Keyboard::Key::PageDown // 267 GLFW_KEY_PAGE_DOWN
			, Keyboard::Key::Home // 268 GLFW_KEY_HOME
			, Keyboard::Key::End // 269 GLFW_KEY_END
			, Keyboard::Key::Unknown // 270
			, Keyboard::Key::Unknown // 271
			, Keyboard::Key::Unknown // 272
			, Keyboard::Key::Unknown // 273
			, Keyboard::Key::Unknown // 274
			, Keyboard::Key::Unknown // 275
			, Keyboard::Key::Unknown // 276
			, Keyboard::Key::Unknown // 277
			, Keyboard::Key::Unknown // 278
			, Keyboard::Key::Unknown // 279
			, Keyboard::Key::Unknown // 280 GLFW_KEY_CAPS_LOCK
			, Keyboard::Key::Unknown // 281 GLFW_KEY_SCROLL_LOCK
			, Keyboard::Key::Unknown // 282 GLFW_KEY_NUM_LOCK
			, Keyboard::Key::Unknown // 283 GLFW_KEY_PRINT_SCREEN
			, Keyboard::Key::Pause // 284 GLFW_KEY_PAUSE
			, Keyboard::Key::Unknown // 285
			, Keyboard::Key::Unknown // 286
			, Keyboard::Key::Unknown // 287
			, Keyboard::Key::Unknown // 288
			, Keyboard::Key::Unknown // 289
			, Keyboard::Key::F1 // 290 GLFW_KEY_F1
			, Keyboard::Key::F2 // 291 GLFW_KEY_F2
			, Keyboard::Key::F3 // 292 GLFW_KEY_F3
			, Keyboard::Key::F4 // 293 GLFW_KEY_F4
			, Keyboard::Key::F5 // 294 GLFW_KEY_F5
			, Keyboard::Key::F6 // 295 GLFW_KEY_F6
			, Keyboard::Key::F7 // 296 GLFW_KEY_F7
			, Keyboard::Key::F8 // 297 GLFW_KEY_F8
			, Keyboard::Key::F9 // 298 GLFW_KEY_F9
			, Keyboard::Key::F10 // 299 GLFW_KEY_F10
			, Keyboard::Key::F11 // 300 GLFW_KEY_F11
			, Keyboard::Key::F12 // 301 GLFW_KEY_F12
			, Keyboard::Key::F13 // 302 GLFW_KEY_F13
			, Keyboard::Key::F14 // 303 GLFW_KEY_F14
			, Keyboard::Key::F15 // 304 GLFW_KEY_F15
			, Keyboard::Key::Unknown // 305 GLFW_KEY_F16
			, Keyboard::Key::Unknown // 306 GLFW_KEY_F17
			, Keyboard::Key::Unknown // 307 GLFW_KEY_F18
			, Keyboard::Key::Unknown // 308 GLFW_KEY_F19
			, Keyboard::Key::Unknown // 309 GLFW_KEY_F20
			, Keyboard::Key::Unknown // 310 GLFW_KEY_F21
			, Keyboard::Key::Unknown // 311 GLFW_KEY_F22
			, Keyboard::Key::Unknown // 312 GLFW_KEY_F23
			, Keyboard::Key::Unknown // 313 GLFW_KEY_F24
			, Keyboard::Key::Unknown // 314 GLFW_KEY_F25
			, Keyboard::Key::Unknown // 315
			, Keyboard::Key::Unknown // 316
			, Keyboard::Key::Unknown // 317
			, Keyboard::Key::Unknown // 318
			, Keyboard::Key::Unknown // 319
			, Keyboard::Key::Numpad0 // 320 GLFW_KEY_KP_0
			, Keyboard::Key::Numpad1 // 321 GLFW_KEY_KP_1
			, Keyboard::Key::Numpad2 // 322 GLFW_KEY_KP_2
			, Keyboard::Key::Numpad3 // 323 GLFW_KEY_KP_3
			, Keyboard::Key::Numpad4 // 324 GLFW_KEY_KP_4
			, Keyboard::Key::Numpad5 // 325 GLFW_KEY_KP_5
			, Keyboard::Key::Numpad6 // 326 GLFW_KEY_KP_6
			, Keyboard::Key::Numpad7 // 327 GLFW_KEY_KP_7
			, Keyboard::Key::Numpad8 // 328 GLFW_KEY_KP_8
			, Keyboard::Key::Numpad9 // 329 GLFW_KEY_KP_9
			, Keyboard::Key::Unknown // 330 GLFW_KEY_KP_DECIMAL
			, Keyboard::Key::Unknown // 331 GLFW_KEY_KP_DIVIDE
			, Keyboard::Key::Unknown // 332 GLFW_KEY_KP_MULTIPLY
			, Keyboard::Key::Unknown // 333 GLFW_KEY_KP_SUBTRACT
			, Keyboard::Key::Unknown // 334 GLFW_KEY_KP_ADD
			, Keyboard::Key::Unknown // 335 GLFW_KEY_KP_ENTER
			, Keyboard::Key::Unknown // 336 GLFW_KEY_KP_EQUAL
			, Keyboard::Key::Unknown // 337
			, Keyboard::Key::Unknown // 338
			, Keyboard::Key::Unknown // 339
			, Keyboard::Key::LShift // 340 GLFW_KEY_LEFT_SHIFT
			, Keyboard::Key::LControl // 341 GLFW_KEY_LEFT_CONTROL
			, Keyboard::Key::LAlt // 342 GLFW_KEY_LEFT_ALT
			, Keyboard::Key::LSystem // 343 GLFW_KEY_LEFT_SUPER
			, Keyboard::Key::RShift // 344 GLFW_KEY_RIGHT_SHIFT
			, Keyboard::Key::RControl // 345 GLFW_KEY_RIGHT_CONTROL
			, Keyboard::Key::RAlt // 346 GLFW_KEY_RIGHT_ALT
			, Keyboard::Key::RSystem // 347 GLFW_KEY_RIGHT_SUPPER
			, Keyboard::Key::Unknown // 348 GLFW_KEY_MENU
		};

		if (a_glfwKeyId < GLFW_KEY_SPACE || a_glfwKeyId > GLFW_KEY_MENU)
		{
			return Keyboard::Key::Unknown;
		}

		return s_source[a_glfwKeyId - GLFW_KEY_SPACE];
	}
}
