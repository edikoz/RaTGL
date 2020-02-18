#pragma once
#include "RaTwindow.h"
#include "RaTElement.h"

//TODO: WS_EX_ACCEPTFILES 

class PropertiesView final : public RaTwindow {
	friend class RaTElement;
	friend class CameraElement;

	std::list<RaTElement*> elements;
	static LRESULT CALLBACK proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	int scrollY;

	void resize(int w, int h);
	void update();
	void updateScroll();
	void updateElements();
	void generateShader();
	void generateBuffer();

	HBITMAP hbmpUnroll, hbmpRoll, hbmpDelete, hbmpDeleteDisabled, hbmpInsert, hbmpInsertDisabled;

	static bool isFirst(std::list<RaTElement*> *l, RaTElement *cmp);
	static bool isLast(std::list<RaTElement*> *l, RaTElement *cmp);

public:
	static PropertiesView *propertiesView;
	HFONT hFont;
	PropertiesView(HWND parent, Dims dim);

	void clearElements();
	void generate();
	static void drawElements(Camera *camera);

	int getNumDots();
	EmitterElement* getEmitter();
	CameraElement* getSensor();
};
