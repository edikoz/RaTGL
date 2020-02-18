#include "stdafx.h"
#include "PropertyView.h"

StringProperty::StringProperty(const wchar_t* name, const std::string &defaultValue) : PropertyTemplate(defaultValue) {
	//
	displayValue();
}

void StringProperty::displayValue() const {
	//SendMessage(propertyHWND, BM_SETCHECK, (WPARAM)(value ? BST_CHECKED : BST_UNCHECKED), NULL);
}
