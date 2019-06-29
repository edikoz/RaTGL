#pragma once

//#define CHECK(chk) if (chk) LOG("OK "#chk); else { MessageBox(NULL, TEXT("ERROR "#chk), NULL, NULL); LOG("ERROR "#chk); return 0;}
//#define sizeofArray(rArray) sizeof(rArray)/sizeof((rArray)[0])

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

void GetLocalRect(HWND hWnd, LPRECT rect);

void toggleConsole();
