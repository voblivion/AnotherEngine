#include "vob/aoe/physics/DebugCollisionSystem.h"

#include "vob/aoe/debug/DebugNameUtils.h"
#include "vob/aoe/physics/MathUtils.h"

#include "imgui.h"

#include <array>
#include <string>
#include <vector>


namespace vob::aoeph
{
	void DebugCollisionSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_debugMeshContext.init(a_wdar);
		m_staticColliderEntities.init(a_wdar);
	}

	void DebugCollisionSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto& debugMeshContext = m_debugMeshContext.get(a_wdap);

		// TODO: move to context
		static bool k_debugStaticColliders = false;
		static bool k_debugCarCollider = false;
		static bool k_debugCarChassisContacts = false;
		static bool k_debugCarWheelContacts = false;
		static bool k_debugCarPhases = false;
		static std::vector<std::array<bool, 4>> k_debugCarContacts;
		static entt::entity k_debugCarEntity =
			[&]() -> entt::entity {
				for (auto [entity, position, rotation, carCollider] : m_carColliderEntities.get(a_wdap).each())
				{
					return entity;
				}
				return entt::null;
			}();

		if (ImGui::Begin("Physics"))
		{
			ImGui::Checkbox("Static Colliders", &k_debugStaticColliders);

			auto const& debugNameEntities = m_debugNameEntities.get(a_wdap);
			if (ImGui::BeginCombo("Active Car", aoedb::getImmediateUseDebugNameCStr(debugNameEntities, k_debugCarEntity)))
			{
				for (auto [carEntity, carPosition, carRotation, carCollider] : m_carColliderEntities.get(a_wdap).each())
				{
					auto const isActive = k_debugCarEntity == carEntity;
					if (ImGui::Selectable(aoedb::getImmediateUseDebugNameCStr(debugNameEntities, carEntity), isActive))
					{
						k_debugCarEntity = carEntity;
					}

					if (isActive)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				auto const isActive = k_debugCarEntity == entt::null;
				if (ImGui::Selectable(aoedb::getImmediateUseDebugNameCStr(debugNameEntities, entt::null), isActive))
				{
					k_debugCarEntity = entt::null;

					if (isActive)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
			ImGui::Checkbox("Car Collider", &k_debugCarCollider);

			if (m_carColliderEntities.get(a_wdap).contains(k_debugCarEntity))
			{
				auto const [position, rotation, carCollider] = m_carColliderEntities.get(a_wdap).get(k_debugCarEntity);
				auto carPosition = position;
				ImGui::BeginDisabled();
				ImGui::InputFloat3("Position", &carPosition.x);
				auto carEulerAngles = glm::eulerAngles(rotation);
				ImGui::InputFloat3("Rotation", &carEulerAngles.x);
				auto wheelsGrounded = glm::vec4{ 0.0f };
				auto wheelsSuspensionLengths = glm::vec4{ 0.0f };
				wheelsGrounded[0] = carCollider.wheels[0].isGrounded;
				wheelsGrounded[1] = carCollider.wheels[1].isGrounded;
				wheelsGrounded[2] = carCollider.wheels[2].isGrounded;
				wheelsGrounded[3] = carCollider.wheels[3].isGrounded;
				wheelsSuspensionLengths[0] = carCollider.wheels[0].suspensionLength;
				wheelsSuspensionLengths[1] = carCollider.wheels[1].suspensionLength;
				wheelsSuspensionLengths[2] = carCollider.wheels[2].suspensionLength;
				wheelsSuspensionLengths[3] = carCollider.wheels[3].suspensionLength;
				ImGui::InputFloat4("Wheels Grounded", &wheelsGrounded.x);
				ImGui::InputFloat4("Wheels Suspension L.", &wheelsSuspensionLengths.x);
				ImGui::EndDisabled();
				ImGui::Checkbox("Car Phases", &k_debugCarPhases);
				ImGui::Checkbox("Car Chassis Contacts", &k_debugCarChassisContacts);
				ImGui::Checkbox("Car Wheel Contacts", &k_debugCarWheelContacts);
				ImGui::Text("Car Contacts");
				ImGui::SameLine();
				k_debugCarContacts.resize(carCollider.wheels[0].contacts.size(), std::array<bool, 4>{false});
				if (ImGui::BeginTable("CarContacts", carCollider.wheels[0].contacts.size() + 2))
				{
					ImGui::TableNextRow();

					auto multiCheck = [](int32_t r0, int32_t r1, int32_t c0, int32_t c1)
						{
							int32_t checkedCount = 0;
							for (int32_t r = r0; r < r1; ++r)
							{
								for (int32_t c = c0; c < c1; ++c)
								{
									if (k_debugCarContacts[c][r])
									{
										++checkedCount;
									}
								}
							}

							int32_t flags = (checkedCount > 0) | ((checkedCount == (c1 - c0) * (r1 - r0)) << 1);
							if (ImGui::CheckboxFlags("##col", &flags, 0b11))
							{
								for (int32_t r = r0; r < r1; ++r)
								{
									for (int32_t c = c0; c < c1; ++c)
									{
										k_debugCarContacts[c][r] = (flags & 0b11);
									}
								}
							}
						};

					for (int32_t c = 0; c < k_debugCarContacts.size(); ++c)
					{
						ImGui::TableSetColumnIndex(c + 2);
						ImGui::Text("%d", c);
					}

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(1);
					ImGui::PushID(0);
					multiCheck(0, 4, 0, k_debugCarContacts.size());
					ImGui::PopID();

					for (int32_t c = 0; c < k_debugCarContacts.size(); ++c)
					{
						ImGui::TableSetColumnIndex(c + 2);
						ImGui::PushID((c + 1) * 5 + 0);
						multiCheck(0, 4, c, c + 1);
						ImGui::PopID();
					}

					for (int32_t r = 0; r < 4; ++r)
					{
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("%d", r);

						ImGui::TableSetColumnIndex(1);
						ImGui::PushID(r + 1);
						multiCheck(r, r + 1, 0, k_debugCarContacts.size());
						ImGui::PopID();

						for (int32_t c = 0; c < k_debugCarContacts.size(); ++c)
						{
							ImGui::TableSetColumnIndex(c + 2);
							ImGui::PushID((c + 1) * 5 + r);
							ImGui::Checkbox("##k", &k_debugCarContacts[c][r]);
							ImGui::PopID();
						}
					}

					ImGui::EndTable();
				}
			}
		}

		if (k_debugStaticColliders)
		{
			aoeph::CarCollider const* debuggedCarCollider = nullptr;
			if (k_debugCarPhases && m_carColliderEntities.get(a_wdap).contains(k_debugCarEntity))
			{
				auto [position, rotation, carCollider] = m_carColliderEntities.get(a_wdap).get(k_debugCarEntity);
				debuggedCarCollider = &carCollider;
			}
			auto isInBroadPhase = [debuggedCarCollider](glm::vec3 const& p0, glm::vec3 const& p1, glm::vec3 const& p2)
				{
					if (debuggedCarCollider == nullptr)
					{
						return false;
					}

					for (auto const& triangle : debuggedCarCollider->broadPhaseCandidates)
					{
						if (p0 == triangle.p0 && p1 == triangle.p1 && p2 == triangle.p2)
						{
							return true;
						}
					}

					return false;
				};

			for (auto [entity, position, rotation, staticCollider] : m_staticColliderEntities.get(a_wdap).each())
			{
				debugMeshContext.addAabb(staticCollider.bounds.min, staticCollider.bounds.max, aoegl::k_blueprint);

				for (auto const& staticPart : staticCollider.parts)
				{
					for (auto const& staticTriangle : staticPart.triangles)
					{
						auto const p0 = position + rotation * staticTriangle.p0;
						auto const p1 = position + rotation * staticTriangle.p1;
						auto const p2 = position + rotation * staticTriangle.p2;
						if (!isInBroadPhase(p0, p1, p2))
						{
							debugMeshContext.addTriangle(
								p0,
								p1,
								p2,
								aoegl::k_blue,
								0 /* subdivisions */);
						}
					}
				}
			}
		}

		if (m_carColliderEntities.get(a_wdap).contains(k_debugCarEntity))
		{
			auto [position, rotation, carCollider] = m_carColliderEntities.get(a_wdap).get(k_debugCarEntity);

			if (k_debugCarCollider)
			{
				auto const carBounds = computeBounds(position + rotation * carCollider.boundsCenterLocal, rotation, carCollider.boundsHalfExtentsLocal);
				debugMeshContext.addAabb(carBounds.min, carBounds.max, aoegl::k_chartreuse);
			}

			int32_t candidateIndex = 0;
			for (auto const& triangle : carCollider.broadPhaseCandidates)
			{
				if (k_debugCarPhases)
				{
					auto const narrowPhaseIt = std::find(carCollider.narrowPhaseContacts.begin(), carCollider.narrowPhaseContacts.end(), candidateIndex);
					auto const color = narrowPhaseIt != carCollider.narrowPhaseContacts.end() ? aoegl::k_white : aoegl::k_gray;
					debugMeshContext.addTriangle(triangle.p0, triangle.p1, triangle.p2, color);
				}
				++candidateIndex;
			}

			for (auto const& chassisPart : carCollider.chassisParts)
			{
				if (k_debugCarCollider)
				{
					debugMeshContext.addEllipsoid(position + rotation * chassisPart.position, rotation * chassisPart.rotation, chassisPart.radiuses, aoegl::k_chartreuse);
				}

				if (k_debugCarChassisContacts)
				{
					for (int i = 0; i < k_debugCarContacts.size(); ++i)
					{
						for (int k = 0; k < 4; ++k)
						{
							if (!k_debugCarContacts[i][k])
							{
								continue;
							}

							for (auto const& debugContact : chassisPart.contacts[i][k])
							{
								debugMeshContext.addLine(debugContact.carPoint, debugContact.staticPoint, aoegl::k_yellow);
								debugMeshContext.addLine(debugContact.staticPoint, debugContact.carPoint + debugContact.force, aoegl::k_red);
								debugMeshContext.addLine(debugContact.carPoint, debugContact.carPoint + debugContact.torque, aoegl::k_blue);
							}
						}
					}
				}
			}

			for (auto const& wheel : carCollider.wheels)
			{
				if (k_debugCarCollider)
				{
					auto const wheelPositionLocal = wheel.suspensionAttachPosition + wheel.rotation * glm::vec3{ 0.0f, -wheel.suspensionLength, 0.0f };
					debugMeshContext.addEllipsoid(position + rotation * wheelPositionLocal, rotation * wheel.rotation, wheel.radiuses, aoegl::k_green);
				}

				for (int i = 0; i < k_debugCarContacts.size(); ++i)
				{
					for (int k = 0; k < 4; ++k)
					{
						if (!k_debugCarContacts[i][k])
						{
							continue;
						}

						if (k_debugCarChassisContacts)
						{
							for (auto const& debugContact : wheel.chassisContacts[i][k])
							{
								debugMeshContext.addLine(debugContact.carPoint, debugContact.staticPoint, aoegl::k_yellow);
								debugMeshContext.addLine(debugContact.carPoint, debugContact.carPoint + debugContact.force, aoegl::k_red);
								debugMeshContext.addLine(debugContact.carPoint, debugContact.carPoint + debugContact.torque, aoegl::k_blue);
							}
						}
						if (k_debugCarWheelContacts)
						{
							for (auto const& debugContact : wheel.contacts[i][k])
							{
								debugMeshContext.addLine(debugContact.carPoint, debugContact.staticPoint, aoegl::k_yellow);
								debugMeshContext.addLine(debugContact.carPoint, debugContact.carPoint + debugContact.force, aoegl::k_red);
								debugMeshContext.addLine(debugContact.carPoint, debugContact.carPoint + debugContact.torque, aoegl::k_blue);
							}
						}
					}
				}
			}
		}
	}
}
