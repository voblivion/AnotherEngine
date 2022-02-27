#pragma once


namespace vob::aoe::common
{
	template <typename TVisitor>
	class visitor_stream_lodaer
	{
	public:
		explicit visitor_stream_loader(TVisitor& a_visitor)
			: m_visitor{ a_visitor }
		{}

		std::shared_ptr<ADynamicType> load(std::istream& a_inputStream) const
		{

		}

	private:
		TVisitor& m_visitor;
	};
}