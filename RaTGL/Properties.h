#pragma once
#include "RaTwindow.h"
#include "ShaderText.h"

//FIXME should be in Properties or Scene class
struct EmitterProps {
	double cFA, cSA;
	int cfStep, csStep;
	int cMode;
	float cw, ch;
	int cNum;
	float cDist;
};

struct CameraProps {
	int px, py;
};

class Properties final : public RaTwindow {
	static LRESULT CALLBACK proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static Properties *properties;

	int scrollY;
	void resize(int w, int h);
	void update();
	void updateScroll();
	void updateElements();
	void generateShader();
	void generateBuffer();

	HFONT hFont;
	HBITMAP hbmpUnroll, hbmpRoll, hbmpDelete, hbmpDeleteDisabled, hbmpInsert, hbmpInsertDisabled;

	enum ELEMENT_TYPE : int { LENS, MIRROR, EMITTER, CAMERA };

	struct HWNDArray {
		HWND num, numIndex, numCount;
	};

	class element {
		static LRESULT CALLBACK proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		static const int default_element_height;	//Высота неразвернутого окна элемента
		static const int prop_height;				//Высота параметра элемента
		static const int prop_pad;					//Расстояние между параметрами элемента
		static const int prop_shift;				//dy параметров элемента
		static const int but_size;					//Размер кнопок
		static const double fibRec;					//Обратное золотое сечение
		static const double fib;					//Золотое сечение
		int y = 0, w, h = default_element_height, hRoll = 0;

		const ELEMENT_TYPE element_type;

		HWND capsule, props, cmbType, eName, bRoll, bAdd, bDel, separator;
		bool roll = false;

		//void recalcPropHeight(HWND prop);
		//void recalcPropArrayHeight(HWNDArray propA);

	protected:
		float distance = 0.0f;
		int verts = 0;
		GLuint bufferId = 0, vaoId = 0;
		GLfloat modelMat[16] = { 0 };

		HWND insertNumDoubleProp(const wchar_t* name, double initVal);
		HWND insertNumIntProp(const wchar_t* name, int initVal);
		HWND insertButBooleanProp(const wchar_t* name, bool initVal, HMENU group = 0);
		HWND insertComboBoxProp(const wchar_t* name, const std::wstring* initVal,  size_t count, HMENU group = 0, int initIndex = 0);
		HWNDArray insertNumDoubleArrayProp(const wchar_t* name, double initVal, int count);

		void hideProp(HWND prop);
		void showProp(HWND prop);
		void hidePropArray(HWNDArray propA);
		void showPropArray(HWNDArray propA);

		virtual float getDistance() const;

		element(std::wstring name, ELEMENT_TYPE e_type);

	public:
		static void regClass();

		bool isCapsule(HWND hWnd);
		void capsuleInit();
		void changeRoll();
		void resize(int newY, int newW);
		int getH();
		static bool cmpDistance(const element* a, const element* b);

		void releaseGLresources();
		virtual std::string getShader();
		virtual void createBuffer();
		virtual void draw();

		virtual ~element();
	};

	class emitter : public element {
		static int count;
		HWND divF, divS, rayF, rayS, 
			//FIXME angF, angS, 
			gridType,
			h, l;

		float getDistance() const override;

	public:
		emitter();
		std::string getShader() override;
		void getEmitterProps(EmitterProps &ep);
		//void setEmitterProps();
		~emitter();
	};

	class lens : public element {
	public:
		enum LENS_TYPE : int { PLANE, CYL_Z_ASPHER, SPHERE };

	private:
		static int count;
		HWND x, y, h, w, d, n;
		HWND lensType;
		HWND r;
		HWNDArray coefA;
		std::vector<float> coef;

		float surface(float y0, const float *p, size_t size);
		float dPolyAspher(float y0, const float *p, size_t size);
		LENS_TYPE getType();

		float getDistance() const override;

	public:
		lens();
		lens(LENS_TYPE lt, float x, float y, float h, float w, float d, float n, float r, std::vector<float> coef);
		std::string getShader() override;
		void createBuffer() override;
		void draw() override;

		void changeLensTypeProps();	//FIXME Should be static init, not real-time call
		void changeLensArrayNumProps();
		void changeLensArrayIndProps();
		void changeLensArrayCntProps();

		~lens();
	};

	class mirror : public element {
		static int count;
		HWND x, y, angle, d, spherical, r;

		bool isSpherical();

		float getDistance() const override;

	public:
		mirror();
		std::string getShader() override;
		void createBuffer() override;
		void draw() override;

		void changeSphericalProps();	//FIXME Should be static init, not real-time call

		~mirror();
	};

	class camera : public element {
		static int count;
		HWND x, y, sizeX, sizeY, resX, resY;

		float getDistance() const override;

	public:
		camera();
		camera(float x, float y, float sx, float sy, int rx, int ry);
		std::string getShader() override;
		void createBuffer() override;
		void draw() override;

		void getCameraProps(CameraProps &cp);

		~camera();
	};

	std::list<element*> elements;
	static bool isFirst(std::list<element*> *l, element *cmp);
	static bool isLast(std::list<element*> *l, element *cmp);

public:
	Properties(HWND parent, Dims dim);

	void clearElements();
	void generate();
	static void drawElements();

	void getEmitterProps(EmitterProps &ep);
	void getCameraProps(CameraProps &cp);
	int getNumDots();
};
