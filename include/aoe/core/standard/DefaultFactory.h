#pragma once


namespace aoe
{
	namespace sta
	{
		template <typename ValueType>
		struct DefaultFactory
		{
			ValueType operator()() const
			{
				return {};
			}
		};

		template <>
		struct DefaultFactory<std::type_index>
		{
			std::type_index operator()() const
			{
				return typeid(void);
			}
		};

		template <typename LeftType, typename RightType>
		struct DefaultFactory<std::pair<LeftType, RightType>>
		{
			DefaultFactory(DefaultFactory<LeftType> a_leftFactory = {}
				, DefaultFactory<RightType> a_rightFactory = {})
				: m_leftFactory{ a_leftFactory }
				, m_rightFactory{ a_rightFactory }
			{}

			std::pair<LeftType, RightType> operator()() const
			{
				return std::make_pair(m_leftFactory(), m_rightFactory());
			}

		private:
			DefaultFactory<LeftType> m_leftFactory;
			DefaultFactory<RightType> m_rightFactory;
		};
	}
}