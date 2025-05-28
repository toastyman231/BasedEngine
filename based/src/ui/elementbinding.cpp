#include "pch.h"
#include "ui/elementbinding.h"

based::ui::ElementBinding::ElementBinding(Rml::Element* element, std::function<void(Rml::Element*)> binding)
{
	mBoundElement = element;
	mBinding = binding;
}

void based::ui::ElementBinding::ResolveBinding() const
{
	mBinding(mBoundElement);
}
