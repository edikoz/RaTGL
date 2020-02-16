#include "stdafx.h"
#include "PropertyView.h"
#include "RaTwindow.h"

PropertyView::PropertyView() {

}


template<typename T>
PropertyTemplate<T>::PropertyTemplate(const T &defaultValue) : value(defaultValue) { }

template<typename T>
T PropertyTemplate<T>::getValue() const {
	return value;
}

template<typename T>
void PropertyTemplate<T>::setValue(const T &newValue) {
	value = newValue;
	displayValue();
}


BooleanProperty::BooleanProperty(const wchar_t* name, const bool &defaultValue) : PropertyTemplate(defaultValue) {
	/*HINSTANCE hInst = RaTwindow::getInstance();
	HWND ret = CreateWindow(L"button", name, MWS_CHK, 0, hRoll, w - 2 * prop_pad, prop_height, props, group, hInst, 0);
	SendMessage(ret, BM_SETCHECK, (WPARAM)(initVal ? BST_CHECKED : BST_UNCHECKED), NULL);
	hRoll += prop_shift;
	return ret;*/

	displayValue();
}

void BooleanProperty::displayValue() const {
	SendMessage(propertyHWND, BM_SETCHECK, (WPARAM)(value ? BST_CHECKED : BST_UNCHECKED), NULL);
}


