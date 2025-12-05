#ifndef SIMPLE_MESH_HPP_C6B749D6_C83B_434C_9E58_F05FC27FEFC9
#define SIMPLE_MESH_HPP_C6B749D6_C83B_434C_9E58_F05FC27FEFC9

#include <glad/glad.h>
#include <string>

#include <vector>

#include "../vmlib/vec3.hpp"
#include "../vmlib/vec2.hpp"

struct SimpleMeshData
{
	std::vector<Vec3f> positions;
	std::vector<Vec3f> normals;
	std::vector<Vec3f> colors;
	std::vector<Vec2f> texcoords;
	std::vector<float> shininess;

	bool has_texture() const
	{
		return !texture_filepath.empty();
	}


	std::string texture_filepath;
};

SimpleMeshData concatenate( SimpleMeshData, SimpleMeshData const& );


GLuint create_vao( SimpleMeshData const& );

#endif // SIMPLE_MESH_HPP_C6B749D6_C83B_434C_9E58_F05FC27FEFC9
