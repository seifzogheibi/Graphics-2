#include "texture.hpp"

#include <cassert>

#include <stb_image.h>

#include "../support/error.hpp"

GLuint load_texture_2d( char const* aPath )
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
glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 6.f );
return tex;
}
	
	
	
	// // Load image data using stb_image
	// stbi_set_flip_vertically_on_load(1); // Flip image vertically (OpenGL expects bottom-left origin)
	
	// int width, height, channels;
	// unsigned char* data = stbi_load(aPath, &width, &height, &channels, 0);
	
	// if (!data)
	// {
	// 	throw Error("Failed to load texture: %s", aPath);
	// }
	
	// // Determine format based on number of channels
	// GLenum format;
	// if (channels == 1)
	// 	format = GL_RED;
	// else if (channels == 3)
	// 	format = GL_RGB;
	// else if (channels == 4)
	// 	format = GL_RGBA;
	// else
	// {
	// 	stbi_image_free(data);
	// 	throw Error("Unsupported number of channels (%d) in texture: %s", channels, aPath);
	// }
	
	// // Generate and bind texture
	// GLuint textureId;
	// glGenTextures(1, &textureId);
	// glBindTexture(GL_TEXTURE_2D, textureId);
	
	// // Upload texture data
	// glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	
	// // Set texture parameters
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	// // Generate mipmaps
	// glGenerateMipmap(GL_TEXTURE_2D);
	
	// // Free image data
	// stbi_image_free(data);
	
	// return textureId;
