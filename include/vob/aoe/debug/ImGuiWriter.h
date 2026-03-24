#pragma once

#include "vob/misc/visitor/is_visitable.h"
#include "vob/misc/visitor/name_value_pair.h"

#include <glm/glm.hpp>

#include <imgui.h>

#include <array>
#include <deque>
#include <stack>
#include <type_traits>


namespace vob::aoedb
{
	class ImGuiWriter
	{
		template <size_t t_maxSize = 32>
		std::array<char, t_maxSize + 1> toSmallString(std::string_view a_stringView)
		{
			std::array<char, t_maxSize + 1> smallString;
			auto const size = std::min(a_stringView.size(), t_maxSize);
			std::memcpy(smallString.data(), a_stringView.data(), size);
			smallString[size] = 0;
			return smallString;
		}

	public:
		template <typename TValue>
		bool visit(misvi::name_value_pair<TValue> a_nameValuePair)
		{
			m_scopeNames.push(a_nameValuePair.name);
			auto const result = visit(a_nameValuePair.value);
			m_scopeNames.pop();
			return result;
		}

		template <typename TValue>
		requires misvi::is_visitable_free<ImGuiWriter, TValue> && (!std::is_arithmetic_v<TValue>)
		bool visit(TValue& a_value)
		{
			auto const scopeName = toSmallString(m_scopeNames.top());
			if (ImGui::CollapsingHeader(scopeName.data()))
			{
				ImGui::Indent();
				auto const result = vob::misvi::accept(*this, a_value);
				ImGui::Unindent();
				ImGui::Separator();
				return result;
			}
			return true;
		}

		template <typename TValue>
		requires misvi::is_visitable_member<ImGuiWriter, TValue>
		bool visit(TValue& a_value)
		{
			auto const scopeName = toSmallString(m_scopeNames.top());
			if (ImGui::CollapsingHeader(scopeName.data()))
			{
				ImGui::Indent();
				auto const result = a_value.accept(*this);
				ImGui::Unindent();
				ImGui::Separator();
				return result;
			}
			return true;
		}

		template <typename TValue>
		requires misvi::is_visitable_static<ImGuiWriter, TValue>
		bool visit(TValue& a_value)
		{
			auto const scopeName = toSmallString(m_scopeNames.top());
			if (ImGui::CollapsingHeader(scopeName.data()))
			{
				ImGui::Indent();
				auto const result = TValue::accept(*this, a_value);
				ImGui::Unindent();
				ImGui::Separator();
				return result;
			}
			return true;
		}

		bool visit(bool& a_boolean)
		{
			auto const scopeName = toSmallString(m_scopeNames.top());
			ImGui::Checkbox(scopeName.data(), &a_boolean);
			return true;
		}

		bool visit(int32_t& a_integer)
		{
			auto const scopeName = toSmallString(m_scopeNames.top());
			ImGui::InputInt(scopeName.data(), &a_integer);
			return true;
		}

		bool visit(glm::ivec1& a_integerVector)
		{
			auto const scopeName = toSmallString(m_scopeNames.top());
			ImGui::InputInt(scopeName.data(), &a_integerVector.x);
			return true;
		}

		bool visit(glm::ivec2& a_integerVector)
		{
			auto const scopeName = toSmallString(m_scopeNames.top());
			ImGui::InputInt2(scopeName.data(), &a_integerVector.x);
			return true;
		}

		bool visit(glm::ivec3& a_integerVector)
		{
			auto const scopeName = toSmallString(m_scopeNames.top());
			ImGui::InputInt3(scopeName.data(), &a_integerVector.x);
			return true;
		}

		bool visit(glm::ivec4& a_integerVector)
		{
			auto const scopeName = toSmallString(m_scopeNames.top());
			ImGui::InputInt4(scopeName.data(), &a_integerVector.x);
			return true;
		}

		bool visit(float& a_float)
		{
			auto const scopeName = toSmallString(m_scopeNames.top());
			ImGui::InputFloat(scopeName.data(), &a_float);
			return true;
		}

		bool visit(glm::vec1& a_floatVector)
		{
			auto const scopeName = toSmallString(m_scopeNames.top());
			ImGui::InputFloat(scopeName.data(), &a_floatVector.x);
			return true;
		}

		bool visit(glm::vec2& a_floatVector)
		{
			auto const scopeName = toSmallString(m_scopeNames.top());
			ImGui::InputFloat2(scopeName.data(), &a_floatVector.x);
			return true;
		}

		bool visit(glm::vec3& a_floatVector)
		{
			auto const scopeName = toSmallString(m_scopeNames.top());
			ImGui::InputFloat3(scopeName.data(), &a_floatVector.x);
			return true;
		}

		bool visit(glm::vec4& a_floatVector)
		{
			auto const scopeName = toSmallString(m_scopeNames.top());
			ImGui::InputFloat4(scopeName.data(), &a_floatVector.x);
			return true;
		}

	private:
		std::stack<std::string_view, std::deque<std::string_view>> m_scopeNames;
	};
}
