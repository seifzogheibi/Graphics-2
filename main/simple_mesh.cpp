#include "simple_mesh.hpp"

SimpleMeshData concatenate( SimpleMeshData aM, SimpleMeshData const& aN )
{
	aM.positions.insert( aM.positions.end(), aN.positions.begin(), aN.positions.end() );
	aM.colors.insert( aM.colors.end(), aN.colors.begin(), aN.colors.end() );
	aM.normals.insert( aM.normals.end(), aN.normals.begin(), aN.normals.end() );
	aM.texcoords.insert( aM.texcoords.end(), aN.texcoords.begin(), aN.texcoords.end() );
	aM.shininess.insert( aM.shininess.end(), aN.shininess.begin(), aN.shininess.end() );
	return aM;
}


GLuint create_vao( SimpleMeshData const& aMeshData )
{
	GLuint positionsVBO = 0;
	glGenBuffers( 1, &positionsVBO );
	glBindBuffer( GL_ARRAY_BUFFER, positionsVBO );
	glBufferData(
		GL_ARRAY_BUFFER,
		aMeshData.positions.size() * sizeof(Vec3f),
		aMeshData.positions.data(),
		GL_STATIC_DRAW
	);

	GLuint normalsVBO = 0;
	glGenBuffers( 1, &normalsVBO );
	glBindBuffer( GL_ARRAY_BUFFER, normalsVBO );
	glBufferData(
		GL_ARRAY_BUFFER,
		aMeshData.normals.size() * sizeof(Vec3f),
		aMeshData.normals.data(),
		GL_STATIC_DRAW
	);
	
	GLuint texcoordsVBO = 0;
	GLuint colorsVBO = 0;
	GLuint shininessVBO = 0;
	
	if ( aMeshData.has_texture() ){
		glGenBuffers( 1, &texcoordsVBO );
		glBindBuffer( GL_ARRAY_BUFFER, texcoordsVBO );
		glBufferData(
			GL_ARRAY_BUFFER,
			aMeshData.texcoords.size() * sizeof(Vec2f),
			aMeshData.texcoords.data(),
			GL_STATIC_DRAW
		);
	}
	else {
		glGenBuffers( 1, &colorsVBO );
		glBindBuffer( GL_ARRAY_BUFFER, colorsVBO );
		glBufferData(
			GL_ARRAY_BUFFER,
			aMeshData.colors.size() * sizeof(Vec3f),
			aMeshData.colors.data(),
			GL_STATIC_DRAW
		);

		glGenBuffers( 1, &shininessVBO );
		glBindBuffer( GL_ARRAY_BUFFER, shininessVBO );
		glBufferData(
			GL_ARRAY_BUFFER,
			aMeshData.shininess.size() * sizeof(float),
			aMeshData.shininess.data(),
			GL_STATIC_DRAW
		);
	}

	// cleanup
	glBindBuffer( GL_ARRAY_BUFFER, 0 );


	//TODO: implement me
	GLuint vao = 0;
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );

	glBindBuffer( GL_ARRAY_BUFFER, positionsVBO );
	
	glVertexAttribPointer(
		0, // location = 0 in vertex shader
		3, GL_FLOAT, GL_FALSE, // 3 floats, not normalized to [0..1] (GL FALSE)
		0, // stride = 0 indicates that there is no padding between inputs
		0 // data starts at offset 0 in the VBO.
	);
	glEnableVertexAttribArray( 0 );

	glBindBuffer( GL_ARRAY_BUFFER, normalsVBO );
	glVertexAttribPointer(
		1, // location = 1 in vertex shader
		3, GL_FLOAT, GL_FALSE, // 3 floats, not normalized to [0..1] (GL FALSE)
		0, // stride = 0 indicates that there is no padding between inputs
		0 // data starts at offset 0 in the VBO.
	);
	glEnableVertexAttribArray( 1 );

	if ( aMeshData.has_texture() ){
		glBindBuffer( GL_ARRAY_BUFFER, texcoordsVBO );
		glVertexAttribPointer(
			3,              // layout(location = 3) in vertex shader
			2,              // 2 components (u,v)
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);
	}
	else{
		glBindBuffer( GL_ARRAY_BUFFER, colorsVBO );
		glVertexAttribPointer(
			3,              // layout(location = 3) in vertex shader
			3,              // r, g, b
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);
		glEnableVertexAttribArray( 3 );

		glBindBuffer( GL_ARRAY_BUFFER, shininessVBO );
		glVertexAttribPointer(
			4,              // layout(location = 4) in vertex shader
			1,              // single float
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);
		glEnableVertexAttribArray( 4 );
	}
	


	glBindVertexArray( 0 );

	// Unbind VBO (good practice)
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	// Discard vbos
	glDeleteBuffers( 1, &positionsVBO );
	glDeleteBuffers( 1, &normalsVBO );
	glDeleteBuffers( 1, &texcoordsVBO );
	glDeleteBuffers( 1, &colorsVBO );
	glDeleteBuffers( 1, &shininessVBO );


	return vao;
}

