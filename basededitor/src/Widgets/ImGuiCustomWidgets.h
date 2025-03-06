#pragma once
#include "external/imgui/imgui.h"

namespace based::graphics
{
	class Mesh;
	class Material;
}

namespace ImGui
{
	using namespace based;

	template <typename Type>
	std::shared_ptr<Type> ObjectPicker(const std::string& label, std::shared_ptr<Type> current) = delete;

	bool ToggleButton(uint32_t textureId, ImVec2 size, bool isSelected);

	// Explicit specialization declarations
	template <>
	std::shared_ptr<graphics::Material> ObjectPicker<graphics::Material>(
		const std::string& label, std::shared_ptr<graphics::Material> current);

	template <>
	std::shared_ptr<graphics::Mesh> ObjectPicker<graphics::Mesh>(
		const std::string& label, std::shared_ptr<graphics::Mesh> current);

}
