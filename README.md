# Oskar Niesen & Alex Young

### Building Locally

In order to build the game engine locally you need to install vcpkg and the Ninja build system.

To install vcpkg:
1. Run `git clone https://github.com/microsoft/vcpkg.git` to download the vcpkg project in the directory that you want.
2. Run `vcpkg/bootstrap-vcpkg.sh -disableMetrics` to set up vcpkg
3. Add VCPKG_ROOT to your environment variables with the directory to vcpkg.
   To do this run `export VCPKG_ROOT=<path/to/vcpkg>`

**NOTE**: If you are using brew on macOS and encountering errors at build-time, try installing vcpkg manually.

To install Ninja:
* Apt: `sudo apt install ninja-build`
* Brew: `brew install ninja`

After vcpkg and Ninja are set up run the following commands to download the repository.

1. `git clone git@github.com:tuvus/GameEngine.git`
2. `cd GameEngine`
3. `make run`

The game should now run and the window should pop up.

To list all commands simply run `make` at the root directory
