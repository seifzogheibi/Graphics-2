#include "texture.hpp"

#include <cassert>

#include <stb_image.h>

#include "../support/error.hpp"

GLuint load_texture_2d( char const* aPath, bool linearFilter )
{
	assert( aPath );

	//TODO: implement me
	
	assert( aPath );
// Load image first
// This may fail (e.g., image does not exist), so there’s no point in
// allocating OpenGL resources ahead of time.
stbi_set_flip_vertically_on_load( true );
int w, h, channels;
stbi_uc* ptr = stbi_load( aPath, &w, &h, &channels, 4 );
if( !ptr )
	 throw Error( "Unable to load image ’{}’\n", aPath );

GLuint tex = 0;
glGenTextures( 1, &tex );
glBindTexture( GL_TEXTURE_2D, tex );
glTexImage2D( GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptr );

stbi_image_free( ptr );

// Generate mipmap hierarchy
glGenerateMipmap( GL_TEXTURE_2D );

// Configure texture
glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linearFilter ? GL_LINEAR : GL_NEAREST );
glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 6.f );
return tex;
}
