#include "loadobj.hpp"

#include <rapidobj/rapidobj.hpp>

#include "../support/error.hpp"

SimpleMeshData load_wavefront_obj( char const* aPath )
{
	// Ask rapidobj to load the requested file
	auto res = rapidobj::ParseFile( aPath );
	if( res.error )
	{
		throw Error( "Unable to load OBJ file ’{}’: {}", aPath, res.error.code.message() 
	);
	}
// OBJ files can define faces that are not triangles. However, OpenGL will only render triangles (and lines
// and points), so we must triangulate any faces that are not already triangles. Fortunately, rapidobj can do
// this for us.
rapidobj::Triangulate( res );

// Convert the OBJ data into a SimpleMeshData structure. For now, we simply turn the object into a triangle
// soup, ignoring the indexing information that the OBJ file contains.
SimpleMeshData ret;

if ( res.materials.size() > 0 ){
	ret.texture_filepath = res.materials[0].diffuse_texname;
}

for( auto const& shape : res.shapes )
{
	for( std::size_t i = 0; i < shape.mesh.indices.size(); ++i )
	{
		auto const& idx = shape.mesh.indices[i];

		ret.positions.emplace_back( Vec3f{
			res.attributes.positions[idx.position_index*3+0],
			res.attributes.positions[idx.position_index*3+1],
			res.attributes.positions[idx.position_index*3+2]
		} );

		ret.normals.emplace_back( Vec3f{
			res.attributes.normals[idx.normal_index*3+0],
			res.attributes.normals[idx.normal_index*3+1],
			res.attributes.normals[idx.normal_index*3+2]
		} );

		ret.texcoords.emplace_back( Vec2f{
			res.attributes.texcoords[idx.texcoord_index*2+0],
			res.attributes.texcoords[idx.texcoord_index*2+1]
		} );

		// Always triangles, so we can find the face index by dividing the vertex index by three
		auto const& mat = res.materials[shape.mesh.material_ids[i/3]];
			// Just replicate the material ambient color for each vertex...
			ret.colors.emplace_back( Vec3f{
				mat.ambient[0],
				mat.ambient[1],
				mat.ambient[2]
			} );

			ret.shininess.emplace_back( mat.shininess );
			
	
		}
}
return ret;
}

