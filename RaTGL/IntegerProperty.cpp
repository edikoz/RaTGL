#include "stdafx.h"
#include "PropertyView.h"

IntegerProperty::IntegerProperty(const wchar_t* name, const int &defaultValue) : PropertyTemplate(defaultValue) {
	//
	displayValue();
}

void IntegerProperty::displayValue() const {
	//SendMessage(propertyHWND, BM_SETCHECK, (WPARAM)(value ? BST_CHECKED : BST_UNCHECKED), NULL);
}
