#pragma once

class Shader {
public:
	static void INIT();
	static void createBuffer(GLenum usage, GLuint *vaoId, GLuint *bufferId, const float* buffer, GLsizeiptr bufferSize, const int *attributeSizes, GLuint attribSize, int offset = 0);
	static void InitRays(std::string* u, size_t count);
	static void Destroy();

	struct Line {
		static GLuint programHandle;
		struct Uniform {
			static GLuint uMatHandle;
		};
		static std::vector<GLuint> Uniforms;
		enum Input {
			Position
		};
	};

	struct Point {
		static GLuint programHandle;
		struct Uniform {
			static GLuint uMatHandle, uColorHandle;
		};
		enum Input {
			Position
		};
	};

	struct Triag {
		static GLuint programHandle;
		struct Uniform {
			static GLuint uMatHandle, uColorHandle;
		};
		enum Input {
			Position, Normal
		};
	};

	struct textureTriag {
		static GLuint programHandle;
		struct Uniform {
			static GLuint uMatHandle, uTexture0;
		};
		enum Input {
			Position, TextureCoord
		};
	};

	struct Graph {
		static GLuint programHandle;
		struct Uniform {
			static GLuint uMatHandle, uColor;
		};
		enum Input {
			Position
		};
	};

	struct Text {
		static GLuint programHandle;
		struct Uniform {
			static GLuint uColor, uFont, uScale, uTranslate, uFontShift;
		};
		enum Input {
			Position, TextureCoord
		};
	};

	struct Ticks {
		static GLuint programHandle;
		struct Uniform {
			static GLuint uScale, uTranslate, uGap, uColor;
		};
		enum Input {
			Position
		};
	};

	struct Background {
		static GLuint programHandle;
		enum Input {
			Position
		};
	};

private:
	static GLuint Load(std::string fname, const GLchar ** vargs = 0, GLsizei size = 0);
	static GLuint subLoad(std::string fname, GLenum type);
};
