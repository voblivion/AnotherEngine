#pragma once

#include <string>

#include <vob/aoe/core/data/ALoader.h>
#include <vob/aoe/common/render/Shader.h>
#include <vob/aoe/core/type/TypeFactory.h>


namespace vob::aoe::common
{
	template <typename ShaderT>
	class ShaderLoader final
		: public data::ALoader
	{
	public:
		// Constructors
		explicit ShaderLoader(std::pmr::memory_resource* a_resource
			= std::pmr::get_default_resource())
			: m_resource{ a_resource }
		{}

		// Methods
		virtual std::shared_ptr<ADynamicType> load(
			std::istream& a_inputStream) override
		{
			std::pmr::string t_shaderSource{
				std::istreambuf_iterator<char>(a_inputStream), {}, m_resource
			};
			auto r_shader = sta::allocate_polymorphic<ShaderT>(
				std::pmr::polymorphic_allocator<ShaderT>{ m_resource }
			);
			r_shader->m_source = std::move(t_shaderSource);
			return r_shader;
		}

	private:
		// Attributes
		std::pmr::memory_resource* m_resource;
	};
}
