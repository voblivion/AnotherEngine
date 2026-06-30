#pragma once

#include <mutex>
#include <vector>


namespace vob::aoegl
{
	class RegistryControl
	{
	public:
		void addReference(int32_t a_index)
		{
			std::lock_guard lock(m_mutex);
			if (a_index >= m_refCounts.size())
			{
				m_refCounts.resize(static_cast<size_t>(a_index + 1));
			}
			++m_refCounts[a_index];
		}

		void removeReference(int32_t a_index)
		{
			std::lock_guard lock(m_mutex);
			if (--m_refCounts[a_index] == 0)
			{
				m_releasedIndices.push_back(a_index);
			}
		}

		std::vector<int32_t> pollReleasedIndices()
		{
			std::lock_guard lock(m_mutex);
			std::vector<int32_t> releasedIndices;
			std::swap(m_releasedIndices, releasedIndices);
			return releasedIndices;
		}

	private:
		mutable std::mutex m_mutex;
		std::vector<int32_t> m_refCounts;
		std::vector<int32_t> m_releasedIndices;
	};
}
