# Vessel Tools

Scripts and utility programs for extracting and processing placental vasculature


# Authors

 * Maria A Zuluaga
 * Tom Doel


# Build instructions for the C++ components

The C++ components of the code should be built using CMake.

## Requirements

 *	Cmake 3 or higher
 *	ITK 4.8

## Building

Use CMake to configure the project. Make a build directory in a different location to your source code and run the following, or else use `cmake-gui`.

```
    mkdir vessel-tools-build
    cd vessel-tools-build
    ccmake <path-to-vessel-tools>
```

Then press `c` to configure. If you see errors, you may need to set variables (such as for your ITK location).
Once the configuration is complete, you can generate the build files by pressing 'g'.
Then you can build the project using `make`:

```
    make -j
```


# Installing the python ImageSplit package

The python package ImageSplit is used in some of the bash scripts. If you need it, the easiest way is to install using `pip` (assuming you have Python installed).
Note: if you are using the system default python (especially on MacOS), it is recommended that you do not modify the system installation. Instead, either install ImageSplit locally, or install an alternative version of Python, or use `virtualenv` to create your own python virtual environment.


# Copyright and licensing

At present this code is not released as open source. Therefore it should not be shared outside of UCL and collaborators.

Copyright UCL 2017
