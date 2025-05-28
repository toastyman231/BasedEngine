#pragma once

namespace based::ui
{
	class ElementBinding
	{
	public:
		ElementBinding(Rml::Element* element, std::function<void(Rml::Element*)> binding);

		void ResolveBinding() const;

		[[nodiscard]] bool IsValid() const { return mBoundElement; }
	private:
		Rml::Element* mBoundElement;
		std::function<void(Rml::Element*)> mBinding;
	};
}