#include "program.hpp"

#include <print>
#include <vector>
#include <utility>

#include <cstdio>

#include <glad/glad.h>

#include "error.hpp"
#include "checkpoint.hpp"

namespace
{
	GLuint load_shader_( 
		GLenum aShaderType, 
		char const* aSourcePath
	);

	// lightweight std::experimental::scope_exit alternative
	// Not the most complete or convenient implementation...
	template< typename tFunc >
	struct ScopeExit_
	{
		template< typename tFuncIn >
		ScopeExit_( tFuncIn&& aFunc ) : mFunc( std::forward<tFuncIn>(aFunc) ) {}
		~ScopeExit_() { mFunc(); }

		tFunc mFunc;
	};

	template< typename tFunc > inline
	ScopeExit_<tFunc> scope_exit_( tFunc&& aFunc )
	{
		return ScopeExit_<tFunc>( std::forward<tFunc>(aFunc) );
	}
}

ShaderProgram::ShaderProgram( std::vector<ShaderSource> aShaderSources )
	: mProgram( 0 )
	, mSources( std::move(aShaderSources) )
{
	reload();
}

ShaderProgram::~ShaderProgram()
{
	if( 0 != mProgram )
		glDeleteProgram( mProgram );
}

ShaderProgram::ShaderProgram( ShaderProgram&& aOther ) noexcept
	: mProgram( std::exchange( aOther.mProgram, 0 ) )
	, mSources( std::move(aOther.mSources) )
{}
ShaderProgram& ShaderProgram::operator= (ShaderProgram&& aOther) noexcept
{
	std::swap( mProgram, aOther.mProgram );
	std::swap( mSources, aOther.mSources );
	return *this;
}

GLuint ShaderProgram::programId() const noexcept
{
	return mProgram;
}

void ShaderProgram::reload()
{
	// Space to hold the shaders when we load them
	std::vector<GLuint> shaders;
	shaders.reserve( mSources.size() );

	// Ensure that shaders are cleaned up properly, regardless of how we leave
	// the function (e.g., either by returning or by exception)
	auto const scopeShaders_ = scope_exit_( [&shaders] {
		for( auto const shader : shaders )
			glDeleteShader( shader );
	} );

	// Load shaders
	for( auto const& source : mSources )
		shaders.emplace_back( load_shader_( source.type, source.sourcePath.c_str() ) );

	// Create program object
	OGL_CHECKPOINT_ALWAYS();

	GLuint prog = glCreateProgram();

	// Ensure that the program is cleaned up. 

	/* There is a small trick here. If we successfully compile and link the new
	 * program, we will replace the value of the "prog" variable with the old
	 * program's ID. In this case, the following will delete the old program
	 * (if there was any). If we do not reach the end (e.g. exception thrown),
	 * the new program ID will still be in the "prog" variable, and we will
	 * delete it appropriately. (However, the old program in mProgram is left
	 * intact).
	 */
	auto const scopeProgram_ = scope_exit_( [&prog] {
		if( 0 != prog )
			glDeleteProgram( prog );
	} );

	// Link individual shaders to create the final shader program
	for( auto const shader : shaders )
		glAttachShader( prog, shader );

	glLinkProgram( prog );

	{
		// Get info log
		GLint logLength = 0;
		glGetProgramiv( prog, GL_INFO_LOG_LENGTH, &logLength );

		std::vector<GLchar> log;
		if( logLength )
		{
			log.resize( logLength );
			glGetProgramInfoLog( prog, GLsizei(log.size()), nullptr, log.data() );
		}

		// Check link status
		GLint status = 0;
		glGetProgramiv( prog, GL_LINK_STATUS, &status );

		if( GL_TRUE != status )
			throw Error( "Shader program linking failed: \n{}\n", log.data() );

		if( !log.empty() )
			std::print( stderr, "Note: shader program linking log:\n{}\n", log.data() );
	}
	
	OGL_CHECKPOINT_ALWAYS();

	// Replace the old shader program (if any) with the new one
	std::swap( mProgram, prog );
}

namespace
{
	GLuint load_shader_( GLenum aShaderType, char const* aSourcePath )
	{
		// Load the shader source code from file
		std::vector<GLchar> source;

		if( std::FILE* fin = std::fopen( aSourcePath, "rb" ) )
		{
			auto const scopeFile_ = scope_exit_( [&fin] {
				std::fclose( fin );
			} );

			std::fseek( fin, 0, SEEK_END );
			auto const length = std::size_t(std::ftell( fin ));
			std::fseek( fin, 0, SEEK_SET );

			source.resize( length );
			for( std::size_t read = 0; read != length; )
			{
				auto const ret = std::fread( source.data()+read, 1, length-read, fin );

				if( 0 == ret )
				{
					if( auto const err = std::ferror( fin ) )
						throw Error( "load_shader_(): error while reading from '{}': {} ({} bytes read, {} total)", aSourcePath, err, read, length );
					if( std::feof( fin ) )
						throw Error( "load_shader_(): unexpected EOF in '{}' ({} bytes read, {} total)", aSourcePath, read, length );
				}
			
				read += ret;
			}
		}
		else
		{
			throw Error( "load_shader_(): unable to open input file '{}'", aSourcePath );
		}

		// Create shader object
		OGL_CHECKPOINT_ALWAYS();

		GLuint shader = glCreateShader( aShaderType );

		// Compile shader
		GLchar const* sources[] = {
			source.data()
		};
		GLsizei lengths[] = {
			GLsizei(source.size())
		};

		glShaderSource( shader, sizeof(sources)/sizeof(sources[0]), sources, lengths );

		glCompileShader( shader );

		OGL_CHECKPOINT_ALWAYS();

		// Get compile info log
		/* The compile log is mainly relevant if there is an error. However, on some
		 * systems, it can include additional information even if compilation was
		 * successful. This might include warnings and/or usage hints.
		 */
		GLint logLength = 0;
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logLength );

		std::vector<GLchar> log;
		if( logLength )
		{
			log.resize( logLength );
			glGetShaderInfoLog( shader, GLsizei(log.size()), nullptr, log.data() );
		}

		char const* shaderTypeName = "unknown shader";
		switch( aShaderType )
		{
			case GL_VERTEX_SHADER: shaderTypeName = "vertex shader"; break;
			case GL_FRAGMENT_SHADER: shaderTypeName = "fragment shader"; break;
			case GL_GEOMETRY_SHADER: shaderTypeName = "geometry shader"; break;
			case GL_TESS_CONTROL_SHADER: shaderTypeName = "tessellation control shader"; break;
			case GL_TESS_EVALUATION_SHADER: shaderTypeName = "tessellation evaluation shader"; break;
			case GL_COMPUTE_SHADER: shaderTypeName = "compute shader"; break;
		}

		// Check compile status
		GLint status = 0;
		glGetShaderiv( shader, GL_COMPILE_STATUS, &status );

		if( GL_TRUE != status )
		{
			glDeleteShader( shader );
			throw Error( "{} \"{}\" compilation failed:\n{}\n", shaderTypeName, aSourcePath, log.data() );
		}

		if( !log.empty() )
			std::print( stderr, "Note: {} \"{}\" log:\n{}\n", shaderTypeName, aSourcePath, log.data() );

		OGL_CHECKPOINT_ALWAYS();

		return shader;
	}
}
