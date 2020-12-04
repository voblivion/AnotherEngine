#pragma once

#include <vob/aoe/common/render/Manager.h>
#include <vob/aoe/common/render/GraphicResourceHandle.h>
#include <vob/aoe/common/render/resources/RenderTexture.h>

namespace vob::aoe::common
{
	struct SceneRenderComponent final
		: public ecs::AComponent
	{
		// Constructors
		template <typename... TArgs>
		explicit SceneRenderComponent(
			IGraphicResourceManager<RenderTexture>& a_renderTextureManager
			, TArgs&&... a_args
		)
			: m_renderTextureManager{ a_renderTextureManager }
			, m_renderTexture{ a_renderTextureManager, std::forward<TArgs>(a_args)... }
		{}

		// Attributes
		IGraphicResourceManager<RenderTexture>& m_renderTextureManager;
		GraphicResourceHandle<RenderTexture> m_renderTexture;
	};
}
