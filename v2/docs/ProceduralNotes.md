# Procedural Notes

The goal with this file is to keep an ongoing log of my thoughts and to chronicle the engine's development, in case I need to come up with cool stuff to say in an interview or on my portfolio (I'm happily employed at the time of writing, but you never know). Oldest notes are at the bottom.

### [6/19/2026]
I'm planning to start implementing the lowest level systems: memory management, window context, etc. My plan was always to use a third party high speed allocator, since I highly doubt I can do better than Microsoft or Google, and wrap it in my own memory pools and logic. So the choices I settled on were:
- mimalloc
- tcmalloc
- rpmalloc

I ruled out tcmalloc pretty quick, because it seems designed to handle extremely large multi-core cloud stuff, which I don't care about, and also seems to have a bigger overhead. rpmalloc is supposedly better for games because of how light and fast it is, but it's support for allocating from existing arenas isn't as good, and mimalloc handles multithreading better it seems. You can just blast out calls to new on any thread and mimalloc will just handle it, but rpmalloc requires a separate heap for each thread, or thread locks spewed everywhere. Even though rpmalloc seems really nice for my purposes, I'm going to go with mimalloc for simplicity's sake. However, ultimately I think it probably doesn't matter which one I pick, any of them will be better than just calling raw C malloc (or new or whatever) like I used to.

Mimalloc ended up being a huge hassle to get compiling and linking. Different combinations of MI_SHARED_LIB and MI_SHARED_LIB_EXPORT on different projects would cause different issues, MSVC would work but clang-cl wouldn't, everything compiled and linked fine but it couldn't find a module for mimalloc-redirect, etc. In the end mimalloc-redirect needed to be linked by both mimalloc and the user project, and I needed a special linker pragma included in my main TU. Absolute nightmare but it works. I'm going to bundle minject and their other tools with the engine so people can check to make sure their exes are good. I also moved tracy's tools to the tools folder for consistency.

I also switched the VERSION.txt to VERSION.h so I can use it in C++ land without defining the version in two places. I updated the tool scripts accordingly.

### [6/14/2026]  
Set up the project structure today. It's so much cleaner! All external libs in one place, instead of split over two locations, and I still preserve the publicness of them!   
I also did some prep work for an eventual cleanup of the engine path resolution - instead of emitting an environment variable, I have a tool script that writes the path for the current version to a file in AppData. So my tools and the engine itself can read that to know what the engine path is, and I don't have to worry about different engine versions conflicting.   
I'm also compiling all my libraries from source, no precompiled binaries anywhere. Since everything I use can be statically linked, I don't have to even bother with PostBuildCopy anymore, although I'm going to keep the code for it in my tools just in case I introduce some dlls later.  
I was smart enough this time to put all the VS project files into an Intermediate directory, which makes all the project folders so much cleaner! No more clutter!  
Claude seems to be pretty damn good at translating CMakeLists to premake5.lua, decent use of AI if you can get past the resources data centers use.  
I put a text file listing the version of each external library into their folders, so I don't have to worry about forgetting which version I was using anymore.  
I added Platforms to my premake config, so now I can target multiple platforms! I'm also using this to drive switching between MSVC and Clang on Windows.