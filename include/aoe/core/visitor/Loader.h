#pragma once

#include <aoe/core/data/ALoader.h>
#include <memory>

namespace aoe
{
	namespace vis
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
			virtual std::shared_ptr<ADynamicType> load(
				std::istream& a_inputStream) override
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
}