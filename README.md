OpenGL Collecting Game
Based on P3Base provided by Dr. Wood in CPE-476 

Build instructions:

    Linux: 

        Perform the following step if it does not build with the default
        CMakelists.txt (CMakeLists-Mac.txt)

        $ cp CMakeLists-linux.txt CMakeLists.txt

        $ mkdir build

        $ cd build

        $ cmake ..

        $ make [-jN]

    Mac OSX:

        Perform the following step if, for some reason, the CMakeLists.txt file
        has been replaced by the linux version. 

        $ cp CMakeLists-Mac.txt CMakeLists.txt

        $ mkdir build

        $ cd build

        $ cmake ..

        $ make [-jN]

    run with argument ../../resources if it complains about not finding resources

    Windows:

        ¯\_(ツ)_/¯