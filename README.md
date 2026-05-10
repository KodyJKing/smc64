# SMC64

Super Master Chief 64

## Installation

Download the latest [release](https://github.com/KodyJKing/smc64/releases) and follow the instructions in `SMC64_REAMDE.md`.

## Developer Setup

Clone the repository recursively to get submodules:

```powershell
git clone --recursive
``` 

Install [Visual Studio 2022](https://visualstudio.microsoft.com/) and add MSBuild to your PATH. Location may vary. For me, it's located under `C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin`.

## Building and Running

```powershell
scripts/build.ps1
scripts/run_launcher.ps1
```

Halo MCC will need to be running when you run `run_launcher.ps1`.

## Workflow

When developing, run `scripts/watch_launcher.ps1` to build/run in watch mode. When you save a source file, this script will uninject, rebuild, and reinject the mod DLL.

If you're using Cheat Engine as part of your workflow, turn off symbols in the debug build configuration (`smc64/premake5.lua`). Otherwise, Cheat Engine will hold on to the PDB file and prevent rebuilding.

## Packaging

If you want to package the mod for distribution, you will need to build MSDetours' `setdll.exe` first. 

Make sure that `nmake` is on your path. For me, it is located under `C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\<latest>\bin\Hostx64\x64`.

Then run

```powershell
.scripts/build_setdll.bat
```

To compile and generate the package, run
```powershell
./scripts/package.ps1 -Config Release
```
The generated `.zip` file will be under `bin/Release-Win64`.

## Scripts

All scripts assume you are running from the root of the repository.

### `build.ps1`

Builds mod once.

Arguments:

- `-Config` Configuration to build. Values are `Debug` and `Release`. Default is `Debug`.
- `-IDE` IDE premake will generate project files for. Default is `vs2022`.

### `run_launcher.ps1`

Injects mod into game once. MCC must already be running.

Arguments:

- `-Config` Configuration to run. Values are `Debug` and `Release`. Default is `Debug`.

- `-Arguments` Arguments to pass to the launcher executable. Optional.

### `watch_build.ps1`

Builds mod and recompiles on file change. 

Press `R` in the terminal to rebuild without waiting for a file change.

Arguments:

- Inherits from `build.ps1`.

### `watch_launcher.ps1`

Builds mod and runs launcher. Uninjects mod from game, recompiles and reruns launcher on file change. 

Press `R` in the terminal to rebuild without waiting for a file change.

Arguments:

- Inherits from `watch_build.ps1` and `run_launcher.ps1`.

### `package.ps1`

Packages mod into a `.zip` file. Creates a copy of xaudio2_9redist.dll that imports the mod.

You must manually run `build_setdll.bat` before running this script.

Arguments:

- Inherits from `build.ps1`.
- `-MCCPath` Path to the root of the MCC installation. This is where the xaudio dll is pulled from. Default is `C:\Program Files (x86)\Steam\steamapps\common\Halo The Master Chief Collection`.

### `install_package.ps1`

Runs `package.ps1` and installs the mod into the game. This script is for testing, not for end users.

Arguments:

- Inherits from `package.ps1`.
- `-MCCPath` Path to the root of the MCC installation. Default is `C:\Program Files (x86)\Steam\steamapps\common\Halo The Master Chief Collection`.
- `-Uninstall` Uninstalls the mod instead of installing it.

### `build_setdll.bat`

Builds `setdll.exe` (from MS Detours). This is only needed when packaging the mod for distribution.
