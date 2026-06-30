#include "vob/aoe/editor/TranslateGizmoSystem.h"

#include "vob/aoe/rendering/CameraUtils.h"


namespace vob::aoedi
{
	namespace
	{
		void processActiveAxisTranslateHandleChange(
			aoest::PositionComponent& a_positionCmp,
			GizmoContext const& a_gizmoCtx,
			std::shared_ptr<AxisTranslateHandle> const& a_handle)
		{
			if (a_gizmoCtx.activeHandle.lock() == a_handle)
			{
				a_positionCmp.value = a_handle->getPosition();
			}
		}

		void processActiveAxisRotateHandleChange(
			aoest::RotationComponent& a_rotationCmp,
			GizmoContext const& a_gizmoCtx,
			std::shared_ptr<AxisRotateHandle> const& a_handle)
		{
			if (a_gizmoCtx.activeHandle.lock() == a_handle)
			{
				a_rotationCmp.value = a_handle->getRotation();
			}
		}

		void updateAxisTranslateHandle(
			aoest::PositionComponent const& a_positionCmp,
			aoest::RotationComponent const& a_rotationCmp,
			GizmoContext const& a_gizmoCtx,
			std::shared_ptr<AxisTranslateHandle> const& a_handle,
			aoegl::DebugMeshContext& a_debugMeshCtx,
			glm::vec3 const& a_color)
		{
			auto const color = aoegl::toRgba(a_color * (a_gizmoCtx.hoveredHandle.lock() == a_handle ? 1.0f : 0.5f));

			auto const dir = a_rotationCmp.value * a_handle->getAxis() * glm::vec3{ 1.0f, 0.0f, 0.0f };

			a_handle->updateTransform(a_positionCmp.value, a_rotationCmp.value);
			a_debugMeshCtx.addLine(a_positionCmp.value, a_positionCmp.value + dir, color);
		}

		void updateAxisRotateHandle(
			aoest::PositionComponent const& a_positionCmp,
			aoest::RotationComponent const& a_rotationCmp,
			GizmoContext const& a_gizmoCtx,
			std::shared_ptr<AxisRotateHandle> const& a_handle,
			aoegl::DebugMeshContext& a_debugMeshCtx,
			glm::vec3 const& a_color)
		{
			auto const color = aoegl::toRgba(a_color * (a_gizmoCtx.hoveredHandle.lock() == a_handle ? 1.0f : 0.5f));

			a_handle->updateTransform(a_positionCmp.value, a_rotationCmp.value);
			a_debugMeshCtx.addArc(
				a_positionCmp.value,
				a_rotationCmp.value * a_handle->getAxis(),
				a_handle->getRadius(),
				0.0f,
				0.5f * std::numbers::pi_v<float>,
				color);
		}
	}

	void TranslateGizmoSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
	}

	void TranslateGizmoSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto const& gizmoCtx = m_gizmoCtx.get(a_wdap);
		auto& debugMeshCtx = m_debugMeshCtx.get(a_wdap);

		for (auto [entity, positionCmp, rotationCmp, translateGizmoCmp] : m_gizmoEntities.get(a_wdap).each())
		{
			processActiveAxisTranslateHandleChange(positionCmp, gizmoCtx, translateGizmoCmp.x);
			processActiveAxisTranslateHandleChange(positionCmp, gizmoCtx, translateGizmoCmp.y);
			processActiveAxisTranslateHandleChange(positionCmp, gizmoCtx, translateGizmoCmp.z);

			processActiveAxisRotateHandleChange(rotationCmp, gizmoCtx, translateGizmoCmp.rx);
			processActiveAxisRotateHandleChange(rotationCmp, gizmoCtx, translateGizmoCmp.ry);
			processActiveAxisRotateHandleChange(rotationCmp, gizmoCtx, translateGizmoCmp.rz);

			updateAxisTranslateHandle(
				positionCmp, rotationCmp, gizmoCtx, translateGizmoCmp.x, debugMeshCtx, glm::vec3{ 1.0f, 0.0f, 0.0f });
			updateAxisTranslateHandle(
				positionCmp, rotationCmp, gizmoCtx, translateGizmoCmp.y, debugMeshCtx, glm::vec3{ 0.0f, 1.0f, 0.0f });
			updateAxisTranslateHandle(
				positionCmp, rotationCmp, gizmoCtx, translateGizmoCmp.z, debugMeshCtx, glm::vec3{ 0.0f, 0.0f, 1.0f });

			updateAxisRotateHandle(
				positionCmp, rotationCmp, gizmoCtx, translateGizmoCmp.rx, debugMeshCtx, glm::vec3{ 1.0f, 0.0f, 0.0f });
			updateAxisRotateHandle(
				positionCmp, rotationCmp, gizmoCtx, translateGizmoCmp.ry, debugMeshCtx, glm::vec3{ 0.0f, 1.0f, 0.0f });
			updateAxisRotateHandle(
				positionCmp, rotationCmp, gizmoCtx, translateGizmoCmp.rz, debugMeshCtx, glm::vec3{ 0.0f, 0.0f, 1.0f });
		}
	}
}
