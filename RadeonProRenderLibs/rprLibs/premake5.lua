project "RprLibs"
	location "../build"
	kind "SharedLib"
	defines { "RPRLIB_DLL" }
	defines { "OPENVDB_STATICLIB" }
	includedirs { ".", "../contrib/RadeonProRenderSDK/RadeonProRender/inc" }
	libdirs { "../contrib/RadeonProRenderSDK/RadeonProRender/libWin64/" }
	filter "action:vs2019 or vs2017 or vs2015"
		includedirs { "../contrib/OpenVDB/include/" }
		libdirs { "../contrib/OpenVDB/libWin64/vs2017/" }
		files { "../contrib/OpenVDB/vs2017/include/openvdb/**.h" }		
		links { "libopenvdb.lib", "Half-2_3.lib", "libblosc.lib", "lz4.lib", "snappy.lib", "zlib.lib", "zstd_static.lib" }
	filter {}

	links{ "RadeonProRender64.lib" }

	files { "../contrib/RadeonProRenderSDK/RadeonProRender/inc/**.h" }
	files{ "./**.cpp" }
	files{ "./**.h" }
