#include "stdafx.h"
#include "PropertyView.h"

BooleanProperty::BooleanProperty(const wchar_t* name, const bool &defaultValue) : PropertyTemplate(defaultValue) {
	/*HINSTANCE hInst = RaTwindow::getInstance();
	HWND ret = CreateWindow(L"button", name, MWS_CHK, 0, hRoll, w - 2 * prop_pad, prop_height, props, group, hInst, 0);
	SendMessage(ret, BM_SETCHECK, (WPARAM)(initVal ? BST_CHECKED : BST_UNCHECKED), NULL);
	hRoll += prop_shift;
	return ret;*/

	displayValue();
}

void BooleanProperty::displayValue() const {
	//SendMessage(propertyHWND, BM_SETCHECK, (WPARAM)(value ? BST_CHECKED : BST_UNCHECKED), NULL);
}
