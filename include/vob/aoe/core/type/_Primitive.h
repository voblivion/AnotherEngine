#pragma once

#include <cinttypes>
#include <string>

#include <glm/glm.hpp>

#include <vob/misc/hash/string_id.h>


namespace vob::aoe
{
	// C++ primitive types
	using byte = std::byte;

	using u8 = std::uint8_t;
	using u16 = std::uint16_t;
	using u32 = std::uint32_t;
	using u64 = std::uint64_t;

	using i8 = std::int8_t;
	using i16 = std::int16_t;
	using i32 = std::int32_t;
	using i64 = std::int64_t;

	using f32 = float;
	using f64 = double;

	// GLM types
	template <glm::length_t t_length, typename Type>
	using vec = glm::vec<t_length, Type, glm::defaultp>;

	using i32vec1 = vec<1, i32>;
	using i32vec2 = vec<2, i32>;
	using i32vec3 = vec<3, i32>;
	using i32vec4 = vec<4, i32>;

	using ivec1 = i32vec1;
	using ivec2 = i32vec2;
	using ivec3 = i32vec3;
	using ivec4 = i32vec4;

	using i16vec2 = vec<2, i16>;
	using i16vec4 = vec<4, i16>;

	using u32vec1 = vec<1, u32>;
	using u32vec2 = vec<2, u32>;
	using u32vec3 = vec<3, u32>;
	using u32vec4 = vec<4, u32>;

	using uvec1 = u32vec1;
	using uvec2 = u32vec2;
	using uvec3 = u32vec3;
	using uvec4 = u32vec3;

	using f32vec1 = vec<1, f32>;
	using f32vec2 = vec<2, f32>;
	using f32vec3 = vec<3, f32>;
	using f32vec4 = vec<4, f32>;

	using fvec1 = f32vec1;
	using fvec2 = f32vec2;
	using fvec3 = f32vec3;
	using fvec4 = f32vec4;

	using vec1 = fvec1;
	using vec2 = fvec2;
	using vec3 = fvec3;
	using vec4 = fvec4;

	using mat3 = glm::mat3;
	using mat4 = glm::mat4;
	using quat = glm::quat;

	using rgb_color = vec<3, u8>;
	using rgba_color = vec<4, u8>;
}