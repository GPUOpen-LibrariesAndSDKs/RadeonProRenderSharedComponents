project "unittest"
	location "../build"
	kind "ConsoleApp"

	includedirs { ".","../contrib/include", "../contrib/RadeonProRenderSDK/RadeonProRender/inc", "../contrib/RadeonProRenderSDK/tutorials/" }
	libdirs { "../contrib/RadeonProRenderSDK/RadeonProRender/libWin64/" }
	filter { "configurations:Debug" }
		libdirs { "../dist/debug/bin/" }
	filter {}
	filter { "configurations:Release" }
		libdirs { "../dist/release/bin/" }
	filter {}
	links { "RadeonProRender64.lib", "RprLoadStore64.lib", "ProRenderGLTF.lib", "RprLibs64.lib" }
	defines{ "GTEST_HAS_TR1_TUPLE=0" }

	files{ "../contrib/src/gtest-1.6.0/**" }
	files{ "./**.cpp" }
