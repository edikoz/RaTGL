#include "stdafx.h"
#include "PropertyView.h"

EnumProperty::EnumProperty(const wchar_t* name, const int &defaultValue) : PropertyTemplate(defaultValue) {
	//
	displayValue();
}

void EnumProperty::displayValue() const {
	//SendMessage(propertyHWND, BM_SETCHECK, (WPARAM)(value ? BST_CHECKED : BST_UNCHECKED), NULL);
}
