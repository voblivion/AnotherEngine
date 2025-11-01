#pragma once

#include <vob/aoe/rendering/Color.h>
#include <vob/aoe/rendering/GraphicTypes.h>

#include <vob/misc/physics/measure.h>

#include <glm/gtc/type_ptr.hpp>

#include <span>


namespace vob::aoegl
{
	inline void setUniform(GraphicUniformLocation a_uniformLocation, float const a_value)
	{
		glUniform1f(a_uniformLocation, a_value);
	}

	template <glm::qualifier t_qualifier>
	inline void setUniform(
		GraphicUniformLocation a_uniformLocation
		, glm::vec<1, float, t_qualifier> const& a_vector)
	{
		glUniform1f(a_uniformLocation, a_vector.x);
	}

	template <glm::qualifier t_qualifier>
	inline void setUniform(
		GraphicUniformLocation a_uniformLocation
		, glm::vec<2, float, t_qualifier> const& a_vector)
	{
		glUniform2f(a_uniformLocation, a_vector.x, a_vector.y);
	}

	template <glm::qualifier t_qualifier>
	inline void setUniform(
		GraphicUniformLocation a_uniformLocation
		, glm::vec<3, float, t_qualifier> const& a_vector)
	{
		glUniform3f(a_uniformLocation, a_vector.x, a_vector.y, a_vector.z);
	}

	template <glm::qualifier t_qualifier>
	inline void setUniform(
		GraphicUniformLocation a_uniformLocation
		, glm::vec<4, float, t_qualifier> const& a_vector)
	{
		glUniform4f(a_uniformLocation, a_vector.x, a_vector.y, a_vector.z, a_vector.w);
	}

	template <typename TUnit>
	inline void setUniform(
		GraphicUniformLocation a_uniformLocation, misph::measure<TUnit, float> a_measure)
	{
		glUniform1f(a_uniformLocation, a_measure.get_value());
	}

	template <typename TUnit, glm::qualifier t_qualifier>
	inline void setUniform(
		GraphicUniformLocation a_uniformLocation
		, glm::vec<1, misph::measure<TUnit, float>, t_qualifier> const& a_vector)
	{
		glUniform1f(a_uniformLocation, a_vector.x.get_value());
	}

	template <typename TUnit, glm::qualifier t_qualifier>
	inline void setUniform(
		GraphicUniformLocation a_uniformLocation
		, glm::vec<2, misph::measure<TUnit, float>, t_qualifier> const& a_vector)
	{
		glUniform2f(a_uniformLocation, a_vector.x.get_value(), a_vector.y.get_value());
	}

	template <typename TUnit, glm::qualifier t_qualifier>
	inline void setUniform(
		GraphicUniformLocation a_uniformLocation
		, glm::vec<3, misph::measure<TUnit, float>, t_qualifier> const& a_vector)
	{
		glUniform3f(
			a_uniformLocation
			, a_vector.x.get_value()
			, a_vector.y.get_value()
			, a_vector.z.get_value());
	}

	template <typename TUnit, glm::qualifier t_qualifier>
	inline void setUniform(
		GraphicUniformLocation a_uniformLocation
		, glm::vec<4, misph::measure<TUnit, float>, t_qualifier> const& a_vector)
	{
		glUniform4f(
			a_uniformLocation
			, a_vector.x.get_value()
			, a_vector.y.get_value()
			, a_vector.z.get_value()
			, a_vector.w.get_value());
	}

	inline void setUniform(GraphicUniformLocation a_uniformLocation, Rgb const& a_color)
	{
		glUniform3f(a_uniformLocation, a_color.r.value, a_color.g.value, a_color.b.value);
	}

	inline void setUniform(GraphicUniformLocation a_uniformLocation, Rgba const& a_color)
	{
		glUniform4f(
			a_uniformLocation, a_color.r.value, a_color.g.value, a_color.b.value, a_color.a.value);
	}

	inline void setUniform(GraphicUniformLocation a_uniformLocation, std::int32_t const a_value)
	{
		glUniform1i(a_uniformLocation, a_value);
	}

	template <glm::qualifier t_qualifier>
	inline void setUniform(
		GraphicUniformLocation a_uniformLocation
		, glm::vec<1, std::int32_t, t_qualifier> const& a_vector)
	{
		glUniform1i(a_uniformLocation, a_vector.x);
	}

	template <glm::qualifier t_qualifier>
	inline void setUniform(
		GraphicUniformLocation a_uniformLocation
		, glm::vec<2, std::int32_t, t_qualifier> const& a_vector)
	{
		glUniform2i(a_uniformLocation, a_vector.x, a_vector.y);
	}

	template <glm::qualifier t_qualifier>
	inline void setUniform(
		GraphicUniformLocation a_uniformLocation
		, glm::vec<3, std::int32_t, t_qualifier> const& a_vector)
	{
		glUniform3i(a_uniformLocation, a_vector.x, a_vector.y, a_vector.z);
	}

	template <glm::qualifier t_qualifier>
	inline void setUniform(
		GraphicUniformLocation a_uniformLocation
		, glm::vec<4, std::int32_t, t_qualifier> const& a_vector)
	{
		glUniform4i(a_uniformLocation, a_vector.x, a_vector.y, a_vector.z, a_vector.w);
	}

	inline void setUniform(GraphicUniformLocation a_uniformLocation, std::uint32_t const a_value)
	{
		glUniform1ui(a_uniformLocation, a_value);
	}

	template <glm::qualifier t_qualifier>
	inline void setUniform(
		GraphicUniformLocation a_uniformLocation
		, glm::vec<1, std::uint32_t, t_qualifier> const& a_vector)
	{
		glUniform1ui(a_uniformLocation, a_vector.x);
	}

	template <glm::qualifier t_qualifier>
	inline void setUniform(
		GraphicUniformLocation a_uniformLocation
		, glm::vec<2, std::uint32_t, t_qualifier> const& a_vector)
	{
		glUniform2ui(a_uniformLocation, a_vector.x, a_vector.y);
	}

	template <glm::qualifier t_qualifier>
	inline void setUniform(
		GraphicUniformLocation a_uniformLocation
		, glm::vec<3, std::uint32_t, t_qualifier> const& a_vector)
	{
		glUniform3ui(a_uniformLocation, a_vector.x, a_vector.y, a_vector.z);
	}

	template <glm::qualifier t_qualifier>
	inline void setUniform(
		GraphicUniformLocation a_uniformLocation
		, glm::vec<4, std::uint32_t, t_qualifier> const& a_vector)
	{
		glUniform4ui(a_uniformLocation, a_vector.x, a_vector.y, a_vector.z, a_vector.w);
	}

	template <glm::qualifier t_qualifier>
	inline void setUniform(
		GraphicUniformLocation a_uniformLocation
		, glm::mat<4, 4, float, t_qualifier> const& a_matrix)
	{
		glUniformMatrix4fv(a_uniformLocation, 1, GL_FALSE, glm::value_ptr(a_matrix));
	}

	template <glm::qualifier t_qualifier>
	inline void setUniform(
		GraphicUniformLocation a_uniformLocation
		, glm::mat<3, 3, float, t_qualifier> const& a_matrix)
	{
		glUniformMatrix3fv(a_uniformLocation, 1, GL_FALSE, glm::value_ptr(a_matrix));
	}


	inline void setUniform(
		GraphicUniformLocation a_uniformLocation, std::span<glm::mat4 const> a_matrices)
	{
		glUniformMatrix4fv(
			a_uniformLocation,
			static_cast<GraphicSize>(a_matrices.size()),
			GL_TRUE,
			a_matrices.size() > 0 ? glm::value_ptr(a_matrices[0]) : nullptr);
	}
}
