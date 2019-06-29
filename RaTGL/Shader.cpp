#include "stdafx.h"
#include "Shader.h"

using namespace std;

GLuint Shader::Point::programHandle = 0;
GLuint Shader::Line::programHandle = 0;
GLuint Shader::Triag::programHandle = 0;
GLuint Shader::textureTriag::programHandle = 0;
GLuint Shader::Graph::programHandle = 0;
GLuint Shader::Text::programHandle = 0;
GLuint Shader::Ticks::programHandle = 0;
GLuint Shader::Background::programHandle = 0;

vector<GLuint> Shader::Line::Uniforms;

GLuint
Shader::Line::Uniform::uMatHandle = 0,
Shader::Point::Uniform::uMatHandle = 0,
Shader::Point::Uniform::uColorHandle = 0,
Shader::Triag::Uniform::uMatHandle = 0,
Shader::Triag::Uniform::uColorHandle = 0,
Shader::textureTriag::Uniform::uMatHandle = 0,
Shader::textureTriag::Uniform::uTexture0 = 0,
Shader::Graph::Uniform::uMatHandle = 0,
Shader::Graph::Uniform::uColor = 0,
Shader::Text::Uniform::uScale = 0,
Shader::Text::Uniform::uTranslate = 0,
Shader::Text::Uniform::uFontShift = 0,
Shader::Text::Uniform::uColor = 0,
Shader::Text::Uniform::uFont = 0,
Shader::Ticks::Uniform::uScale = 0,
Shader::Ticks::Uniform::uTranslate = 0,
Shader::Ticks::Uniform::uGap = 0,
Shader::Ticks::Uniform::uColor = 0;

void Shader::INIT() {
	Point::programHandle = Load("Res/Shaders/point");
	Triag::programHandle = Load("Res/Shaders/triag");
	textureTriag::programHandle = Load("Res/Shaders/textureTriag");
	Graph::programHandle = Load("Res/Shaders/graph");
	Text::programHandle = Load("Res/Shaders/text");
	Ticks::programHandle = Load("Res/Shaders/ticks");
	Background::programHandle = Load("Res/Shaders/background");

	Point::Uniform::uMatHandle = glGetUniformLocation(Point::programHandle, "u_viewMat");
	Point::Uniform::uColorHandle = glGetUniformLocation(Point::programHandle, "u_color");
	Triag::Uniform::uMatHandle = glGetUniformLocation(Triag::programHandle, "u_viewMat");
	Triag::Uniform::uColorHandle = glGetUniformLocation(Triag::programHandle, "u_diffColor");
	textureTriag::Uniform::uMatHandle = glGetUniformLocation(textureTriag::programHandle, "u_viewMat");
	textureTriag::Uniform::uTexture0 = glGetUniformLocation(textureTriag::programHandle, "s2D_Texture0");
	Graph::Uniform::uMatHandle = glGetUniformLocation(Graph::programHandle, "u_viewMat");
	Graph::Uniform::uColor = glGetUniformLocation(Graph::programHandle, "u_color");
	Text::Uniform::uColor = glGetUniformLocation(Text::programHandle, "u_color");
	Text::Uniform::uFont = glGetUniformLocation(Text::programHandle, "s2D_font");
	Text::Uniform::uTranslate = glGetUniformLocation(Text::programHandle, "u_translate");
	Text::Uniform::uScale = glGetUniformLocation(Text::programHandle, "u_scale");
	Text::Uniform::uFontShift = glGetUniformLocation(Text::programHandle, "u_fontShift");
	Ticks::Uniform::uTranslate = glGetUniformLocation(Ticks::programHandle, "u_translate");
	Ticks::Uniform::uScale = glGetUniformLocation(Ticks::programHandle, "u_scale");
	Ticks::Uniform::uGap = glGetUniformLocation(Ticks::programHandle, "u_gap");
	Ticks::Uniform::uColor = glGetUniformLocation(Ticks::programHandle, "u_color");
}
void Shader::Destroy() {
	glUseProgram(0);
	glDeleteProgram(Point::programHandle);
	glDeleteProgram(Line::programHandle);
	glDeleteProgram(Triag::programHandle);
	glDeleteProgram(textureTriag::programHandle);
	glDeleteProgram(Graph::programHandle);
	glDeleteProgram(Text::programHandle);
	glDeleteProgram(Ticks::programHandle);
	glDeleteProgram(Background::programHandle);
}

GLuint Shader::Load(string fname, const GLchar ** vargs, GLsizei size) {
	GLuint vertexShaderHandle = subLoad(fname + ".vsr", GL_VERTEX_SHADER);
	GLuint geometryShaderHandle = subLoad(fname + ".gsr", GL_GEOMETRY_SHADER);
	GLuint fragmentShaderHandle = subLoad(fname + ".fsr", GL_FRAGMENT_SHADER);
	GLuint programHandle = glCreateProgram();
	if (programHandle != 0)
	{
		glAttachShader(programHandle, vertexShaderHandle);
		if (geometryShaderHandle) glAttachShader(programHandle, geometryShaderHandle);
		glAttachShader(programHandle, fragmentShaderHandle);
		if (vargs != 0 && size != 0) glTransformFeedbackVaryings(programHandle, size, vargs, GL_INTERLEAVED_ATTRIBS);
		glLinkProgram(programHandle);

		GLint linkStatus;
		glGetProgramiv(programHandle, GL_LINK_STATUS, &linkStatus);
		if (linkStatus == 0)
		{
			cout << fname << " Link ERROR\n";
			glDeleteProgram(programHandle);
			programHandle = 0;
		}
		else cout << fname << " Link OK\n";
	}
	else cout << fname << " CreateProgram ERROR\n";

	glDetachShader(programHandle, vertexShaderHandle);
	if (geometryShaderHandle) glDetachShader(programHandle, geometryShaderHandle);
	glDetachShader(programHandle, fragmentShaderHandle);
	glDeleteShader(vertexShaderHandle);
	if (geometryShaderHandle) glDeleteShader(geometryShaderHandle);
	glDeleteShader(fragmentShaderHandle);
	glReleaseShaderCompiler();

	return programHandle;
}

GLuint Shader::subLoad(string fname, GLenum type) {
	string shaderString = "";

	ifstream shaderFile(fname, ios::in | ios_base::beg);
	if (shaderFile.is_open())
	{
		string tmp;
		while (getline(shaderFile, tmp))
			shaderString += tmp + '\n';
		shaderFile.close();
	}
	else return 0;
	GLuint shaderHandle = glCreateShader(type);
	if (shaderHandle != 0)
	{
		const GLchar *c = shaderString.c_str();
		glShaderSource(shaderHandle, 1, &c, 0);
		glCompileShader(shaderHandle);

		GLint compileStatus;
		glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileStatus);
		if (compileStatus == 0)
		{
			GLsizei len;
			GLchar inf[512];
			glGetShaderInfoLog(shaderHandle, 512, &len, inf);
			cout << fname << " Compiler ERROR:\n" << inf << endl;
			glDeleteShader(shaderHandle);
			shaderHandle = 0;
		}
		else cout << fname << " Compile OK\n";
	}
	else cout << fname << " CreateShader ERROR\n";
	return shaderHandle;
}

void Shader::createBuffer(GLenum usage, GLuint *vaoId, GLuint *bufferId, float* buffer, GLsizeiptr bufferSize, int *attributeSizes, GLuint attribSize, int feedbackOffset) {
	glGenVertexArrays(1, vaoId);
	RETURNonERROR(vaoId, "GenVertexArrays ERROR\n");

	glGenBuffers(1, bufferId);
	RETURNonERROR(bufferId, "GenBuffers ERROR\n");

	glBindVertexArray(*vaoId);
	glBindBuffer(GL_ARRAY_BUFFER, *bufferId);
	glBufferData(GL_ARRAY_BUFFER, bufferSize, buffer, usage);
	GLsizei stride = feedbackOffset;
	for (GLuint i = 0; i < attribSize; ++i)
		stride += attributeSizes[i];
	for (GLuint i = 0, localOffset = 0; i < attribSize; ++i) {
		glVertexAttribPointer(i, attributeSizes[i], GL_FLOAT, GL_FALSE, sizeof(float) * stride, (GLchar*)0 + sizeof(float) * (localOffset + feedbackOffset));
		localOffset += attributeSizes[i];
	}
	if (feedbackOffset > 0) glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, *bufferId);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Shader::InitRays(string* u, size_t count) {
	glUseProgram(0);
	if (Line::programHandle) glDeleteProgram(Line::programHandle);

	const GLchar *vargs[] = { "feedbackOut" };
	Line::programHandle = Load("Res/Shaders/line", vargs, sizeof(vargs) / sizeof(GLchar *));
	Line::Uniform::uMatHandle = glGetUniformLocation(Line::programHandle, "u_viewMat");
	Line::Uniforms.clear();
	for (size_t i = 0; i < count; ++i)
		Line::Uniforms.push_back(glGetUniformLocation(Line::programHandle, u[i].c_str()));
}
