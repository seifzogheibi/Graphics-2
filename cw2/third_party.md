Third Party Software
====================

This code uses the following third party software/source code.

## Premake

- Where: https://premake.github.io/download
- What: Build configuration and generator
- License: BSD 3-Clause

Premake generates platform/toolchain specific build files, such as Visual
Studio project files or standard Makefiles. This project includes prebuilt
binaries for Linux and Windows.

## GLFW

- Where: https://www.glfw.org/
- What: Multi-platform lib for OpenGL & Vulkan development (desktop)
- License: Zlib/libpng

GLFW abstracts platform-specific functions, such as creating windows and
receiving/handling events. 

## GLAD

- Where: https://github.com/Dav1dde/glad  &  https://glad.dav1d.de/
- What: Multi-Language GL/GLES/EGL/GLX/WGL Loader-Generator.
- License: MIT, Apache-2

GLAD is a OpenGL loader generator. Only the generated loader is included.  The
loader includes the core profile for OpenGL version up to 4.6, and the
following extensions:

- GL_ARB_debug_output
- GL_EXT_debug_label
- GL_EXT_debug_marker
- GL_KHR_debug
- GL_ARB_bindless_texture
- GL_ARB_gl_spirv
- GL_ARB_multi_draw_indirect
- GL_ARB_shader_clock
- GL_ARB_shader_ballot
- GL_ARB_shader_group_vote
- GL_EXT_memory_object
- GL_EXT_memory_object_fd
- GL_EXT_memory_object_win32
- GL_EXT_semaphore
- GL_EXT_semaphore_fd
- GL_EXT_semaphore_win32

## STB libraries

- Where: https://github.com/nothings/stb
- What: single-header libraries
- License: Public Domain / MIT License

Collection of single-header libraries by Sean Barrett. Included are:
- stb_image.h         (image loading)
- stb_image_write.h   (image writing)

Note: the files in src/ are custom.

## Catch2

- Where: https://github.com/catchorg/Catch2
- What: C++ unity testing framework
- License: Boost Software License v1

Catch2 is a C++ testing framework. Note that this uses the 'amalgamated'
version (e.g., one where the library has been combined into a single header
and source file).
## rapidobj

- Where: https://github.com/guybrush77/rapidobj
- What: single-header .OBJ file loader
- License: MIT

C++17 single header library for loading Wavefront OBJ files. Reasonably fast.
Uses multithreading to load larger OBJ files.

Note: only includes the actual header, README and LICENSE files.
## Fontstash

- Where: https://github.com/memononen/fontstash
- What: single-header font renderign helpers
- License: zlib

Note: also includes stb_truetype.h, which is Public Domain / MIT and by
Sean Barrett (see above).

Note: the files in src/ are custom.
