#include "simple_mesh.hpp"

SimpleMeshData concatenate( SimpleMeshData aM, SimpleMeshData const& aN )
{
	aM.positions.insert( aM.positions.end(), aN.positions.begin(), aN.positions.end() );
	aM.colors.insert( aM.colors.end(), aN.colors.begin(), aN.colors.end() );
	aM.normals.insert( aM.normals.end(), aN.normals.begin(), aN.normals.end() );
	aM.texcoords.insert( aM.texcoords.end(), aN.texcoords.begin(), aN.texcoords.end() );
	aM.Ns.insert( aM.Ns.end(), aN.Ns.begin(), aN.Ns.end() );
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
	GLuint NsVBO = 0;
	GLuint KaVBO = 0;
	GLuint KdVBO = 0;
	GLuint KeVBO = 0;
	GLuint KsVBO = 0;
	
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

		glGenBuffers( 1, &NsVBO );
		glBindBuffer( GL_ARRAY_BUFFER, NsVBO );
		glBufferData(
			GL_ARRAY_BUFFER,
			aMeshData.Ns.size() * sizeof(float),
			aMeshData.Ns.data(),
			GL_STATIC_DRAW
		);

		glGenBuffers( 1, &KaVBO );
		glBindBuffer( GL_ARRAY_BUFFER, KaVBO );
		glBufferData(
			GL_ARRAY_BUFFER,
			aMeshData.Ka.size() * sizeof(Vec3f),
			aMeshData.Ka.data(),
			GL_STATIC_DRAW
		);

		glGenBuffers( 1, &KdVBO );
		glBindBuffer( GL_ARRAY_BUFFER, KdVBO );
		glBufferData(
			GL_ARRAY_BUFFER,
			aMeshData.Kd.size() * sizeof(Vec3f),
			aMeshData.Kd.data(),
			GL_STATIC_DRAW
		);

		glGenBuffers( 1, &KeVBO );
		glBindBuffer( GL_ARRAY_BUFFER, KeVBO );
		glBufferData(
			GL_ARRAY_BUFFER,
			aMeshData.Ke.size() * sizeof(Vec3f),
			aMeshData.Ke.data(),
			GL_STATIC_DRAW
		);

		glGenBuffers( 1, &KsVBO );
		glBindBuffer( GL_ARRAY_BUFFER, KsVBO );
		glBufferData(
			GL_ARRAY_BUFFER,
			aMeshData.Ks.size() * sizeof(Vec3f),
			aMeshData.Ks.data(),
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
		glEnableVertexAttribArray( 3 );   // <-- ADD THIS
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


		glBindBuffer( GL_ARRAY_BUFFER, NsVBO );
		glVertexAttribPointer(
			4,              // layout(location = 4) in vertex shader
			1,              // single float
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);
		glEnableVertexAttribArray( 4 );


		glBindBuffer( GL_ARRAY_BUFFER, KaVBO );
		glVertexAttribPointer(
			5,              // layout(location = 5) in vertex shader
			3,              // r, g, b
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);
		glEnableVertexAttribArray( 5 );	

		glBindBuffer( GL_ARRAY_BUFFER, KdVBO );
		glVertexAttribPointer(
			6,              // layout(location = 6) in vertex shader
			3,              // r, g, b
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);
		glEnableVertexAttribArray( 6 );

		glBindBuffer( GL_ARRAY_BUFFER, KeVBO );
		glVertexAttribPointer(
			7,              // layout(location = 7) in vertex shader
			3,              // r, g, b
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);
		glEnableVertexAttribArray( 7 );

		glBindBuffer( GL_ARRAY_BUFFER, KsVBO );
		glVertexAttribPointer(
			8,              // layout(location = 8) in vertex shader
			3,              // r, g, b
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);
		glEnableVertexAttribArray( 8 );
	}

	


	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	// Discard vbos
	glDeleteBuffers( 1, &positionsVBO );
	glDeleteBuffers( 1, &normalsVBO );
	glDeleteBuffers( 1, &texcoordsVBO );
	glDeleteBuffers( 1, &colorsVBO );
	glDeleteBuffers( 1, &NsVBO );
	glDeleteBuffers( 1, &KaVBO );
	glDeleteBuffers( 1, &KdVBO );
	glDeleteBuffers( 1, &KeVBO );
	glDeleteBuffers( 1, &KsVBO );

	return vao;
}

