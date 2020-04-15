workspace "RadeonProRenderLibs"
	configurations { "Debug", "Release" }
	language "C++"
	cppdialect "C++11"
	platforms "x64"
	architecture "x86_64"


	filter { "platforms:x64", "configurations:Debug" }
		targetsuffix "64D"
		targetdir "./dist/debug/bin/"
		defines { "_DEBUG" }
		if os.ishost( "windows" ) then
			symbols "Full"
		else
			symbols "On"
		end
		optimize "Off"
	filter { "platforms:x64", "configurations:Release" }
		targetsuffix "64"
		targetdir "./dist/release/bin/"
		defines { "NDEBUG" }
		symbols "On"
		optimize "Speed"
	filter {}

	filter "platforms:x64"
		defines {"_X64"}
	filter {}
	
	os.mkdir( "build" )
	os.copyfile("contrib/RadeonProRenderSDK/RadeonProRender/binWin64/RadeonProRender64.dll","build/RadeonProRender64.dll")
	os.copyfile("contrib/RadeonProRenderSDK/RadeonProRender/binWin64/RprLoadStore64.dll","build/RprLoadStore64.dll")
	os.copyfile("contrib/RadeonProRenderSDK/RadeonProRender/binWin64/Tahoe64.dll","build/Tahoe64.dll")

	include "rprLibs" 
	include "unittest" 

