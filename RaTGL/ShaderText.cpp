#include "stdafx.h"
#include "ShaderText.h"

float ShaderText::prevX;

int ShaderText::ray_count, ShaderText::norm_count,
ShaderText::consts_count, ShaderText::uniforms_count,
ShaderText::vertices_count;

std::string ShaderText::uniforms,
ShaderText::consts, ShaderText::emits, ShaderText::rays,
ShaderText::ray_traces;

void ShaderText::empty() {
	prevX = 0;
	uniforms = ""; consts = ""; emits = ""; rays = ""; ray_traces = "";
	ray_count = norm_count = consts_count = uniforms_count = vertices_count = 0;
}

//Основа шейдера
std::string ShaderText::shdrMain =
R"RaT(#version 330 core

uniform mat4 u_viewMat;

REPLACE_UNIFORMS

REPLACE_CONSTS

layout(lines) in;
layout(line_strip, max_vertices = MAX_VERTICES) out;

flat out vec4 col;
out vec4 feedbackOut;

const float eps = 1.0e-10;
const float iters = 10.0;

struct Ray{
	vec3 origin, direction;
} RAYS;

void emit(const vec4 c_Col, vec4 r_Out){
	col = c_Col;
	feedbackOut = r_Out;
	gl_Position = u_viewMat * feedbackOut;
	EmitVertex();
}

REPLACE_FUNCTIONS

void main() {

REPLACE_RAY_TRACE

//REPLACE_CHECK
	{
REPLACE_EMIT		
	}
	EndPrimitive();
}
)RaT";

//ASPHERIC BLOCK BEGIN

//Уравнение асферической поверхности
const std::string ShaderText::shdrFunctions = R"RaT(
float polyAspher(float y0, const float p[8]){
    return
	(y0*y0/p[0]) / 
	(1.0+sqrt( abs(1.0-(1.0+p[1])*y0*y0/(p[0]*p[0])) )) +
	p[2]*y0*y0*y0*y0 + 
	p[3]*y0*y0*y0*y0*y0*y0  + 
	p[4]*y0*y0*y0*y0*y0*y0*y0*y0 + 
	p[5]*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0 + 
	p[6]*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0 + 
	p[7]*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0;
}
)RaT"
//Система из уравнений прямой и асферической поверхности
R"RaT(
vec4 aspherSystem(vec4 A, Ray ray, vec3 L0, const float p[8]){
	return vec4(	A.w*ray.direction + ray.origin - A.xyz,
					polyAspher(A.y - L0.y, p) + L0.x - A.x	);
}
)RaT"
//Производная уравнения асферической поверхности
R"RaT(
float dPolyAspher(float y0, const float p[8]){
    float sq = sqrt(1.0 - (1.0 + p[1])*y0*y0/(p[0]*p[0]));
    float b = p[0] * (1.0 + sq);
    float db = -(1.0 + p[1]) * y0 / (p[0]*sq);
    float dx = ((2.0 * y0) * b - (y0*y0) * db) / (b * b) + 
	4.0*p[2]*y0*y0*y0 + 
	6.0*p[3]*y0*y0*y0*y0*y0 + 
	8.0*p[4]*y0*y0*y0*y0*y0*y0*y0 + 
	10.0*p[5]*y0*y0*y0*y0*y0*y0*y0*y0*y0 + 
	12.0*p[6]*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0 + 
	14.0*p[7]*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0;
	return dx;
}
)RaT"
//Якобиан системы
R"RaT(
mat4 jacPolyAspher(vec4 A, vec3 cosAng, vec3 L0, const float p[8]){
    mat4 mx = mat4(0.0);
    mx[0][0] = -1.0;
    mx[1][1] = -1.0;
    mx[2][2] = -1.0;
    mx[3][0] = cosAng.x;
    mx[3][1] = cosAng.y;
    mx[3][2] = cosAng.z;
    mx[0][3] = -1.0;
    mx[1][3] = dPolyAspher(A.y - L0.y, p);
	return mx;
}
)RaT"
//Решение системы уравнений методом Ньютона
R"RaT(
vec3 aspherSolver(vec4 k0, Ray ray, vec3 L0, const float p[8]){
    vec4 k1 = k0 - inverse(jacPolyAspher(k0, ray.direction, L0, p))*aspherSystem(k0, ray, L0, p);
	float a = 0.0;
    while (length(k1-k0) >= eps && a++ < iters){
        k0 = k1;
        k1 = k0 - inverse(jacPolyAspher(k0, ray.direction, L0, p))*aspherSystem(k0, ray, L0, p);
    }
	return k1.xyz;
}
)RaT"

//ASPHERIC BLOCK END

//SPHERE BLOCK BEGIN

//Пересечение луча со сферой
R"RaT(
float intersectSphereF(Ray ray, vec3 sph, const float r) {
	vec3 k = ray.origin - sph;
	float a = dot(ray.direction, ray.direction);
	float b = dot(ray.direction, k);
	float c = dot(k, k) - r * r;
	float d = b * b - a * c;
	float t = 0.0;
	if (d > 0.0) {
		float sd = sqrt(d);
		float t1 = (-b - sd) / a;
		float t2 = (-b + sd) / a;
		float min_t = min(t1, t2);
		float max_t = max(t1, t2);
		//t = (min_t >= 0.0) ? min_t : max_t;
		t = (r < 0.0) ? min_t : max_t;
	}
	return t;
}
)RaT";

//SPHERE BLOCK END

//GENERATION BLOCK BEGIN

std::string ShaderText::shdrEmitter = R"RaT(
	RAY.origin = gl_in[0].gl_Position.xyz;
	RAY.direction = gl_in[1].gl_Position.xyz;
)RaT";

std::string ShaderText::shdrRayTransform = R"RaT(
	RAY.origin = ( MAT_TRANSFORM_ORIGIN * vec4(RAY_P.origin, 1.0) ).xyz;
	RAY.direction = ( MAT_TRANSFORM_DIRECTION * vec4(RAY_P.direction, 1.0) ).xyz;
)RaT";

std::string ShaderText::shdrPlane = R"RaT(
	RAY.origin = (PLANE_POS_X - RAY_P.origin.x)*RAY_P.direction / RAY_P.direction.x + RAY_P.origin;
	RAY.direction = RAY_P.direction;
)RaT";

std::string ShaderText::shdrPlaneLens = R"RaT(
	RAY.origin = (PLANE_POS_X - RAY_P.origin.x)*RAY_P.direction / RAY_P.direction.x + RAY_P.origin;
	RAY.direction = refract( RAY_P.direction, vec3(-1.0, 0.0, 0.0), REFRACTION_INDEX );
)RaT";

std::string ShaderText::shdrSphereLens = R"RaT(
	RAY.origin = RAY_P.direction*intersectSphereF(RAY_P, LENS_POS, LENS_RADIUS) + RAY_P.origin;
	vec3 NORM = normalize(LENS_POS - RAY.origin);
	if (dot(RAY_P.direction, NORM) > 0.0) NORM = -NORM;
	RAY.direction = refract(RAY_P.direction, NORM, REFRACTION_INDEX);
)RaT";

std::string ShaderText::shdrAsphericLens = R"RaT(
	RAY.origin = aspherSolver( vec4(LENS_POS.x + LENS_WIDTH, 0, 0, LENS_POS.x),
		RAY_P, vec3(LENS_POS.x + LENS_WIDTH, LENS_POS.y, LENS_POS.z), LENS_COEF);
	vec3 NORM = normalize( vec3(-1.0, dPolyAspher( RAY.origin.y - LENS_POS.y, LENS_COEF), 0.0) );
	RAY.direction = refract( RAY_P.direction, NORM, REFRACTION_INDEX );
)RaT";

std::string ShaderText::shdrBTSBack = R"RaT(
	RAY.origin = BTSsolverBack( vec4(0.0), RAY_P, vec3(BTS_RADIUS, 0.0, 0.0));
	float NORM_F = dBTSequation( RAY.origin, BTS_RADIUS, BTS_PERIOD );
	vec3 NORM = normalize( vec3(-1.0, NORM_F, -NORM_F) );
	RAY.direction = refract( RAY_P.direction, NORM, REFRACTION_INDEX );
)RaT";

std::string ShaderText::shdrBTSFront = R"RaT(
	RAY.origin = BTSsolverFront( vec4(0.0), RAY_P, vec3(BTS_WIDTH - BTS_RADIUS, 0.0, 0.0));
	float NORM_F = dBTSequation( RAY.origin, BTS_RADIUS, BTS_PERIOD );
	vec3 NORM = normalize(vec3(-1.0, -NORM_F, NORM_F));
	RAY.direction = refract( RAY_P.direction, NORM, REFRACTION_INDEX );
)RaT";

std::string ShaderText::shdrPlaneMirror = R"RaT(
	RAY.origin = RAY_P.origin + RAY_P.direction * dot(NORM, -RAY_P.origin) / dot(NORM, RAY_P.direction);
	RAY.direction = reflect(RAY_P.direction, NORM);
)RaT";

std::string ShaderText::shdrSphereMirror = R"RaT(
	RAY.origin = RAY_P.direction*intersectSphereF(RAY_P, MIRROR_POS, MIRROR_RADIUS) + RAY_P.origin;
	vec3 NORM = normalize(MIRROR_POS - RAY.origin);
	RAY.direction = reflect(RAY_P.direction, NORM);
)RaT";


std::string ShaderText::shdrCheck = R"RaT(if ( all(not( isnan(RAY.origin) )) && all(not( isinf(RAY.origin) )) ))
)RaT";

std::string ShaderText::shdrOut = R"RaT(
		emit(COLOR_OUT, RAY_OUT);)RaT";

//GENERATION BLOCK END



/*
//BTS BLOCK BEGIN

//Уравнение поверхности BTS
R"RaT(
float BTSequation(vec3 P, const float radius, const float period) {
	float b = mod(P.y - P.z - period/2.0, period) - period / 2.0;
	return sqrt(radius*radius - b * b);
}
)RaT"
//Система из уравнений прямой и передней поверхности BTS
R"RaT(
vec4 BTSsystemFront(vec4 A, Ray ray, vec3 L0, const float radius, const float period) {
	return vec4(	A.w*ray.direction + ray.origin - A.xyz,
					BTSequation(A - L0, radius, period) + L0.x - A.x	);
}
)RaT"
//Система из уравнений прямой и задней поверхности BTS
R"RaT(
vec4 BTSsystemBack(vec4 A, Ray ray, vec3 L0, const float radius, const float period) {
	return vec4(	A.w*ray.direction + ray.origin - A.xyz,
					-BTSequation(A - L0, radius, period) + L0.x - A.x	);
}
)RaT"
//Производная уравнения поверхности BTS
R"RaT(
float dBTSequation(vec3 P, const float radius, const float period) {
	float b = mod(P.y - P.z - period/2.0, period) - period / 2.0;
	return b / sqrt(radius*radius - b * b);
}
)RaT"
//Якобиан системы для передней поверхности
R"RaT(
mat4 jacBTSequationFront(vec4 A, vec3 cosang, vec3 L0, const float radius, const float period) {
	mat4 mx = mat4(0.0);
	mx[0][0] = -1.0;
	mx[1][1] = -1.0;
	mx[2][2] = -1.0;
	mx[3][0] = cosang.x;
	mx[3][1] = cosang.y;
	mx[3][2] = cosang.z;
	mx[0][3] = -1.0;
	mx[1][3] = -dBTSequation(A - L0, radius, period);
	mx[2][3] = dBTSequation(A - L0, radius, period);
	return mx;
}
)RaT"
//Якобиан системы для задней поверхности
R"RaT(
mat4 jacBTSequationBack(vec4 A, vec3 cosang, vec3 L0, const float radius, const float period) {
	mat4 mx = mat4(0.0);
	mx[0][0] = -1.0;
	mx[1][1] = -1.0;
	mx[2][2] = -1.0;
	mx[3][0] = cosang.x;
	mx[3][1] = cosang.y;
	mx[3][2] = cosang.z;
	mx[0][3] = -1.0;
	mx[1][3] = dBTSequation(A - L0, radius, period);
	mx[2][3] = -dBTSequation(A - L0, radius, period);
	return mx;
}
)RaT"
//Решение системы уравнений методом Ньютона для передней поверхности
R"RaT(
vec3 BTSsolverFront(vec4 k0, Ray ray, vec3 L0, const float radius, const float period) {
	vec4 k1 = k0 - inverse(jacBTSequationFront(k0, ray.direction, L0, radius, period))*BTSsystemFront(k0, ray, L0, radius, period);
	float a = 0.0;
	while (length(k1 - k0) >= eps && a++ < iters) {
		k0 = k1;
		k1 = k0 - inverse(jacBTSequationFront(k0, ray.direction, L0, radius, period))*BTSsystemFront(k0, ray, L0, radius, period);
	}
	return k1.xyz;
}
)RaT"
//Решение системы уравнений методом Ньютона для задней поверхности
R"RaT(
vec3 BTSsolverBack(vec4 k0, Ray ray, vec3 L0, const float radius, const float period) {
	vec4 k1 = k0 - inverse(jacBTSequationBack(k0, ray.direction, L0, radius, period))*BTSsystemBack(k0, ray, L0, radius, period);
	float a = 0.0;
	while (length(k1 - k0) >= eps && a++ < iters) {
		k0 = k1;
		k1 = k0 - inverse(jacBTSequationBack(k0, ray.direction, L0, radius, period))*BTSsystemBack(k0, ray, L0, radius, period);
	}
	return k1.xyz;
}
)RaT"

//BTS BLOCK END
*/
