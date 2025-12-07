#include "ui.hpp"
#include <cmath>
#include <stdexcept>
#include <print>

// Helper to create fontstash texture and update it
namespace
{
    struct GLFONScontext
    {
        GLuint texture;
    };
    
    int glfonsRenderCreate(void* userPtr, int width, int height)
    {
        auto* gl = static_cast<GLFONScontext*>(userPtr);
        
        glGenTextures(1, &gl->texture);
        glBindTexture(GL_TEXTURE_2D, gl->texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        return 1;
    }
    
    int glfonsRenderResize(void* userPtr, int width, int height)
    {
        auto* gl = static_cast<GLFONScontext*>(userPtr);
        
        glBindTexture(GL_TEXTURE_2D, gl->texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        return 1;
    }
    
    void glfonsRenderUpdate(void* userPtr, int* rect, const unsigned char* data)
    {
        auto* gl = static_cast<GLFONScontext*>(userPtr);
        
        int w = rect[2] - rect[0];
        int h = rect[3] - rect[1];
        
        if (w <= 0 || h <= 0) return;
        
        glBindTexture(GL_TEXTURE_2D, gl->texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, rect[0], rect[1], w, h, GL_RED, GL_UNSIGNED_BYTE, data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    void glfonsRenderDraw(void* userPtr, const float* verts, const float* tcoords, 
                          const unsigned int* colors, int nverts)
    {
        // We handle drawing ourselves
        (void)userPtr;
        (void)verts;
        (void)tcoords;
        (void)colors;
        (void)nverts;
    }
    
    void glfonsRenderDelete(void* userPtr)
    {
        auto* gl = static_cast<GLFONScontext*>(userPtr);
        
        if (gl->texture)
            glDeleteTextures(1, &gl->texture);
        
        delete gl;
    }
}

UIRenderer::UIRenderer(int windowWidth, int windowHeight)
    : mWindowWidth(windowWidth)
    , mWindowHeight(windowHeight)
    , mShader({
        { GL_VERTEX_SHADER,   "assets/cw2/ui.vert" },
        { GL_FRAGMENT_SHADER, "assets/cw2/ui.frag" }
      })
    , mFontTexture(0)
{

    std::print("UIRenderer: Shader program created with ID: {}\n", mShader.programId());

    // Validate shader program
    GLint isLinked = 0;
    glGetProgramiv(mShader.programId(), GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        std::print("ERROR: UI shader program failed to link!\n");
    }
    else
    {
        std::print("UI shader program linked successfully\n");
    }

    // Create fontstash context
    FONSparams params{};
    auto* gl = new GLFONScontext{0};
    
    params.width = 512;
    params.height = 512;
    params.flags = FONS_ZERO_TOPLEFT;
    params.renderCreate = glfonsRenderCreate;
    params.renderResize = glfonsRenderResize;
    params.renderUpdate = glfonsRenderUpdate;
    params.renderDraw = glfonsRenderDraw;
    params.renderDelete = glfonsRenderDelete;
    params.userPtr = gl;
    
    mFontContext = fonsCreateInternal(&params);
    if (!mFontContext){
        throw std::runtime_error("Failed to create font context");
    }

    std::print("UIRenderer: Loading font...\n");
    
    // Load font
    mFont = fonsAddFont(mFontContext, "sans", "assets/cw2/DroidSansMonoDotted.ttf");
    if (mFont == FONS_INVALID){
        throw std::runtime_error("Failed to load font");
    }

    std::print("UIRenderer: Font loaded successfully!\n");
    
    mFontTexture = gl->texture;
    
    setupGL();
    std::print("UIRenderer: Initialization complete!\n");
}

UIRenderer::~UIRenderer()
{
    if (mFontContext)
        fonsDeleteInternal(mFontContext);
    
    glDeleteVertexArrays(1, &mVAO);
    glDeleteBuffers(1, &mVBO);
}

void UIRenderer::setupGL()
{
    glGenVertexArrays(1, &mVAO);
    glGenBuffers(1, &mVBO);
    
    glBindVertexArray(mVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    
    // Layout: pos(2) + texcoord(2) + color(4) = 8 floats per vertex
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
}

void UIRenderer::setWindowSize(int width, int height)
{
    mWindowWidth = width;
    mWindowHeight = height;
}

void UIRenderer::beginFrame()
{
    mVertices.clear();
    std::print("UIRenderer::beginFrame() - vertices cleared\n");
}

void UIRenderer::renderText(float x, float y, const char* text, float size, Vec4f color)
{
    std::print("UIRenderer::renderText() called: '{}' at ({}, {})\n", text, x, y);

    // Setup fontstash
    fonsClearState(mFontContext);
    fonsSetFont(mFontContext, mFont);
    fonsSetSize(mFontContext, size);
    fonsSetAlign(mFontContext, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);
    
    // Get text bounds for background
    float bounds[4];
    fonsTextBounds(mFontContext, x, y, text, nullptr, bounds);
    
    // Render semi-transparent background for readability
    float padding = 4.0f;
    renderQuad(bounds[0] - padding, bounds[1] - padding,
               bounds[2] - bounds[0] + 2*padding, bounds[3] - bounds[1] + 2*padding,
               Vec4f{0.0f, 0.0f, 0.0f, 0.7f});
    
    // Get quads for each character
    FONStextIter iter;
    FONSquad quad;
    fonsTextIterInit(mFontContext, &iter, x, y, text, nullptr);
    
    while (fonsTextIterNext(mFontContext, &iter, &quad))
    {
        // Add 2 triangles (6 vertices) for this glyph
        float verts[] = {
            // Triangle 1
            quad.x0, quad.y0,  quad.s0, quad.t0,  color.x, color.y, color.z, color.w,
            quad.x0, quad.y1,  quad.s0, quad.t1,  color.x, color.y, color.z, color.w,
            quad.x1, quad.y1,  quad.s1, quad.t1,  color.x, color.y, color.z, color.w,
            
            // Triangle 2
            quad.x0, quad.y0,  quad.s0, quad.t0,  color.x, color.y, color.z, color.w,
            quad.x1, quad.y1,  quad.s1, quad.t1,  color.x, color.y, color.z, color.w,
            quad.x1, quad.y0,  quad.s1, quad.t0,  color.x, color.y, color.z, color.w,
        };
        
        mVertices.insert(mVertices.end(), std::begin(verts), std::end(verts));
    }
}

bool UIRenderer::renderButton(Button& button, double mouseX, double mouseY, bool mouseDown)
{
    // Calculate button bounds
    float halfW = button.width * 0.5f;
    float halfH = button.height * 0.5f;
    float minX = button.x - halfW;
    float maxX = button.x + halfW;
    float minY = button.y - halfH;
    float maxY = button.y + halfH;
    
    // Check if mouse is over button
    bool isHovered = (mouseX >= minX && mouseX <= maxX && 
                     mouseY >= minY && mouseY <= maxY);
    
    bool wasClicked = false;
    
    // Update button state
    if (isHovered && mouseDown)
    {
        button.state = ButtonState::Pressed;
        button.wasPressed = true;
    }
    else if (isHovered)
    {
        button.state = ButtonState::Hover;
        if (button.wasPressed)
            wasClicked = true;  // Click completed
        button.wasPressed = false;
    }
    else
    {
        button.state = ButtonState::Normal;
        button.wasPressed = false;
    }
    
    // Choose colors based on state
    Vec4f fillColor, outlineColor;
    
    switch (button.state)
    {
    case ButtonState::Normal:
        fillColor = Vec4f{0.2f, 0.2f, 0.2f, 0.6f};
        outlineColor = Vec4f{0.8f, 0.8f, 0.8f, 1.0f};
        break;
    case ButtonState::Hover:
        fillColor = Vec4f{0.3f, 0.3f, 0.4f, 0.7f};
        outlineColor = Vec4f{1.0f, 1.0f, 1.0f, 1.0f};
        break;
    case ButtonState::Pressed:
        fillColor = Vec4f{0.4f, 0.4f, 0.5f, 0.8f};
        outlineColor = Vec4f{0.6f, 0.8f, 1.0f, 1.0f};
        break;
    }
    
    // Render filled rectangle
    renderQuad(minX, minY, button.width, button.height, fillColor);
    
    // Render outline (4 rectangles)
    float lineWidth = 2.0f;
    renderQuad(minX, minY, button.width, lineWidth, outlineColor);  // Top
    renderQuad(minX, maxY - lineWidth, button.width, lineWidth, outlineColor);  // Bottom
    renderQuad(minX, minY, lineWidth, button.height, outlineColor);  // Left
    renderQuad(maxX - lineWidth, minY, lineWidth, button.height, outlineColor);  // Right
    
    // Render centered text label
    float fontSize = 20.0f;
    
    fonsClearState(mFontContext);
    fonsSetFont(mFontContext, mFont);
    fonsSetSize(mFontContext, fontSize);
    fonsSetAlign(mFontContext, FONS_ALIGN_CENTER | FONS_ALIGN_MIDDLE);
    
    Vec4f textColor{1.0f, 1.0f, 1.0f, 1.0f};
    
    FONStextIter iter;
    FONSquad quad;
    fonsTextIterInit(mFontContext, &iter, button.x, button.y, 
                     button.label.c_str(), nullptr);
    
    while (fonsTextIterNext(mFontContext, &iter, &quad))
    {
        float verts[] = {
            quad.x0, quad.y0,  quad.s0, quad.t0,  textColor.x, textColor.y, textColor.z, textColor.w,
            quad.x0, quad.y1,  quad.s0, quad.t1,  textColor.x, textColor.y, textColor.z, textColor.w,
            quad.x1, quad.y1,  quad.s1, quad.t1,  textColor.x, textColor.y, textColor.z, textColor.w,
            
            quad.x0, quad.y0,  quad.s0, quad.t0,  textColor.x, textColor.y, textColor.z, textColor.w,
            quad.x1, quad.y1,  quad.s1, quad.t1,  textColor.x, textColor.y, textColor.z, textColor.w,
            quad.x1, quad.y0,  quad.s1, quad.t0,  textColor.x, textColor.y, textColor.z, textColor.w,
        };
        
        mVertices.insert(mVertices.end(), std::begin(verts), std::end(verts));
    }
    
    return wasClicked;
}

void UIRenderer::renderQuad(float x, float y, float w, float h, Vec4f color)
{
    // Quad without texture (texcoords are 0)
    float verts[] = {
        x,     y,      0,0,  color.x, color.y, color.z, color.w,
        x,     y+h,    0,0,  color.x, color.y, color.z, color.w,
        x+w,   y+h,    0,0,  color.x, color.y, color.z, color.w,
        
        x,     y,      0,0,  color.x, color.y, color.z, color.w,
        x+w,   y+h,    0,0,  color.x, color.y, color.z, color.w,
        x+w,   y,      0,0,  color.x, color.y, color.z, color.w,
    };
    
    mVertices.insert(mVertices.end(), std::begin(verts), std::end(verts));
}

void UIRenderer::endFrame()
{
    std::print("=== UIRenderer::endFrame() START ===\n");
    std::print("Vertex count: {}\n", mVertices.size());
    
    if (mVertices.empty())
    {
        std::print("No vertices to render!\n");
        return;
    }
    
    std::print("Window size: {}x{}\n", mWindowWidth, mWindowHeight);
    
    // Save GL state
    GLboolean wasBlendEnabled = glIsEnabled(GL_BLEND);
    GLboolean wasDepthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean wasCullFaceEnabled = glIsEnabled(GL_CULL_FACE);
    GLboolean wasSRGBEnabled = glIsEnabled(GL_FRAMEBUFFER_SRGB);
    
    std::print("Previous GL state - Blend: {}, Depth: {}, Cull: {}, SRGB: {}\n", 
           wasBlendEnabled, wasDepthTestEnabled, wasCullFaceEnabled, wasSRGBEnabled);
    
    // Setup for 2D rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_FRAMEBUFFER_SRGB);
    
    std::print("Set GL state for UI rendering\n");
    
    // Orthographic projection
    Mat44f projection{};
    projection[0,0] = 2.0f / mWindowWidth;
    projection[1,1] = -2.0f / mWindowHeight;
    projection[2,2] = -1.0f;
    projection[3,3] = 1.0f;
    projection[0,3] = -1.0f;
    projection[1,3] = 1.0f;
    
    std::print("Projection matrix created\n");
    
    GLuint progId = mShader.programId();
    std::print("Shader program ID: {}\n", progId);
    
    glUseProgram(progId);
    
    // Check for errors after using program
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        std::print("ERROR after glUseProgram: 0x{:X}\n", err);
    
    glUniformMatrix4fv(0, 1, GL_TRUE, projection.v);
    
    err = glGetError();
    if (err != GL_NO_ERROR)
        std::print("ERROR after uniform matrix: 0x{:X}\n", err);
    
    // Bind font texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mFontTexture);
    std::print("Font texture ID: {}\n", mFontTexture);
    
    glUniform1i(1, 0);  // Texture unit 0
    glUniform1i(2, 1);  // Use texture = 1
    
    err = glGetError();
    if (err != GL_NO_ERROR)
        std::print("ERROR after texture uniforms: 0x{:X}\n", err);
    
    std::print("About to upload vertices...\n");
    uploadVertices();
    
    err = glGetError();
    if (err != GL_NO_ERROR)
        std::print("ERROR after uploadVertices: 0x{:X}\n", err);
    
    // Restore GL state
    if (!wasBlendEnabled) glDisable(GL_BLEND);
    if (wasDepthTestEnabled) glEnable(GL_DEPTH_TEST);
    if (wasCullFaceEnabled) glEnable(GL_CULL_FACE);
    if (wasSRGBEnabled) glEnable(GL_FRAMEBUFFER_SRGB);
    
    std::print("=== UIRenderer::endFrame() COMPLETE ===\n\n");
}

void UIRenderer::uploadVertices()
{
    std::print("  uploadVertices() - VAO: {}, VBO: {}\n", mVAO, mVBO);
    
    glBindVertexArray(mVAO);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        std::print("  ERROR after glBindVertexArray: 0x{:X}\n", err);
    
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    err = glGetError();
    if (err != GL_NO_ERROR)
        std::print("  ERROR after glBindBuffer: 0x{:X}\n", err);
    
    size_t byteSize = mVertices.size() * sizeof(float);
    std::print("  Uploading {} bytes ({} floats, {} vertices)\n", 
               byteSize, mVertices.size(), mVertices.size() / 8);
    
    glBufferData(GL_ARRAY_BUFFER, byteSize, mVertices.data(), GL_STREAM_DRAW);
    err = glGetError();
    if (err != GL_NO_ERROR)
        std::print("  ERROR after glBufferData: 0x{:X}\n", err);
    
    int vertexCount = mVertices.size() / 8;
    std::print("  Drawing {} vertices ({} triangles)\n", vertexCount, vertexCount / 3);
    
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    err = glGetError();
    if (err != GL_NO_ERROR)
        std::print("  ERROR after glDrawArrays: 0x{:X}\n", err);
    else
        std::print("  glDrawArrays completed successfully!\n");
    
    glBindVertexArray(0);
}