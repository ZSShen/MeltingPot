##BinaryCluster-YaraGenerator
+ Continuous Integration (Travis-CI)  
[![Build Status](https://travis-ci.org/ZSShen/BinaryCluster-YaraGenerator.svg?branch=master)](https://travis-ci.org/ZSShen/BinaryCluster-YaraGenerator)

###Abstract  
+ Input - A homogeneous file set with the same format type.  
+ Objective - Similarity clustering for the files.  
+ Output - A pattern set each of which represents a file cluster.  
<img src="https://raw.githubusercontent.com/ZSShen/BinaryCluster-YaraGenerator/master/res/picture/EngineIntro.png" width="650px" height="550px" />  

###Introduction  
This tool correlates files by examining the similar byte sequences shared among them. To summarize the result, the engine produces a set of [YARA] formatted patterns each of which represents the common features extracted from a certain file group. Essentially, such patterns can be directly applied by YARA scan engine.

In general, the tool is composed of the main engine and the supporting plugins. Their interaction is briefly illustrated here:   
+ The engine first loads the user specified configuration to determine the workflow.  
+ It then applies the `file slicing plugin` to divide each input file into smaller pieces of biniries each of which is called slice.  
+ It then correlates the slices by examining their binary similarity with the help of `similarity comparison plugin`.  
+ At this stage, the engine acquire the slice groups. It then extracts a set of byte sequences from each group. Such sequences are small (only tens of bytes) and should contain less dummy information.  
+ Finally, the engine formats the sequences with `pattern formation plugin` to output the patterns which can be used to scan the corresponding file type.   


###Installation  
####***Basic***
First of all, we need to prepare the following utilities:
- [CMake] - A cross platform build system.
- [Valgrind] - An instrumentation framework help for memory debug.
- [SSDeep] - A fuzzy hash generation and comparison library.
- [GLib] - A large set of libraries to handle common data structures.
- [libconfig] - A library to process structured configuration file.

For Ubuntu 12.04 and above, it should be easy:
``` sh
$ sudo apt-get install -qq cmake
$ sudo apt-get install -qq valgrind
$ sudo apt-get install -qq libfuzzy-dev
$ sudo apt-get install -qq libglib2.0-dev
$ sudo apt-get install -qq libconfig-dev
```
Now we can build the entire source tree under the project root folder:
``` sh
$ ./clean.py --rebuild
$ cd build
$ cmake ..
$ make
```
Then the engine should be under:  
- `/engine/bin/release/cluster`  

And the relevant plugins should be under:
- `/plugin/slice/lib/release/libslc_*.so`
- `/plugin/similarity/lib/release/libsim_*.so`
- `/plugin/format/lib/release/libfmt_*.so`

####***Advanced***
If we change the functionalities of main engine or plugins, we can move to the corresponding subtree to rebuild the binary.   
To build the engine independently:
``` sh
$ cd engine
$ ./clean.py --rebuild
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Debug|Release
$ make
```
 Note that we have two build types. For debug build, the relevant debug flags are turned on for compiler, and the binary should locate at `/engine/bin/debug/cluster`. For release build, the optimized binary should locate at `/engine/bin/release/cluster`.

To build the plugin independently:   
``` sh
$ cd plugin/slice
$ ./clean.py --rebuild
$ cd build
$ cmake .. --DCMAKE_BUILD_TYPE=Debug|Release
$ make
```
Again, we must specify the build type for compiliation. Upon finishing, the corresponding binary should locate at `/plugin/slice/debug/libslc_*.so` or `/plugin/slice/release/libslc_*.so`. Also note that I just use the file slicing plugin to illustrate the approach. The other two plugins can be built with the same way.


###Usage

[YARA]:http://plusvic.github.io/yara/
[CMake]:http://www.cmake.org/
[Valgrind]:http://valgrind.org/
[SSDeep]:http://ssdeep.sourceforge.net/
[GLib]:https://developer.gnome.org/glib/
[libconfig]:http://www.hyperrealm.com/libconfig/

