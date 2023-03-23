#pragma once

#include <vob/aoe/rendering/resources/material.h>
#include <vob/aoe/rendering/resources/mesh.h>
#include <vob/aoe/rendering/resources/vertex.h>


namespace vob::aoegl::bind_util
{
	void bind_static_mesh(mesh const& a_mesh)
	{
		glBindVertexBuffer(0, a_mesh.m_staticVbo, 0, sizeof(vertex_static));
	}

	void bind_rigged_mesh(mesh const& a_mesh)
	{
		bind_static_mesh(a_mesh);
		glBindVertexBuffer(1, a_mesh.m_riggedVbo, 0, sizeof(vertex_rigged));
	}

	void bind_material(material const& a_material)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, a_material.m_albedo);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, a_material.m_normal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, a_material.m_metallicRoughness);
	}
}
