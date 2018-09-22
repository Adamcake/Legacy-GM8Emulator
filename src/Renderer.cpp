#include <pch.h>
#define PI 3.14159265358979324
#include "Renderer.hpp"
#include "GameSettings.hpp"
#include "InputHandler.hpp"

const GLchar* fragmentShaderCode = "#version 330\n"
"uniform sampler2D tex;\n"
"uniform float objAlpha;\n"
"uniform vec3 objBlend;\n"
"uniform vec2 atlasXY;\n"
"uniform vec2 atlasWH;\n"
"in vec2 fragTexCoord;\n"
"out vec4 colourOut;\n"
"void main() {\n"
"vec4 col = texture(tex, vec2(atlasXY.x + (fragTexCoord.x * atlasWH.x), atlasXY.y + (fragTexCoord.y * atlasWH.y)));\n"
"colourOut = vec4((col.x * objBlend.x), (col.y * objBlend.y), (col.z * objBlend.z), (col.w * objAlpha));\n"
"}";

const GLchar* vertexShaderCode = "#version 330\n"
"in vec3 vert;\n"
"in vec2 vertTexCoord;\n"
"uniform mat4 project;\n"
"out vec2 fragTexCoord;\n"
"void main() {\n"
"fragTexCoord = vertTexCoord;\n"
"gl_Position = project * vec4(vert.x, vert.y, vert.z, 1);\n"
"}";


void mat4Mult(const GLfloat* lhs, const GLfloat* rhs, GLfloat* out) {
	for (unsigned int y = 0; y < 4; y++) {
		for (unsigned int x = 0; x < 4; x++) {
			out[(y * 4) + x] = (lhs[y * 4] * rhs[x]) + (lhs[(y * 4) + 1] * rhs[x + 4]) + (lhs[(y * 4) + 2] * rhs[x + 8]) + (lhs[(y * 4) + 3] * rhs[x + 12]);
		}
	}
}

// Images that need to be batched into an atlas - linked list structure to allow for easy sorting
struct RPreImage {
	unsigned int w;
	unsigned int h;
	unsigned char* data;
	unsigned int imgIndex;
	RPreImage* next;
};

// References to images that may or may not have yet been put into an atlas (usually happens in RMakeGameWindow)
struct RAtlasImage {
	unsigned int atlasId;
	unsigned int x;
	unsigned int y;
	unsigned int w;
	unsigned int h;
	unsigned int originX;
	unsigned int originY;
};

// Represents a command to draw a single image
struct RDrawCommand {
	GLfloat transform[16];
	GLfloat alpha;
	GLfloat blend[3];
	GLfloat atlasXY[2];
	GLfloat atlasWH[2];

	unsigned int atlasId;
	unsigned int imageIndex;
	GLint atlasGlTex;
};
std::vector<RDrawCommand> _drawCommands;

// Atlas structure, can exist without being used
struct RAtlas {
	GLuint glTex;
	unsigned int w;
	unsigned int h;
};

RPreImage* _preImages;
std::vector<RAtlasImage> _atlasImages;
RAtlas _atlases[32];
int boundAtlas;


// Builds all added images into atlases so that they can be drawn. Must be called before attempting to draw. Only intended to be called once.
bool _Compile(unsigned int firstAtlas);

unsigned int _tallest;
unsigned int _widest;
unsigned int _pixelCount;

unsigned int _colourOutsideRoom;
unsigned int _roomBGColour;

GLFWwindow* _window;
bool _contextSet;
unsigned int _windowW;
unsigned int _windowH;

unsigned int _maxTextures;
unsigned int _maxTextureSize;

GLuint _glProgram;
GLuint _vao;
GLuint _vbo;



void RInit() {
	_window = NULL;
	_contextSet = false;
	_tallest = 0;
	_widest = 0;
	_pixelCount = 0;
	_preImages = NULL;
	boundAtlas = -1;
}

void RTerminate() {
	RPreImage* n = _preImages;
	while (n != NULL) {
		RPreImage* n2 = n->next;
		free(n->data);
		free(n);
		n = n2;
	}
	glfwDestroyWindow(_window); // This function is allowed be called on NULL
	glfwTerminate();
}

bool RMakeGameWindow(GameSettings* settings, unsigned int w, unsigned int h) {
	// Fail if we already did this
	if (_contextSet) return false;

	// Init glfw
	if (!glfwInit()) {
		// Failed to initialize GLFW
		return false;
	}

	// Create window
	_window = glfwCreateWindow(w, h, "", NULL, NULL);
	if (!_window) {
		return false;
	}
	_windowW = w;
	_windowH = h;

	// Make this the current GL context for this thread. Rendering assumes that this will never be changed.
	glfwMakeContextCurrent(_window);
	_contextSet = true;

	// Init GLEW - must be done after context creation
	glewExperimental = GL_TRUE;
	if (glewInit()) {
		// Failed to init GLEW
		return false;
	}

	// Required GL features
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_LIGHTING);
	glEnable(GL_SCISSOR_TEST);
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, (GLint*)(&_maxTextures));
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)(&_maxTextureSize));
	_colourOutsideRoom = settings->colourOutsideRoom;
	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapInterval(0);
	InputInit(_window);

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

	// Make texture atlases
	if (!_Compile(0)) return false;

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

	return !glGetError();
}

void RResizeGameWindow(unsigned int w, unsigned int h) {
	// Only do this if the w and h settings aren't equal to what size the window was last set to - even if the user has resized it since then.
	if (w != _windowW || h != _windowH) {
		glfwSetWindowSize(_window, w, h);
		_windowW = w;
		_windowH = h;
	}
}

void RSetGameWindowTitle(const char* title) {
	glfwSetWindowTitle(_window, title);
}

void RGetCursorPos(int* xpos, int* ypos) {
	double xp, yp;
	int actualWinW, actualWinH;
	glfwGetCursorPos(_window, &xp, &yp);
	glfwGetWindowSize(_window, &actualWinW, &actualWinH);
	xp /= actualWinW;
	yp /= actualWinH;
	xp *= _windowW;
	yp *= _windowH;
	if(xpos) (*xpos) = (int)xp;
	if(ypos) (*ypos) = (int)yp;
}

bool RShouldClose() {
	return glfwWindowShouldClose(_window);
}

void RSetBGColour(unsigned int col) {
	_roomBGColour = col;
}

RImageIndex RMakeImage(unsigned int w, unsigned int h, unsigned int originX, unsigned int originY, unsigned char * bytes) {
	if (_contextSet) {
		// Image being registered after the game is already loaded - what do we do here!?
		abort(); // I don't know exactly what this does, but it's definitely better than what would happen if I didn't do it
	}

	// Make pre-image object for later compiling into an atlas
	RPreImage* pImg = new RPreImage;
	pImg->w = w;
	pImg->h = h;
	pImg->data = (unsigned char*)malloc(w * h * 4);
	pImg->next = NULL;
	memcpy(pImg->data, bytes, (w * h * 4));

	// Make reference object for this image
	RAtlasImage aImg;
	aImg.w = w;
	aImg.h = h;
	aImg.originX = originX;
	aImg.originY = originY;
	pImg->imgIndex = static_cast<unsigned int>(_atlasImages.size());
	_atlasImages.push_back(aImg);

	// Sort pre-image into the linked list by descending pixel count (ie largest first)
	unsigned int wh = w * h;
	RPreImage** n = &_preImages;
	while (true) {
		RPreImage* i = *n;
		if (i == NULL) {
			(*n) = pImg;
			break;
		}
		if (wh >= i->w * i->h) {
			pImg->next = i;
			(*n) = pImg;
			break;
		}
		n = &(i->next);
	}

	// Keep track of the widest and tallest sprites and the total number of pixels added
	_tallest = std::max(_tallest, w);
	_widest = std::max(_widest, h);
	_pixelCount += (w * h);

	return pImg->imgIndex;
}

void RDrawImage(RImageIndex ix, double x, double y, double xscale, double yscale, double rot, unsigned int blend, double alpha) {
	RAtlasImage* r = _atlasImages._Myfirst() + ix;
	RDrawPartialImage(ix, x, y, xscale, yscale, rot, blend, alpha, 0, 0, r->w, r->h);
}

void RDrawPartialImage(RImageIndex ix, double x, double y, double xscale, double yscale, double rot, unsigned int blend, double alpha, unsigned int partX, unsigned int partY, unsigned int partW, unsigned int partH) {
	RDrawCommand command;

	RAtlasImage* aImg = _atlasImages._Myfirst() + ix;

	// Calculate a single matrix for scaling and transforming the sprite
	double dRot = rot * PI / 180;
	GLfloat sinRot = (GLfloat)sin(dRot);
	GLfloat cosRot = (GLfloat)cos(dRot);
	GLfloat dx = ((GLfloat)aImg->originX / aImg->w);
	GLfloat dy = ((GLfloat)aImg->originY / aImg->h);

	// TODO: We can optimize this a lot by pre-calculating the final matrix instead of multiplying a ton of hard-coded values for every draw.
	//       But that should wait until we've implemented views, as that will affect the result.
	GLfloat toMiddle[16] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		-dx, -dy, 0, 1
	};
	GLfloat scale[16] = {
		(GLfloat)(partW * xscale), 0, 0, 0,
		0,(GLfloat)(partH * -yscale), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	GLfloat rotate[16] = {
		cosRot, sinRot, 0, 0,
		-sinRot, cosRot, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	GLfloat scale2[16] = {
		2.0f / _windowW, 0, 0, 0,
		0, 2.0f / _windowH, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	GLfloat transform[16] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		(GLfloat)(x * 2.0f / _windowW) - 1, -(GLfloat)((y * 2.0f / _windowH) - 1), 0, 1
	};


	GLfloat tmp[16];
	GLfloat project[16];

	mat4Mult(toMiddle, scale, tmp);
	mat4Mult(tmp, rotate, project);
	mat4Mult(project, scale2, tmp);
	mat4Mult(tmp, transform, project);

	RAtlas* atlas = &_atlases[aImg->atlasId];

	// Create draw command
	memcpy(command.transform, project, 16 * sizeof(GLfloat));
	command.alpha = (GLfloat)alpha;
	command.blend[0] = (GLfloat)(blend & 0xFF) / 0xFF;
	command.blend[1] = (GLfloat)(blend & 0xFF00) / 0xFF00;
	command.blend[2] = (GLfloat)(blend & 0xFF0000) / 0xFF0000;
	command.atlasXY[0] = (GLfloat)((double)(aImg->x + partX) / (double)atlas->w);
	command.atlasXY[1] = (GLfloat)((double)(aImg->y + partY) / (double)atlas->h);
	command.atlasWH[0] = (GLfloat)((double)(partW) / (double)atlas->w);
	command.atlasWH[1] = (GLfloat)((double)(partH) / (double)atlas->h);
	command.imageIndex = ix;
	command.atlasId = aImg->atlasId;
	command.atlasGlTex = atlas->glTex;
	_drawCommands.push_back(command);

	// Bind uniform shader values
	//glUniformMatrix4fv(glGetUniformLocation(_glProgram, "project"), 1, GL_FALSE, project);
	//glUniform1f(glGetUniformLocation(_glProgram, "objAlpha"), (GLfloat)alpha);
	//glUniform3f(glGetUniformLocation(_glProgram, "objBlend"), (GLfloat)(blend & 0xFF) / 0xFF, (GLfloat)(blend & 0xFF00) / 0xFF00, (GLfloat)(blend & 0xFF0000) / 0xFF0000);
	//glUniform2f(glGetUniformLocation(_glProgram, "atlasXY"), (GLfloat)((double)(aImg->x + partX) / (double)atlas->w), (GLfloat)((double)(aImg->y + partY) / (double)atlas->h));
	//glUniform2f(glGetUniformLocation(_glProgram, "atlasWH"), (GLfloat)((double)(partW) / (double)atlas->w), (GLfloat)((double)(partH) / (double)atlas->h));

	// Do drawing
	//GLint tex = glGetUniformLocation(_glProgram, "tex");
	//GLint vertTexCoord = glGetAttribLocation(_glProgram, "vertTexCoord");
	//glUniform1i(tex, aImg->atlasId);
	//if (boundAtlas != aImg->atlasId) {
	//	glActiveTexture(GL_TEXTURE0 + aImg->atlasId);
	//	glBindTexture(GL_TEXTURE_2D, atlas->glTex);
	//	boundAtlas = aImg->atlasId;
	//}
	//glVertexAttribPointer(vertTexCoord, 2, GL_FLOAT, GL_TRUE, 3 * sizeof(GLfloat), 0);
	//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


void RStartFrame() {
	int actualWinW, actualWinH;
	glfwGetWindowSize(_window, &actualWinW, &actualWinH);
	glUseProgram(_glProgram);
	glClearColor((GLclampf)(_colourOutsideRoom & 0xFF) / 0xFF, (GLclampf)((_colourOutsideRoom >> 8) & 0xFF) / 0xFF, (GLclampf)((_colourOutsideRoom >> 16) & 0xFF) / 0xFF, (GLclampf)1.0);
	glViewport(0, 0, actualWinW, actualWinH);
	glScissor(0, 0, actualWinW, actualWinH);
	glClear(GL_COLOR_BUFFER_BIT);

	// Later, we'll use this for clearing the background of each active view's viewport. But views aren't supported right now.
	glClearColor((GLclampf)(_roomBGColour & 0xFF) / 0xFF, (GLclampf)((_roomBGColour >> 8) & 0xFF) / 0xFF, (GLclampf)((_roomBGColour >> 16) & 0xFF) / 0xFF, (GLclampf)1.0);
	glViewport(0, 0, actualWinW, actualWinH);
	glScissor(0, 0, actualWinW, actualWinH);
	glClear(GL_COLOR_BUFFER_BIT);

	GLint vertTexCoord = glGetAttribLocation(_glProgram, "vertTexCoord");
	glEnableVertexAttribArray(vertTexCoord);

	_drawCommands.clear();
}

void RRenderFrame() {
	int actualWinW, actualWinH;
	glfwGetWindowSize(_window, &actualWinW, &actualWinH);

	for (RDrawCommand command : _drawCommands) {
		// Bind uniform shader values
		glUniformMatrix4fv(glGetUniformLocation(_glProgram, "project"), 1, GL_FALSE, command.transform);
		glUniform1f(glGetUniformLocation(_glProgram, "objAlpha"), command.alpha);
		glUniform3f(glGetUniformLocation(_glProgram, "objBlend"), command.blend[0], command.blend[1], command.blend[2]);
		glUniform2f(glGetUniformLocation(_glProgram, "atlasXY"), command.atlasXY[0], command.atlasXY[1]);
		glUniform2f(glGetUniformLocation(_glProgram, "atlasWH"), command.atlasWH[0], command.atlasWH[1]);
		glUniform1i(glGetUniformLocation(_glProgram, "tex"), command.atlasId);

		// Do drawing
		if (boundAtlas != command.atlasId) {
			glActiveTexture(GL_TEXTURE0 + command.atlasId);
			glBindTexture(GL_TEXTURE_2D, command.atlasGlTex);
			boundAtlas = command.atlasId;
		}
		glVertexAttribPointer(glGetAttribLocation(_glProgram, "vertTexCoord"), 2, GL_FLOAT, GL_TRUE, 3 * sizeof(GLfloat), 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	glViewport(0, 0, actualWinW, actualWinH);
	glfwSwapBuffers(_window);
}



bool _Compile(unsigned int firstAtlas) {
	// Sanity checks
	if (_tallest > _maxTextureSize) return false;
	if (_widest > _maxTextureSize) return false;
	if (firstAtlas > _maxTextures) return false;
	if (_pixelCount == 0) return true;

	// Decide how large to make the atlas initially (may be expanded later if required)
	/*
	unsigned int size;
	unsigned int c = 1;
	unsigned int p = _pixelCount;
	while (p != 1) {
		p >>= 1;
		c++;
	}
	if (c & 1) c++;
	size = std::max((unsigned int)(1 << (c / 2)), std::max(_tallest, _widest));
	*/
	
	// Create usage map
	bool* used = new bool[_maxTextureSize * _maxTextureSize];
	unsigned int usedWidth = 0;
	unsigned int usedHeight = 0;
	for (unsigned int i = 0; i < (_maxTextureSize * _maxTextureSize); i++) used[i] = false;

	// Keep linked lists of PreImages that we did and didn't pack
	RPreImage* packed = NULL;
	RPreImage** packedNext = &packed;
	RPreImage* notPacked = NULL;
	RPreImage** notPackedNext = &notPacked;

	// Try to pack each image
	std::vector<unsigned int> firstFree;
	firstFree.reserve(_maxTextureSize);
	while (_preImages != NULL) {

		// Special case for the first image
		if (usedWidth == 0 || usedHeight == 0) {
			usedWidth = _preImages->w;
			usedHeight = _preImages->h;
			for (unsigned int iY = 0; iY < _preImages->h; iY++) {
				memset(used + (iY * _maxTextureSize), true, _preImages->w * sizeof(bool));
			}
			_atlasImages[_preImages->imgIndex].atlasId = firstAtlas;
			_atlasImages[_preImages->imgIndex].x = 0;
			_atlasImages[_preImages->imgIndex].y = 0;

			for (unsigned int i = 0; i < usedHeight; i++) {
				firstFree.push_back(usedWidth);
			}

			RPreImage* n = _preImages->next;
			(*packedNext) = _preImages;
			_preImages->next = NULL;
			packedNext = &_preImages->next;
			_preImages = n;
			continue;
		}

		// Try to fit this image into the existing used rectangle
		bool placementFound = false;
		if (_preImages->w < usedWidth && _preImages->h < usedHeight) {
			for (unsigned int iY = 0; iY <= usedHeight - _preImages->h; iY++) {
				for (unsigned int iX = firstFree[iY]; iX <= usedWidth - _preImages->w; iX++) {

					// Check if this space is free
					bool free = true;
					for (unsigned int jY = iY; jY < iY + _preImages->h; jY++) {
						for (unsigned int jX = iX; jX < iX + _preImages->w; jX++) {
							if (used[(jY * _maxTextureSize) + jX]) {
								free = false;
								break;
							}
						}
						if (!free) break;
					}

					if (free) {
						// This space is free, place the image here
						placementFound = true;

						for (unsigned int jY = iY; jY < iY + _preImages->h; jY++) {
							memset(used + (jY * _maxTextureSize) + iX, true, _preImages->w * sizeof(bool));
							if (firstFree[jY] == iX) firstFree[jY] += _preImages->w;
						}
						_atlasImages[_preImages->imgIndex].atlasId = firstAtlas;
						_atlasImages[_preImages->imgIndex].x = iX;
						_atlasImages[_preImages->imgIndex].y = iY;

						RPreImage* n = _preImages->next;
						(*packedNext) = _preImages;
						_preImages->next = NULL;
						packedNext = &_preImages->next;
						_preImages = n;
						break;
					}
				}
				if (placementFound) break;
			}
		}

		if (placementFound) continue;

		// Place the image by expanding the rectangle
		if ((usedWidth + _preImages->w) < (usedHeight + _preImages->h)) {
			// Try to expand rightward

			for (unsigned int iY = 0; iY <= _maxTextureSize - _preImages->h; iY++) {
				for (unsigned int iX = usedWidth - _preImages->w; iX <= _maxTextureSize - _preImages->w; iX++) { // maybe change these back around?
					if (iX < firstFree[iY]) continue;
					
					// Check if this space is free
					bool free = true;
					for (unsigned int jY = iY; jY < iY + _preImages->h; jY++) {
						for (unsigned int jX = iX; jX < usedWidth; jX++) {
							if (used[(jY * _maxTextureSize) + jX]) {
								free = false;
								break;
							}
						}
						if (!free) break;
					}

					if (free) {
						// This space is free, place the image here
						placementFound = true;

						for (unsigned int jY = iY; jY < iY + _preImages->h; jY++) {
							memset(used + (jY * _maxTextureSize) + iX, true, _preImages->w * sizeof(bool));
							if (firstFree[jY] == iX) firstFree[jY] += _preImages->w;
						}
						_atlasImages[_preImages->imgIndex].atlasId = firstAtlas;
						_atlasImages[_preImages->imgIndex].x = iX;
						_atlasImages[_preImages->imgIndex].y = iY;
						
						usedWidth = _preImages->w + iX;
						if (_preImages->h > usedHeight) usedHeight = _preImages->h;
						RPreImage* n = _preImages->next;
						(*packedNext) = _preImages;
						_preImages->next = NULL;
						packedNext = &_preImages->next;
						_preImages = n;
						break;
					}
				}
				if (placementFound) break;
			}

			if(placementFound) continue;
		}
		
		// Try to expand downward

		for (unsigned int iY = (_preImages->h > usedHeight ? 0 : (usedHeight - _preImages->h)); iY <= _maxTextureSize - _preImages->h; iY++) {
			for (unsigned int iX = (iY < usedHeight ? firstFree[iY] : 0); iX <= (_preImages->w > usedWidth ? 0 : usedWidth - _preImages->w); iX++) {
			
				// Check if this space is free
				bool free = true;
				for (unsigned int jY = iY; jY < usedHeight; jY++) {
					for (unsigned int jX = iX; jX < iX + _preImages->w; jX++) {
						if (used[(jY * _maxTextureSize) + jX]) {
							free = false;
							break;
						}
					}
					if (!free) break;
				}

				if (free) {
					// This space is free, place the image here
					placementFound = true;

					for (unsigned int jY = iY; jY < iY + _preImages->h; jY++) {
						memset(used + (jY * _maxTextureSize) + iX, true, _preImages->w * sizeof(bool));
						if(jY < usedHeight) if (firstFree[jY] == iX) firstFree[jY] += _preImages->w;
					}
					_atlasImages[_preImages->imgIndex].atlasId = firstAtlas;
					_atlasImages[_preImages->imgIndex].x = iX;
					_atlasImages[_preImages->imgIndex].y = iY;
					
					unsigned int px = (iX == 0 ? _preImages->w : 0);
					for (unsigned int iH = usedHeight; iH < _preImages->h + iY; iH++) {
						firstFree.push_back(px);
					}
					usedHeight = _preImages->h + iY;
					if (_preImages->w > usedWidth) usedWidth = _preImages->w;

					RPreImage* n = _preImages->next;
					(*packedNext) = _preImages;
					_preImages->next = NULL;
					packedNext = &_preImages->next;
					_preImages = n;

					break;
				}
			}
			if (placementFound) break;
		}

		if (!placementFound) {
			// We didn't manage to place this image in this atlas.
			RPreImage* n = _preImages->next;
			(*notPackedNext) = _preImages;
			_preImages->next = NULL;
			notPackedNext = &_preImages->next;
			_preImages = n;
		}
	}
	delete[] used;

	// Create the pixel data for the atlas
	bool success = true;
	unsigned char* pixelData = (unsigned char*)malloc(usedWidth * usedHeight * 4);
	if (pixelData) {
		RPreImage* cTex = packed;
		while (cTex) {
			RAtlasImage* aImg = &_atlasImages[cTex->imgIndex];
			for (unsigned int iY = aImg->y; iY < aImg->y + aImg->h; iY++) {
				memcpy(pixelData + (iY * usedWidth * 4) + (aImg->x * 4), cTex->data + (aImg->w * (iY - aImg->y) * 4), aImg->w * 4);
			}
			cTex = cTex->next;
		}

		// Upload atlas to GPU
		glGenTextures(1, &_atlases[firstAtlas].glTex);
		_atlases[firstAtlas].w = usedWidth;
		_atlases[firstAtlas].h = usedHeight;
		glActiveTexture(GL_TEXTURE0 + firstAtlas);
		glBindTexture(GL_TEXTURE_2D, _atlases[firstAtlas].glTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, NULL, GL_RGBA, usedWidth, usedHeight, NULL, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
		glBindTexture(GL_TEXTURE_2D, NULL);
	}
	else {
		success = false;
	}

	// Test - print pixel data to file
	/*
	FILE* f;
	unsigned int filesize = 54 + (usedWidth * usedHeight * 3);
	unsigned char bmpfileheader[14] = { 'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0 };
	unsigned char bmpinfoheader[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0 };
	unsigned char bmppad[3] = { 0,0,0 };
	bmpfileheader[2] = (unsigned char)(filesize);
	bmpfileheader[3] = (unsigned char)(filesize >> 8);
	bmpfileheader[4] = (unsigned char)(filesize >> 16);
	bmpfileheader[5] = (unsigned char)(filesize >> 24);
	bmpinfoheader[4] = (unsigned char)(usedWidth);
	bmpinfoheader[5] = (unsigned char)(usedWidth >> 8);
	bmpinfoheader[6] = (unsigned char)(usedWidth >> 16);
	bmpinfoheader[7] = (unsigned char)(usedWidth >> 24);
	bmpinfoheader[8] = (unsigned char)(usedHeight);
	bmpinfoheader[9] = (unsigned char)(usedHeight >> 8);
	bmpinfoheader[10] = (unsigned char)(usedHeight >> 16);
	bmpinfoheader[11] = (unsigned char)(usedHeight >> 24);
	f = fopen((std::string("atlas") + std::to_string(firstAtlas) + std::string(".bmp")).c_str(), "wb");
	fwrite(bmpfileheader, 1, 14, f);
	fwrite(bmpinfoheader, 1, 40, f);
	for (int i = usedHeight - 1; i >= 0; i--) {
		for (int j = 0; j < usedWidth; j++) {
			fwrite(pixelData + (usedWidth * i * 4) + (j * 4) + 2, 1, 1, f);
			fwrite(pixelData + (usedWidth * i * 4) + (j * 4) + 1, 1, 1, f);
			fwrite(pixelData + (usedWidth * i * 4) + (j * 4) + 0, 1, 1, f);
		}
		fwrite(bmppad, 1, (4 - ((usedWidth * 3) % 4)) % 4, f);
	}
	fclose(f);
	*/

	// Clean up
	delete[] pixelData;
	while (packed != NULL) {
		free(packed->data);
		RPreImage* n = packed->next;
		delete packed;
		packed = n;
	}

	if (success) {
		// If we failed to compile anything into this atlas, get another atlas going.
		// But only bother to do this if everything else was successful, because otherwise the app is exiting anyway.
		if (notPacked != NULL) {
			_preImages = notPacked;
			success = _Compile(firstAtlas + 1);
		}
	}

	_preImages = NULL;
	return success;
}