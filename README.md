# 3D City Editor

A 3D editor to manipulate cityscapes. Built during a university project.

## Usage

There is no *open project* dialog implemented yet, so the project path has to be specified via a CLI argument.
```
./3d <path to directory or model>
```
If a path to a directory is provided, the containing project is loaded, or a new project is created there. Without an argument, the current working directory is used instead.
In case the path points to a model file, a new project in the working directory is created and the given file is loaded.

## Building

### NixOS or MacOS with nix

The experimental features `flakes` and `nix-command` must be enabled.

```sh
nix build
```

### Other Linux or MacOS

Requires
- cmake
- ninja
- clang/gcc (tested with clang 18)

```sh
cmake -B build -G Ninja -D CMAKE_BUILD_TYPE=Release
cmake --build build
```

The executable can be found in `build/3d`.

### Windows

Requires
- cmake
- Visual Studio 2022

```powershell
cmake -B build -G 'Visual Studio 17 2022'
cmake --build build --config Release
```

The executable can be found in `build\Release\3d.exe`.

## Developing

### Linux and MacOS

Install the dependencies listed above or, if using nix, enter a development shell using

```sh
nix develop
```

Then build a debug build with the following commands:

```sh
cmake -B build -G Ninja -D CMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Windows

Getting ASAN to work on Windows is a little trickier.
The following PowerShell script might work:

```powershell
cmake -B build -G 'Visual Studio 17 2022'
cmake --build build --config Debug

$MSVC_HOME = "D:\Programme\VisualStudio\VC\Tools\MSVC"

$version_dirs = Get-ChildItem -Directory -Path "${MSVC_HOME}"
$version = $version_dirs[0]
$env:Path += "${MSVC_HOME}\${version}\bin\Hostx64\x64"

.\build\Debug\3d.exe
```
