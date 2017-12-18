#define GLEW_STATIC
#define PI 3.14159265358979324
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include "GameRenderer.hpp"
#include "GameSettings.hpp"

const GLchar* fragmentShaderCode = "#version 440\n"
"uniform sampler2D tex;\n"
"uniform double objAlpha;\n"
"uniform vec3 objBlend;\n"
"uniform vec3 hardColour;\n"
"in vec2 fragTexCoord;\n"
"out vec4 colourOut;\n"
"void main() {\n"
"vec4 col = texture2D(tex, fragTexCoord);\n"
"if(hardColour.xyz == vec3(0, 0, 0)) {\n"
"colourOut = vec4((col.x * objBlend.x), (col.y * objBlend.y), (col.z * objBlend.z), (col.w * clamp(objAlpha, 0, 1)));\n"
"}\n"
"else {\n"
"colourOut = vec4(hardColour, 1.0);\n"
"}\n"
"}";

const GLchar* vertexShaderCode = "#version 440\n"
"in vec3 vert;\n"
"in vec2 vertTexCoord;\n"
"uniform mat4 translateToMiddle;\n"
"uniform mat4 rotate;\n"
"uniform mat4 translateBack;\n"
"uniform mat4 project;\n"
"uniform int calced;\n"
"out vec2 fragTexCoord;\n"

"void main() {\n"
"fragTexCoord = vertTexCoord;\n"
"gl_Position = project * translateBack * rotate * translateToMiddle * vec4(vert.x, vert.y, vert.z, 1);\n"
"}";


GameRenderer::GameRenderer() {
	window = NULL;
	_gpuTextures = 0;
}

GameRenderer::~GameRenderer() {
	for (RImage img : _images) {
		free(img.data);
	}
	_images.clear();
	glfwDestroyWindow(window); // This function is allowed be called on NULL
}

bool GameRenderer::MakeGameWindow(GameSettings* settings, unsigned int w, unsigned int h) {
	// Shouldn't really be necessary, but prevents a memory leak in the event this is called more than once.
	glfwDestroyWindow(window);

	// Create window
	window = glfwCreateWindow(w, h, "", NULL, NULL);
	if (!window) {
		return false;
	}
	windowW = w;
	windowH = h;

	// Make this the current GL context for this thread. Rendering assumes that this will never be changed.
	glfwMakeContextCurrent(window);
	contextSet = true;

	// Init GLEW - must be done after context creation
	glewExperimental = GL_TRUE;
	glewInit();
	if (glewInit()) {
		// Failed to init GLEW
		return false;
	}

	// Required GL features
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_LIGHTING);
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, (GLint*)(&_maxGpuTextures));
	glClearColor((GLclampf)(settings->colourOutsideRoom&0xFF000000)/0xFF000000, (GLclampf)(settings->colourOutsideRoom&0xFF0000)/0xFF0000, (GLclampf)(settings->colourOutsideRoom&0xFF00)/0xFF00, (GLclampf)(settings->colourOutsideRoom&0xFF)/0xFF);
	glClear(GL_COLOR_BUFFER_BIT);

	// Make shaders
	GLint vertexCompiled, fragmentCompiled, linked;
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLint vSize = (GLint)strlen(vertexShaderCode);
	GLint fSize = (GLint)strlen(fragmentShaderCode);
	glShaderSourceARB(vertexShader, 1, &vertexShaderCode, &vSize);
	glShaderSourceARB(fragmentShader, 1, &fragmentShaderCode, &fSize);
	glCompileShaderARB(vertexShader);
	glCompileShaderARB(fragmentShader);
	glGetObjectParameterivARB(vertexShader, GL_COMPILE_STATUS, &vertexCompiled);
	glGetObjectParameterivARB(fragmentShader, GL_COMPILE_STATUS, &fragmentCompiled);
	if ((!vertexCompiled) || (!fragmentCompiled)) {
		// Failed to compile shaders
		return false;
	}
	_glProgram = glCreateProgram();
	glAttachShader(_glProgram, vertexShader);
	glAttachShader(_glProgram, fragmentShader);
	glLinkProgram(_glProgram);
	glGetProgramiv(_glProgram, GL_LINK_STATUS, &linked);
	if (!linked) {
		// Failed to link GL program
		return false;
	}
	glUseProgram(_glProgram);

	// Make textures
	GLuint* ix = new GLuint[_images.size()];
	glGenTextures((GLsizei)_images.size(), ix);
	for (unsigned int i = 0; i < _images.size(); i++) {
		_images[i].glTexObject = ix[i];
	}
	delete ix;

	// Make VAO
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	// Make VBO
	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	GLfloat vertexData[] = {
		0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, vertexData, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);

	return true;
}

void GameRenderer::ResizeGameWindow(unsigned int w, unsigned int h) {
	// Only do this if the w and h settings aren't equal to what size the window was last set to - even if the user has resized it since then.
	if (w != windowW || h != windowH) {
		glfwSetWindowSize(window, w, h);
		windowW = w;
		windowH = h;
	}
}

void GameRenderer::SetGameWindowTitle(const char* title) {
	glfwSetWindowTitle(window, title);
}

void GameRenderer::GetCursorPos(double* xpos, double* ypos) {
	glfwGetCursorPos(window, xpos, ypos);
}

bool GameRenderer::ShouldClose() {
	return glfwWindowShouldClose(window);
}

void GameRenderer::SetBGColour(unsigned int col) {
	bgR = (double)(col & 0xFF) / 0xFF;
	bgG = (double)((col >> 8) & 0xFF) / 0xFF;
	bgB = (double)((col >> 16) & 0xFF) / 0xFF;
}

RImageIndex GameRenderer::MakeImage(unsigned int w, unsigned int h, unsigned int originX, unsigned int originY, unsigned char * bytes) {
	RImage img;
	img.w = w;
	img.h = h;
	img.originX = originX;
	img.originY = originY;
	img.registered = false;
	img.data = (unsigned char*)malloc(w * h * 4);
	memcpy(img.data, bytes, (w * h * 4));

	if (contextSet) {
		GLuint ix;
		glGenTextures(1, &ix);
		img.glTexObject = ix;
	}

	_images.push_back(img);
	return ((unsigned int)_images.size()) - 1;
}

void GameRenderer::DrawImage(RImageIndex ix, double x, double y, double xscale, double yscale, double rot, unsigned int blend, double alpha) {
	RImage* img = _images._Myfirst() + ix;

	// Bind shader values needed for GPU calculations
	double dRot = rot * PI / 180;
	GLfloat sinRot = (GLfloat)sin(dRot);
	GLfloat cosRot = (GLfloat)cos(dRot);
	GLfloat translateToMiddle[16] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		-((GLfloat)img->originX / img->w), -((GLfloat)img->originY / img->h), 0, 1
	};
	GLfloat rotate[16] = {
		cosRot, sinRot, 0, 0,
		-sinRot, cosRot, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	GLfloat translateBack[16] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		((GLfloat)img->originX / img->w), ((GLfloat)img->originY / img->h), 0, 1
	};
	GLfloat project[16] = {
		(GLfloat)(img->w * xscale * 2.0 / windowW), 0, 0, 0,
		0, -(GLfloat)(img->h * yscale * 2.0 / windowH), 0, 0,
		0, 0, 1, 0,
		(GLfloat)(((x - (img->originX * xscale)) / windowW) * 2.0 - 1.0), -(GLfloat)(((y - (img->originY * yscale)) / windowH) * 2.0 - 1.0), 0, 1
	};
	glUniformMatrix4fv(glGetUniformLocation(_glProgram, "translateToMiddle"), 1, GL_FALSE, translateToMiddle);
	glUniformMatrix4fv(glGetUniformLocation(_glProgram, "rotate"), 1, GL_FALSE, rotate);
	glUniformMatrix4fv(glGetUniformLocation(_glProgram, "translateBack"), 1, GL_FALSE, translateBack);
	glUniformMatrix4fv(glGetUniformLocation(_glProgram, "project"), 1, GL_FALSE, project);
	glUniform1d(glGetUniformLocation(_glProgram, "objAlpha"), alpha);
	glUniform3f(glGetUniformLocation(_glProgram, "objBlend"), (GLfloat)(blend & 0xFF) / 0xFF, (GLfloat)(blend & 0xFF00) / 0xFF00, (GLfloat)(blend & 0xFF0000) / 0xFF0000);

	if (img->registered) {
		// This image is already registered in the GPU, so we can draw it.
		_DrawTex(img->glIndex, img->glTexObject);
	}
	else {
		// We need to register this image in the GPU first.
		if (_gpuTextures < _maxGpuTextures) {
			// There's room for more images in the GPU, so we'll register it to an unused one and then draw it.
			_RegImg(_gpuTextures, img);
			img->glIndex = _gpuTextures;
			img->registered = true;
			_DrawTex(_gpuTextures, img->glTexObject);
			_gpuTextures++;
		}
		else {
			// TODO: we've exceeded the max number of allowed textures, so at this point we'll have to recycle an old one.
			// theory: add a frame counter that's incremented every call to RenderFrame() and keep track of when each image
			// was last used. When we reach this code point, unload the one that was used longest ago.
		}
	}
}

void GameRenderer::RenderFrame() {
	int actualWinW, actualWinH;
	glfwGetWindowSize(window, &actualWinW, &actualWinH);
	glViewport(0, 0, actualWinW, actualWinH);
	glfwSwapBuffers(window);
	glfwPollEvents(); // This probably won't be here later on, but it needs to happen every frame for the window to update properly.
	glUseProgram(_glProgram);
	glClear(GL_COLOR_BUFFER_BIT);

	GLfloat ident[16] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	GLfloat proj[16] = {
		2, 0, 0, 0,
		0, 2, 0, 0,
		0, 0, 1, 0,
		-1, -1, 0, 1
	};
	glUniformMatrix4fv(glGetUniformLocation(_glProgram, "translateToMiddle"), 1, GL_FALSE, ident);
	glUniformMatrix4fv(glGetUniformLocation(_glProgram, "rotate"), 1, GL_FALSE, ident);
	glUniformMatrix4fv(glGetUniformLocation(_glProgram, "translateBack"), 1, GL_FALSE, ident);
	glUniformMatrix4fv(glGetUniformLocation(_glProgram, "project"), 1, GL_FALSE, proj);

	GLint vertTexCoord = glGetAttribLocation(_glProgram, "vertTexCoord");
	glEnableVertexAttribArray(vertTexCoord);
	glUniform3f(glGetUniformLocation(_glProgram, "hardColour"), bgR, bgG, bgB);
	glVertexAttribPointer(vertTexCoord, 2, GL_FLOAT, GL_TRUE, 3 * sizeof(GLfloat), 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glUniform3f(glGetUniformLocation(_glProgram, "hardColour"), 0, 0, 0);
}



// Private

void GameRenderer::_DrawTex(unsigned int ix, GLuint obj) {
	GLint tex = glGetUniformLocation(_glProgram, "tex");
	GLint vertTexCoord = glGetAttribLocation(_glProgram, "vertTexCoord");
	glUniform1i(tex, ix);
	glActiveTexture(GL_TEXTURE0 + ix);
	glBindTexture(GL_TEXTURE_2D, obj);
	glVertexAttribPointer(vertTexCoord, 2, GL_FLOAT, GL_TRUE, 3 * sizeof(GLfloat), 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindTexture(GL_TEXTURE_2D, NULL);
	glActiveTexture(NULL);
}

void GameRenderer::_RegImg(unsigned int ix, RImage* i) {
	glActiveTexture(GL_TEXTURE0 + ix);
	glBindTexture(GL_TEXTURE_2D, i->glTexObject);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, NULL, GL_RGBA, i->w, i->h, NULL, GL_RGBA, GL_UNSIGNED_BYTE, i->data);
	glBindTexture(GL_TEXTURE_2D, NULL);
}