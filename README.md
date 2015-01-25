##BinaryCluster-YaraGenerator
+ Continuous Integration (Travis-CI)  
[![Build Status](https://travis-ci.org/ZSShen/BinaryCluster-YaraGenerator.svg?branch=master)](https://travis-ci.org/ZSShen/BinaryCluster-YaraGenerator)

###Abstract  
+ Input - A set of files with the same format type.  
+ Objective - Similarity clustering for the file slices and common byte sequence extraction for the slice clusters.    
+ Output - A set of patterns each of which describes the attributes of the common sequences of a slice cluster.  
<img src="https://raw.githubusercontent.com/ZSShen/BinaryCluster-YaraGenerator/master/res/picture/Engine Intro.png" width="520px" height="440px"/>  

###Introduction  
In this project, we term the `slice` as an arbitrary binary segment of a file with a designated size. This tool correlates such file slices by examining the similar byte sequences shared among them. To summarize the result, the tool produces a set of [YARA] formatted patterns each of which represents the common byte sequences extracted from a certain file cluster. Essentially, such patterns can be directly applied by YARA scan engine.

In general, the tool is composed of the main engine and the supporting plugins. The relation is briefly illustrated here:   
+ The engine first loads the user specified configuration.  
+ It then applies the `file slicing plugin` to slice input files.
+ It then correlates the slices by examining their similarity with the help of `similarity comparison plugin`.  
+ At this stage, the engine acquires the slice clusters. It then extracts common byte sequences from each cluster . Such sequences are small (only tens of bytes) and should contain less dummy information.  
+ Finally, the engine formats the sequences with `pattern formation plugin` to output the patterns.  

<img src="https://raw.githubusercontent.com/ZSShen/BinaryCluster-YaraGenerator/master/res/picture/Pattern.png" width="450px" height="500px"/> 

As mentioned above, we have three kinds of plugins:  
+ File slicing - Slicing an input file via specific format parsing. (E.g. PE, DEX)  
+ Similarity comparison - Measuring the similarity for a pair of slices. (E.g. ssdeep, ngram)  
+ Pattern formation - Producing YARA pattern with external file format module if necessary.   

If developers intend for additional supports , they can patch new plugins in the plugin source directory and use the build system introduced below to come out the libraries.  


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
- `./engine/bin/release/cluster`  

And the relevant plugins should be under:
- `./plugin/slice/lib/release/libslc_*.so`
- `./plugin/similarity/lib/release/libsim_*.so`
- `./plugin/format/lib/release/libfmt_*.so`

####***Advanced***
If we patch the main engine or the plugins, we can move to the corresponding subtree to rebuild the binary.   
To build the engine independently:
``` sh
$ cd engine
$ ./clean.py --rebuild
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Debug|Release
$ make
```
Note that we have two build types.  
For debug build, the compiler debug flags are on, and the binary locates at `./engine/bin/debug/cluster`.  
For release build, the optimized binary locates at `./engine/bin/release/cluster`.  

To build the plugin independently (using File Slicing plugin as example):   
``` sh
$ cd plugin/slice
$ ./clean.py --rebuild
$ cd build
$ cmake .. --DCMAKE_BUILD_TYPE=Debug|Release
$ make
```
Again, we must specify the build type for compiliation.  
For debug build, the binary locates at `./plugin/slice/debug/libslc_*.so`.  
For release build, the binary locates at `./plugin/slice/release/libslc_*.so`.  
For the other two kinds of plugins, the build rule is the same.  


###Usage
To run the engine, we should first specify some relevant configurations.  
The example is shown in `./engine/cluster.conf`.  
<img src="https://raw.githubusercontent.com/ZSShen/BinaryCluster-YaraGenerator/master/res/picture/Configuration.png" width="450px" height="370px"/> 

And we discuss these parameters below:  

| Parameter     | Description           |
| ------------- | ------------- |
| `SIZE_SLICE` | The size of file slice which is the basic unit of clustering process |
| `SIZE_HEX_BLOCK` | The length of distilled common byte sequence extracted from a slice group |
| `COUNT_HEX_BLOCK` | The number of to be extracted sequences per each group |
| `THRESHOLD_SIMILARITY` | The threshold to group similar slices |
| `RATIO_NOISE` | The ratio of dummy bytes (0x00 or 0xff) in a hex block (extracted sequence) |
| `RATIO_WILDCARD` | The ratio of wildcard characters in a hex block |
| `TRUNCATE_GROUP_SIZE_LESS_THAN` | The threshold to truncate trivial slice groups |
| `FLAG_COMMENT` | The control flag for detailed clustering comment to be shown in pattern |
| `PATH_ROOT_INPUT` | The pathname of input sample set |
| `PATH_ROOT_OUTPUT` | The pathname of output pattern folder |
| `PATH_PLUGIN_SLICE` | The pathname of the file slicing plugin |
| `PATH_PLUGIN_SIMILARITY` | The pathname of similarity comparison plugin |
| `PATH_PLUGIN_FORMAT` | The pathname of pattern formation plugin |

In addition, we have the following advanced parameters:  

| Parameter     | Description           |
| ------------- | ------------- |
| `COUNT_THREAD` | The number of running threads |
| `IO_BANDWIDTH` | The maximum number of files a thread can simultaneously open |

With the configuration file prepared, we can run the engine like:  
`./engine/bin/release/cluster --conf ./engine/cluster.conf` or  
`valgrind ./engine/bin/debug/cluster --conf ./engine/cluster.conf`  
Of course, the usage depends on our objective.  
Also note that if we apply valgrind for memory debugging, there will be a "still-reachable" alert in the summary report. This is due to the side effect provided by GLib. Essentially, the entire project source is memory safe :-).  


[YARA]:http://plusvic.github.io/yara/
[CMake]:http://www.cmake.org/
[Valgrind]:http://valgrind.org/
[SSDeep]:http://ssdeep.sourceforge.net/
[GLib]:https://developer.gnome.org/glib/
[libconfig]:http://www.hyperrealm.com/libconfig/

