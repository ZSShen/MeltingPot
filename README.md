##BinaryCluster-YaraGenerator
+ Continuous Integration (Travis-CI)  
[![Build Status](https://travis-ci.org/ZSShen/BinaryCluster-YaraGenerator.svg?branch=master)](https://travis-ci.org/ZSShen/BinaryCluster-YaraGenerator)

###Abstract  
+ Input - A homogeneous file set with the same format type.  
+ Objective - Similarity clustering for the files.  
+ Output - A pattern set each of which represents a file cluster.  
<img src="https://raw.githubusercontent.com/ZSShen/BinaryCluster-YaraGenerator/master/res/picture/EngineIntro.png" width="650px" height="550px" />  

###Introduction  
This engine correlates files by examining the similar byte sequences shared among them.  
To summarize the result, the engine produces a set of YARA formatted patterns each of  
which represents the common features extracted from a certain file group. Essentially,  
such patterns can be directly applied by YARA engine.
 
