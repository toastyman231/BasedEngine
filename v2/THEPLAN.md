# THE PLAN

## The Goal
v1 currently renders the Sponza scene with 1 point light and 1 directional light at ~100 FPS on my RX 7600 XT. It should be way higher. My goal is to use the same demo scene, but get at least 400 FPS. It should be capable of fairly realistic rendering alongside any gameplay logic at a consistent 60 FPS.

## Language
The engine will be written in C++20. Concepts are something I want to play around with, and modules should probably be avoided since compiler support is spotty (and the lack of headers would mess up my header tool). The goal is to be compilable on any C++ compiler, but my priority will be clang, since I am using libclang for the header tool, and MSVC since I'm currently on Windows.

In order to be more portable ALL external libraries will be compiled from source. No precompiled binaries here!

## Targets
The engine will primarily target Windows (Vulkan), Linux (Vulkan), and Web (WebGPU). Support for WebGPU Native might come alongside regular Web support. Support for DX12 can wait for later. 

The project will have several configurations:  
DebugEditor - Optimizations off, debug systems on, export as a DLL for Editor interfacing  
DebugGame - Optimizations off, debug systems on, export as a static library for simplicity  
DevelopmentEditor - Optimizations on, debug systems on, export as a DLL for Editor interfacing  
DevelopmentGame - Optimizations on, debug systems on, export as a static library for simplicity  
ReleaseEditor - Optimizations on, debug systems off, export as a DLL for Editor interfacing  
ReleaseGame - Optimizations on, debug systems off, export as a static library for simplicity  

## Memory
No more malloc/new! We will create memory pools to partition memory into groups based on function and CPU/GPU visibility, and override the global new/delete to use them. Some pools will not be useable with new, and will instead call their own allocator functions, like a scratch graphics pool that's actually just an arena that gets cleared each frame. mimalloc seems like a good library to back this. 

The Pools I'll Need:  
Persistent CPU  
Persistent GPU  
Tex CPU  
Tex GPU  
Small Tex GPU  
Scratch CPU  
Scratch GPU  

Allocations can be tagged for easier debugging. An RAII AllocatorContext class will be used to switch the active memory pool. I may add a separate pool for streaming assets later.

## ENTT Systems
There will be a System interface that defines an Execute function (and whatever else I end up needing). Systems will be registered with their dependencies and a scheduler will figure out what can run concurrently and send them off on separate threads. Systems can be added to a Scene in order to be automatically scheduled/removed when the Scene is active. This is to support scheduling Systems from the Editor, as they need a lifetime to be tied to. entt::flow can help with the dependency resolution, and I can do the rest.

## ENTT Entities
Important entity data will no longer be stored in an Entity class that needs to live somewhere. Entity data will be purely within components, and an Entity interface will provide quick access to these fields.

## Multithreading
The engine will be highly multithreaded. Jobs can be submitted to a queue, that the worker threads will pull from to execute. Asset loading will have sync and async versions, but the multithreaded approach will be preferred wherever possible.

## Shaders
All shaders will be written in Slang (may need a translation macro layer to support PSSL if I ever add consoles), and I will re-use the shader compiler tool from v1. However, shaders will be precompiled to a binary format based on platform (spirv for Vulkan, dxir for DX12, not WebGPU unfortunately) to reduce runtime compilation.

A Shader Manager will handle shader compilation (when needed) and loading, and will store file hashes so it can periodically check for updates and hot reload shaders.

## Materials
A material is a shader, and a set of uniform parameter values to be used with that shader. All material params will be stored in CPU visible memory on the GPU, and the shader will be synced to their values before being used. Syncs will be incremental wherever possible, to reduce CPU times.

## Textures
All textures will be converted to ktx2 in a prebuild step, same as before. However, they may be combined into one or multiple asset files depending on the project settings. No other image format will be considered valid. The bake step will also output a 16x16 or 8x8 (not sure which yet) small texture that the loader will load synchronously, and use while the full texture is being loaded. These will be persistently loaded in the Small Tex GPU pool, so that they can be swapped in for the main texture if it's ever evicted from memory but still being used somewhere. 

## Assets
Assets can be output directly to the working directory, or packaged into one or more asset files. A virtual filesystem will ensure that the assets can be referenced by their canonical path regardless of where they're stored. I will probably use vfspp, but write my own FileSystem implementation to load a custom format instead of zips.

Assets will usually be referred to by a Handle (see below) which is managed by the Resource Manager. All asset loading will be kicked off from the Resource Manager, but it may or may not own the resulting resource.

All assets will be tracked in premake through a None buildaction, so that updating one triggers the post build command to copy them to the working directory.

## Infrequent use of Shared Pointers
Shared pointers will be discouraged wherever possible. Things like resources and assets will be passed around by a Resource Handle that indexes into a lookup table that points to the actual object. A raw pointer can be gotten from this handle if needed, otherwise the -> operator will forward operations to the inner type. In this way a Resource Handle will ALWAYS be valid, but the thing it points to may not be. Unique pointers are still allowed.

The Handle will be the hash of the resource name (provided at creation time or at export time) so that resources can be looked up by name. Shader resources are not tracked here, that gets messy.

## No constructors wherever possible
Prefer factories as much as possible, especially since we're focusing on Handles and a constructor won't return that.

## Reflection and Serialization
There will be a header tool that parses the headers for markers like BCLASS, BSTRUCT, BENUM, etc and generates reflection information for that type. It will generate entt reflection registration calls, placed inside a function called RegisterType\<T\>. All functions generated this way will then be placed inside a function called RegisterUserTypes, which is defined in the App class. This function will always be generated even if nothing is reflected, since a definition is still required. The engine will call this function at startup so that the Editor (or the user's app) can inspect it's types. There will be an option to only output reflection data for the Editor. 

There will also be a set of variable attributes that can be added to variables, such as for controlling slider min/max points or speed. The header tool will emit this information into a separate function. 

Serialization will be handled by reflect-cpp, and the header tool will handle generating the needed serialization data for custom classes. Serialization data is always output since it's needed for saving/loading and is a compile time operation anyway. The header tool will hash the header's text and save a list of hashes, and will not output any new code if the hashes match for a given file. The header tool will be written with libclang to effectively parse the code.

Type IDs will be tracked as indices into a lookup table based on the hash of their name, so that IDs stay static across compilations.

## Rendering
When creating world objects, like Meshes, they will be placed into something like an octree or BVH. When it comes time to render the world, all the drawables info and the world partition will be passed to a compute shader to handle culling and LOD selection, and a final buffer of visible drawables will be output. Then the GPU will handle the draw calls via indirect rendering.

Bindless is also something I want to try prioritizing, but not every platform supports it fully, and I'm not sure how painful it would be to make it super general for lots of different types of drawables.

## The Editor
The Editor will launch into a project management screen by default, so you can pick a project to load or create. A command line arg providing the project path will still boot the editor straight into the project. The Editor will also feature a UI Editor that outputs RML and RCSS.

The Editor will be a separately compiled executable. I will compile the game's interface to a DLL the editor can use and hot reload, but it will not call the normal App::Initialize, App::Update, etc stuff. Instead it will run it's own initialization to pull in whatever assets the Scene references, and use the generated reflection data to find the Systems the Scene references, and it will just run those systems when in Play mode. 

## Tracy
Tracy will be set up to track as much as possible, including GPU times, thread operations, memory usage, etc.

## Scenes
Scenes will be built with subscenes in mind from the beginning. The Editor will feature tooling to support this. 

## Meshes
There will be a Mesh interface, and I will have Basic Meshes and Skeletal Meshes that extend it (as opposed to Mesh vs Model in the old engine, which was confusing and lacked a consistent API).

## Movies
I want to support playing movie files, like Bink does, but without paying for Bink. I think ffmpeg can decode Bink 1, so I might go for that (depending on the legality), or I might just have it handle mp4s directly or something. It should also be usable in the UI, which will probably require some finagling to get RML to use it nicely.

## App
The App class will now have a function to handle any command line arguments unhandled by the engine. It will also define the RegisterUserTypes function that gets generated by the header tool.

## Timeline
* 0.1 - Core Engine  
    * [ ] Entry Point, Windowing, Platform Abstractions
    * [ ] Logging, Engine Loop, PCH
    * [ ] Beginning Render API, Hello Triangle \<-- Ideally both Vulkan and WebGPU should be working here
    * [ ] Memory Management, Resource Handles
    * [ ] Virtual Filesystem (no need for custom asset files yet, just want the API in place)
    * [ ] Asset Loading (and tooling)
    * [ ] Profiler