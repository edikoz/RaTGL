#include "stdafx.h"
#include "PropertyView.h"

FloatProperty::FloatProperty(const wchar_t* name, const float &defaultValue) : PropertyTemplate(defaultValue) {
	//
	displayValue();
}

void FloatProperty::displayValue() const {
	//SendMessage(propertyHWND, BM_SETCHECK, (WPARAM)(value ? BST_CHECKED : BST_UNCHECKED), NULL);
}
