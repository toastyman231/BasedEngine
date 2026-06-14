# Procedural Notes

The goal with this file is to keep an ongoing log of my thoughts and to chronicle the engine's development, in case I need to come up with cool stuff to say in an interview or on my portfolio (I'm happily employed at the time of writing, but you never know). Oldest notes are at the bottom.

### [6/14/2024]  
Set up the project structure today. It's so much cleaner! All external libs in one place, instead of split over two locations, and I still preserve the publicness of them!   
I also did some prep work for an eventual cleanup of the engine path resolution - instead of emitting an environment variable, I have a tool script that writes the path for the current version to a file in AppData. So my tools and the engine itself can read that to know what the engine path is, and I don't have to worry about different engine versions conflicting.   
I'm also compiling all my libraries from source, no precompiled binaries anywhere. Since everything I use can be statically linked, I don't have to even bother with PostBuildCopy anymore, although I'm going to keep the code for it in my tools just in case I introduce some dlls later.  
I was smart enough this time to put all the VS project files into an Intermediate directory, which makes all the project folders so much cleaner! No more clutter!  
Claude seems to be pretty damn good at translating CMakeLists to premake5.lua, decent use of AI if you can get past the resources data centers use.  
I put a text file listing the version of each external library into their folders, so I don't have to worry about forgetting which version I was using anymore.  
I added Platforms to my premake config, so now I can target multiple platforms! I'm also using this to drive switching between MSVC and Clang on Windows.