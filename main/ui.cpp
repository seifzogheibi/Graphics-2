#include "ui.hpp"
#include <cmath>
#include <stdexcept>


// Helper to create fontstash texture and update it
namespace
{
    struct UIFONSc {GLuint texture;};
    
    // Create font atlas texture
    int UIFCtexture(void* userPtr, int width, int height){
        auto* atlas = static_cast<UIFONSc*>(userPtr);
        glGenTextures(1, &atlas->texture);
        glBindTexture(GL_TEXTURE_2D, atlas->texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        return 1;
    }
    
    // Resize the atlas when fontstash needs more space
    int UIFRtexture(void* userPtr, int width, int height)
    {
        auto* atlas = static_cast<UIFONSc*>(userPtr);
        glBindTexture(GL_TEXTURE_2D, atlas->texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
        return 1;
    }
    
    // Update a region of  atlas 
    void UIFUregion(void* userPtr, int* rect, const unsigned char* data){
    
    auto* atlas = static_cast<UIFONSc*>(userPtr);
    int x = rect[0];
    int y = rect[1];
    int xx = rect[2];
    int yy = rect[3];
    int w = xx - x;
    int h = yy - y;
    if (w <= 0 || h <= 0) return;
    
    glBindTexture(GL_TEXTURE_2D, atlas->texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // set alignment to 1 byte
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 512); // matches fontstash atlas width
    
    // Offset into the data for the updated region
    const unsigned char* subData = data + x + (y * 512);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RED, GL_UNSIGNED_BYTE, subData);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void UIFTdelete(void* userPtr)
    {
        auto* atlas = static_cast<UIFONSc*>(userPtr);
        if (atlas->texture != 0){glDeleteTextures(1, &atlas->texture);}
        delete atlas;
    }
}

// UIRenderer 
UIRenderer::UIRenderer(int windowWidth, int screenHeight, ShaderProgram& shader)
    :screenWidth(windowWidth), screenHeight(screenHeight), uiShader(shader),fontTexture(0)
{

    // Create fontstash and atlas callbacks
    FONSparams config{};
    auto* atlas = new UIFONSc{}; 
    
    config.width = 512;
    config.height = 512;
    config.flags = FONS_ZERO_TOPLEFT; // origin at top left
    config.renderCreate = UIFCtexture;
    config.renderResize = UIFRtexture;
    config.renderUpdate = UIFUregion;
    config.renderDelete = UIFTdelete;
    config.userPtr = atlas;
    fontC = fonsCreateInternal(&config);

    // Loads the font
    font = fonsAddFont(fontC, "sans", "assets/cw2/DroidSansMonoDotted.ttf");
    if (font == FONS_INVALID){
        throw std::runtime_error("Failed to load font");
    }
    // Keep atlas texture ID
    fontTexture = atlas->texture;
    setupGL();
}

UIRenderer::~UIRenderer()
{
    // cleanup fontstash
    if (fontC) fonsDeleteInternal(fontC);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

// Setup VAO and VBO 
void UIRenderer::setupGL()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    // The vertex layout is 8 floats per vertex
    constexpr int V = 8 * sizeof(float);

    // position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, V, (void*)0);
    glEnableVertexAttribArray(0);

    // texture coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, V, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // color
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, V, (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void UIRenderer::setWindowSize(int width, int height)
{screenWidth = width; screenHeight = height;
}
// Start a new frame
void UIRenderer::beginFrame(){quadVertices.clear(); textVertices.clear();
}

// render text with a dark background for readability
void UIRenderer::renderText(float x, float y, const char* text, float size, Vec4f color)
{
    // Setup fontstash
    fonsClearState(fontC);
    fonsSetFont(fontC, font);
    fonsSetSize(fontC, size);
    fonsSetAlign(fontC, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);
    
    // Get text bounds for background
    float bounds[4];
    fonsTextBounds(fontC, x, y, text, nullptr, bounds);
    
    // semi transparent background
    float padding = 3.0f;
    PQuad(bounds[0] - padding, bounds[1] - padding,
        bounds[2] - bounds[0] + 2*padding,
        bounds[3] - bounds[1] + 2*padding,
        Vec4f{0.0f, 0.0f, 0.0f, 0.7f});
    fonsDrawText(fontC, x, y, text, nullptr);

    FONStextIter iter;
    FONSquad quad;
    fonsTextIterInit(fontC, &iter, x, y, text, nullptr);
    
    while (fonsTextIterNext(fontC, &iter, &quad)){ 
        PtextQuad(quad, color);
}
}

// Draw bitton and return true if clicked
bool UIRenderer::renderButton(Button& button, double mouseX, double mouseY, bool mouseDown)
{
    // Calculates button bounds
    float halfW = button.width * 0.5f;
    float halfH = button.height * 0.5f;
    float left = button.x - halfW;
    float right = button.x + halfW;
    float up = button.y - halfH;
    float down = button.y + halfH;
    
    // Check if mouse is over the buttons
    bool Hover = (mouseX >= left && mouseX <= right && mouseY >= up && mouseY <= down);
    bool wasclicked = false;
    
    // Update button state based on mouse interaction
    if (Hover && mouseDown)
    {
        button.state = ButtonState::Pressed;
        button.wasPressed = true;
    } else if (Hover)
    {
        button.state = ButtonState::Hover;
        if (button.wasPressed)
        wasclicked = true;  // Click completed
        button.wasPressed = false;
    }else
    {
        button.state = ButtonState::Normal;
        button.wasPressed = false;
    }
    
    // Fill and outline colors based on the state
    Vec4f fillColor, outlineColor;
    switch (button.state)
    {
    case ButtonState::Normal:
        fillColor = Vec4f{button.color.x, button.color.y, button.color.z, 0.6f};
        outlineColor = Vec4f{button.outlineColor.x, button.outlineColor.y, button.outlineColor.z, 1.0f};
        break;
    case ButtonState::Hover:
        fillColor = Vec4f{button.color.x + 0.1f, button.color.y + 0.1f, button.color.z + 0.1f, 0.7f};
        outlineColor = Vec4f{button.outlineColor.x, button.outlineColor.y, button.outlineColor.z, 1.0f};
        break;
    case ButtonState::Pressed:
        fillColor = Vec4f{button.color.x + 0.1f, button.color.y + 0.1f, button.color.z + 0.1f, 0.8f};
        outlineColor = Vec4f{0.0f, 0.0f, 0.0f, 1.0f};
        break;
    }
    PQuad(left, up, button.width, button.height, fillColor);
    
    // Render outline using 4 thin quads
    float lineWidth = 2.0f;
    PQuad(left, up, button.width, lineWidth, outlineColor);  // top
    PQuad(left, down - lineWidth, button.width, lineWidth, outlineColor);  // bottom
    PQuad(left, up, lineWidth, button.height, outlineColor);  //left
    PQuad(right - lineWidth, up, lineWidth, button.height, outlineColor);  // right
    
    // center the text on the button
    float fontSize = 20.0f;
    
    fonsClearState(fontC);
    FONStextIter iter;
    fonsSetFont(fontC, font);
    fonsSetSize(fontC, fontSize);
    fonsSetAlign(fontC, FONS_ALIGN_CENTER | FONS_ALIGN_MIDDLE);
    Vec4f textColor{1.0f, 1.0f, 1.0f, 1.0f};
    fonsDrawText(fontC, button.x, button.y, button.label.c_str(), nullptr);
    FONSquad quad;
    fonsTextIterInit(fontC, &iter, button.x, button.y,button.label.c_str(), nullptr);
    
while (fonsTextIterNext(fontC, &iter, &quad))
{
    PtextQuad(quad, textColor);
} return wasclicked; // only true if button was clicked
}

// Pushes a solid quad
void UIRenderer::PQuad(float x, float y, float w, float h, Vec4f color)
{
    float verts[] = {
        x,y, 0,0,color.x, color.y, color.z, color.w,
        x,y+h,0,0,color.x, color.y, color.z, color.w,
        x+w,y+h,0,0,color.x, color.y, color.z, color.w,

        x,y, 0,0,color.x, color.y, color.z, color.w,
        x+w,y+h,0,0,color.x, color.y, color.z, color.w,
        x+w,y,0,0,color.x, color.y, color.z, color.w,
    };
    quadVertices.insert(quadVertices.end(), std::begin(verts), std::end(verts));
}
// Pushes a text quad
void UIRenderer::PtextQuad(FONSquad const& quad, Vec4f color)
{
    float verts[] = {
        // Triangle 1
        quad.x0, quad.y0, quad.s0, quad.t0, color.x, color.y, color.z, color.w,
        quad.x0, quad.y1, quad.s0, quad.t1, color.x, color.y, color.z, color.w,
        quad.x1, quad.y1, quad.s1, quad.t1, color.x, color.y, color.z, color.w,
        // Triangle 2
        quad.x0, quad.y0, quad.s0, quad.t0, color.x, color.y, color.z, color.w,
        quad.x1, quad.y1, quad.s1, quad.t1, color.x, color.y, color.z, color.w,
        quad.x1, quad.y0, quad.s1, quad.t0, color.x, color.y, color.z, color.w,
    };

    textVertices.insert(textVertices.end(), std::begin(verts), std::end(verts));
}
void UIRenderer::endFrame()
{
    // Save GL state
    GLboolean wasBlendEnabled = glIsEnabled(GL_BLEND);
    GLboolean wasDepthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean wasCullFaceEnabled = glIsEnabled(GL_CULL_FACE);
    GLboolean wasSRGBEnabled = glIsEnabled(GL_FRAMEBUFFER_SRGB);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_FRAMEBUFFER_SRGB);
    
    // Screen space projection matrix
    Mat44f projection{};
    projection[0,0] = 2.0f/ screenWidth;
    projection[1,1] = -2.0f/ screenHeight;
    projection[2,2] = -1.0f;
    projection[3,3] = 1.0f;
    projection[0,3] = -1.0f;
    projection[1,3] = 1.0f;
    
    GLuint uiProg = uiShader.programId();
    glUseProgram(uiProg);

    auto U = [&](const char* name) -> GLint {
        return glGetUniformLocation(uiProg, name);
    };

    // projection matrix
    if (GLint loc = U("uProjection"); loc >= 0)
        glUniformMatrix4fv(loc, 1, GL_TRUE, projection.v);

    glBindVertexArray(vao);

    // ----- solid quads first -----
    if (!quadVertices.empty())
    {
        // no texture
        if (GLint loc = U("uUseTexture"); loc >= 0)
            glUniform1i(loc, 0);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     quadVertices.size() * sizeof(float),
                     quadVertices.data(),
                     GL_STREAM_DRAW);

        int quadVertexCount = (int)quadVertices.size() / 8;
        glDrawArrays(GL_TRIANGLES, 0, quadVertexCount);
    }

    // ----- then font atlas text quads -----
    if (!textVertices.empty())
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fontTexture);

        // sampler = texture unit index
        if (GLint loc = U("uFontTexture"); loc >= 0)
            glUniform1i(loc, 0);

        // use texture
        if (GLint loc = U("uUseTexture"); loc >= 0)
            glUniform1i(loc, 1);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     textVertices.size() * sizeof(float),
                     textVertices.data(),
                     GL_STREAM_DRAW);

        int textVertexCount = (int)textVertices.size() / 8;
        glDrawArrays(GL_TRIANGLES, 0, textVertexCount);
    }

    glBindVertexArray(0);

    // Restore GL state
    if (!wasBlendEnabled) glDisable(GL_BLEND);
    if (wasDepthTestEnabled) glEnable(GL_DEPTH_TEST);
    if (wasCullFaceEnabled) glEnable(GL_CULL_FACE);
    if (wasSRGBEnabled) glEnable(GL_FRAMEBUFFER_SRGB);
    
//    glUseProgram(uiShader.programId());
//    glUniformMatrix4fv(0, 1, GL_TRUE, projection.v);
//    glBindVertexArray(vao);
//    
//    // Renders solid quads first
//if (!quadVertices.empty())
//    {
//        glUniform1i(2, 0);  // no texture
//        glBindBuffer(GL_ARRAY_BUFFER, vbo);
//        glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(float), quadVertices.data(), GL_STREAM_DRAW);
//        int quadVertexCount = quadVertices.size() / 8;
//        glDrawArrays(GL_TRIANGLES, 0, quadVertexCount);
//    }  
//
//    // then font atlas text quads
//     if (!textVertices.empty())
//    {
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, fontTexture);
//        glUniform1i(1, 0);  // texture unit 0
//        glUniform1i(2, 1);  // use texture
//        glBindBuffer(GL_ARRAY_BUFFER, vbo);
//        glBufferData(GL_ARRAY_BUFFER, textVertices.size() * sizeof(float), textVertices.data(), GL_STREAM_DRAW);
//        int textVertexCount = textVertices.size() / 8;
//        glDrawArrays(GL_TRIANGLES, 0, textVertexCount);
//    }
//    glBindVertexArray(0);
//    
//    // Restore GL state
//    if (!wasBlendEnabled) glDisable(GL_BLEND);
//    if (wasDepthTestEnabled) glEnable(GL_DEPTH_TEST);
//    if (wasCullFaceEnabled) glEnable(GL_CULL_FACE);
//    if (wasSRGBEnabled) glEnable(GL_FRAMEBUFFER_SRGB);
//    
}
