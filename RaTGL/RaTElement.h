#pragma once
#include "PropertyView.h"

class RaTElement {
protected:
	enum ELEMENT_TYPE : int { LENS, MIRROR, EMITTER, CAMERA };
	typedef std::list<RaTElement*>::const_iterator const_iter;

private:
	static LRESULT CALLBACK proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	static const int default_element_height;	//Высота неразвернутого окна элемента
	static const int prop_height;				//Высота параметра элемента
	static const int prop_pad;					//Расстояние между параметрами элемента
	static const int prop_shift;				//dy параметров элемента
	static const int but_size;					//Размер кнопок
	static const double fibRec;					//Обратное золотое сечение
	static const double fib;					//Золотое сечение

	const ELEMENT_TYPE element_type;

	const_iter iterToSelf;
	int y = 0, w, h = default_element_height, hRoll = 0;
	HWND capsule, props, cmbType, eName, bRoll, bAdd, bDel, separator;
	bool roll = false;
	bool needUpdateBuffer = true;

	//void recalcPropHeight(HWND prop);
	//void recalcPropArrayHeight(HWNDArray propA);

protected:
	struct HWNDArray {
		HWND num, numIndex, numCount;
	};
	enum BUT_ENUM : int {
		IDC_CHANGE_ROLL = 1000, IDC_BUT_ADD, IDC_BUT_DEL, IDC_CMB_TYPE,
		IDC_LENSTYPE, IDC_MIRRORTYPE, IDC_ARR_NUM, IDC_ARR_IND, IDC_ARR_CNT,
		IDC_DISABLED
	};

	static std::string getNextColor(int ci = -1);

	float distance = 0.0f;
	int verts = 0;
	GLuint bufferId = 0, vaoId = 0;
	Matrix4 modelMat = {};

	HWND insertNumDoubleProp(const wchar_t* name, double initVal);
	HWND insertNumIntProp(const wchar_t* name, int initVal);
	HWND insertButBooleanProp(const wchar_t* name, bool initVal, HMENU group = 0);
	HWND insertComboBoxProp(const wchar_t* name, const std::wstring* initVal,
		size_t count, HMENU group = 0, int initIndex = 0);
	HWNDArray insertNumDoubleArrayProp(const wchar_t* name, double initVal, int count);

	void hideProp(HWND prop);
	void showProp(HWND prop);
	void hidePropArray(HWNDArray propA);
	void showPropArray(HWNDArray propA);

	virtual std::string save() const;
	virtual void load(std::string elementString) const;

	RaTElement(std::wstring name, ELEMENT_TYPE e_type);

public:
	static void regClass();

	void setIterToSelf(const_iter iterToSelf);
	bool isCapsule(HWND hWnd);
	void capsuleInit();
	void changeRoll();
	void resize(int newY, int newW);
	int getH();
	static bool cmpDistance(const RaTElement* a, const RaTElement* b);

	virtual void obtainGLresources();
	virtual void releaseGLresources();
	virtual std::string getShader();
	virtual void draw(Camera *camera);
	virtual void calcDistanceTo(Vector3 pos);

	virtual ~RaTElement();
};

class EmitterElement : public RaTElement {
	static int count;
	GLuint feedbackQuery = 0, feedbackPrimitives = 0;

	/*FloatProperty rayF, rayS;
	IntegerProperty rayF, rayS;*/
	BooleanProperty Fa;

	float FA = 0, SA = 0, width = 0, height = 0, Dist = 0;
	int fStep = 0, sStep = 0, Mode = 0, Num = 0;
	int lines = 0;


	HWND divF, divS, rayF, rayS,
		//FIXME angF, angS, 
		gridType,
		h, l;

	float* fillAxis1D(float *axis, int size, float lim);
	void fillAxis2D(float *axis, int pos, float f, float s, float zOffset);
	void fillAxis2Dangle(float *axis, int pos, float f, float s, float zOffset);
	void changeRays();

public:
	static const int MAX_RAYS = 500 * 500;

	EmitterElement();
	EmitterElement(float cdivZ, float cdivY, int craysZ, int craysY, float ch, float cw, int type);
	std::string getShader() override;
	void obtainGLresources() override;
	void releaseGLresources() override;
	std::string save() const override;
	void load(std::string elementString) const override;
	void calcDistanceTo(Vector3 pos) override;
	int calcRays(Camera *camera);
	//void setEmitterProps();
	~EmitterElement();
};

class LensElement : public RaTElement {
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

public:
	LensElement();
	LensElement(LENS_TYPE lt, float x, float y, float h, float w, float d, float n, float r, std::vector<float> coef);
	std::string getShader() override;
	void obtainGLresources() override;
	void releaseGLresources() override;
	std::string save() const override;
	void load(std::string elementString) const override;
	void draw(Camera *camera) override;
	void calcDistanceTo(Vector3 pos) override;

	void changeLensTypeProps();	//FIXME Should be static init, not real-time call
	void changeLensArrayNumProps();
	void changeLensArrayIndProps();
	void changeLensArrayCntProps();

	~LensElement();
};

class SphereLens {
	LensElement *lens;
	SphereLens();
};

class MirrorElement : public RaTElement {
	static int count;
	HWND x, y, angle, d, spherical, r;

	bool isSpherical();

public:
	MirrorElement();
	std::string getShader() override;
	void obtainGLresources() override;
	void releaseGLresources() override;
	std::string save() const override;
	void load(std::string elementString) const override;
	void draw(Camera *camera) override;
	void calcDistanceTo(Vector3 pos) override;

	void changeSphericalProps();	//FIXME Should be static init, not real-time call

	~MirrorElement();
};

class CameraElement : public RaTElement {
	static int count;
	static const int MAX_IMAGE_BUF = 1024 * 1024;
	HWND x, y, z, sizeX, sizeY, resX, resY;

	GLuint feedbackVaoId = 0, feedbackBufferId = 0, fboId = 0, textureId = 0;
	int textureBit = 3;
	float* imageBuf = nullptr;
	float pSize = 1.0f, pIntensity = 0.3f;//0.05f;

	float camX, camY, camZ, camW, camH;
	int camPixX = 128, camPixY = 128;

	void changeSensorResolution();

public:
	CameraElement();
	CameraElement(float x, float y, float z, float sx, float sy, int rx, int ry);
	std::string getShader() override;
	void obtainGLresources() override;
	void releaseGLresources() override;
	std::string save() const override;
	void load(std::string elementString) const override;
	void draw(Camera *camera) override;
	void calcRays(int points);
	void calcDistanceTo(Vector3 pos) override;

	~CameraElement();
};