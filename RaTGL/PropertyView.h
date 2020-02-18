#pragma once

class PropertyView {
protected:
	PropertyView();
	HWND propertyHWND;
};

//Property template
template<typename T>
class PropertyTemplate : PropertyView {
protected:
	T value;
	PropertyTemplate(const T &defaultValue) : value(defaultValue) { };
	virtual void displayValue() const = 0;

public:
	typedef T valueT;

	T getValue() const {
		return value;
	}

	void setValue(const T &newValue) {
		value = newValue;
		displayValue();
	}

	std::string getText() {
		return std::to_string(value);
	}
};

//Array of properties template
template<typename T>
class ArrayProperty : PropertyView {
	typedef typename T::valueT valueT;

	T propertyView;
	std::vector<valueT> valueArray;
	HWND numIndex, numCount;

public:
	ArrayProperty(const wchar_t* name, int length) : propertyView(name, valueT{ 0 }) {
		valueArray.resize(length, valueT{ 0 });
	}

	valueT getValue(int index) const {
		return valueArray[index];
	}
	int length() const {
		return valueArray.size();
	}
};

//Properties from template

class BooleanProperty : public PropertyTemplate<bool> {
	void displayValue() const override;

public:
	BooleanProperty(const wchar_t* name, const bool &defaultValue);
};

class IntegerProperty : public PropertyTemplate<int> {
	void displayValue() const override;

public:
	IntegerProperty(const wchar_t* name, const int &defaultValue);
};

class FloatProperty : public PropertyTemplate<float> {
	void displayValue() const override;

public:
	FloatProperty(const wchar_t* name, const float &defaultValue);
};

class EnumProperty : public PropertyTemplate<int> {
	void displayValue() const override;

public:
	EnumProperty(const wchar_t* name, const int &defaultValue);

};

class StringProperty : public PropertyTemplate<std::string> {
	void displayValue() const override;

public:
	StringProperty(const wchar_t* name, const std::string &defaultValue);
};

//Group Property
/*
class GroupProperty : public PropertyTemplate<int> {
	EnumProperty enumProperty;
	std::vector<std::vector<PropertyView>> tabs;

	void displayValue() const override;

public:
	GroupProperty(const wchar_t* name, const bool &defaultValue);
};
*/