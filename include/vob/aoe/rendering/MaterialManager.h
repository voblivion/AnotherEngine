#pragma once

#include "vob/aoe/rendering/Material.h"

#include <vob/misc/std/container_util.h>

#include <cassert>
#include <vector>


namespace vob::aoegl
{
	class MaterialManager
	{
	public:

		inline Material const& getMaterial(int32_t a_materialIndex) const
		{
			assert(a_materialIndex < mistd::isize(m_materials));
			assert(m_inUses[a_materialIndex]);
			return m_materials[a_materialIndex];
		}

		inline int32_t emplaceMaterial(GraphicId a_paramsUbo, mistd::bounded_vector<GraphicId, k_materialTexturesCapacity> a_textureIds = {})
		{
			if (m_freeIndices.empty())
			{
				auto const materialIndex = mistd::isize(m_materials);
				m_materials.emplace_back(a_paramsUbo, a_textureIds);
				m_inUses.emplace_back(true);
				return materialIndex;
			}

			auto const materialIndex = m_freeIndices.back();
			assert(materialIndex < mistd::isize(m_materials));
			assert(!m_inUses[materialIndex]);
			m_inUses[materialIndex] = true;
			m_freeIndices.pop_back();
			m_materials[materialIndex] = Material{ a_paramsUbo, a_textureIds };
			return materialIndex;
		}

		inline void removeMaterial(int32_t a_materialIndex)
		{
			assert(a_materialIndex < mistd::isize(m_materials));
			assert(m_inUses[a_materialIndex]);
			m_inUses[a_materialIndex] = false;
			m_freeIndices.emplace_back(a_materialIndex);
		}

	private:
		std::vector<Material> m_materials;
		std::vector<int32_t> m_freeIndices;
		std::vector<bool> m_inUses;
	};
}
