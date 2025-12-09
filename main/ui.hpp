#ifndef UI_HPP_INCLUDED
#define UI_HPP_INCLUDED

#include <glad/glad.h>
#include <string>
#include <vector>
#include <fontstash.h>

#include "../vmlib/vec2.hpp"
#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"
#include "../support/program.hpp"

// Button state
enum class ButtonState
{
    Normal,
    Hover,
    Pressed
};

// UI Button
struct Button
{
    std::string label;
    float x, y;          // Center position
    float width, height;
    Vec3f color;
    Vec3f outlineColor; // border color
    ButtonState state = ButtonState::Normal; // current state
    bool wasPressed = false; // pressed in previous frame
};

// UI Renderer class (text and buttons)
class UIRenderer
{
public:
    UIRenderer(int windowWidth, int windowHeight, ShaderProgram& shader);
    ~UIRenderer();
    
    void setWindowSize(int width, int height);
    void beginFrame();
    void renderText(float x, float y, const char* text, float size, Vec4f color);
    bool renderButton(Button& button, double mouseX, double mouseY, bool mouseDown);
    void endFrame();

private:
    void setupGL();
    void renderQuad(float x, float y, float w, float h, Vec4f color); // colored quad
    void uploadVertices();
    
    int mWindowWidth;
    int mWindowHeight;
    
    FONScontext* mFontContext; // Fontstash context
    int mFont; // Font handle
    
    ShaderProgram& mShader;
    GLuint mVAO;
    GLuint mVBO;
    GLuint mFontTexture;
    
    std::vector<float> mQuadVertices;  // Solid quads (backgrounds, outlines)
    std::vector<float> mTextVertices;  // Textured (font glyphs)

};

#endif // UI_HPP_INCLUDED