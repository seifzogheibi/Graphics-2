#include "checkpoint.hpp"

#include <glad/glad.h>

#include "error.hpp"

namespace
{
	char const* error_string_( GLenum aErr )
	{
		switch( aErr )
		{
			// See
			// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetError.xhtml 
			case GL_INVALID_ENUM: return "invalid enum";
			case GL_INVALID_VALUE: return "invalid value";
			case GL_INVALID_OPERATION: return "invalid operation";
			case GL_INVALID_FRAMEBUFFER_OPERATION: return "invalid framebuffer operation";
			case GL_OUT_OF_MEMORY: return "out of memory";
			case GL_STACK_UNDERFLOW: return "underflow";
			case GL_STACK_OVERFLOW: return "overflow";
		}

		return "<unknown error value>";
	}
}

namespace detail
{
	void check_gl_error( char const* aSourceFile, int aSourceLine )
	{
		auto const res = glGetError();
		if( GL_NO_ERROR != res )
		{
			throw Error( "({}:{}) glGetError() returned {} ({})", aSourceFile, aSourceLine, error_string_(res), res );
		}
	}
}
