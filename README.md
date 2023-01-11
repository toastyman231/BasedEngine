# BasedEngine
A simple 2D game engine based on tutorials by https://www.youtube.com/c/Progrematic. Some inspiration was also taken from Hazel Engine by Yan Chernikov.

## Libraries Used
- FreeType (Text Rendering, https://freetype.org/)
- GLAD (OpenGL Rendering, https://glad.dav1d.de/)
- SDL2 (Window Context, https://www.libsdl.org/)
- spdlog (Error Logging, https://github.com/gabime/spdlog)
- EnTT (Entity-Component System, https://github.com/skypjack/entt)
- GLM (OpenGL Math, https://github.com/g-truc/glm)
- ImGUI (GUI Rendering, https://github.com/ocornut/imgui/tree/docking)
- STB (Image Loading, https://github.com/nothings/stb)

## My Additions
The bulk of the engine design and implentation was done by Progrematic, if something is not listed below, it was probably done by him. You can find his original engine here: https://github.com/progrematic/hippo.

- Built in library of assets (Shaders, Materials, etc.)
- Automatic rendering (Any entity with a Renderer component will be rendered automatically)
- Scene System (Currently just determines which entities get rendered or not)
- Entity-Component System
- Text Rendering/UI (WIP)

## Planned Additions
- 2D Physics with Box2D
- Audio capabilities (maybe ffmpeg?)
- Animation System
- Load shaders from file (They are hardcoded currently)
- More UI (Buttons, Sliders, Checkboxes, etc)
- Scene Save/Load (Serialization)

## Thoughts and Concerns
- Currently I have EnTT fully exposed to the user, but it might be better to hide some of it behind a custom Entity class. 
- Sprites can currently only be rotated on one axis, rotation on other axes seems to be quite math intensive, so I'm putting it off for later.
- Entities are rendered automatically as long as they have a Renderer component. Currently this is handled in the Scene code, but it may be worth it to separate all rendering stuff to it's own class.
- Currently only quads can be rendered, things like lines and circles have to be baked into a texture then rendered on a square or something. Adding dedicated render commands for those types of objects is probably worth it.
- The build system is a bit complicated, everything is kind of just all shoved into one giant premake file. If I add more libraries that need to be linked, but I don't add the include paths to all existing projects, the ones that should work will not compile. A way to separate projects so they only care about themselves and the engine would be nice. I might be able to include them the same way I include GLAD's project.
- A GUI editor would be nice to have at some point, but it's a lot of effort to make that I could be using to make actual games (or just improving the engine in general).
- I've only tested it on Windows, theoretically it should work on Mac and Linux, but your mileage will probably vary.

## Contributing
Feel free to contribute if you feel like it, this is my first large C++ project, so the codebase probably sucks (the parts made by me anyway). Any feedback is much appreciated.

### Building the project
Just download the source code, open a terminal in the root directory, and run 'python3 based.py gensln' to generate the Visual Studio project files. You will need Python 3 installed on your machine first. If you have Windows Subsystem for Linux installed, you can run based.bat, then enter 'based gensln' directly into the prompt that opens. You still need Python 3 for this option.
The following commands can be run by replacing 'gensln' in the commands above with the respective command name from this list:
- gensln: Generates project files. Should work on any OS, but I've only tested it on Windows.
- buildsln [config=debug|release]: Builds the entire solution in either debug or release configuration.
- mklink [prj=Your Project Name]: Creates a symlink of the resources folder so that the engine has easier access to it. THIS IS MANDATORY IF YOU ARE USING THE RESOURCES FOLDER!!!!! This command may be a bit buggy, I only used it once.
- run [config=debug|release] [prj=Your Project Name]: Runs an existing project in debug or release mode. Defaults to debug.
- version: Prints the current tools version. Pretty useless, it's mostly just for testing.
