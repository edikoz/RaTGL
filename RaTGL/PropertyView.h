#pragma once

class PropertyView {
protected:
	PropertyView();
	HWND propertyHWND;
};

template<typename T>
class PropertyTemplate : public PropertyView {
protected:
	PropertyTemplate(const T &defaultValue);
	T value;
	virtual void displayValue() const = 0;

public:
	T getValue() const;
	void setValue(const T &newValue);
};

class BooleanProperty : PropertyTemplate<bool> {
	void displayValue() const override;

public:
	BooleanProperty(const wchar_t* name, const bool &defaultValue);
};

class IntegerProperty : PropertyTemplate<int> {
	void displayValue() const override;

public:
	IntegerProperty(const wchar_t* name, const int &defaultValue);
};

class FloatProperty : PropertyTemplate<float> {
	void displayValue() const override;

public:
	FloatProperty(const wchar_t* name, const float &defaultValue);
};

class EnumProperty : PropertyTemplate<int> {
	void displayValue() const override;

public:
	EnumProperty(const wchar_t* name, const int &defaultValue);

};

class StringProperty : PropertyTemplate<std::string> {
	void displayValue() const override;

public:
	StringProperty(const wchar_t* name, const std::string &defaultValue);
};

/*
template<template<typename> class T = PropertyTemplate>
class ArrayProperty : PropertyView {
	std::vector<PropertyTemplate<T>*> valueArray;

public:
	ArrayProperty();

	T getValue(int index) const;
	int length() const;
};


template<template<typename> class T>
ArrayProperty<T>::ArrayProperty() {

}
template<template<typename> class T>
T ArrayProperty<T>::getValue(int index) const {
	return T::value;
}

template<template<typename> class T>
int ArrayProperty<T>::length() const {
	return valueArray.size();
}
*/

typedef int tIType;
typedef float tFType;

class ArrayIntProperty : PropertyView {
	std::vector<tIType> valueArray;

public:
	ArrayIntProperty(int length);

	tIType getValue(int index) const;
	int length() const;
};

class ArrayFloatProperty : PropertyView {
	std::vector<tFType> valueArray;

public:
	ArrayFloatProperty(int length);

	tFType getValue(int index) const;
	int length() const;
};
