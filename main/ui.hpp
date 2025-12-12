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
{ Normal,Hover,Pressed};

// UI Button
struct Button
{
    std::string label;
    float x, y;
    float width, height;
    Vec3f color;
    Vec3f outlineColor;
    ButtonState state = ButtonState::Normal; // current state
    bool wasPressed = false; // pressed in previous frame
};

// Render text and buttons
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

    // Renderer builds quads for backgrounds, buttons and text
private:
    void setupGL();
    void PQuad(float x, float y, float w, float h, Vec4f color); 
    void PtextQuad(FONSquad const& quad, Vec4f color);
    int screenWidth = 0;
    int screenHeight = 0;
    
    FONScontext* fontC = nullptr;
    int font =-1; // Font handle
    ShaderProgram& uiShader;
    GLuint vao=0;
    GLuint vbo=0;
    GLuint fontTexture=0;
    std::vector<float> quadVertices;  // solid quads
    std::vector<float> textVertices;  // text quads
};

#endif