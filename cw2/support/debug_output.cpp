#include "debug_output.hpp"

#include <print>

#include <cassert>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "error.hpp"
#include "checkpoint.hpp"

namespace
{
	// Debug callback
#	if !defined(NDEBUG)
	void GLAPIENTRY callback_gldebug_( GLenum, GLenum, GLuint, GLenum, GLsizei, GLchar const*, void const* );
#	endif // ~ !NDEBUG
}

void setup_gl_debug_output()
{
#	if !defined(NDEBUG)
	OGL_CHECKPOINT_ALWAYS();

	// glDebugMessageCallback() was standardized in 4.3, so it's not available
	// Apple. The extension (ARB_debug_output), which predates standardization
	// doesn't seem to exist on Apple either.
#	if !defined(__APPLE__)
	glDebugMessageCallback( &callback_gldebug_, nullptr );
	glEnable( GL_DEBUG_OUTPUT );

	// Make sure the callback is called synchronously and from the same thread.
	// This makes the debugger more useful.
	glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
#	endif // ~ __APPLE__

	OGL_CHECKPOINT_ALWAYS();
#	endif // ~ !NDEBUG
}

namespace
{
#	if !defined(NDEBUG)
	char const* type_str_( GLenum aType ) noexcept
	{
		switch( aType )
		{
			case GL_DEBUG_TYPE_ERROR: return "Error";
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated Behaviour";
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "Undefined Behaviour";
			case GL_DEBUG_TYPE_PORTABILITY: return "Portability";
			case GL_DEBUG_TYPE_PERFORMANCE: return "Performance";
			case GL_DEBUG_TYPE_MARKER: return "Marker";
			case GL_DEBUG_TYPE_PUSH_GROUP: return "Push";
			case GL_DEBUG_TYPE_POP_GROUP: return "Pop";
			case GL_DEBUG_TYPE_OTHER: return "Other";
		}

		return "<unknown type>";
	}
	char const* severity_str_( GLenum aSeverity ) noexcept
	{
		switch( aSeverity )
		{
			case GL_DEBUG_SEVERITY_HIGH: return "ERROR";
			case GL_DEBUG_SEVERITY_MEDIUM: return "Warning";
			case GL_DEBUG_SEVERITY_LOW: return "Note";
			case GL_DEBUG_SEVERITY_NOTIFICATION: return "Info";
		}

		return "<unknown severity>";
	}

	void GLAPIENTRY callback_gldebug_( GLenum, GLenum aType, GLuint, GLenum aSeverity, GLsizei, GLchar const* aMessage, void const* /*aUser*/ )
	{
		// "Other" can be a bit spammy at times. However, it can include fairly
		// interesting information on e.g. NVIDIA (such as in what memory VBOs
		// are placed, or when shaders are being recompiled). Comment these two
		// lines if you want to see this:
		if( GL_DEBUG_TYPE_OTHER == aType )
			return;

		std::print( stderr, "OpenGL Debug: {} [{}]: {}\n", severity_str_(aSeverity), type_str_(aType), aMessage );

		// For high severity errors, break into the debugger now.
		if( GL_DEBUG_SEVERITY_HIGH == aSeverity )
			assert( false );
	}
#	endif // ~ !NDEBUG
}

