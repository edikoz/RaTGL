#include "stdafx.h"
#include "Properties.h"

#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#define IN_RANGE(x,a,b) ((x) > (a) && (x) < (b))

std::string getNextColor(int ci = -1) {
	std::string colors[] = {
		"vec4(1.0, 0.0, 0.0, 1.0)", "vec4(0.0, 1.0, 0.0, 1.0)", "vec4(0.0, 0.0, 1.0, 1.0)",
		"vec4(1.0, 1.0, 0.0, 1.0)", "vec4(1.0, 0.0, 1.0, 1.0)", "vec4(0.0, 1.0, 1.0, 1.0)",
		"vec4(1.0, 1.0, 1.0, 1.0)" };
	static int i = 0;
	if (ci >= 0) i = ci;
	if (i >= sizeofArray(colors)) i = 0;
	return colors[i++];
}

const int Properties::element::default_element_height = 48;
const int Properties::element::prop_height = 20;
const int Properties::element::prop_pad = 4;
const int Properties::element::prop_shift = prop_height + prop_pad;
const int Properties::element::but_size = 16;
const double Properties::element::fibRec = 0.62;
const double Properties::element::fib = 0.38;

int Properties::emitter::count = 0;
int Properties::lens::count = 0;
int Properties::mirror::count = 0;
int Properties::camera::count = 0;

enum BUT_ENUM : int {
	IDC_CHANGE_ROLL = 1000, IDC_BUT_ADD, IDC_BUT_DEL, IDC_CMB_TYPE, IDC_LENSTYPE, IDC_MIRRORTYPE,
	IDC_ARR_NUM, IDC_ARR_IND, IDC_ARR_CNT,
	IDC_DISABLED
};

#define MWS_EDIT WS_VISIBLE| WS_CHILD | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL
#define MWS_BUT WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_BITMAP
#define MWS_CMB WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS
#define MWS_SPR WS_VISIBLE | WS_CHILD | SS_ETCHEDHORZ
#define MWS_DBL WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT 
#define MWS_INT WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_NUMBER
#define MWS_CHK WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX
#define MWS_LBL WS_VISIBLE | WS_CHILD

bool Properties::isFirst(std::list<Properties::element*> *l, Properties::element *cmp) {
	return cmp == l->front();
}
bool Properties::isLast(std::list<Properties::element*> *l, Properties::element *cmp) {
	return cmp == l->back();
}

LRESULT CALLBACK Properties::element::proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static HWND dragHWND = NULL;
	static std::list<Properties::element*>::iterator draggedIt = {};
	static bool drag = false;
	static std::list<element*> *elements = &(properties->elements);

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
			element *e = *it;
			if (!isFirst(elements, e) && !isLast(elements, e) && e->isCapsule(hWnd))
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
			ScreenToClient(properties->hwnd, &p);

			element* dragged = *draggedIt;
			for (auto it = elements->begin(); it != elements->end(); ++it) {
				element *e = *it;
				if (e != dragged
					&& !isFirst(elements, e) && !isLast(elements, e)
					&& IN_RANGE(p.y, e->y, e->y + e->h))
				{
					if (dragged->y < e->y) {
						if (!IN_RANGE(p.y, e->y, e->y + e->h - dragged->h)) {
							elements->erase(draggedIt);
							it++;
							draggedIt = elements->insert(it, dragged);
							properties->updateElements();
						}
					}
					else {
						if (!IN_RANGE(p.y, e->y + dragged->h, e->y + e->h)) {
							elements->erase(draggedIt);
							draggedIt = elements->insert(it, dragged);
							properties->updateElements();
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
		for (auto it = elements->begin(); it != elements->end(); ++it) {
			element *e = *it;
			if (e->isCapsule(hWnd)) {
				int com = (BUT_ENUM)LOWORD(wParam);
				switch (com) {
				case IDC_CHANGE_ROLL:
					e->changeRoll();
					properties->update();
					break;
				case IDC_BUT_ADD:
					if (!isLast(elements, e) && elements->size() < 10) {
						elements->insert(++it, new lens());
						properties->update();
					}
					break;
				case IDC_BUT_DEL:
					if (!isFirst(elements, e) && !isLast(elements, e)) {
						delete e;
						elements->erase(it);
						properties->update();
					}
					break;
				case IDC_CMB_TYPE:
					if (HIWORD(wParam) == CBN_SELCHANGE) {
						if (!isFirst(elements, e) && !isLast(elements, e)) {
							int type = (int)SendMessage(e->cmbType, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
							if (type != CB_ERR && type != e->element_type && type != EMITTER && type != CAMERA) {
								bool roll = e->bRoll;
								delete e;
								it = elements->erase(it);
								switch (type) {
								case LENS: elements->insert(it, new lens()); break;
								case MIRROR: elements->insert(it, new mirror()); break;
								}
								if (roll) (*--it)->changeRoll();
								properties->update();
							}
							else SendMessage(e->cmbType, CB_SETCURSEL, (WPARAM)e->element_type, (LPARAM)0);
						}
						else SendMessage(e->cmbType, CB_SETCURSEL, (WPARAM)e->element_type, (LPARAM)0);
					}
					break;
				case IDC_LENSTYPE:
					if (HIWORD(wParam) == CBN_SELCHANGE) {
						static_cast<lens*>(e)->changeLensTypeProps();
						properties->update();
					}
					break;
				case IDC_MIRRORTYPE:
					static_cast<mirror*>(e)->changeSphericalProps();
					properties->update();
					break;
				case IDC_ARR_NUM:
					if (HIWORD(wParam) == EN_UPDATE)
						static_cast<lens*>(e)->changeLensArrayNumProps();	//FIXME danger! e can be not lens
					break;
				case IDC_ARR_IND:
					if (HIWORD(wParam) == EN_UPDATE)
						static_cast<lens*>(e)->changeLensArrayIndProps();	//FIXME danger! e can be not lens
					break;
				case IDC_ARR_CNT:
					if (HIWORD(wParam) == EN_UPDATE)
						static_cast<lens*>(e)->changeLensArrayCntProps();	//FIXME danger! e can be not lens
					break;
				}
				break;
			}
		}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void Properties::element::regClass() {
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = proc;
	wcex.hInstance = hInst;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = L"RaTelement";
	RegisterClassEx(&wcex);
}

Properties::element::element(std::wstring name, ELEMENT_TYPE e_type) : element_type(e_type) {
	w = properties->dims.w;
	capsule = CreateWindow(L"RaTelement", L"", WS_VISIBLE | WS_CHILD, 0, 0, w, h, properties->hwnd, 0, hInst, 0);

	eName = CreateWindow(L"edit", name.c_str(), MWS_EDIT, prop_pad, prop_pad, w - (but_size + 2 * prop_pad), prop_height, capsule, 0, hInst, 0);

	bRoll = CreateWindow(L"button", L"", MWS_BUT, w - (but_size + prop_pad), prop_pad, but_size, but_size, capsule, (HMENU)IDC_CHANGE_ROLL, hInst, 0);
	SendMessage(bRoll, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)properties->hbmpUnroll);
	if (e_type == CAMERA) {
		bAdd = CreateWindow(L"button", L"", MWS_BUT, w - 2 * (but_size + prop_pad), h - (but_size + prop_pad), but_size, but_size, capsule, (HMENU)IDC_DISABLED, hInst, 0);
		SendMessage(bAdd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)properties->hbmpInsertDisabled);
	}
	else {
		bAdd = CreateWindow(L"button", L"", MWS_BUT, w - 2 * (but_size + prop_pad), h - (but_size + prop_pad), but_size, but_size, capsule, (HMENU)IDC_BUT_ADD, hInst, 0);
		SendMessage(bAdd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)properties->hbmpInsert);
	}

	props = CreateWindow(L"RaTelement", L"", WS_CHILD, prop_pad, prop_height + 2 * prop_pad, w - 2 * prop_pad, 0, capsule, 0, hInst, 0);

	if (e_type == EMITTER || e_type == CAMERA) {
		bDel = CreateWindow(L"button", L"", MWS_BUT, w - (but_size + prop_pad), h - (but_size + prop_pad), but_size, but_size, capsule, (HMENU)IDC_DISABLED, hInst, 0);
		SendMessage(bDel, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)properties->hbmpDeleteDisabled);

		cmbType = NULL;
	}
	else {
		bDel = CreateWindow(L"button", L"", MWS_BUT, w - (but_size + prop_pad), h - (but_size + prop_pad), but_size, but_size, capsule, (HMENU)IDC_BUT_DEL, hInst, 0);
		SendMessage(bDel, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)properties->hbmpDelete);

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
void Properties::element::capsuleInit() {
	EnumChildWindows(capsule, (WNDENUMPROC)SetFont, (LPARAM)properties->hFont);
}

HWND Properties::element::insertNumDoubleProp(const wchar_t* name, double initVal) {
	wchar_t initValStr[32];
	swprintf_s(initValStr, L"%1.1f", initVal);
	HWND ret = CreateWindow(L"edit", initValStr, MWS_DBL, 0, hRoll, w * fib - 2 * prop_pad, prop_height, props, 0, hInst, 0);
	HWND label = CreateWindow(L"static", name, MWS_LBL, w * fib, hRoll, w * fibRec - 2 * prop_pad, prop_height, props, 0, hInst, 0);
	hRoll += prop_shift;
	return ret;
}
HWND Properties::element::insertNumIntProp(const wchar_t* name, int initVal) {
	wchar_t initValStr[32];
	swprintf_s(initValStr, L"%d", initVal);
	HWND ret = CreateWindow(L"edit", initValStr, MWS_INT, 0, hRoll, w * fib - 2 * prop_pad, prop_height, props, 0, hInst, 0);
	HWND label = CreateWindow(L"static", name, MWS_LBL, w * fib, hRoll, w * fibRec - 2 * prop_pad, prop_height, props, 0, hInst, 0);
	hRoll += prop_shift;
	return ret;
}
HWND Properties::element::insertButBooleanProp(const wchar_t* name, bool initVal, HMENU group) {
	HWND ret = CreateWindow(L"button", name, MWS_CHK, 0, hRoll, w - 2 * prop_pad, prop_height, props, group, hInst, 0);
	SendMessage(ret, BM_SETCHECK, (WPARAM)(initVal ? BST_CHECKED : BST_UNCHECKED), NULL);
	hRoll += prop_shift;
	return ret;
}
HWND Properties::element::insertComboBoxProp(const wchar_t* name, const std::wstring* initVal, size_t count, HMENU group, int initIndex) {
	HWND ret = CreateWindow(L"combobox", name, MWS_CMB, 0, hRoll, w - 2 * prop_pad, prop_height, props, group, hInst, 0);
	for (size_t i = 0; i < count; ++i)
		SendMessage(ret, CB_ADDSTRING, (WPARAM)0, (LPARAM)initVal[i].c_str());
	SendMessage(ret, CB_SETCURSEL, (WPARAM)initIndex, (LPARAM)0);
	hRoll += prop_shift;
	return ret;
}
Properties::HWNDArray Properties::element::insertNumDoubleArrayProp(const wchar_t* name, double initVal, int count) {
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

void Properties::element::hideProp(HWND prop) {
	if (IsWindowEnabled(prop)) {
		HWND propLabel = GetWindow(prop, GW_HWNDNEXT);

		EnableWindow(prop, FALSE);
		EnableWindow(propLabel, FALSE);

		SetWindowPos(propLabel, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
		SetWindowPos(prop, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);

		hRoll -= prop_shift;
	}
}
void Properties::element::showProp(HWND prop) {
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
void Properties::element::hidePropArray(HWNDArray propA) {
	if (IsWindowEnabled(propA.num)) {
		hideProp(propA.numCount);
		hideProp(propA.numIndex);
		hideProp(propA.num);

		hRoll += prop_shift;
	}
}
void Properties::element::showPropArray(HWNDArray prop) {
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

void Properties::element::changeRoll() {
	roll = !roll;
	SendMessage(bRoll, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)(roll ? properties->hbmpRoll : properties->hbmpUnroll));
	ShowWindow(props, roll ? SW_SHOW : SW_HIDE);
}

void Properties::element::resize(int newY, int newW) {
	y = newY; w = newW; h = getH();
	SetWindowPos(capsule, NULL, 0, y, w, h, SWP_NOZORDER);
	SetWindowPos(props, NULL, 0, 0, w - 2 * prop_pad, hRoll, SWP_NOMOVE);
	SetWindowPos(bRoll, NULL, w - 20, 4, 16, 16, SWP_NOZORDER);
	SetWindowPos(bAdd, NULL, w - 40, h - 20, 16, 16, SWP_NOZORDER);
	SetWindowPos(bDel, NULL, w - 20, h - 20, 16, 16, SWP_NOZORDER);
	SetWindowPos(separator, NULL, 4, h - 2, w - 8, 1, SWP_NOZORDER);
	InvalidateRect(capsule, NULL, TRUE);
}

int Properties::element::getH() {
	return (roll) ? default_element_height + hRoll : default_element_height;
}

bool Properties::element::isCapsule(HWND hWnd) {
	return capsule == hWnd || props == hWnd;
}

bool Properties::element::cmpDistance(const element* a, const element* b) {
	return (a->getDistance()) > (b->getDistance());
}

std::string Properties::element::getShader() {
	return std::string("");
}
void Properties::element::createBuffer() {}
void Properties::element::draw() {}
float Properties::element::getDistance() const { return 0.0f; }

static std::string replaceString(std::string subject, const std::string& search, const std::string& replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return subject;
}

int ray_count = 0;
int norm_count = 0;
int consts_count = 0;
int uniforms_count = 0;
int vertices_count = 0;
std::string uniforms = "";
std::string consts = "";
std::string emits = "";
std::string rays = "";
std::string ray_traces = "";

Properties::emitter::emitter() : element(L"Источник света " + std::to_wstring(++count), EMITTER) {
	divF = insertNumDoubleProp(L"Расходимость Z", 15.0);
	divS = insertNumDoubleProp(L"Расходимость Y", 15.0);
	rayF = insertNumIntProp(L"Лучей Z", 100);
	rayS = insertNumIntProp(L"Лучей Y", 100);
	//angF = insertNumIntProp(L"Источников по Z", 1);
	//angS = insertNumIntProp(L"Источников по Y", 1);
	h = insertNumDoubleProp(L"Высота", 0.5);
	l = insertNumDoubleProp(L"Ширина", 0.5);
	std::wstring types[] = { L"Гаусс", L"Прямоугольник", L"Эллипс" };
	gridType = insertComboBoxProp(L"Тип Сетки", types, sizeofArray(types));

	capsuleInit();
}
std::string Properties::emitter::getShader() {
	std::string ret = "", emit = "", ray = "r" + std::to_string(ray_count++);

	rays += ray + ",";

	ret = replaceString(shdrEmitter, "RAY", ray);

	emit = replaceString(shdrOut, "RAY_OUT", "vec4(" + ray + ".origin, 1.0)");
	emit = replaceString(emit, "COLOR_OUT", getNextColor(0));
	emits += emit;

	vertices_count++;

	return ret;
}
void Properties::emitter::getEmitterProps(EmitterProps &ep) {
	ep.cFA = getFloatText(divF);
	ep.cSA = getFloatText(divS);
	ep.cfStep = getIntText(rayF);
	ep.csStep = getIntText(rayS);
	ep.cMode = SendMessage(gridType, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	ep.cw = getFloatText(l);
	ep.ch = getFloatText(h);
	ep.cNum = 1;//FIXME getIntText(angF);
	ep.cDist = 0;
}
float Properties::emitter::getDistance() const {
	return Camera::camera->getDistance(0, 0, 0);
}
Properties::emitter::~emitter() {
	--count;
}

Properties::lens::lens() : lens(PLANE, 0, 0, 10.0f, 10.0f, 10.0f, 1.5f, 5.0f, { 0.0f }) { }
Properties::lens::lens(LENS_TYPE clt, float cx, float cy, float ch, float cw, float cd, float cn, float cr, std::vector<float> ccoef)
	: element(L"Преломляющая пов-сть " + std::to_wstring(++count), LENS) {
	x = insertNumDoubleProp(L"Положение X", cx);
	y = insertNumDoubleProp(L"Положение Y", cy);
	w = insertNumDoubleProp(L"Ширина", cw);
	h = insertNumDoubleProp(L"Высота", ch);
	n = insertNumDoubleProp(L"n", cn);
	std::wstring types[] = { L"Плоская", L"Асферическая Z", L"Сферическая" };
	lensType = insertComboBoxProp(L"Тип", types, sizeofArray(types), (HMENU)IDC_LENSTYPE, clt);
	r = insertNumDoubleProp(L"Радиус кривизны", cr);
	d = insertNumDoubleProp(L"Толщина", cd);
	coef = ccoef;
	coef.resize(8, 0.0f);
	setFloatText(lens::coefA.num, coef[0]);
	coefA = insertNumDoubleArrayProp(L"Коэффициенты", 0.0, coef.size());

	hideProp(r);
	hideProp(d);
	hidePropArray(coefA);
	changeLensTypeProps();

	capsuleInit();
}
std::string Properties::lens::getShader() {
	size_t st;
	static const size_t bSize = 32;
	TCHAR TcharBuffer[bSize] = TEXT("");
	char xPosBuffer[bSize] = "";
	char yPosBuffer[bSize] = "";
	char refrIndexBuffer[bSize] = "";
	char radiusBuffer[bSize] = "";
	char widthBuffer[bSize] = "";
	SendMessage(x, WM_GETTEXT, (WPARAM)(bSize - 1), (LPARAM)TcharBuffer);
	wcstombs_s(&st, xPosBuffer, TcharBuffer, bSize);
	SendMessage(y, WM_GETTEXT, (WPARAM)(bSize - 1), (LPARAM)TcharBuffer);
	wcstombs_s(&st, yPosBuffer, TcharBuffer, bSize);
	SendMessage(n, WM_GETTEXT, (WPARAM)(bSize - 1), (LPARAM)TcharBuffer);
	wcstombs_s(&st, refrIndexBuffer, TcharBuffer, bSize);
	SendMessage(r, WM_GETTEXT, (WPARAM)(bSize - 1), (LPARAM)TcharBuffer);
	wcstombs_s(&st, radiusBuffer, TcharBuffer, bSize);
	SendMessage(d, WM_GETTEXT, (WPARAM)(bSize - 1), (LPARAM)TcharBuffer);
	wcstombs_s(&st, widthBuffer, TcharBuffer, bSize);

	std::string ret1 = "", emit = "";
	std::string rayp1 = "r" + std::to_string(ray_count - 1);
	std::string ray = "r" + std::to_string(ray_count++);

	switch (getType()) {
	case PLANE:
		ret1 = replaceString(shdrPlaneLens, "RAY_P", rayp1);
		ret1 = replaceString(ret1, "RAY", ray);
		ret1 = replaceString(ret1, "REFRACTION_INDEX", refrIndexBuffer);
		ret1 = replaceString(ret1, "PLANE_POS_X", xPosBuffer);
		break;
	case CYL_Z_ASPHER:
	{
		std::string lens_pos = "c_lens_pos_" + std::to_string(consts_count++);
		consts += "const vec3 " + lens_pos + " = vec3(" + xPosBuffer + ", " + yPosBuffer + ", 0.0);\n";

		ret1 = replaceString(shdrAsphericLens, "RAY_P", rayp1);
		ret1 = replaceString(ret1, "LENS_WIDTH", widthBuffer);

		std::string coefStr = "";
		for (float f : coef)
			coefStr += std::to_string(f) + ',';
		if (coefStr.length()) coefStr.pop_back();
		std::string coef = "p" + std::to_string(consts_count++);
		consts += "const float " + coef + "[] = float[](" + coefStr + ");\n";

		ret1 = replaceString(ret1, "RAY", ray);
		ret1 = replaceString(ret1, "NORM", "norm" + std::to_string(norm_count++));
		ret1 = replaceString(ret1, "REFRACTION_INDEX", refrIndexBuffer);
		ret1 = replaceString(ret1, "LENS_COEF", coef);
		ret1 = replaceString(ret1, "LENS_POS", lens_pos);
	}
	break;
	case SPHERE:
	{
		std::string lens_pos = "c_lens_pos_" + std::to_string(consts_count++);
		consts += "const vec3 " + lens_pos + " = vec3((" + xPosBuffer + ")-(" + radiusBuffer + "), " + yPosBuffer + ", 0.0);\n";

		ret1 = replaceString(shdrSphereLens, "RAY_P", rayp1);
		ret1 = replaceString(ret1, "LENS_RADIUS", std::string("(") + radiusBuffer + ")");

		ret1 = replaceString(ret1, "RAY", ray);
		ret1 = replaceString(ret1, "NORM", "norm" + std::to_string(norm_count++));
		ret1 = replaceString(ret1, "REFRACTION_INDEX", refrIndexBuffer);
		ret1 = replaceString(ret1, "LENS_POS", lens_pos);
	}
	break;
	}

	rays += ray + ",";

	emit = replaceString(shdrOut, "RAY_OUT", "vec4(" + ray + ".origin, 1.0)");
	emit = replaceString(emit, "COLOR_OUT", getNextColor());
	emits += emit;

	vertices_count++;

	return ret1;
}

float Properties::lens::surface(float y0, const float *p, size_t size) {
	if (size < 2) return 0;
	float ret = (y0*y0 / p[0]) /
		(1.0f + sqrt(abs(1.0f - (1.0f + p[1])*y0*y0 / (p[0] * p[0]))));
	for (int i = 2; i < size; ++i)
		ret += p[i] * pow(y0, i * 2);
	return ret;
}
float Properties::lens::dPolyAspher(float y0, const float *p, size_t size) {
	if (size < 2) return 0;
	float sq = sqrt(1.0 - (1.0 + p[1])*y0*y0 / (p[0] * p[0]));
	float b = p[0] * (1.0 + sq);
	float db = -(1.0 + p[1])*y0 / (p[0] * sq);
	float ret = ((2.0 * y0) * b - (y0*y0) * db) / (b * b);
	for (int i = 2; i < size; ++i)
		ret += i * 2 * p[i] * pow(y0, i * 2 - 1);
	return ret;
}
void Properties::lens::createBuffer() {
	LENS_TYPE type = getType();

	float x = getFloatText(lens::x);
	float y = getFloatText(lens::y);
	float d = getFloatText(lens::d);
	float w = getFloatText(lens::w);
	float h = getFloatText(lens::h);
	float r = getFloatText(lens::r);
	float w2 = w / 2, h2 = h / 2;
	switch (type) {
	case PLANE:
	{
		float cA[] = {
		0,-0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
		0,-0.5f,-0.5f, -1.0f, 0.0f, 0.0f,
		0, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
		0, 0.5f,-0.5f, -1.0f, 0.0f, 0.0f,
		};

		verts = sizeofArray(cA) / (3 * 2);

		int attribFormat[] = { 3, 3 };
		Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, cA, sizeof(cA), attribFormat, sizeofArray(attribFormat));

		Matrix::setIdentityM(modelMat);
		Matrix::translateM(modelMat, x, y, 0);
		Matrix::scaleM(modelMat, 1.0f, h, w);
	}
	break;
	case CYL_Z_ASPHER:
	{
		const int points = 500;
		const int surfSize = 2 * points * 6;
		float *surf = new float[surfSize];
		for (int i = 0; i < points; ++i) {
			float ySurf = i / (float)(points - 1) * h - h2;
			float xSurf = coef.size() > 1 ? surface(ySurf, &coef[0], coef.size()) : 0.0f;
			int i6 = i * 6 * 2;
			surf[i6 + 0] = surf[i6 + 6] = xSurf;
			surf[i6 + 1] = surf[i6 + 7] = ySurf;
			surf[i6 + 2] = w2;
			surf[i6 + 8] = -w2;
			float n = coef.size() > 1 ? dPolyAspher(ySurf, &coef[0], coef.size()) : 0.0f;
			surf[i6 + 3] = surf[i6 + 9] = -1.0f;
			surf[i6 + 4] = surf[i6 + 10] = -n;
			surf[i6 + 5] = surf[i6 + 11] = 0;
		}

		verts = surfSize / (3 * 2);

		int attribFormat[] = { 3, 3 };
		Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, surf, surfSize * sizeof(float), attribFormat, sizeofArray(attribFormat));
		delete[] surf;

		Matrix::setIdentityM(modelMat);
		Matrix::translateM(modelMat, x + d, y, 0);
	}
	break;
	case SPHERE:
	{
		const int hdiv = 40, rdiv = 60;
		const int surfSize = hdiv * rdiv * 3 * 2 * 2;
		float *surf = new float[surfSize];
		float rSign = -((r > 0) ? 1 : ((r < 0) ? -1 : 0));
		r = abs(r);
		for (int i = 0; i < hdiv; ++i) {
			float xi = rSign * (r - (float)(hdiv - i) / hdiv * (r - sqrt(r*r - d * d / 4)));
			float xi1 = rSign * (r - (float)(hdiv - i - 1) / hdiv * (r - sqrt(r*r - d * d / 4)));
			float ki = sqrt(r*r - xi * xi);
			float ki1 = sqrt(r*r - xi1 * xi1);
			for (int j = 0; j < rdiv; ++j) {
				int ind = (i*rdiv + j) * 3 * 2 * 2;
				float ang = 2 * M_PI*(float)j / (rdiv - 1);
				surf[ind + 0] = -xi;
				surf[ind + 1] = ki * cos(ang);
				surf[ind + 2] = ki * sin(ang);

				surf[ind + 3] = -surf[ind + 0];
				surf[ind + 4] = -surf[ind + 1];
				surf[ind + 5] = -surf[ind + 2];

				surf[ind + 6] = -xi1;
				surf[ind + 7] = ki1 * cos(ang);
				surf[ind + 8] = ki1 * sin(ang);

				surf[ind + 9] = -surf[ind + 6];
				surf[ind + 10] = -surf[ind + 7];
				surf[ind + 11] = -surf[ind + 8];
			}
		}

		verts = surfSize / (3 * 2);

		int attribFormat[] = { 3, 3 };
		Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, surf, surfSize * sizeof(float), attribFormat, sizeofArray(attribFormat));
		delete[] surf;

		Matrix::setIdentityM(modelMat);
		Matrix::translateM(modelMat, x + rSign * r, y, 0);
	}
	break;
	}

	distance = Camera::camera->getDistance(x, y, 0);
}
void Properties::lens::draw() {
	glUseProgram(Shader::Triag::programHandle);

	glBindVertexArray(vaoId);
	glEnableVertexAttribArray(Shader::Triag::Input::Position);
	glEnableVertexAttribArray(Shader::Triag::Input::Normal);

	Matrix::multiplyMM(Camera::camera->mMVPMat, Camera::camera->mVPMat, modelMat);
	glUniformMatrix4fv(Shader::Triag::Uniform::uMatHandle, 1, GL_FALSE, Camera::camera->mMVPMat);
	glUniform3f(Shader::Triag::Uniform::uColorHandle, 0.0f, 0.5f, 0.8f);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, verts);

	glDisableVertexAttribArray(Shader::Triag::Input::Normal);
	glDisableVertexAttribArray(Shader::Triag::Input::Position);
}
float Properties::lens::getDistance() const {
	float x = getFloatText(lens::x);
	float y = getFloatText(lens::y);
	return Camera::camera->getDistance(x, y, 0);
}
Properties::lens::LENS_TYPE Properties::lens::getType() {
	return (LENS_TYPE)SendMessage(lensType, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
}
void Properties::lens::changeLensTypeProps() {
	switch (getType()) {
	case PLANE:
		hideProp(r);
		hideProp(d);
		hidePropArray(coefA);
		break;
	case CYL_Z_ASPHER:
		hideProp(r);
		showProp(d);
		showPropArray(coefA);
		break;
	case SPHERE:
		hidePropArray(coefA);
		showProp(r);
		showProp(d);
		break;
	}
}
void Properties::lens::changeLensArrayNumProps() {
	float n = getFloatText(lens::coefA.num);
	int i = getIntText(lens::coefA.numIndex);
	int c = getIntText(lens::coefA.numCount);
	coef[i] = n;
}
void Properties::lens::changeLensArrayIndProps() {
	int i = getIntText(lens::coefA.numIndex);
	int c = getIntText(lens::coefA.numCount);
	if (i >= c) {
		i = c - 1;
		setIntText(lens::coefA.numIndex, i);
	}
	else
		setFloatText(lens::coefA.num, coef[i]);
}
void Properties::lens::changeLensArrayCntProps() {
	int i = getIntText(lens::coefA.numIndex);
	int c = getIntText(lens::coefA.numCount);
	//if (c < 2) { //FIXME array size must be non constant
	if (c != 8) {
		c = 8;
		setIntText(lens::coefA.numCount, c);
	}
	else {
		if (i >= c)
		{
			i = c - 1;
			setIntText(lens::coefA.numIndex, i);
		}
		setFloatText(lens::coefA.num, coef[i]);
		coef.resize(c, 0.0f);
	}
}
Properties::lens::~lens() {
	--count;
}

Properties::mirror::mirror() : element(L"Отражающая пов-сть " + std::to_wstring(++count), MIRROR) {
	x = insertNumDoubleProp(L"Положение X", 0.0);
	y = insertNumDoubleProp(L"Положение Y", 0.0);
	angle = insertNumDoubleProp(L"Угол наклона", 0.0);
	d = insertNumDoubleProp(L"Диаметр", 10.0);
	spherical = insertButBooleanProp(L"Сферическая", false, (HMENU)IDC_MIRRORTYPE);
	r = insertNumDoubleProp(L"Радиус кривизны", 0.0);

	hideProp(r);

	capsuleInit();
}
std::string Properties::mirror::getShader() {
	size_t st;
	static const size_t bSize = 32;
	TCHAR TcharBuffer[bSize] = TEXT("");
	char xBuffer[bSize] = "";
	char yBuffer[bSize] = "";
	char radiusBuffer[bSize] = "";
	SendMessage(x, WM_GETTEXT, (WPARAM)(bSize - 1), (LPARAM)TcharBuffer);
	wcstombs_s(&st, xBuffer, TcharBuffer, bSize);
	SendMessage(y, WM_GETTEXT, (WPARAM)(bSize - 1), (LPARAM)TcharBuffer);
	wcstombs_s(&st, yBuffer, TcharBuffer, bSize);
	SendMessage(r, WM_GETTEXT, (WPARAM)(bSize - 1), (LPARAM)TcharBuffer);
	wcstombs_s(&st, radiusBuffer, TcharBuffer, bSize);

	std::string ret = "", emit = "";
	std::string rayp = "r" + std::to_string(ray_count - 1);
	std::string ray = "r" + std::to_string(ray_count++);

	if (isSpherical()) {
		std::string mirror_pos = "c_mirror_pos_" + std::to_string(consts_count++);
		consts += "const vec3 " + mirror_pos + " = vec3(" + xBuffer + ", " + yBuffer + ", 0.0);\n";

		ret = replaceString(shdrSphereMirror, "RAY_P", rayp);
		ret = replaceString(ret, "RAY", ray);
		ret = replaceString(ret, "NORM", "norm" + std::to_string(norm_count++));
		ret = replaceString(ret, "MIRROR_RADIUS", radiusBuffer);
		ret = replaceString(ret, "MIRROR_POS", mirror_pos);
	}
	else {
		std::string mirror_norm = "c_mirror_norm_" + std::to_string(consts_count++);
		consts += "const vec3 " + mirror_norm + " = vec3(" + xBuffer + ", " + yBuffer + ", 0.0);\n";

		ret = replaceString(shdrPlaneMirror, "RAY_P", rayp);
		ret = replaceString(ret, "RAY", ray);
		ret = replaceString(ret, "NORM", mirror_norm);
	}

	rays += ray + ",";

	emit = replaceString(shdrOut, "RAY_OUT", "vec4(" + ray + ".origin, 1.0)");
	emit = replaceString(emit, "COLOR_OUT", getNextColor());
	emits += emit;

	vertices_count++;

	return ret;
}
void Properties::mirror::createBuffer() {
	const int hdiv = 20, rdiv = 60;

	float x = getFloatText(mirror::x);
	float y = getFloatText(mirror::y);
	float angle = getFloatText(mirror::angle);
	float w = getFloatText(mirror::d);
	float r = getFloatText(mirror::r);
	float w2 = w / 2;

	if (isSpherical()) {
		const int surfSize = hdiv * rdiv * 3 * 2 * 2;
		float *surf = new float[surfSize];
		for (int i = 0; i < hdiv; ++i) {
			float xi = r - (float)(hdiv - i) / hdiv * (r - sqrt(r*r - w * w / 4));
			float xi1 = r - (float)(hdiv - i - 1) / hdiv * (r - sqrt(r*r - w * w / 4));
			float ki = sqrt(r*r - xi * xi);
			float ki1 = sqrt(r*r - xi1 * xi1);
			for (int j = 0; j < rdiv; ++j) {
				int ind = (i*rdiv + j) * 3 * 2 * 2;
				float ang = 2 * M_PI*(float)j / (rdiv - 1);
				surf[ind + 0] = -xi;
				surf[ind + 1] = ki * cos(ang);
				surf[ind + 2] = ki * sin(ang);

				surf[ind + 3] = -surf[ind + 0];
				surf[ind + 4] = -surf[ind + 1];
				surf[ind + 5] = -surf[ind + 2];

				surf[ind + 6] = -xi1;
				surf[ind + 7] = ki1 * cos(ang);
				surf[ind + 8] = ki1 * sin(ang);

				surf[ind + 9] = -surf[ind + 6];
				surf[ind + 10] = -surf[ind + 7];
				surf[ind + 11] = -surf[ind + 8];
			}
		}

		verts = surfSize / (3 * 2);

		int attribFormat[] = { 3,3 };
		Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, surf, surfSize * sizeof(float), attribFormat, sizeofArray(attribFormat));
		delete[] surf;

		Matrix::setIdentityM(modelMat);
		Matrix::translateM(modelMat, x, y, 0);
		Matrix::rotateM(modelMat, angle * (180.0f / M_PI), Axis::Z);
	}
	else {
		const int surfSize = (rdiv + 1) * 3 * 2;
		float *surf = new float[surfSize];
		surf[0] = 0;
		surf[1] = 0;
		surf[2] = 0;
		surf[3] = -1;
		surf[4] = 0;
		surf[5] = 0;
		for (int i = 0; i < rdiv; ++i) {
			int ind = (i + 1) * 3 * 2;
			float ang = 2 * M_PI*(float)i / (rdiv - 1);
			surf[ind + 0] = 0;
			surf[ind + 1] = 0.5 * cos(ang);
			surf[ind + 2] = 0.5 * sin(ang);

			surf[ind + 3] = -1;
			surf[ind + 4] = 0;
			surf[ind + 5] = 0;
		}

		verts = surfSize / (3 * 2);

		int attribFormat[] = { 3,3 };
		Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, surf, surfSize * sizeof(float), attribFormat, sizeofArray(attribFormat));
		delete[] surf;

		Matrix::setIdentityM(modelMat);
		Matrix::translateM(modelMat, x, y, 0);
		Matrix::rotateM(modelMat, angle, Axis::Z);
		Matrix::scaleM(modelMat, 1, w, w);
	}

	distance = Camera::camera->getDistance(x, y, 0);
}
void Properties::mirror::draw() {
	glUseProgram(Shader::Triag::programHandle);

	glBindVertexArray(vaoId);
	glEnableVertexAttribArray(Shader::Triag::Input::Position);
	glEnableVertexAttribArray(Shader::Triag::Input::Normal);

	Matrix::multiplyMM(Camera::camera->mMVPMat, Camera::camera->mVPMat, modelMat);
	glUniformMatrix4fv(Shader::Triag::Uniform::uMatHandle, 1, GL_FALSE, Camera::camera->mMVPMat);
	glUniform3f(Shader::Triag::Uniform::uColorHandle, 0.3f, 0.3f, 0.3f);
	if (isSpherical()) glDrawArrays(GL_TRIANGLE_STRIP, 0, verts);
	else glDrawArrays(GL_TRIANGLE_FAN, 0, verts);

	glDisableVertexAttribArray(Shader::Triag::Input::Normal);
	glDisableVertexAttribArray(Shader::Triag::Input::Position);
}
float Properties::mirror::getDistance() const {
	float x = getFloatText(mirror::x);
	float y = getFloatText(mirror::y);
	return Camera::camera->getDistance(x, y, 0);
}
bool Properties::mirror::isSpherical() {
	return Button_GetCheck(spherical);
}
void Properties::mirror::changeSphericalProps() {
	isSpherical() ? showProp(r) : hideProp(r);
}
Properties::mirror::~mirror() {
	--count;
}

Properties::camera::camera() : camera(0.0f, 0.0f, 10.0f, 10.0f, 128, 128) {};
Properties::camera::camera(float cx, float cy, float csx, float csy, int crx, int cry) : element(L"Изображение " + std::to_wstring(++count), CAMERA) {
	x = insertNumDoubleProp(L"Положение X", cx);
	y = insertNumDoubleProp(L"Положение Y", cy);
	sizeX = insertNumDoubleProp(L"Размер Z", csx);
	sizeY = insertNumDoubleProp(L"Размер Y", csy);
	resX = insertNumIntProp(L"Разрешение Z", crx);
	resY = insertNumIntProp(L"Разрешение Y", cry);

	capsuleInit();
}
std::string Properties::camera::getShader() {
	size_t st;
	static const size_t bSize = 32;
	TCHAR TcharBuffer[bSize] = TEXT("");
	char xPosBuffer[bSize] = "";
	SendMessage(x, WM_GETTEXT, (WPARAM)(bSize - 1), (LPARAM)TcharBuffer);
	wcstombs_s(&st, xPosBuffer, TcharBuffer, bSize);

	std::string ret = "", emit = "";
	std::string rayp = "r" + std::to_string(ray_count - 1);
	std::string ray = "r" + std::to_string(ray_count++);

	ret = replaceString(shdrPlane, "RAY_P", rayp);
	ret = replaceString(ret, "RAY", ray);
	ret = replaceString(ret, "PLANE_POS_X", xPosBuffer);

	rays += ray + ",";

	emit = replaceString(shdrOut, "RAY_OUT", "vec4(" + ray + ".origin, 1.0)");
	emit = replaceString(emit, "COLOR_OUT", getNextColor());
	emits += emit;

	vertices_count++;

	return ret;
}
void Properties::camera::createBuffer() {
	float x = getFloatText(camera::x);
	float y = getFloatText(camera::y);
	float sx = getFloatText(camera::sizeX);
	float sy = getFloatText(camera::sizeY);

	float cA[] = {
		0,-0.5f,0.5f, 0.0f, 0.0f,
		0,-0.5f,-0.5f, 1.0f, 0.0f,
		0,0.5f,0.5f, 0.0f, 1.0f,
		0,0.5f,-0.5f, 1.0f, 1.0f
	};
	int attribFormat[] = { 3,2 };

	Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, cA, sizeof(cA), attribFormat, sizeof(attribFormat) / sizeof(int));

	Matrix::setIdentityM(modelMat);
	Matrix::translateM(modelMat, x, y, 0);
	Matrix::scaleM(modelMat, 1.0f, sy, sx);

	distance = Camera::camera->getDistance(x, y, 0);
}
void Properties::camera::draw() {
	glUseProgram(Shader::textureTriag::programHandle);

	glBindVertexArray(vaoId);
	glEnableVertexAttribArray(Shader::textureTriag::Input::Position);
	glEnableVertexAttribArray(Shader::textureTriag::Input::TextureCoord);

	Matrix::multiplyMM(Camera::camera->mMVPMat, Camera::camera->mVPMat, modelMat);
	glUniform1i(Shader::textureTriag::Uniform::uTexture0, 0);
	glUniformMatrix4fv(Shader::textureTriag::Uniform::uMatHandle, 1, GL_FALSE, Camera::camera->mMVPMat);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(Shader::textureTriag::Input::TextureCoord);
	glDisableVertexAttribArray(Shader::textureTriag::Input::Position);
}
float Properties::camera::getDistance() const {
	float x = getFloatText(camera::x);
	float y = getFloatText(camera::y);
	return Camera::camera->getDistance(x, y, 0);
}
void Properties::camera::getCameraProps(CameraProps &cp) {
	cp.px = getIntText(camera::resX);
	cp.py = getIntText(camera::resY);
}
Properties::camera::~camera() {
	--count;
}

void Properties::element::releaseGLresources() {
	glDeleteVertexArrays(1, &vaoId);
	glDeleteBuffers(1, &bufferId);
}

Properties::element::~element() {
	PostMessage(capsule, WM_CLOSE, 0, 0);
	releaseGLresources();
}
