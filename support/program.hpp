#ifndef PROGRAM_HPP_EEC27A62_D86E_4D88_A66C_7A8E7142515A
#define PROGRAM_HPP_EEC27A62_D86E_4D88_A66C_7A8E7142515A

#include <glad/glad.h>

#include <string>
#include <vector>

#include <cstdint>
#include <cstdlib>

class ShaderProgram final
{
	public:
		struct ShaderSource
		{
			GLenum type;
			std::string sourcePath;
		};

	public:
		explicit ShaderProgram( 
			std::vector<ShaderSource> = {}
		);

		~ShaderProgram();

		ShaderProgram( ShaderProgram const& ) = delete;
		ShaderProgram& operator= (ShaderProgram const&) = delete;

		ShaderProgram( ShaderProgram&& ) noexcept;
		ShaderProgram& operator= (ShaderProgram&&) noexcept;

	public:
		GLuint programId() const noexcept;

		void reload();

	private:
		GLuint mProgram;
		std::vector<ShaderSource> mSources;
};

#endif // PROGRAM_HPP_EEC27A62_D86E_4D88_A66C_7A8E7142515A
