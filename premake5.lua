require "extensions/premake-qt/qt"

local qtdir_x86 = io.readfile("tmp/.qtdir_x86")
local qtdir_x64 = io.readfile("tmp/.qtdir_x64")

workspace "CharnoBot"
	location "sln/"
	configurations {
		"Debug",
		"Release",
	}
	platforms {
		"x86",
		"x64",
	}

project "CharnoBot"
	premake.extensions.qt.enable()
	location "sln/prj/"
	kind "ConsoleApp"
	qtgenerateddir "../demos/statemachine/GeneratedFiles/"
	qtprefix "Qt5"
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
		"../core/",
		"../demos/statemachine/",
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

	filter {"configurations:Debug"}
		qtsuffix "d"

	filter {"configurations:Release"}
		optimize "Full"
		defines {
			"QT_NO_DEBUG",
		}

	filter {"platforms:x86"}
		debugdir "bin/x86/"
		objdir "bin/x86/obj/"
		qtpath (qtdir_x86)
		targetdir "bin/x86/"

	filter {"platforms:x64"}
		debugdir "bin/x64/"
		objdir "bin/x64/obj/"
		qtpath (qtdir_x64)
		targetdir ("bin/x64/")
