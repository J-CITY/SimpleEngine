# SimpleEngine
This is my sandbox project for learning computer graphics.

What's already there:

- Forward rendering with deferred rendering
- Support for custom shaders (grass, water, volumetric clouds)
- SSAO
- hdr, gamma correction
- Blinn Phong or PBD lightning
- Shadow map
- Bloom
- Motion blur
- God rays
- Skeletal animation
- FXAA


Also support irrKlang 3d sound

## Supported Platforms & Graphics APIs

| Platform | Graphics APIs | Details / Build Flags |
| :--- | :--- | :--- |
| **Windows** | OpenGL, Vulkan, DirectX 12 | Multi-API binary (`-g opengl vulkan dx12`) |
| **Linux** | OpenGL, Vulkan | `-g opengl vulkan` |
| **macOS** | Metal, Vulkan (via MoltenVK), OpenGL | Multi-API binary (`-g opengl vulkan metal`), Xcode / Make |
| **iOS** | Metal, OpenGL ES | `-g opengl metal`, Xcode project |
| **Android** | OpenGL ES (3.0, 3.1, 3.2) | SDL3 NativeActivity, configured via `--esVer` |
| **Meta Quest** | OpenGL ES (3.0, 3.1, 3.2) | OpenXR NativeActivity + GLES |
| **Web (Emscripten)** | WebGL 2.0 (OpenGL ES 3.0) | WebAssembly build (`-p web`) |

The active backend can be selected at startup via `--render-backend=<backend>` argument or configured in `/Configs/render.json`. For more details, see [render-backends.md](docs/render-backends.md).

Some screenshots

![Screenshot](https://github.com/J-CITY/SimpleEngine/blob/ecs/screens/editor0.png)

![Screenshot](https://github.com/J-CITY/SimpleEngine/blob/master/screens/0.gif)

![Screenshot](https://github.com/J-CITY/SimpleEngine/blob/master/screens/1.gif)

![Screenshot](https://github.com/J-CITY/SimpleEngine/blob/master/screens/2.gif)

![Screenshot](https://github.com/J-CITY/SimpleEngine/blob/master/screens/3.gif)
