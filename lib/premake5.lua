project "lib"
	kind "StaticLib"
	language "C"
	files { "main.h", "main.c" }

	filter "system:macosx"
		includedirs {
			"/usr/local/include",
			"/opt/homebrew/opt/glfw/include"
		}

	filter "system:windows"
		includedirs {
			"C:\\VulkanSDK\\1.3.243.0\\Include",
			"C:\\glfw-3.3.8.bin.WIN64\\include"
		}
