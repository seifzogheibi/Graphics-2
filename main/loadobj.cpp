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

		        // Safe texcoord fetch: if no texcoord, use (0,0)
        Vec2f uv{ 0.f, 0.f };
        if( idx.texcoord_index >= 0 )
        {
            std::size_t t = static_cast<std::size_t>(idx.texcoord_index) * 2;
            uv.x = res.attributes.texcoords[t + 0];
            uv.y = res.attributes.texcoords[t + 1];
        }
        ret.texcoords.emplace_back( uv );


		// Always triangles, so we can find the face index by dividing the vertex index by three
		auto const& mat = res.materials[shape.mesh.material_ids[i/3]];
			// Just replicate the material ambient color for each vertex...
			ret.colors.emplace_back( Vec3f{
				mat.ambient[0],
				mat.ambient[1],
				mat.ambient[2]
			} );
			ret.Ka.emplace_back( Vec3f{
				mat.ambient[0],
				mat.ambient[1],
				mat.ambient[2]
			} );
			ret.Kd.emplace_back( Vec3f{
				mat.diffuse[0],
				mat.diffuse[1],
				mat.diffuse[2]
			} );
			ret.Ke.emplace_back( Vec3f{
				mat.emission[0],
				mat.emission[1],
				mat.emission[2]
			} );
			ret.Ks.emplace_back( Vec3f{
				mat.specular[0],
				mat.specular[1],
				mat.specular[2]
			} );
			ret.Ns.emplace_back( mat.shininess );
			
		}
}
return ret;
}