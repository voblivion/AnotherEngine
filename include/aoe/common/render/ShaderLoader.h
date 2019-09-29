#pragma once

#include <string>

#include <aoe/core/data/ALoader.h>
#include <aoe/common/render/Shader.h>
#include <aoe/core/standard/Allocator.h>
#include <aoe/core/standard/TypeFactory.h>


namespace aoe
{
	namespace common
	{
		template <typename ShaderType>
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
					std::istreambuf_iterator<char>(a_inputStream)
					, {}, m_resource };
				auto r_shader = sta::allocatePolymorphicWith<ShaderType>(m_resource);
				r_shader->loadFrom(t_shaderSource);
				return r_shader;
			}

		private:
			// Attributes
			std::pmr::memory_resource* m_resource;
		};
	}
}
