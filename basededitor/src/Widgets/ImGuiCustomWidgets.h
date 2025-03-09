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

	bool TransformEditor(
		float pos[3], 
		float oldPos[3],
		float rot[3],
		float oldRot[3],
		float scale[3],
		float oldScale[3],
		float speed = 1.f);

	// Explicit specialization declarations
	template <>
	std::shared_ptr<graphics::Material> ObjectPicker<graphics::Material>(
		const std::string& label, std::shared_ptr<graphics::Material> current);

	template <>
	std::shared_ptr<graphics::Mesh> ObjectPicker<graphics::Mesh>(
		const std::string& label, std::shared_ptr<graphics::Mesh> current);

}
