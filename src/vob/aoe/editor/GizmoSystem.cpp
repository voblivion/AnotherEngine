#include "vob/aoe/editor/GizmoSystem.h"

#include "vob/aoe/debug/Check.h"
#include "vob/aoe/rendering/CameraUtils.h"
#include "vob/aoe/spacetime/TransformUtils.h"
#include "vob/aoe/math/MathUtils.h"

#include <limits>
#include <variant>

#pragma optimize("", off)
namespace vob::aoedi
{
	void GizmoSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
	}

	void GizmoSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto const& window = m_windowCtx.get(a_wdap).window.get();
		auto& gizmoCtx = m_gizmoCtx.get(a_wdap);
		auto const cameraEntities = m_cameraEntities.get(a_wdap);
		auto const& activeCameraEntity = m_cameraDirectorCtx.get(a_wdap).activeCameraEntity;
		auto const cameraProperties = aoegl::getCameraProperties(cameraEntities, activeCameraEntity);
		auto const windowSize = glm::vec2{ window.getSize() };

		auto const viewToWorld = aoest::combine(cameraProperties.position, cameraProperties.rotation);
		auto const viewToClip = glm::perspective(
			cameraProperties.fov,
			windowSize.x / windowSize.y,
			cameraProperties.nearClip,
			cameraProperties.farClip);
		auto const clipToView = glm::inverse(viewToClip);
		auto const clipToWorld = viewToWorld * clipToView;

		auto const windowToWorld = [&](auto const& windowPosition)
			{
				auto const ndc = glm::vec2{
					2.0f * windowPosition.x / windowSize.x - 1.0f,
					1.0f - 2.0f * windowPosition.y / windowSize.y
				};
				auto const ndcNear = glm::vec4(ndc.x, ndc.y, -1.0f, 1.0f);
				auto const ndcFar = glm::vec4(ndc.x, ndc.y, 1.0f, 1.0f);

				auto const position = aoest::transformPositionSkewed(clipToWorld, ndcNear);
				auto const positionFar = aoest::transformPositionSkewed(clipToWorld, ndcFar);
				auto const direction = aoema::normalizeWithDefault(positionFar - position, glm::vec3{ 0.0f, 0.0f, 1.0f });

				return std::make_pair(position, direction);
			};

		if (auto const activeHandle = gizmoCtx.activeHandle.lock())
		{
			for (auto const& windowEvent : window.getPolledEvents())
			{
				if (const auto mouseMoveEvent = std::get_if<aoewi::MouseMoveEvent>(&windowEvent))
				{
					auto const [mousePosition, mouseDirection] = windowToWorld(mouseMoveEvent->position);
					activeHandle->drag(mousePosition, mouseDirection);
				}
				else if (const auto mouseButtonEvent = std::get_if<aoewi::MouseButtonEvent>(&windowEvent))
				{
					if (mouseButtonEvent->button == aoein::Mouse::Button::Left && !mouseButtonEvent->pressed)
					{
						gizmoCtx.activeHandle.reset();
						break;
					}
				}
				else if (const auto mouseHoverEvent = std::get_if<aoewi::MouseHoverEvent>(&windowEvent))
				{
					VOB_AOE_CHECK_BREAK(!mouseHoverEvent->entered, "No active should have been active while mouse outside window.");
					
					gizmoCtx.activeHandle.reset();
					break;
				}
			}
		}
		else
		{
			auto const [mousePosition, mouseDirection] = windowToWorld(window.getMousePosition());

			auto hasSetActiveHandle = false;
			if (auto const hoveredHandle = gizmoCtx.hoveredHandle.lock())
			{
				for (auto const& windowEvent : window.getPolledEvents())
				{
					if (const auto mouseButtonEvent = std::get_if<aoewi::MouseButtonEvent>(&windowEvent))
					{
						if (mouseButtonEvent->button == aoein::Mouse::Button::Left && mouseButtonEvent->pressed)
						{
							hoveredHandle->beginDrag(mousePosition, mouseDirection);
							gizmoCtx.activeHandle = hoveredHandle;
							hasSetActiveHandle = true;
							break;
						}
					}
				}
			}

			if (!hasSetActiveHandle)
			{
				auto closestHandleDistance = std::numeric_limits<float>::infinity();
				std::shared_ptr<AGizmoHandle> closestHandle = nullptr;
				for (auto const& [entity, gizmoCmp] : m_gizmoEntities.get(a_wdap).each())
				{
					for (auto const& handle : gizmoCmp.handles)
					{
						auto const handleDistance = handle->getDistance(mousePosition, mouseDirection);
						if (handleDistance < closestHandleDistance)
						{
							closestHandleDistance = handleDistance;
							closestHandle = handle;
						}
					}
				}

				if (closestHandle != nullptr)
				{
					closestHandle->beginDrag(mousePosition, mouseDirection);
				}
				gizmoCtx.hoveredHandle = closestHandle;
			}
		}
	}
}
