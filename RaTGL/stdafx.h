// stdafx.h: включаемый файл для стандартных системных включаемых файлов
// или включаемых файлов для конкретного проекта, которые часто используются, но
// нечасто изменяются
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Исключите редко используемые компоненты из заголовков Windows
// Файлы заголовков Windows
#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include <uxtheme.h>					//Only for resolve bug with scrollbar draw error when it's hiding

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "UxTheme.lib")		//Only for resolve bug with scrollbar draw error when it's hiding
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32s.lib")

// Файлы заголовков среды выполнения C
#define _USE_MATH_DEFINES
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <cmath>
#include <cstring>
#include <random>

//3-d party headers
#define GLEW_STATIC
#include <GL\glew.h>
#include <GL\wglew.h>

// установите здесь ссылки на дополнительные заголовки, требующиеся для программы
#include "Helper.h"
#include "Matrix.h"
#include "Shader.h"
#include "RaTwindow.h"
#include "Frame.h"
#include "WorkSpace.h"
#include "Properties.h"
#include "Measure.h"
#include "GLwindow.h"
#include "Camera.h"
#include "Graph.h"
#include "Scene.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
