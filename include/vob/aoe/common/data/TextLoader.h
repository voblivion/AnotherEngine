#pragma once

#include <vob/aoe/core/data/ALoader.h>

#include <vob/aoe/common/data/Text.h>

namespace vob::aoe::common
{
	class TextLoader final
		: public data::ALoader
	{
	public:
#pragma region Constructors
		explicit TextLoader(
			std::pmr::memory_resource* a_resource = std::pmr::get_default_resource()
		)
			: m_resource{ a_resource }
		{}
#pragma endregion
#pragma region Methods
		std::shared_ptr<type::ADynamicType> load(std::istream& a_inputStream) override
		{
			return std::allocate_shared<Text>(
				std::pmr::polymorphic_allocator<Text>{ m_resource }
				, std::istreambuf_iterator<char>(a_inputStream)
				, std::istreambuf_iterator<char>{}
				, m_resource
			);
		}
#pragma endregion
	private:
#pragma region Attributes
		std::pmr::memory_resource* m_resource;
#pragma endregion
	};
}