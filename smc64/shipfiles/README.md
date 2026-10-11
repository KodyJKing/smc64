## Install

Paste the contents of this archive under `...\Halo The Master Chief Collection`. Allow file replacement if prompted.

If you don't have the latest maps, download them from the [latest release](https://github.com/KodyJKing/smc64/releases). Paste the contents of the maps archive into `...\Halo The Master Chief Collection`. Back up your map files first if you don't want to have to redownload them.

Place a SM64 rom under `...\Halo The Master Chief Collection\MCC\Binaries\Win64` as `baserom.us.z64`

## Uninstall

Under `...\Halo The Master Chief Collection\MCC\Binaries\Win64`,

- Delete `spark.dll`
- Delete `xaudio2_9redist.dll`
- Rename `xaudio2_9redist.dll~` to `xaudio2_9redist.dll`.
- Optionally delete /mods directory.

Under `...\Halo The Master Chief Collection\halo1\maps`,

- Restore your backed up map files 
- OR redownload them with Steam via `Properties > Installed Files > Verify Integrity of Game Files`

## Usage

Start Halo MCC *without* Easy Anti-Cheat. Spark loads every `*.dll` in `mods\` (plus any
directories listed in the `SPARK_MODS_PATH` environment variable) on startup.
