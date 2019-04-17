OpenGL Collecting Game
Based on P3Base provided by Dr. Wood in CPE-476 

Build instructions:
    Linux: 
        $ cp CMakeLists-linux.txt CMakeLists.txt
        Edit source files to add in GLM prefixes to 
        the include statements
            TODO: conditional includes to prevent needing
            this step
        $ mkdir build
        $ cd build
        $ cmake ..
        $ make
    Mac OSX:
        $ cp CMakeLists-mac.txt CMakeLists.txt
        $ mkdir build
        $ cd build
        $ cmake ..
        $ make
        run with argument ../../resources if it complains about not finding resources