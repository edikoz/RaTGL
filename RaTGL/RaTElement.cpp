#include "stdafx.h"
#include "RaTElement.h"
#include "PropertiesView.h"

#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#define IN_RANGE(x,a,b) ((x) > (a) && (x) < (b))

//Static members
const int RaTElement::default_element_height = 48;
const int RaTElement::prop_height = 20;
const int RaTElement::prop_pad = 4;
const int RaTElement::prop_shift = prop_height + prop_pad;
const int RaTElement::but_size = 16;
const double RaTElement::fibRec = 0.62;
const double RaTElement::fib = 0.38;

int EmitterElement::count = 0;
int LensElement::count = 0;
int MirrorElement::count = 0;
int CameraElement::count = 0;

std::string RaTElement::getNextColor(int ci) {
	std::string colors[] = {
		"vec4(1.0, 0.0, 0.0, 1.0)", "vec4(0.0, 1.0, 0.0, 1.0)", "vec4(0.0, 0.0, 1.0, 1.0)",
		"vec4(1.0, 1.0, 0.0, 1.0)", "vec4(1.0, 0.0, 1.0, 1.0)", "vec4(0.0, 1.0, 1.0, 1.0)",
		"vec4(1.0, 1.0, 1.0, 1.0)" };
	static int i = 0;
	if (ci >= 0) i = ci;
	if (i >= sizeofArray(colors)) i = 0;
	return colors[i++];
}

LRESULT CALLBACK RaTElement::proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static HWND dragHWND = NULL;
	static std::list<RaTElement*>::iterator draggedIt = {};
	static bool drag = false;
	static std::list<RaTElement*> *elements = &(PropertiesView::propertiesView->elements);

	switch (message)
	{
	case WM_ERASEBKGND:
		if (hWnd == dragHWND) {
			RECT rc;
			HDC hdc = (HDC)wParam;
			HBRUSH gray = CreateSolidBrush(RGB(220, 220, 220));
			GetClientRect(hWnd, &rc);
			FillRect(hdc, &rc, gray);
			DeleteObject(gray);
			return 1L;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	case WM_LBUTTONDOWN:
		for (auto it = elements->begin(); it != elements->end(); ++it) {
			RaTElement *e = *it;
			if (!PropertiesView::isFirst(elements, e) && !PropertiesView::isLast(elements, e) && e->isCapsule(hWnd))
			{
				SetCapture(e->capsule);
				dragHWND = e->props;// hWnd;
				draggedIt = it;
				drag = true;
				InvalidateRect(e->props, NULL, TRUE);
				break;
			}
		}
		break;
	case WM_MOUSEMOVE:
		if ((wParam & MK_LBUTTON) && drag) {
			POINT p;
			GetCursorPos(&p);
			ScreenToClient(PropertiesView::propertiesView->getHWND(), &p);

			RaTElement* dragged = *draggedIt;
			for (auto it = elements->begin(); it != elements->end(); ++it) {
				RaTElement *e = *it;
				if (e != dragged
					&& !PropertiesView::isFirst(elements, e) && !PropertiesView::isLast(elements, e)
					&& IN_RANGE(p.y, e->y, e->y + e->h))
				{
					if (dragged->y < e->y) {
						if (!IN_RANGE(p.y, e->y, e->y + e->h - dragged->h)) {
							elements->erase(draggedIt);
							it++;
							draggedIt = elements->insert(it, dragged);
							PropertiesView::propertiesView->updateElements();
						}
					}
					else {
						if (!IN_RANGE(p.y, e->y + dragged->h, e->y + e->h)) {
							elements->erase(draggedIt);
							draggedIt = elements->insert(it, dragged);
							PropertiesView::propertiesView->updateElements();
						}
					}
					break;
				}
			}
		}
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
		break;
	case WM_CAPTURECHANGED:
		if (dragHWND) InvalidateRect(dragHWND, NULL, TRUE);
		dragHWND = NULL;
		drag = false;
		break;
	case WM_COMMAND:
	{
		RaTElement *e = reinterpret_cast<RaTElement*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		switch ((BUT_ENUM)LOWORD(wParam)) {
		case IDC_CHANGE_ROLL:
			e->changeRoll();
			PropertiesView::propertiesView->update();
			break;
		case IDC_BUT_ADD:
			if (!PropertiesView::isLast(elements, e) && elements->size() < 10) {
				elements->insert(++(e->iterToSelf), new LensElement());
				PropertiesView::propertiesView->update();
			}
			break;
		case IDC_BUT_DEL:
			if (!PropertiesView::isFirst(elements, e) && !PropertiesView::isLast(elements, e)) {
				elements->erase(e->iterToSelf);
				delete e;
				PropertiesView::propertiesView->update();
			}
			break;
		case IDC_CMB_TYPE:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				if (!PropertiesView::isFirst(elements, e) && !PropertiesView::isLast(elements, e)) {
					int type = (int)SendMessage(e->cmbType, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
					if (type != CB_ERR && type != e->element_type && type != EMITTER && type != CAMERA) {
						bool roll = e->bRoll;
						auto it = elements->erase(e->iterToSelf);
						delete e;
						switch (type) {
						case LENS: elements->insert(it, new LensElement()); break;
						case MIRROR: elements->insert(it, new MirrorElement()); break;
						}
						if (roll) (*--it)->changeRoll();
						PropertiesView::propertiesView->update();
					}
					else SendMessage(e->cmbType, CB_SETCURSEL, (WPARAM)e->element_type, (LPARAM)0);
				}
				else SendMessage(e->cmbType, CB_SETCURSEL, (WPARAM)e->element_type, (LPARAM)0);
			}
			break;
		case IDC_LENSTYPE:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				static_cast<LensElement*>(e)->changeLensTypeProps();
				PropertiesView::propertiesView->update();
			}
			break;
		case IDC_MIRRORTYPE:
			static_cast<MirrorElement*>(e)->changeSphericalProps();
			PropertiesView::propertiesView->update();
			break;
		case IDC_ARR_NUM:
			if (HIWORD(wParam) == EN_UPDATE)
				static_cast<LensElement*>(e)->changeLensArrayNumProps();	//FIXME danger! e can be not lens
			break;
		case IDC_ARR_IND:
			if (HIWORD(wParam) == EN_UPDATE)
				static_cast<LensElement*>(e)->changeLensArrayIndProps();	//FIXME danger! e can be not lens
			break;
		case IDC_ARR_CNT:
			if (HIWORD(wParam) == EN_UPDATE)
				static_cast<LensElement*>(e)->changeLensArrayCntProps();	//FIXME danger! e can be not lens
			break;
		}
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void RaTElement::regClass() {
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = proc;
	wcex.hInstance = PropertiesView::hInst;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = L"RaTelement";
	RegisterClassEx(&wcex);
}

RaTElement::RaTElement(std::wstring name, ELEMENT_TYPE e_type) : element_type(e_type) {
	HINSTANCE hInst = PropertiesView::hInst;
	w = PropertiesView::propertiesView->dims.w;
	capsule = CreateWindow(L"RaTelement", L"", WS_VISIBLE | WS_CHILD, 0, 0, w, h, PropertiesView::propertiesView->getHWND(), 0, hInst, 0);
	SetWindowLongPtr(capsule, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	eName = CreateWindow(L"edit", name.c_str(), MWS_EDT, prop_pad, prop_pad, w - (but_size + 2 * prop_pad), prop_height, capsule, 0, hInst, 0);

	bRoll = CreateWindow(L"button", L"", MWS_BUT, w - (but_size + prop_pad), prop_pad, but_size, but_size, capsule, (HMENU)IDC_CHANGE_ROLL, hInst, 0);
	SendMessage(bRoll, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)PropertiesView::propertiesView->hbmpUnroll);
	if (e_type == CAMERA) {
		bAdd = CreateWindow(L"button", L"", MWS_BUT, w - 2 * (but_size + prop_pad), h - (but_size + prop_pad), but_size, but_size, capsule, (HMENU)IDC_DISABLED, hInst, 0);
		SendMessage(bAdd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)PropertiesView::propertiesView->hbmpInsertDisabled);
	}
	else {
		bAdd = CreateWindow(L"button", L"", MWS_BUT, w - 2 * (but_size + prop_pad), h - (but_size + prop_pad), but_size, but_size, capsule, (HMENU)IDC_BUT_ADD, hInst, 0);
		SendMessage(bAdd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)PropertiesView::propertiesView->hbmpInsert);
	}

	props = CreateWindow(L"RaTelement", L"", WS_CHILD, prop_pad, prop_height + 2 * prop_pad, w - 2 * prop_pad, 0, capsule, 0, hInst, 0);
	SetWindowLongPtr(props, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	if (e_type == EMITTER || e_type == CAMERA) {
		bDel = CreateWindow(L"button", L"", MWS_BUT, w - (but_size + prop_pad), h - (but_size + prop_pad), but_size, but_size, capsule, (HMENU)IDC_DISABLED, hInst, 0);
		SendMessage(bDel, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)PropertiesView::propertiesView->hbmpDeleteDisabled);

		cmbType = NULL;
	}
	else {
		bDel = CreateWindow(L"button", L"", MWS_BUT, w - (but_size + prop_pad), h - (but_size + prop_pad), but_size, but_size, capsule, (HMENU)IDC_BUT_DEL, hInst, 0);
		SendMessage(bDel, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)PropertiesView::propertiesView->hbmpDelete);

		cmbType = CreateWindow(L"combobox", L"", MWS_CMB, 0, 0, w - 2 * prop_pad, prop_height, props, (HMENU)IDC_CMB_TYPE, hInst, 0);
		const std::wstring types[] = { L"Преломляющая поверхность", L"Отражающая поверхность" };
		for (std::wstring element_type : types)
			SendMessage(cmbType, CB_ADDSTRING, (WPARAM)0, (LPARAM)element_type.c_str());
		SendMessage(cmbType, CB_SETCURSEL, (WPARAM)element_type, (LPARAM)0);
		hRoll += prop_shift;
	}

	separator = CreateWindow(L"static", L"", MWS_SPR, prop_pad, h - 2, w - 2 * prop_pad, 1, capsule, 0, hInst, 0);
}

BOOL CALLBACK SetFont(HWND child, LPARAM font) {
	SendMessage(child, WM_SETFONT, font, true);
	return TRUE;
}
void RaTElement::capsuleInit() {
	EnumChildWindows(capsule, (WNDENUMPROC)SetFont, (LPARAM)PropertiesView::propertiesView->hFont);
}

HWND RaTElement::insertNumDoubleProp(const wchar_t* name, double initVal) {
	HINSTANCE hInst = PropertiesView::hInst;
	wchar_t initValStr[32];
	swprintf_s(initValStr, L"%1.3f", initVal);
	HWND ret = CreateWindow(L"edit", initValStr, MWS_DBL, 0, hRoll, w * fib - 2 * prop_pad, prop_height, props, 0, hInst, 0);
	HWND label = CreateWindow(L"static", name, MWS_LBL, w * fib, hRoll, w * fibRec - 2 * prop_pad, prop_height, props, 0, hInst, 0);
	hRoll += prop_shift;
	return ret;
}
HWND RaTElement::insertNumIntProp(const wchar_t* name, int initVal) {
	HINSTANCE hInst = PropertiesView::hInst;
	wchar_t initValStr[32];
	swprintf_s(initValStr, L"%d", initVal);
	HWND ret = CreateWindow(L"edit", initValStr, MWS_INT, 0, hRoll, w * fib - 2 * prop_pad, prop_height, props, 0, hInst, 0);
	HWND label = CreateWindow(L"static", name, MWS_LBL, w * fib, hRoll, w * fibRec - 2 * prop_pad, prop_height, props, 0, hInst, 0);
	hRoll += prop_shift;
	return ret;
}
HWND RaTElement::insertButBooleanProp(const wchar_t* name, bool initVal, HMENU group) {
	HINSTANCE hInst = PropertiesView::hInst;
	HWND ret = CreateWindow(L"button", name, MWS_CHK, 0, hRoll, w - 2 * prop_pad, prop_height, props, group, hInst, 0);
	SendMessage(ret, BM_SETCHECK, (WPARAM)(initVal ? BST_CHECKED : BST_UNCHECKED), NULL);
	hRoll += prop_shift;
	return ret;
}
HWND RaTElement::insertComboBoxProp(const wchar_t* name, const std::wstring* initVal, size_t count, HMENU group, int initIndex) {
	HINSTANCE hInst = PropertiesView::hInst;
	HWND ret = CreateWindow(L"combobox", name, MWS_CMB, 0, hRoll, w - 2 * prop_pad, prop_height, props, group, hInst, 0);
	for (size_t i = 0; i < count; ++i)
		SendMessage(ret, CB_ADDSTRING, (WPARAM)0, (LPARAM)initVal[i].c_str());
	SendMessage(ret, CB_SETCURSEL, (WPARAM)initIndex, (LPARAM)0);
	hRoll += prop_shift;
	return ret;
}
RaTElement::HWNDArray RaTElement::insertNumDoubleArrayProp(const wchar_t* name, double initVal, int count) {
	HINSTANCE hInst = PropertiesView::hInst;
	wchar_t initValStr[32] = L"0.0";
	swprintf_s(initValStr, L"%1.1f", initVal);
	HWND num = CreateWindow(L"edit", initValStr, MWS_DBL, 0, hRoll, w * fib - 2 * prop_pad, prop_height, props, (HMENU)IDC_ARR_NUM, hInst, 0);
	HWND label1 = CreateWindow(L"static", name, MWS_LBL, w * fib, hRoll, w * fibRec - 2 * prop_pad, prop_height, props, 0, hInst, 0);
	hRoll += prop_shift;
	HWND numIndex = CreateWindow(L"edit", L"0", MWS_INT, 0, hRoll, w / 6 - 2 * prop_pad, prop_height, props, (HMENU)IDC_ARR_IND, hInst, 0);
	HWND label2 = CreateWindow(L"static", L"Индекс", MWS_LBL, w / 6, hRoll, 2 * w / 6 - 2 * prop_pad, prop_height, props, 0, hInst, 0);
	swprintf_s(initValStr, L"%d", count);
	HWND numCount = CreateWindow(L"edit", initValStr, MWS_INT, w / 2, hRoll, w / 6 - 2 * prop_pad, prop_height, props, (HMENU)IDC_ARR_CNT, hInst, 0);
	HWND label3 = CreateWindow(L"static", L"Степень", MWS_LBL, w / 6 + w / 2, hRoll, 2 * w / 6 - 2 * prop_pad, prop_height, props, 0, hInst, 0);
	hRoll += prop_shift;
	return { num, numIndex, numCount };
}

void RaTElement::hideProp(HWND prop) {
	if (IsWindowEnabled(prop)) {
		HWND propLabel = GetWindow(prop, GW_HWNDNEXT);

		EnableWindow(prop, FALSE);
		EnableWindow(propLabel, FALSE);

		SetWindowPos(propLabel, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
		SetWindowPos(prop, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);

		hRoll -= prop_shift;
	}
}
void RaTElement::showProp(HWND prop) {
	if (!IsWindowEnabled(prop)) {
		RECT rect;
		HWND prev = GetWindow(prop, GW_HWNDLAST);
		HWND propLabel = GetWindow(prop, GW_HWNDNEXT);

		EnableWindow(prop, TRUE);
		EnableWindow(propLabel, TRUE);

		GetLocalRect(prev, &rect);

		SetWindowPos(prop, HWND_BOTTOM, 0, rect.bottom + prop_pad, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		SetWindowPos(propLabel, HWND_BOTTOM, w * fib, rect.bottom + prop_pad, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);

		hRoll += prop_shift;
	}
}
void RaTElement::hidePropArray(HWNDArray propA) {
	if (IsWindowEnabled(propA.num)) {
		hideProp(propA.numCount);
		hideProp(propA.numIndex);
		hideProp(propA.num);

		hRoll += prop_shift;
	}
}
void RaTElement::showPropArray(HWNDArray prop) {
	if (!IsWindowEnabled(prop.num)) {
		RECT rect;
		HWND prev = GetWindow(prop.num, GW_HWNDLAST);
		HWND propALabel = GetWindow(prop.num, GW_HWNDNEXT);
		HWND propILabel = GetWindow(prop.numIndex, GW_HWNDNEXT);
		HWND propCLabel = GetWindow(prop.numCount, GW_HWNDNEXT);

		EnableWindow(prop.num, TRUE);
		EnableWindow(propALabel, TRUE);
		EnableWindow(prop.numIndex, TRUE);
		EnableWindow(propILabel, TRUE);
		EnableWindow(prop.numCount, TRUE);
		EnableWindow(propCLabel, TRUE);

		GetLocalRect(prev, &rect);

		SetWindowPos(prop.num, HWND_BOTTOM, 0, rect.bottom + prop_pad, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		SetWindowPos(propALabel, HWND_BOTTOM, w * fib, rect.bottom + prop_pad, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);

		SetWindowPos(prop.numIndex, HWND_BOTTOM, 0, rect.bottom + prop_shift + prop_pad, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		SetWindowPos(propILabel, HWND_BOTTOM, w / 6, rect.bottom + prop_shift + prop_pad, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);

		SetWindowPos(prop.numCount, HWND_BOTTOM, w / 2, rect.bottom + prop_shift + prop_pad, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		SetWindowPos(propCLabel, HWND_BOTTOM, w / 6 + w / 2, rect.bottom + prop_shift + prop_pad, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);

		hRoll += 2 * prop_shift;
	}
}

void RaTElement::changeRoll() {
	roll = !roll;
	SendMessage(bRoll, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)(roll ? PropertiesView::propertiesView->hbmpRoll : PropertiesView::propertiesView->hbmpUnroll));
	ShowWindow(props, roll ? SW_SHOW : SW_HIDE);
}

void RaTElement::resize(int newY, int newW) {
	y = newY; w = newW; h = getH();
	SetWindowPos(capsule, NULL, 0, y, w, h, SWP_NOZORDER);
	SetWindowPos(props, NULL, 0, 0, w - 2 * prop_pad, hRoll, SWP_NOMOVE);
	SetWindowPos(bRoll, NULL, w - 20, 4, 16, 16, SWP_NOZORDER);
	SetWindowPos(bAdd, NULL, w - 40, h - 20, 16, 16, SWP_NOZORDER);
	SetWindowPos(bDel, NULL, w - 20, h - 20, 16, 16, SWP_NOZORDER);
	SetWindowPos(separator, NULL, 4, h - 2, w - 8, 1, SWP_NOZORDER);
	InvalidateRect(capsule, NULL, TRUE);
}

int RaTElement::getH() {
	return (roll) ? default_element_height + hRoll : default_element_height;
}

void RaTElement::setIterToSelf(const_iter iterToSelf) {
	this->iterToSelf = iterToSelf;
}

bool RaTElement::isCapsule(HWND hWnd) {
	return capsule == hWnd || props == hWnd;
}

bool RaTElement::cmpDistance(const RaTElement* a, const RaTElement* b) {
	return (a->distance) > (b->distance);
}

std::string RaTElement::getShader() {
	return std::string("");
}
void RaTElement::obtainGLresources() {}
void RaTElement::releaseGLresources() {}
void RaTElement::draw(Camera *camera) {}
void RaTElement::calcDistanceTo(Vector3 pos) {}

std::string RaTElement::save() const {
	return "";
}
void RaTElement::load(std::string elementString) const { }


RaTElement::~RaTElement() {
	PostMessage(capsule, WM_CLOSE, 0, 0);
	releaseGLresources();
}
