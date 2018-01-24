HotlineMiami WAD Extractor is a tool to extract all files from WAD file used by Hotline Miami 1 & 2.

Currently, it only allows to extract all files and directory in current directory.

Syntax :

extractor <wad file>

Requires :

- CMake
- C++ compiler
- boost

Compilation :

1. Clone Mercurial directory : http://hg.kervala.net/cmake/ to <path>
2. Create an environment variable CMAKE_MODULE_PATH pointing to <path>/modules
3. Launch CMake and generate a project for your compiler and specify boost paths
4. Compile the project
5. Enjoy
