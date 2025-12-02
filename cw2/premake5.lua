workspace "COMP3811-glcode"
	language "C++"
	cppdialect "C++23"

	platforms { "x64" }
	configurations { "debug", "release" }

	flags "NoPCH"
	flags "MultiProcessorCompile"

	startproject "main"

	debugdir "%{wks.location}"
	objdir "_build_/%{cfg.buildcfg}-%{cfg.platform}-%{cfg.toolset}"
	targetsuffix "-%{cfg.buildcfg}-%{cfg.platform}-%{cfg.toolset}"
	
	-- Default toolset options
	filter "toolset:gcc or toolset:clang"
		linkoptions { "-pthread" }
		buildoptions { "-march=native", "-Wall", "-pthread" }

		-- Varriable-length arrays (VLAs) are an extension that GCC and clang
		-- have long supported. However, they are not part of the C++ standard.
		-- (MSVC will not compile code with VLAs.)
		buildoptions { "-Werror=vla" }

	filter "toolset:gcc" 
		links { "stdc++exp" }

	filter "toolset:msc-*"
		warnings "extra" -- this enables /W4; default is /W3
		--buildoptions { "/W4" }
		buildoptions { "/utf-8" }
		buildoptions { "/permissive-" }
		defines { "_CRT_SECURE_NO_WARNINGS=1" }
		defines { "_SCL_SECURE_NO_WARNINGS=1" }

		buildoptions {
			"/wd4456", -- declaration of 'foo' hides previous local declaration
		}

	filter "*"

	-- default libraries
	filter "system:linux"
		links "dl"
	
	filter "system:windows"
		links "OpenGL32"

	filter "*"

	-- default outputs
	filter "kind:StaticLib"
		targetdir "lib/"

	filter "kind:ConsoleApp"
		targetdir "bin/"
		targetextension ".exe"
	
	filter "*"

	--configurations
	filter "debug"
		symbols "On"
		defines { "_DEBUG=1" }

	filter "release"
		optimize "On"
		defines { "NDEBUG=1" }

	filter "*"


defines( "SOLUTION_CODE=1" )


-- Third party dependencies
include "third_party" 

-- Projects
project "main"
	local sources = { 
		"main/**.cpp",
		"main/**.hpp",
		"main/**.hxx",
		"main/**.inl"
	}

	kind "ConsoleApp"
	location "main"

	files( sources )

	dependson "main-shaders"
	dependson "x-rapidobj"

	links "vmlib"
	links "support"

	links "x-stb"
	links "x-glad"
	links "x-glfw"
	links "x-fontstash"

project "main-shaders"
	local shaders = { 
		"assets/cw2/*.vert",
		"assets/cw2/*.frag",
		"assets/cw2/*.geom",
		"assets/cw2/*.tesc",
		"assets/cw2/*.tese",
		"assets/cw2/*.comp"
	}

	kind "Utility"
	location "assets/cw2"

	files( shaders )

project "vmlib-test"
	local sources = { 
		"vmlib-test/**.cpp",
		"vmlib-test/**.hpp",
		"vmlib-test/**.hxx",
		"vmlib-test/**.inl"
	}

	kind "ConsoleApp"
	location "vmlib-test"

	files( sources )

	links "vmlib"

	links "x-catch2"

project "support"
	local sources = { 
		"support/**.cpp",
		"support/**.hpp",
	}

	kind "StaticLib"
	location "support"

	files( sources )

	filter "*"

project "vmlib"
	local sources = { 
		"vmlib/**.cpp",
		"vmlib/**.hpp",
		"vmlib/**.hxx",
		"vmlib/**.inl"
	}

	kind "StaticLib"
	location "vmlib"

	files( sources )


--EOF
