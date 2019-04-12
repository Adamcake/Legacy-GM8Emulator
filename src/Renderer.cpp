#define PI 3.14159265358979324
#include "Renderer.hpp"
#include "GameSettings.hpp"
#include "InputHandler.hpp"

// It's supposed to be in GLAD *then* GLFW, don't remove the newline inbetween.
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <finders_interface.h>  // rectpack2D

#include <cmath>
#include <cstring>

const GLchar* fragmentShaderCode =
    "#version 330\n"
    "uniform sampler2D tex;\n"
    "in float objAlpha;\n"
    "in vec3 objBlend;\n"
    "in vec2 atlasXY;\n"
    "in vec2 atlasWH;\n"
    "in vec2 fragTexCoord;\n"
    "out vec4 colourOut;\n"
    "void main() {\n"
    "vec4 col = texture(tex, vec2(atlasXY.x + (fragTexCoord.x * atlasWH.x), atlasXY.y + (fragTexCoord.y * atlasWH.y)));\n"
    "colourOut = vec4((col.x * objBlend.x), (col.y * objBlend.y), (col.z * objBlend.z), (col.w * objAlpha));\n"
    "}";

const GLchar* vertexShaderCode =
    "#version 330\n"
    "in vec3 vert;\n"
    "in vec2 vertTexCoord;\n"
    "in float vObjAlpha;\n"
    "in vec3 vObjBlend;\n"
    "in vec2 vAtlasXY;\n"
    "in vec2 vAtlasWH;\n"
    "in mat4 project;\n"
    "out float objAlpha;\n"
    "out vec3 objBlend;\n"
    "out vec2 atlasXY;\n"
    "out vec2 atlasWH;\n"
    "out vec2 fragTexCoord;\n"
    "void main() {\n"
    "fragTexCoord = vertTexCoord;\n"
    "objAlpha = vObjAlpha;\n"
    "objBlend = vObjBlend;\n"
    "atlasXY = vAtlasXY;\n"
    "atlasWH = vAtlasWH;\n"
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

    unsigned int _x;
    unsigned int _y;
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

std::vector<RPreImage> _preImages;
std::vector<RAtlasImage> _atlasImages;
RAtlas _atlases[32];
int boundAtlas;


// Builds all added images into atlases so that they can be drawn. Must be called before attempting to draw. Only intended to be called once.
bool _Compile(unsigned int firstAtlas = 0);

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
    boundAtlas = -1;
}

void RTerminate() {
    for (const RPreImage& n : _preImages) {
        free(n.data);
    }
    glfwDestroyWindow(_window);  // This function is allowed be called on NULL
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

    // Load OpenGL Stuff
    if (!gladLoadGL()) {
        return false;
    }

    // Required GL features
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_LIGHTING);
    glEnable(GL_SCISSOR_TEST);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, ( GLint* )(&_maxTextures));
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, ( GLint* )(&_maxTextureSize));
    _colourOutsideRoom = settings->colourOutsideRoom;
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapInterval(0);
    InputInit(_window);

    // Make shaders
    GLint vertexCompiled, fragmentCompiled, linked;
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    GLint vSize = ( GLint )strlen(vertexShaderCode);
    GLint fSize = ( GLint )strlen(fragmentShaderCode);
    glShaderSource(vertexShader, 1, &vertexShaderCode, &vSize);
    glShaderSource(fragmentShader, 1, &fragmentShaderCode, &fSize);
    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexCompiled);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentCompiled);
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
    if (!_Compile()) return false;

    // Make VAO
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    // Make VBO
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    GLfloat vertexData[] = {0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};
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

void RSetGameWindowTitle(const char* title) { glfwSetWindowTitle(_window, title); }

void RGetCursorPos(int* xpos, int* ypos) {
    double xp, yp;
    int actualWinW, actualWinH;
    glfwGetCursorPos(_window, &xp, &yp);
    glfwGetWindowSize(_window, &actualWinW, &actualWinH);
    xp /= actualWinW;
    yp /= actualWinH;
    xp *= _windowW;
    yp *= _windowH;
    if (xpos) (*xpos) = ( int )xp;
    if (ypos) (*ypos) = ( int )yp;
}

bool RShouldClose() { return glfwWindowShouldClose(_window); }

void RSetBGColour(unsigned int col) { _roomBGColour = col; }

RImageIndex RMakeImage(unsigned int w, unsigned int h, unsigned int originX, unsigned int originY, unsigned char* bytes) {
    if (_contextSet) {
        // Image being registered after the game is already loaded - what do we do here!?
        abort();  // I don't know exactly what this does, but it's definitely better than what would happen if I didn't do it
    }

    // Make pre-image object for later compiling into an atlas
    RPreImage pImg;
    pImg.w = w;
    pImg.h = h;
    pImg.data = ( unsigned char* )malloc(w * h * 4);
    memcpy(pImg.data, bytes, (w * h * 4));

    // Make reference object for this image
    RAtlasImage aImg;
    aImg.w = w;
    aImg.h = h;
    aImg.originX = originX;
    aImg.originY = originY;
    pImg.imgIndex = static_cast<unsigned int>(_atlasImages.size());
    _atlasImages.push_back(aImg);

    // Put pre-image into list
    _preImages.push_back(pImg);

    // Keep track of the widest and tallest sprites and the total number of pixels added
    _tallest = std::max(_tallest, w);
    _widest = std::max(_widest, h);
    _pixelCount += (w * h);

    return pImg.imgIndex;
}

void RDrawImage(RImageIndex ix, double x, double y, double xscale, double yscale, double rot, unsigned int blend, double alpha) {
    RAtlasImage* r = _atlasImages.data() + ix;
    RDrawPartialImage(ix, x, y, xscale, yscale, rot, blend, alpha, 0, 0, r->w, r->h);
}

void RDrawPartialImage(RImageIndex ix, double x, double y, double xscale, double yscale, double rot, unsigned int blend, double alpha, unsigned int partX, unsigned int partY, unsigned int partW, unsigned int partH) {
    RDrawCommand command;

    RAtlasImage* aImg = _atlasImages.data() + ix;

    if (partX >= aImg->w) return;
    if (partY >= aImg->h) return;
    if (partX + partW > aImg->w) partW = aImg->w - partX;
    if (partY + partH > aImg->h) partH = aImg->h - partY;

    // Calculate a single matrix for scaling and transforming the sprite
    double dRot = rot * PI / 180;
    GLfloat sinRot = ( GLfloat )sin(dRot);
    GLfloat cosRot = ( GLfloat )cos(dRot);
    GLfloat dx = (( GLfloat )aImg->originX / aImg->w);
    GLfloat dy = (( GLfloat )aImg->originY / aImg->h);

    // TODO: We can optimize this a lot by pre-calculating the final matrix instead of multiplying a ton of hard-coded values for every draw.
    //       But that should wait until we've implemented views, as that will affect the result.
    GLfloat toMiddle[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -dx, -dy, 0, 1};
    GLfloat scale[16] = {(GLfloat)(partW * xscale), 0, 0, 0, 0, (GLfloat)(partH * -yscale), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    GLfloat rotate[16] = {cosRot, sinRot, 0, 0, -sinRot, cosRot, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    GLfloat scale2[16] = {2.0f / _windowW, 0, 0, 0, 0, 2.0f / _windowH, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    GLfloat transform[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, (GLfloat)(x * 2.0f / _windowW) - 1, -(GLfloat)((y * 2.0f / _windowH) - 1), 0, 1};


    GLfloat tmp[16];
    GLfloat project[16];

    mat4Mult(toMiddle, scale, tmp);
    mat4Mult(tmp, rotate, project);
    mat4Mult(project, scale2, tmp);
    mat4Mult(tmp, transform, project);

    RAtlas* atlas = &_atlases[aImg->atlasId];

    // Create draw command
    memcpy(command.transform, project, 16 * sizeof(GLfloat));
    command.alpha = ( GLfloat )alpha;
    command.blend[0] = (GLfloat)(blend & 0xFF) / 0xFF;
    command.blend[1] = (GLfloat)(blend & 0xFF00) / 0xFF00;
    command.blend[2] = (GLfloat)(blend & 0xFF0000) / 0xFF0000;
    command.atlasXY[0] = (GLfloat)(( double )(aImg->x + partX) / ( double )atlas->w);
    command.atlasXY[1] = (GLfloat)(( double )(aImg->y + partY) / ( double )atlas->h);
    command.atlasWH[0] = (GLfloat)(( double )(partW) / ( double )atlas->w);
    command.atlasWH[1] = (GLfloat)(( double )(partH) / ( double )atlas->h);
    command.imageIndex = ix;
    command.atlasId = aImg->atlasId;
    command.atlasGlTex = atlas->glTex;
    _drawCommands.push_back(command);
}


void RStartFrame() {
    int actualWinW, actualWinH;
    glfwGetWindowSize(_window, &actualWinW, &actualWinH);
    glClearColor((GLclampf)(_colourOutsideRoom & 0xFF) / 0xFF, (GLclampf)((_colourOutsideRoom >> 8) & 0xFF) / 0xFF, (GLclampf)((_colourOutsideRoom >> 16) & 0xFF) / 0xFF, ( GLclampf )1.0);
    glViewport(0, 0, actualWinW, actualWinH);
    glScissor(0, 0, actualWinW, actualWinH);
    glClear(GL_COLOR_BUFFER_BIT);

    // Later, we'll use this for clearing the background of each active view's viewport. But views aren't supported right now.
    glClearColor((GLclampf)(_roomBGColour & 0xFF) / 0xFF, (GLclampf)((_roomBGColour >> 8) & 0xFF) / 0xFF, (GLclampf)((_roomBGColour >> 16) & 0xFF) / 0xFF, ( GLclampf )1.0);
    glViewport(0, 0, actualWinW, actualWinH);
    glScissor(0, 0, actualWinW, actualWinH);
    glClear(GL_COLOR_BUFFER_BIT);

    _drawCommands.clear();
}

void RRenderFrame() {
    int actualWinW, actualWinH;
    glfwGetWindowSize(_window, &actualWinW, &actualWinH);

    // Buffer all draw commands into VBO
    GLuint commandsVBO;
    glGenBuffers(1, &commandsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, commandsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RDrawCommand) * _drawCommands.size(), _drawCommands.data(), GL_STATIC_DRAW);

    unsigned int drawn = 0;
    while (drawn < _drawCommands.size()) {
        // Calculate how many commands to process in this instanced draw
        unsigned int toDraw = 0;
        for (unsigned int i = drawn; i < _drawCommands.size(); i++) {
            if (_drawCommands[i].atlasId != _drawCommands[drawn].atlasId) {
                break;
            }
            toDraw++;
        }

        // Activate atlas texture
        if (boundAtlas != _drawCommands[drawn].atlasId) {
            glActiveTexture(GL_TEXTURE0 + _drawCommands[drawn].atlasId);
            glBindTexture(GL_TEXTURE_2D, _drawCommands[drawn].atlasGlTex);
            boundAtlas = _drawCommands[drawn].atlasId;
        }

        // Bind instanced inputs
        glUniform1i(glGetUniformLocation(_glProgram, "tex"), _drawCommands[drawn].atlasId);

        GLint shaderAlpha = glGetAttribLocation(_glProgram, "vObjAlpha");
        GLint shaderBlend = glGetAttribLocation(_glProgram, "vObjBlend");
        GLint shaderXY = glGetAttribLocation(_glProgram, "vAtlasXY");
        GLint shaderWH = glGetAttribLocation(_glProgram, "vAtlasWH");
        GLint shaderProject = glGetAttribLocation(_glProgram, "project");

        glEnableVertexAttribArray(shaderAlpha);
        glVertexAttribPointer(shaderAlpha, 1, GL_FLOAT, GL_FALSE, sizeof(RDrawCommand), ( void* )(offsetof(struct RDrawCommand, alpha) + (sizeof(RDrawCommand) * drawn)));
        glEnableVertexAttribArray(shaderBlend);
        glVertexAttribPointer(shaderBlend, 3, GL_FLOAT, GL_FALSE, sizeof(RDrawCommand), ( void* )(offsetof(struct RDrawCommand, blend) + (sizeof(RDrawCommand) * drawn)));
        glEnableVertexAttribArray(shaderXY);
        glVertexAttribPointer(shaderXY, 2, GL_FLOAT, GL_FALSE, sizeof(RDrawCommand), ( void* )(offsetof(struct RDrawCommand, atlasXY) + (sizeof(RDrawCommand) * drawn)));
        glEnableVertexAttribArray(shaderWH);
        glVertexAttribPointer(shaderWH, 2, GL_FLOAT, GL_FALSE, sizeof(RDrawCommand), ( void* )(offsetof(struct RDrawCommand, atlasWH) + (sizeof(RDrawCommand) * drawn)));
        glEnableVertexAttribArray(shaderProject);
        glVertexAttribPointer(shaderProject, 4, GL_FLOAT, GL_FALSE, sizeof(RDrawCommand), ( void* )(offsetof(struct RDrawCommand, transform) + (sizeof(RDrawCommand) * drawn)));
        glEnableVertexAttribArray(shaderProject + 1);
        glVertexAttribPointer(
            shaderProject + 1, 4, GL_FLOAT, GL_FALSE, sizeof(RDrawCommand), ( void* )(offsetof(struct RDrawCommand, transform) + (sizeof(RDrawCommand) * drawn) + (sizeof(GLfloat) * 4)));
        glEnableVertexAttribArray(shaderProject + 2);
        glVertexAttribPointer(
            shaderProject + 2, 4, GL_FLOAT, GL_FALSE, sizeof(RDrawCommand), ( void* )(offsetof(struct RDrawCommand, transform) + (sizeof(RDrawCommand) * drawn) + (sizeof(GLfloat) * 8)));
        glEnableVertexAttribArray(shaderProject + 3);
        glVertexAttribPointer(
            shaderProject + 3, 4, GL_FLOAT, GL_FALSE, sizeof(RDrawCommand), ( void* )(offsetof(struct RDrawCommand, transform) + (sizeof(RDrawCommand) * drawn) + (sizeof(GLfloat) * 12)));

        glVertexAttribDivisor(shaderAlpha, 1);
        glVertexAttribDivisor(shaderBlend, 1);
        glVertexAttribDivisor(shaderXY, 1);
        glVertexAttribDivisor(shaderWH, 1);
        glVertexAttribDivisor(shaderProject, 1);
        glVertexAttribDivisor(shaderProject + 1, 1);
        glVertexAttribDivisor(shaderProject + 2, 1);
        glVertexAttribDivisor(shaderProject + 3, 1);

        // Do instanced draw
        glUseProgram(_glProgram);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glEnableVertexAttribArray(glGetAttribLocation(_glProgram, "vertTexCoord"));
        glVertexAttribPointer(glGetAttribLocation(_glProgram, "vertTexCoord"), 2, GL_FLOAT, GL_TRUE, 3 * sizeof(GLfloat), 0);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, toDraw);

        drawn += toDraw;
    }

    glDeleteBuffers(1, &commandsVBO);
    glViewport(0, 0, actualWinW, actualWinH);
    glfwSwapBuffers(_window);
}


bool _Compile(unsigned int firstAtlas) {
    // Sanity checks
    if (_tallest > _maxTextureSize) return false;
    if (_widest > _maxTextureSize) return false;
    if (firstAtlas > _maxTextures) return false;
    if (_pixelCount == 0) return true;

    using spaces_type = rectpack2D::empty_spaces<false, rectpack2D::default_empty_spaces>;  // don't allow flipping
    using rect_type = rectpack2D::output_rect_t<spaces_type>;

    std::vector<rect_type> rectangles;
    rectangles.reserve(_preImages.size());

    std::vector<RPreImage> packed;
    packed.reserve(_preImages.size());
    auto report_successful = [&packed, &rectangles](rect_type& r) {
        unsigned int index = static_cast<unsigned int>((&r) - rectangles.data());
        _preImages[index]._x = r.x;
        _preImages[index]._y = r.y;
        packed.push_back(_preImages[index]);
        return rectpack2D::callback_result::CONTINUE_PACKING;
    };

    std::vector<RPreImage> unpacked;
    auto report_unsuccessful = [&unpacked, &rectangles](rect_type& r) {
        unsigned int index = static_cast<unsigned int>((&r) - rectangles.data());
        unpacked.push_back(_preImages[index]);
        return rectpack2D::callback_result::CONTINUE_PACKING;
    };

    const auto max_side = _maxTextureSize;
    const auto discard_step = 256;

    for (const RPreImage& p : _preImages) {
        rectangles.emplace_back(rectpack2D::rect_xywh(0, 0, p.w, p.h));
    }

    // Do packing
    const auto result_size =
        rectpack2D::find_best_packing<spaces_type>(rectangles, rectpack2D::make_finder_input(max_side, discard_step, report_successful, report_unsuccessful, rectpack2D::flipping_option::ENABLED));

    // Copy everything into pixeldata
    unsigned char* pixelData = ( unsigned char* )malloc(result_size.w * result_size.h * 4);
    if (!pixelData) return false;
    for (RPreImage& img : packed) {
        RAtlasImage& aImg = _atlasImages[img.imgIndex];
        aImg.x = img._x;
        aImg.y = img._y;
        aImg.w = img.w;
        aImg.h = img.h;
        aImg.atlasId = firstAtlas;
        for (unsigned int iY = aImg.y; iY < aImg.y + aImg.h; iY++) {
            memcpy(pixelData + (iY * result_size.w * 4) + (aImg.x * 4), img.data + (aImg.w * (iY - aImg.y) * 4), aImg.w * 4);
        }
    }

    // Upload atlas to GPU
    glGenTextures(1, &_atlases[firstAtlas].glTex);
    _atlases[firstAtlas].w = result_size.w;
    _atlases[firstAtlas].h = result_size.h;
    glActiveTexture(GL_TEXTURE0 + firstAtlas);
    glBindTexture(GL_TEXTURE_2D, _atlases[firstAtlas].glTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, result_size.w, result_size.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, static_cast<void*>(pixelData));
    glBindTexture(GL_TEXTURE_2D, 0);

    free(pixelData);

    if (unpacked.size()) {
        // We didn't manage to pack every texture so let's move onto the next atlas
        _preImages = unpacked;
        return _Compile(firstAtlas + 1);
    }
    else {
        return true;
    }
}
