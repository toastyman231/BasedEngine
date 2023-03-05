# BasedEngine
A simple 2D game engine based on tutorials by https://www.youtube.com/c/Progrematic. Some inspiration was also taken from Hazel Engine by Yan Chernikov.

## Libraries Used
- FreeType (Text Rendering, https://freetype.org/)
- GLAD (OpenGL Rendering, https://glad.dav1d.de/)
- SDL2 (Window Context and Text Rendering, https://www.libsdl.org/)
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
- Entity-Component System (Custom Entity class exposes entity functionality to the user)
- Custom window icons (set up automatically if you include an icon.png file in your res folder)
- Text Rendering/UI
- Toolchain to create projects in directories outside the engine's directory

## Planned Additions
- 2D Physics with Box2D
- Audio capabilities (maybe ffmpeg?)
- Animation System
- ~~Load shaders from file (They are hardcoded currently)~~ Complete!  
- More UI (Buttons, Sliders, Checkboxes, etc)
- Scene Save/Load (Serialization)
- 3D Rendering capability with PBR materials
- GUI to make creating/loading projects easier

## Thoughts and Concerns
- ~~Currently I have EnTT fully exposed to the user, but it might be better to hide some of it behind a custom Entity class.~~  
    - I have since added a custom Entity class to expose EnTT functionality to the user. Currently it pretty much maps 1:1 to EnTT, but now there's room for me to add my own logic if need be.
    - EnTT is still exposed to the user, as it may be useful to have access to those functionalities, but it is recommended to just use the Entity class.  
- ~~The current workflow of creating a PostBuildCopy folder with a res folder to hold resources, then creating a symlink of res so the engine can access it is very annoying. At some point I would like to create a script which automatically sets up a new project, which will contain an assets folder that the engine can refer to directly.~~  
    - I have added a mkproject script to create new projects from a template, and I have removed the requirement that the res folder (renamed to Assets) be inside the PostBuildCopy folder AND symlinked to the project root folder. Now all you need is an Assets folder in the project root and the engine will be able to find it just fine, and the postbuild script will copy it with no issues.  
- ~~Sprites can currently only be rotated on one axis, rotation on other axes seems to be quite math intensive, so I'm putting it off for later. This may also require a projection camera, since currently everything is displayed orthographically.~~  
    - Sprites can be rotated on any axis, rotations should be passed in degrees. However, X and Y rotations seem a bit broken. This may be because of the ortho camera, but I'm not entirely sure yet.
- ~~Entities are rendered automatically as long as they have a Renderer component. Currently this is handled in the Scene code, but it may be worth it to separate all rendering stuff to it's own class.~~
    - I have done this
- Currently only quads can be rendered, things like lines and circles have to be baked into a texture then rendered on a square or something. Adding dedicated render commands for those types of objects is probably worth it.
- ~~The build system is a bit complicated, everything is kind of just all shoved into one giant premake file. If I add more libraries that need to be linked, but I don't add the include paths to all existing projects, the ones that should work will not compile. A way to separate projects so they only care about themselves and the engine would be nice. I might be able to include them the same way I include GLAD's project.~~  
    - The mkproject command mentioned above takes care of this concern as well.
- A GUI editor would be nice to have at some point, but it's a lot of effort to make that I could be using to make actual games (or just improving the engine in general).
- I've only tested it on Windows, theoretically it should work on Mac and Linux, but your mileage will probably vary.

## Contributing
Feel free to contribute if you feel like it, this is my first large C++ project, so the codebase probably sucks (the parts made by me anyway). Any feedback is much appreciated.

### Building the project
Just download the source code, open a terminal in the root directory, and run 'python3 based.py gensln' to generate the Visual Studio project files. You will need Python 3 installed on your machine first. Some commands may require you to set up your compiler location.  
The following commands can be run with 'python3 based.py {command name} {arguments}'
- gensln: Generates project files. Should work on any OS, but I've only tested it on Windows.
- buildsln [-config=debug|release]: Builds the entire solution in either debug or release configuration.
- mkproject [-prj=New Project Name] [-config=Template Name] [-location=Project Location]: Will create a new project from a template at the specified directory. Templates can be found in BasedEngine/Templates.
- **(DEPRECATED)** mklink [-prj=Your Project Name]: Creates a symlink of the resources folder so that the engine has easier access to it. The engine can now find the Assets folder without needing a symlink, so this command no longer serves a purpose.
- run [-config=debug|release] [-prj=Your Project Name]: Runs an existing project in debug or release mode. Defaults to debug.
- version: Prints the current tools version. Pretty useless, it's mostly just for testing.
