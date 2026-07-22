param(
    [string]$Config = "Debug",
    [string]$MCCPath = ""
)

# Stages mod DLLs (currently just smc64.dll) into <MCC>/MCC/Binaries/Win64/mods/ so
# Spark's ModLoader (vendor/spark/spark/src/spark/mod/ModLoader.cpp) picks them up next launch.
# Mirrors the staging pattern used by build_libsm64.ps1 for sm64.dll.

$dest = if ($MCCPath) {
    "$MCCPath\MCC\Binaries\Win64"
} elseif ($env:MCC_HOME) {
    "$env:MCC_HOME\MCC\Binaries\Win64"
} else {
    "C:\Program Files (x86)\Steam\steamapps\common\Halo The Master Chief Collection\MCC\Binaries\Win64"
}

if (!(Test-Path $dest)) {
    Write-Warning "MCC directory not found at $dest; skipping mod copy. Pass -MCCPath or set `$env:MCC_HOME."
    exit 0
}

$modsDir = "$dest\mods"
New-Item -ItemType Directory -Force -Path $modsDir | Out-Null

$srcDll = "bin\$Config-Win64\smc64\smc64.dll"
if (Test-Path $srcDll) {
    Copy-Item -Force $srcDll "$modsDir\smc64.dll"
    $srcPdb = "bin\$Config-Win64\smc64\smc64.pdb"
    if (Test-Path $srcPdb) {
        Copy-Item -Force $srcPdb "$modsDir\smc64.pdb"
    }
} else {
    Write-Warning "$srcDll not found; build it first."
}
