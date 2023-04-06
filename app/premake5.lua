project "app"
	kind "ConsoleApp"
	language "C"
	files { "main.c" }
	links { "lib" }

	includedirs { "../lib" }
	links { "lib" }

	filter "system:macosx"
		libdirs {
			"/usr/local/lib",
			"/opt/homebrew/opt/glfw/lib"
		}

		links { "vulkan", "glfw" }

	filter "system:windows"
		libdirs {
			"C:\\VulkanSDK\\1.3.243.0\\Lib",
			"C:\\glfw-3.3.8.bin.WIN64\\lib-vc2022"
		}

		links { "vulkan-1", "glfw3" }
