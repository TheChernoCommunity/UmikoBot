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
	qtgenerateddir "src/GeneratedFiles/"
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

	filter {"configurations:Debug"}
		qtsuffix "d"

	filter {"configurations:Release"}
		optimize "Full"
		defines {
			"QT_NO_DEBUG",
		}

	filter {"platforms:x86"}
		debugdir "res/"
		objdir "obj/x86/"
		qtpath (qtdir_x86)
		targetdir "bin/x86/"

	filter {"platforms:x64"}
		debugdir "res/"
		objdir "obj/x64/"
		qtpath (qtdir_x64)
		targetdir "bin/x64/"

group "QDiscord"

project "QDiscordCore"
	premake.extensions.qt.enable()
	location "sln/prj/"
	kind "StaticLib"
	qtgenerateddir "dep/QDiscord/src/core/GeneratedFiles/"
	qtprefix "Qt5"
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
		qtpath (qtdir_x86)
		targetdir "bin/x86/"

	filter {"platforms:x64"}
		objdir "obj/x64/"
		qtpath (qtdir_x64)
		targetdir "bin/x64/"
