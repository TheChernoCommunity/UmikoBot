if _ACTION:match("vs*") then
	require "pmk/extensions/premake-qt/qt"
elseif _ACTION == "qmake" then
	require "pmk/extensions/premake-qmake/qmake"
end

local qtdir_x86 = io.readfile("tmp/.qtdir_x86")
local qtdir_x64 = io.readfile("tmp/.qtdir_x64")

workspace "UmikoBot"
	location "sln/"
	configurations {
		"Debug",
		"Release",
	}
	platforms {
		"x86",
		"x64",
	}

project "UmikoBot"
	location "sln/prj/"
	kind "ConsoleApp"
	cppdialect "C++11"
	files {
		"src/**.cpp",
		"src/**.h",
		"src/**.qrc",
		"src/**.ui",
	}
	flags {
		"MultiProcessorCompile",
	}
	includedirs {
		"src/",
		"dep/QDiscord/src/core/",
	}
	links {
		"QDiscordCore",
	}
	qtmodules {
		"core",
		"gui",
		"network",
		"websockets",
		"widgets",
	}

	filter {"configurations:Release"}
		optimize "Full"
		defines {
			"QT_NO_DEBUG",
		}

	filter {"platforms:x86"}
		debugdir "res/"
		objdir "obj/x86/"
		targetdir "bin/x86/"

	filter {"platforms:x64"}
		debugdir "res/"
		objdir "obj/x64/"
		targetdir "bin/x64/"

	filter {"toolset:msc"}
		disablewarnings { "C4996" }

	filter {}

	-- Enable premake-qt when targeting Visual Studio
	if _ACTION:match("vs*") then
		premake.extensions.qt.enable()
		qtprefix "Qt5"
		qtgenerateddir "src/GeneratedFiles/"

		filter {"configurations:Debug"}
			qtsuffix "d"

		filter {"platforms:x86"}
			qtpath (qtdir_x86)

		filter {"platforms:x64"}
			qtpath (qtdir_x64)
	end

group "QDiscord"

project "QDiscordCore"
	location "sln/prj/"
	kind "StaticLib"
	cppdialect "C++11"
	files {
		"dep/QDiscord/src/core/**.h",
		"dep/QDiscord/src/core/**.cpp",
	}
	flags {
		"MultiProcessorCompile",
	}
	includedirs {
		"dep/QDiscord/src/core/",
	}

	filter {"configurations:Release"}
		optimize "Full"
		defines {
			"QT_NO_DEBUG",
		}

	filter {"platforms:x86"}
		objdir "obj/x86/"
		targetdir "bin/x86/"

	filter {"platforms:x64"}
		objdir "obj/x64/"
		targetdir "bin/x64/"

	filter {}

	-- Enable premake-qt when targeting Visual Studio
	if _ACTION:match("vs*") then
		premake.extensions.qt.enable()
		qtprefix "Qt5"
		qtgenerateddir "dep/QDiscord/src/core/GeneratedFiles/"

		filter {"configurations:Debug"}
			qtsuffix "d"

		filter {"platforms:x86"}
			qtpath (qtdir_x86)

		filter {"platforms:x64"}
			qtpath (qtdir_x64)
	end
