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

enum class ButtonState
{
    Normal,
    Hover,
    Pressed
};

struct Button
{
    std::string label;
    float x, y;          // Center position
    float width, height;
    Vec3f color;
    Vec3f outlineColor; // Use these!!
    ButtonState state = ButtonState::Normal;
    bool wasPressed = false;
};

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
    void renderQuad(float x, float y, float w, float h, Vec4f color);
    void uploadVertices();
    
    int mWindowWidth;
    int mWindowHeight;
    
    FONScontext* mFontContext;
    int mFont;
    
    ShaderProgram& mShader;
    GLuint mVAO;
    GLuint mVBO;
    GLuint mFontTexture;
    
    std::vector<float> mQuadVertices;  // Non-textured (backgrounds, outlines)
    std::vector<float> mTextVertices;  // Textured (font glyphs)

};

#endif // UI_HPP_INCLUDED