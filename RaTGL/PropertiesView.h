#pragma once
#include "RaTwindow.h"
#include "RaTElement.h"

//TODO: WS_EX_ACCEPTFILES 

class PropertiesView final : public RaTwindow {
	friend class RaTElement;
	friend class CameraElement;

	HBITMAP hbmpUnroll, hbmpRoll, hbmpDelete, hbmpDeleteDisabled, hbmpInsert, hbmpInsertDisabled;
	std::list<RaTElement*> elements;
	int scrollY;

	static bool isFirst(std::list<RaTElement*> *l, RaTElement *cmp);
	static bool isLast(std::list<RaTElement*> *l, RaTElement *cmp);

	LRESULT CALLBACK handleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
	void resize(int w, int h) override;

	void update();
	void updateScroll();
	void updateElements();
	void generateShader();
	void generateBuffer();

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
