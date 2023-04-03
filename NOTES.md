# papirus client


## steps for vulkan

the initial setup is too f-ing big, but here it is.

- select the best physical device
- create a logical device handle with the device selected above
- create a surface and double buffered swapchain using the device created above
- create the image view and frame buffers for the aformentioned swapchain
- create a render pass to clear to screen first, than draw the appropriately selected framebuffer
- create a graphics pipeline and reference our newly created render pass to it
- create a CommandPool and ask for a CommandBuffer then send these commands:
    - start this render pass
    - use this graphics pipeline
    - draw 3 vertices
    - end this render pass

then in the main loop:

- we aquire a framebuffer from the swapchain
- submit the CommandBuffer to the graphics queue
- wait for the drawing operation to finish
- send the framebuffer back to the swapchain

this part is directly stolen from [vulkan-tutorial.com](vulkan-tutorial.com).

- Create a VkInstance
- Select a supported graphics card (VkPhysicalDevice)
- Create a VkDevice and VkQueue for drawing and presentation
- Create a window, window surface and swap chain
- Wrap the swap chain images into VkImageView
- Create a render pass that specifies the render targets and usage
- Create framebuffers for the render pass
- Set up the graphics pipeline
- Allocate and record a command buffer with the draw commands for every possible swap chain image
- Draw frames by acquiring images, submitting the right draw command buffer and returning the images back to the swap chain
