#pragma once

class ShaderText {
public:
	static float prevX;
	static void empty();

	static int ray_count, norm_count,
		consts_count, uniforms_count,
		vertices_count;

	static std::string uniforms,
		consts, emits, rays,
		ray_traces, conditions;

	//Основа шейдера
	static std::string shdrMain;

	//Функции трассировки шейдера
	static const std::string shdrFunctions;

	//GENERATION BLOCK BEGIN

	static std::string shdrEmitter;
	static std::string shdrRayTransform;
	static std::string shdrPlane;
	static std::string shdrPlaneLens;
	static std::string shdrAsphericLens;
	static std::string shdrBTSBack;
	static std::string shdrBTSFront;
	static std::string shdrPlaneMirror;
	static std::string shdrSphereMirror;
	static std::string shdrSphereLens;
	static std::string shdrCheck;
	static std::string shdrOut;

	//GENERATION BLOCK END
};
