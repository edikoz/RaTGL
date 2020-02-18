#pragma once

//#define CHECK(chk) if (chk) LOG("OK "#chk); else { MessageBox(NULL, TEXT("ERROR "#chk), NULL, NULL); LOG("ERROR "#chk); return 0;}
//#define sizeofArray(rArray) sizeof(rArray)/sizeof((rArray)[0])

#define MWS_EDT WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL
#define MWS_BUT WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_BITMAP
#define MWS_CMB WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS
#define MWS_SPR WS_VISIBLE | WS_CHILD | SS_ETCHEDHORZ
#define MWS_DBL WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT 
#define MWS_INT WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_NUMBER
#define MWS_CHK WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX
#define MWS_LBL WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE

template <typename T>
constexpr auto sizeofArray(const T& rArray) {
	return sizeof(rArray) / sizeof(rArray[0]);
}

void inline LOG(const char *message) { std::cout << message << std::endl; }
void inline LOG(std::string message) { std::cout << message << std::endl; }
#define RETURNonERROR( check, error ) if (!(check)) LOG(error); if (!(check)) return
#define EXITonERROR( check, error ) if (!(check)) {LOG(error); MessageBox(NULL, TEXT(error), NULL, NULL);} if (!(check)) exit(0)

void setIntText(HWND hWnd, int i);
void setFloatText(HWND hWnd, float f);
int getIntText(HWND hWnd);
float getFloatText(HWND hWnd);
//std::string getWndText(HWND hWnd);

void GetLocalRect(HWND hWnd, LPRECT rect);

void toggleConsole();

std::string replaceString(std::string subject, const std::string& search,
	const std::string& replace);
