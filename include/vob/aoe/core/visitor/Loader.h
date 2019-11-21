#pragma once

#include <vob/aoe/core/data/ALoader.h>
#include <memory>

namespace vob::aoe::vis
{
	template <typename WriterType>
	class Loader final
		: public data::ALoader
	{
	public:
		// Constructors
		template <typename... Args>
		explicit Loader(Args&&... a_args)
			: m_visitorWriter{ std::forward<Args>(a_args)... }
		{}

		// Methods
		std::shared_ptr<ADynamicType> load(std::istream& a_inputStream) override
		{
			std::shared_ptr<ADynamicType> t_data;
			m_visitorWriter.load(a_inputStream, t_data);
			return t_data;
		}

		WriterType& getVisitor()
		{
			return m_visitorWriter;
		}

	private:
		// Attributes
		WriterType m_visitorWriter;
	};
}