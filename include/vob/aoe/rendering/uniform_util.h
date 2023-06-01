#pragma once

#include <vob/aoe/rendering/color.h>
#include <vob/aoe/rendering/graphic_types.h>

#include <vob/misc/std/message_macros.h>

#include <glm/gtc/type_ptr.hpp>

#include <span>


namespace vob::aoegl::uniform_util
{
	inline void set(graphic_uniform_location a_uniformLocation, float const a_value)
	{
		glUniform1f(a_uniformLocation, a_value);
	}

	template <glm::qualifier t_qualifier>
	inline void set(
		graphic_uniform_location a_uniformLocation
		, glm::vec<1, float, t_qualifier> const& a_vector)
	{
		glUniform1f(a_uniformLocation, a_vector.x);
	}

	template <glm::qualifier t_qualifier>
	inline void set(
		graphic_uniform_location a_uniformLocation
		, glm::vec<2, float, t_qualifier> const& a_vector)
	{
		glUniform2f(a_uniformLocation, a_vector.x, a_vector.y);
	}

	template <glm::qualifier t_qualifier>
	inline void set(
		graphic_uniform_location a_uniformLocation
		, glm::vec<3, float, t_qualifier> const& a_vector)
	{
		glUniform3f(a_uniformLocation, a_vector.x, a_vector.y, a_vector.z);
	}

	template <glm::qualifier t_qualifier>
	inline void set(
		graphic_uniform_location a_uniformLocation
		, glm::vec<4, float, t_qualifier> const& a_vector)
	{
		glUniform4f(a_uniformLocation, a_vector.x, a_vector.y, a_vector.z, a_vector.w);
	}

	template <typename TUnit>
	inline void set(
		graphic_uniform_location a_uniformLocation, misph::measure<TUnit, float> a_measure)
	{
		glUniform1f(a_uniformLocation, a_measure.get_value());
	}

	template <typename TUnit, glm::qualifier t_qualifier>
	inline void set(
		graphic_uniform_location a_uniformLocation
		, glm::vec<1, misph::measure<TUnit, float>, t_qualifier> const& a_vector)
	{
		glUniform1f(a_uniformLocation, a_vector.x.get_value());
	}

	template <typename TUnit, glm::qualifier t_qualifier>
	inline void set(
		graphic_uniform_location a_uniformLocation
		, glm::vec<2, misph::measure<TUnit, float>, t_qualifier> const& a_vector)
	{
		glUniform2f(a_uniformLocation, a_vector.x.get_value(), a_vector.y.get_value());
	}

	template <typename TUnit, glm::qualifier t_qualifier>
	inline void set(
		graphic_uniform_location a_uniformLocation
		, glm::vec<3, misph::measure<TUnit, float>, t_qualifier> const& a_vector)
	{
		glUniform3f(
			a_uniformLocation
			, a_vector.x.get_value()
			, a_vector.y.get_value()
			, a_vector.z.get_value());
	}

	template <typename TUnit, glm::qualifier t_qualifier>
	inline void set(
		graphic_uniform_location a_uniformLocation
		, glm::vec<4, misph::measure<TUnit, float>, t_qualifier> const& a_vector)
	{
		glUniform4f(
			a_uniformLocation
			, a_vector.x.get_value()
			, a_vector.y.get_value()
			, a_vector.z.get_value()
			, a_vector.w.get_value());
	}

	inline void set(graphic_uniform_location a_uniformLocation, rgb const& a_color)
	{
		glUniform3f(a_uniformLocation, a_color.r.m_value, a_color.g.m_value, a_color.b.m_value);
	}

	inline void set(graphic_uniform_location a_uniformLocation, rgba const& a_color)
	{
		glUniform4f(
			a_uniformLocation
			, a_color.r.m_value
			, a_color.g.m_value
			, a_color.b.m_value
			, a_color.a.m_value);
	}

	inline void set(graphic_uniform_location a_uniformLocation, std::int32_t const a_value)
	{
		glUniform1i(a_uniformLocation, a_value);
	}

	template <glm::qualifier t_qualifier>
	inline void set(
		graphic_uniform_location a_uniformLocation
		, glm::vec<1, std::int32_t, t_qualifier> const& a_vector)
	{
		glUniform1i(a_uniformLocation, a_vector.x);
	}

	template <glm::qualifier t_qualifier>
	inline void set(
		graphic_uniform_location a_uniformLocation
		, glm::vec<2, std::int32_t, t_qualifier> const& a_vector)
	{
		glUniform2i(a_uniformLocation, a_vector.x, a_vector.y);
	}

	template <glm::qualifier t_qualifier>
	inline void set(
		graphic_uniform_location a_uniformLocation
		, glm::vec<3, std::int32_t, t_qualifier> const& a_vector)
	{
		glUniform3i(a_uniformLocation, a_vector.x, a_vector.y, a_vector.z);
	}

	template <glm::qualifier t_qualifier>
	inline void set(
		graphic_uniform_location a_uniformLocation
		, glm::vec<4, std::int32_t, t_qualifier> const& a_vector)
	{
		glUniform4i(a_uniformLocation, a_vector.x, a_vector.y, a_vector.z, a_vector.w);
	}

	inline void set(graphic_uniform_location a_uniformLocation, std::uint32_t const a_value)
	{
		glUniform1ui(a_uniformLocation, a_value);
	}

	template <glm::qualifier t_qualifier>
	inline void set(
		graphic_uniform_location a_uniformLocation
		, glm::vec<1, std::uint32_t, t_qualifier> const& a_vector)
	{
		glUniform1ui(a_uniformLocation, a_vector.x);
	}

	template <glm::qualifier t_qualifier>
	inline void set(
		graphic_uniform_location a_uniformLocation
		, glm::vec<2, std::uint32_t, t_qualifier> const& a_vector)
	{
		glUniform2ui(a_uniformLocation, a_vector.x, a_vector.y);
	}

	template <glm::qualifier t_qualifier>
	inline void set(
		graphic_uniform_location a_uniformLocation
		, glm::vec<3, std::uint32_t, t_qualifier> const& a_vector)
	{
		glUniform3ui(a_uniformLocation, a_vector.x, a_vector.y, a_vector.z);
	}

	template <glm::qualifier t_qualifier>
	inline void set(
		graphic_uniform_location a_uniformLocation
		, glm::vec<4, std::uint32_t, t_qualifier> const& a_vector)
	{
		glUniform4ui(a_uniformLocation, a_vector.x, a_vector.y, a_vector.z, a_vector.w);
	}

#pragma message(VOB_MISTD_TODO "add v versions? other matrix versions?")

	template <glm::qualifier t_qualifier>
	inline void set(
		graphic_uniform_location a_uniformLocation
		, glm::mat<4, 4, float, t_qualifier> const& a_matrix)
	{
		glUniformMatrix4fv(a_uniformLocation, 1, GL_FALSE, glm::value_ptr(a_matrix));
	}

	template <glm::qualifier t_qualifier>
	inline void set(
		graphic_uniform_location a_uniformLocation
		, glm::mat<3, 3, float, t_qualifier> const& a_matrix)
	{
		glUniformMatrix3fv(a_uniformLocation, 1, GL_FALSE, glm::value_ptr(a_matrix));
	}


	inline void set(
		graphic_uniform_location a_uniformLocation, std::span<glm::mat4 const> a_matrices)
	{
		glUniformMatrix4fv(
			a_uniformLocation,
			static_cast<graphic_size>(a_matrices.size()),
			GL_TRUE,
			a_matrices.size() > 0 ? glm::value_ptr(a_matrices[0]) : nullptr);
	}
}
