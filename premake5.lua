workspace "papirus"
	configurations { "debug", "release" }
	architecture "x64"
	startproject "papirus"

	BASE_DIR = path.getabsolute(".")
	targetdir (BASE_DIR .. "/bin")
	objdir (BASE_DIR .. "/bin/int")

	filter "configurations:debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:release"
		defines { "NDEBUG" }
		optimize "On"

project "papirus"
	kind "ConsoleApp"
	language "C"
	files { "src/main.c" }

	filter "system:windows"
		includedirs {
			"C:\\VulkanSDK\\1.3.243.0\\Include",
			"C:\\glfw-3.3.8.bin.WIN64\\include"
		}

		libdirs {
			"C:\\VulkanSDK\\1.3.243.0\\Lib",
			"C:\\glfw-3.3.8.bin.WIN64\\lib-vc2022"
		}

		links { "vulkan-1", "glfw3" }