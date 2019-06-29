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
			explicit ShaderLoader(sta::Allocator<char> const& a_allocator)
				: m_allocator{ a_allocator }
			{}

			// Methods
			virtual std::shared_ptr<ADynamicType> load(
				std::istream& a_inputStream) override
			{
				std::pmr::string t_shaderSource{
					std::istreambuf_iterator<char>(a_inputStream)
					, {}, m_allocator };
				return sta::allocatePolymorphic<ShaderType>(m_allocator
					, t_shaderSource);
			}

		private:
			// Attributes
			sta::Allocator<char> m_allocator;
		};
	}
}
