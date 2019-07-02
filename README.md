# rs4

## How to build and run

1. Clone git repository and update submodules

    * all operating systems:

      ```bash
      git clone https://github.com/dp304/rs4.git
      cd rs4
      git submodule update --init --recursive
      ```

1. Install C++17 compatible compiler and CMake >=3.10.2 (and libtool)

    * Debian/Ubuntu:

      ```bash
      sudo apt-get update
      # g++ >=7 should work
      # libtool is possibly needed for building dependencies
      sudo apt-get install g++ cmake libtool
      ```

    * Windows:

      * Install [Visual Studio 2019 Community](https://visualstudio.microsoft.com/vs/)

        * MinGW g++ (>=7) might also work instead of VS

      * Install the [latest stable CMake](https://cmake.org/download/)

        * Select "Add CMake to the system PATH" (for current user or all users) at installation

1. Install Conan

    * Installing with pip is recommended

      * Debian/Ubuntu: get Python 3 and pip

        ```bash
        sudo apt-get install python3-pip
        ```

      * Windows: install the [latest Python](https://www.python.org/downloads/), which comes with pip

        * Select "Add Python 3.x to PATH" at installation

    * all operating systems: install Conan with pip

      ```bash
      # optionally create and activate a venv:
      # python3 -m venv ./venv
      # . ./venv/bin/activate
      pip3 install conan
      ```

1. Set up Conan

    * all operating systems:

      ```bash
      # generate default profile:
      conan profile new default --detect
      # only if using gcc: set libcxx to C++11 ABI:
      conan profile update settings.compiler.libcxx=libstdc++11 default
      # add bincrafters repository:
      conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
      ```

1. Install dependencies

    * all operating systems:

      ```bash
      mkdir build
      cd build
      conan install .. --build=missing
      ```

1. Build

    * all operating systems:

      ```bash
      cmake ..
      cmake --build .
      ```

1. Copy data directory (not included) to directory of binary

    * Linux:

      ```bash
      cd src/bin
      cp -rp <path_to_data_dir>/data .
      ```

    * Windows:

      ```cmd
      cd src\bin
      xcopy <path_to_data_dir>\data .\data /I /E /K /H
      ```

1. Run

    * Linux:

      ```bash
      ./rs4game
      ```

    * Windows:

      ```cmd
      rs4game.exe
      ```

1. Have fun!

## Dependencies

* [SDL2](https://www.libsdl.org)
* [OpenAL Soft](https://www.openal.org/)
* [GLEW](https://github.com/nigels-com/glew)
* [GLM](https://github.com/g-truc/glm)
* [Vorbis](https://xiph.org/vorbis/)
* [Nuklear](https://github.com/vurtun/nuklear) (no Conan package)
* [stb](https://github.com/nothings/stb)
* [EnTT](https://github.com/skypjack/entt)
