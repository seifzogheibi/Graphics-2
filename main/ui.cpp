#include "ui.hpp"
#include <cmath>
#include <stdexcept>
#include <print>

// Uncomment the line below to enable UI debug output
// #define UI_DEBUG_PRINTS

// Helper to create fontstash texture and update it
namespace
{
    struct GLFONScontext
    {
        GLuint texture; // atlas texture ID
    };
    
    // Create font texture
    int glfonsRenderCreate(void* userPtr, int width, int height)
    {
        
        auto* gl = static_cast<GLFONScontext*>(userPtr);
    
        
        glGenTextures(1, &gl->texture);
#ifdef UI_DEBUG_PRINTS
        std::print("glfonsRenderCreate: Created texture ID {} ({}x{})\n", 
               gl->texture, width, height);
#endif
        glBindTexture(GL_TEXTURE_2D, gl->texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        return 1;
    }
    
    // Resize atlas when fontstash needs more space
    int glfonsRenderResize(void* userPtr, int width, int height)
    {
        auto* gl = static_cast<GLFONScontext*>(userPtr);
        
        glBindTexture(GL_TEXTURE_2D, gl->texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        return 1;
    }
    
    // Update a region of  atlas
    void glfonsRenderUpdate(void* userPtr, int* rect, const unsigned char* data)
    {
#ifdef UI_DEBUG_PRINTS
        std::print("glfonsRenderUpdate called: rect=[{},{},{},{}]\n", 
            rect[0], rect[1], rect[2], rect[3]);
#endif

        auto* gl = static_cast<GLFONScontext*>(userPtr);
        
        int x = rect[0];
        int y = rect[1];
        int w = rect[2] - rect[0];
        int h = rect[3] - rect[1];
        
        if (w <= 0 || h <= 0) return;
        
        glBindTexture(GL_TEXTURE_2D, gl->texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // set alignment to 1 byte
        
        // Set row length to 512 (the full width of atlas)
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 512);
        
        // Offset into the data for the updated region
        const unsigned char* subData = data + x + (y * 512);
        
        glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RED, GL_UNSIGNED_BYTE, subData);
        
        // Reset row length to default
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // not used, drawing handled in UIRenderer
    void glfonsRenderDraw(void* userPtr, const float* verts, const float* tcoords, 
        const unsigned int* colors, int nverts)
    {
        (void)userPtr;
        (void)verts;
        (void)tcoords;
        (void)colors;
        (void)nverts;
    }
    
    // Delete font texture
    void glfonsRenderDelete(void* userPtr)
    {
        auto* gl = static_cast<GLFONScontext*>(userPtr);
        
        if (gl->texture)
            glDeleteTextures(1, &gl->texture);
        
        delete gl;
    }
}

// UIRenderer implementation
UIRenderer::UIRenderer(int windowWidth, int windowHeight, ShaderProgram& shader)
    : mWindowWidth(windowWidth)
    , mWindowHeight(windowHeight)
    , mShader(shader) // reference to shader program
    , mFontTexture(0) // will be created in fontstash
{
#ifdef UI_DEBUG_PRINTS
    std::print("UIRenderer: Using shader program with ID: {}\n", mShader.programId());
#endif

    // Checks if shader program is linked
    GLint isLinked = 0;
    glGetProgramiv(mShader.programId(), GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
#ifdef UI_DEBUG_PRINTS
        std::print("ERROR: UI shader program failed to link!\n");
#endif
    }
    else
    {
#ifdef UI_DEBUG_PRINTS
        std::print("UI shader program linked successfully\n");
#endif
    }

    // Create fontstash context
    FONSparams params{};
    auto* gl = new GLFONScontext{0}; // context for atlas texture
    
    params.width = 512;
    params.height = 512;
    params.flags = FONS_ZERO_TOPLEFT; // origin at top left
    params.renderCreate = glfonsRenderCreate;
    params.renderResize = glfonsRenderResize;
    params.renderUpdate = glfonsRenderUpdate;
    params.renderDraw = glfonsRenderDraw;
    params.renderDelete = glfonsRenderDelete;
    params.userPtr = gl; // pass our GL context
    
    mFontContext = fonsCreateInternal(&params);
    if (!mFontContext){
        throw std::runtime_error("Failed to create font context");
    }

#ifdef UI_DEBUG_PRINTS
    std::print("UIRenderer: Loading font...\n");
#endif
    
    // Load the font
    mFont = fonsAddFont(mFontContext, "sans", "assets/cw2/DroidSansMonoDotted.ttf");
    if (mFont == FONS_INVALID){
        throw std::runtime_error("Failed to load font");
    }

#ifdef UI_DEBUG_PRINTS
    std::print("UIRenderer: Font loaded successfully!\n");
#endif
    
    // keep atlas texture ID
    mFontTexture = gl->texture;
    
    setupGL();
#ifdef UI_DEBUG_PRINTS
    std::print("UIRenderer: Initialization complete!\n");
#endif
}

UIRenderer::~UIRenderer()
{
    // cleanup fontstash
    if (mFontContext)
        fonsDeleteInternal(mFontContext);
    
    glDeleteVertexArrays(1, &mVAO);
    glDeleteBuffers(1, &mVBO);
}

// Setup VAO and VBO 
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

// Update when window is resized
void UIRenderer::setWindowSize(int width, int height)
{
    mWindowWidth = width;
    mWindowHeight = height;
}

// Start a new frame
void UIRenderer::beginFrame()
{
    mQuadVertices.clear();
    mTextVertices.clear();
#ifdef UI_DEBUG_PRINTS
    std::print("UIRenderer::beginFrame() - vertices cleared\n");
#endif
}

// render text with the background for readability
void UIRenderer::renderText(float x, float y, const char* text, float size, Vec4f color)
{
#ifdef UI_DEBUG_PRINTS
    std::print("UIRenderer::renderText() called: '{}' at ({}, {})\n", text, x, y);
#endif

    // Setup fontstash
    fonsClearState(mFontContext);
    fonsSetFont(mFontContext, mFont);
    fonsSetSize(mFontContext, size);
    fonsSetAlign(mFontContext, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);
    
    // Get text bounds for background
    float bounds[4];
    fonsTextBounds(mFontContext, x, y, text, nullptr, bounds);
    
    float padding = 4.0f;
    renderQuad(bounds[0] - padding, bounds[1] - padding,
               bounds[2] - bounds[0] + 2*padding, bounds[3] - bounds[1] + 2*padding,
               Vec4f{0.0f, 0.0f, 0.0f, 0.7f});
    
    // Force fontstash to rasterize and update texture
    fonsDrawText(mFontContext, x, y, text, nullptr);

    // Build quads 
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
        
        mTextVertices.insert(mTextVertices.end(), std::begin(verts), std::end(verts));
    }
}

// Draw bitton and return true if clicked
bool UIRenderer::renderButton(Button& button, double mouseX, double mouseY, bool mouseDown)
{
    // Calculate button bounds
    float halfW = button.width * 0.5f;
    float halfH = button.height * 0.5f;
    float minX = button.x - halfW;
    float maxX = button.x + halfW;
    float minY = button.y - halfH;
    float maxY = button.y + halfH;
    
    // Check if mouse is over the buttons
    bool isHovered = (mouseX >= minX && mouseX <= maxX && 
                     mouseY >= minY && mouseY <= maxY);
    
    bool wasClicked = false;
    
    // Update button state based on mouse interaction
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
    
    // Choose fill and outline colors based on the state
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
    
    // Render filled rectangle
    renderQuad(minX, minY, button.width, button.height, fillColor);
    
    // Render outline using 4 thin quads
    float lineWidth = 2.0f;
    renderQuad(minX, minY, button.width, lineWidth, outlineColor);  // Top
    renderQuad(minX, maxY - lineWidth, button.width, lineWidth, outlineColor);  // Bottom
    renderQuad(minX, minY, lineWidth, button.height, outlineColor);  // Left
    renderQuad(maxX - lineWidth, minY, lineWidth, button.height, outlineColor);  // Right
    
    // center the text on the button
    float fontSize = 20.0f;
    
    fonsClearState(mFontContext);
    
    FONStextIter iter;
    fonsSetFont(mFontContext, mFont);
    fonsSetSize(mFontContext, fontSize);
    fonsSetAlign(mFontContext, FONS_ALIGN_CENTER | FONS_ALIGN_MIDDLE);
    
    Vec4f textColor{1.0f, 1.0f, 1.0f, 1.0f};

    
    // Rasterize glyphs into atlas
    fonsDrawText(mFontContext, button.x, button.y, button.label.c_str(), nullptr);
    
    
    FONSquad quad;
    fonsTextIterInit(mFontContext, &iter, button.x, button.y, 
                     button.label.c_str(), nullptr);
    
    while (fonsTextIterNext(mFontContext, &iter, &quad))
    {
        float verts[] = {
            quad.x0,quad.y0, quad.s0, quad.t0, textColor.x, textColor.y, textColor.z, textColor.w,
            quad.x0,quad.y1, quad.s0, quad.t1, textColor.x, textColor.y, textColor.z, textColor.w,
            quad.x1,quad.y1, quad.s1, quad.t1, textColor.x, textColor.y, textColor.z, textColor.w,

            quad.x0,quad.y0, quad.s0, quad.t0, textColor.x, textColor.y, textColor.z, textColor.w,
            quad.x1,quad.y1, quad.s1, quad.t1, textColor.x, textColor.y, textColor.z, textColor.w,
            quad.x1,quad.y0, quad.s1, quad.t0, textColor.x, textColor.y, textColor.z, textColor.w,
        };
        
        mTextVertices.insert(mTextVertices.end(), std::begin(verts), std::end(verts));
    }
    
    return wasClicked; // only true if button was clicked
}

void UIRenderer::renderQuad(float x, float y, float w, float h, Vec4f color)
{
    // Quad without texture (texcoords are 0)
    float verts[] = {
        x,y, 0,0,color.x, color.y, color.z, color.w,
        x,y+h,0,0,color.x, color.y, color.z, color.w,
        x+w,y+h,0,0,color.x, color.y, color.z, color.w,

        x,y, 0,0,color.x, color.y, color.z, color.w,
        x+w,y+h,0,0,color.x, color.y, color.z, color.w,
        x+w,y,0,0,color.x, color.y, color.z, color.w,
    };
    
    mQuadVertices.insert(mQuadVertices.end(), std::begin(verts), std::end(verts));
}


// Finish frame and render everything
void UIRenderer::endFrame()
{
#ifdef UI_DEBUG_PRINTS
    std::print("=== UIRenderer::endFrame() START ===\n");
    std::print("Quad vertices: {}, Text vertices: {}\n", mQuadVertices.size(), mTextVertices.size());
#endif
    
    if (mQuadVertices.empty() && mTextVertices.empty())
    {
#ifdef UI_DEBUG_PRINTS
        std::print("No vertices to render!\n");
#endif
        return;
    }
    
#ifdef UI_DEBUG_PRINTS
    std::print("Window size: {}x{}\n", mWindowWidth, mWindowHeight);
#endif
    
    // Save GL state
    GLboolean wasBlendEnabled = glIsEnabled(GL_BLEND);
    GLboolean wasDepthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean wasCullFaceEnabled = glIsEnabled(GL_CULL_FACE);
    GLboolean wasSRGBEnabled = glIsEnabled(GL_FRAMEBUFFER_SRGB);
    
    // Setup for 2D rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_FRAMEBUFFER_SRGB);
    
    // Orthographic projection
    Mat44f projection{};
    projection[0,0] = 2.0f / mWindowWidth;
    projection[1,1] = -2.0f / mWindowHeight;
    projection[2,2] = -1.0f;
    projection[3,3] = 1.0f;
    projection[0,3] = -1.0f;
    projection[1,3] = 1.0f;
    
    glUseProgram(mShader.programId());
    glUniformMatrix4fv(0, 1, GL_TRUE, projection.v);
    
    glBindVertexArray(mVAO);
    
    // Render non textured quads (backgrounds, outlines)
    if (!mQuadVertices.empty())
    {
        glUniform1i(2, 0);  // uUseTexture = 0 (no texture)
        
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, mQuadVertices.size() * sizeof(float), 
                     mQuadVertices.data(), GL_STREAM_DRAW);
        
        int quadVertexCount = mQuadVertices.size() / 8;
        glDrawArrays(GL_TRIANGLES, 0, quadVertexCount);
#ifdef UI_DEBUG_PRINTS
        std::print("  Drew {} quad vertices\n", quadVertexCount);
#endif
    }    
    // Render textured quads (text glyphs)
   
    if (!mTextVertices.empty())
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mFontTexture);
        glUniform1i(1, 0);  // uFontTexture = texture unit 0
        glUniform1i(2, 1);  // uUseTexture = 1
        
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, mTextVertices.size() * sizeof(float), 
                     mTextVertices.data(), GL_STREAM_DRAW);
        
        int textVertexCount = mTextVertices.size() / 8;
        glDrawArrays(GL_TRIANGLES, 0, textVertexCount);
#ifdef UI_DEBUG_PRINTS
        std::print("  Drew {} text vertices\n", textVertexCount);
#endif
    }
    
    
    glBindVertexArray(0);
    
    // Restore GL state
    if (!wasBlendEnabled) glDisable(GL_BLEND);
    if (wasDepthTestEnabled) glEnable(GL_DEPTH_TEST);
    if (wasCullFaceEnabled) glEnable(GL_CULL_FACE);
    if (wasSRGBEnabled) glEnable(GL_FRAMEBUFFER_SRGB);
    
#ifdef UI_DEBUG_PRINTS
    std::print("=== UIRenderer::endFrame() COMPLETE ===\n\n");
#endif
}

void UIRenderer::uploadVertices()
{
#ifdef UI_DEBUG_PRINTS
    std::print("  uploadVertices() - VAO: {}, VBO: {}\n", mVAO, mVBO);
#endif
    
    glBindVertexArray(mVAO);
#ifdef UI_DEBUG_PRINTS
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        std::print("  ERROR after glBindVertexArray: 0x{:X}\n", err);
#endif
    
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
#ifdef UI_DEBUG_PRINTS
    err = glGetError();
    if (err != GL_NO_ERROR)
        std::print("  ERROR after glBindBuffer: 0x{:X}\n", err);
#endif
    
    size_t byteSize = mTextVertices.size() * sizeof(float);
#ifdef UI_DEBUG_PRINTS
    std::print("  Uploading {} bytes ({} floats, {} text vertices)\n", 
               byteSize, mTextVertices.size(), mTextVertices.size() / 8);
#endif
    
    glBufferData(GL_ARRAY_BUFFER, byteSize, mTextVertices.data(), GL_STREAM_DRAW);
#ifdef UI_DEBUG_PRINTS
    err = glGetError();
    if (err != GL_NO_ERROR)
        std::print("  ERROR after glBufferData: 0x{:X}\n", err);
#endif
    
    int vertexCount = mTextVertices.size() / 8;
#ifdef UI_DEBUG_PRINTS
    std::print("  Drawing {} text vertices ({} triangles)\n", vertexCount, vertexCount / 3);
#endif
    
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
#ifdef UI_DEBUG_PRINTS
    err = glGetError();
    if (err != GL_NO_ERROR)
        std::print("  ERROR after glDrawArrays: 0x{:X}\n", err);
    else
        std::print("  glDrawArrays completed successfully!\n");
#endif
    
    glBindVertexArray(0);
}