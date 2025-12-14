#include "simple_mesh.hpp"

// combine two SimpleMeshData objects
SimpleMeshData concatenate( SimpleMeshData aM, SimpleMeshData const& aN )
{
	// append all vertex positions from first shape aN to second shape aM
	aM.positions.insert( aM.positions.end(), aN.positions.begin(), aN.positions.end() );
	// append colours
	aM.colors.insert( aM.colors.end(), aN.colors.begin(), aN.colors.end() );
	// append normals
	aM.normals.insert( aM.normals.end(), aN.normals.begin(), aN.normals.end() );
	// append texture coordinates
	aM.texcoords.insert( aM.texcoords.end(), aN.texcoords.begin(), aN.texcoords.end() );

	// append material properties
	aM.Ns.insert( aM.Ns.end(), aN.Ns.begin(), aN.Ns.end() );
	aM.Ka.insert( aM.Ka.end(), aN.Ka.begin(), aN.Ka.end() );
	aM.Kd.insert( aM.Kd.end(), aN.Kd.begin(), aN.Kd.end() );
	aM.Ke.insert( aM.Ke.end(), aN.Ke.begin(), aN.Ke.end() );
	aM.Ks.insert( aM.Ks.end(), aN.Ks.begin(), aN.Ks.end() );
	
	// final combined shape is the new aM
	return aM;
}


// creating a vertex array object from a SimpleMeshData object to tell opengl which vertex buffer objects to use
// it stores the vertex attribute layout from the shaders
GLuint create_vao( SimpleMeshData const& aMeshData )
{
	// create VBO for positions
	GLuint positionsVBO = 0;
	glGenBuffers( 1, &positionsVBO );
	glBindBuffer( GL_ARRAY_BUFFER, positionsVBO );
	// upload data from CPU memory to GPU (the vbo)
	glBufferData(
		GL_ARRAY_BUFFER,
		aMeshData.positions.size() * sizeof(Vec3f),
		aMeshData.positions.data(),
		GL_STATIC_DRAW
	);

	// create VBO for normals
	GLuint normalsVBO = 0;
	glGenBuffers( 1, &normalsVBO );
	glBindBuffer( GL_ARRAY_BUFFER, normalsVBO );
	glBufferData(
		GL_ARRAY_BUFFER,
		aMeshData.normals.size() * sizeof(Vec3f),
		aMeshData.normals.data(),
		GL_STATIC_DRAW
	);
	
	// create vbos for other attributes if needed
	GLuint texcoordsVBO = 0;
	GLuint colorsVBO = 0;
	GLuint NsVBO = 0;
	GLuint KaVBO = 0;
	GLuint KdVBO = 0;
	GLuint KeVBO = 0;
	GLuint KsVBO = 0;
	
	//  create and upload vbos for texture coordinates if the mesh has them
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
		// create vbo for colors
		glGenBuffers( 1, &colorsVBO );
		glBindBuffer( GL_ARRAY_BUFFER, colorsVBO );
		glBufferData(
			GL_ARRAY_BUFFER,
			aMeshData.colors.size() * sizeof(Vec3f),
			aMeshData.colors.data(),
			GL_STATIC_DRAW
		);

		// create vbos for material properties
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

	// cleanup, unbind any buffers
	glBindBuffer( GL_ARRAY_BUFFER, 0 );


	// create and bind the vertex array object
	GLuint vao = 0;
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );

	// bind position and normal vbo to vao so attribute locations are setup
	glBindBuffer( GL_ARRAY_BUFFER, positionsVBO );
	
	glVertexAttribPointer(
		// location 0 in vertex shader
		0, 
		// 3 components(xyz), all floats, not normalized to since positions are the actual values
		3, GL_FLOAT, GL_FALSE, 
		// no stide between inputs (no padding)
		0, 
		// starting at beginning of vbo data
		0 
	);
	glEnableVertexAttribArray( 0 );

	glBindBuffer( GL_ARRAY_BUFFER, normalsVBO );
	glVertexAttribPointer(
		// normals in location 1
		1,
		3, GL_FLOAT, GL_FALSE, 
		0,
		0
	);
	glEnableVertexAttribArray( 1 );

	if ( aMeshData.has_texture() ){
		glBindBuffer( GL_ARRAY_BUFFER, texcoordsVBO );
		glVertexAttribPointer(
			// location 3 in vertex shader
			3, 
			// 2 components (uv)
			2,
			GL_FLOAT,
			GL_FALSE,
			0,
			0
		);
		glEnableVertexAttribArray( 3 );
	}
	else{
		glBindBuffer( GL_ARRAY_BUFFER, colorsVBO );
		glVertexAttribPointer(
			// location 3 can be reused since texture coordinates are not used
			3,
			// rgb
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			0
		);
		glEnableVertexAttribArray( 3 );


		glBindBuffer( GL_ARRAY_BUFFER, NsVBO );
		glVertexAttribPointer(
			4,
			// shininess is a single float
			1,
			GL_FLOAT,
			GL_FALSE,
			0,
			0
		);
		glEnableVertexAttribArray( 4 );


		glBindBuffer( GL_ARRAY_BUFFER, KaVBO );
		glVertexAttribPointer(
			5,
			// ambience amd other materials uses vec3 for rgb
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			0
		);
		glEnableVertexAttribArray( 5 );	

		glBindBuffer( GL_ARRAY_BUFFER, KdVBO );
		glVertexAttribPointer(
			6,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			0
		);
		glEnableVertexAttribArray( 6 );

		glBindBuffer( GL_ARRAY_BUFFER, KeVBO );
		glVertexAttribPointer(
			7,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			0
		);
		glEnableVertexAttribArray( 7 );

		glBindBuffer( GL_ARRAY_BUFFER, KsVBO );
		glVertexAttribPointer(
			8,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			0
		);
		glEnableVertexAttribArray( 8 );
	}

	

	// cleanup, unbind vao and vbo
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	// discard buffers, the vao keeps references to them
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

