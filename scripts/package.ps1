param(
    [string]$Config = "Debug",
    [string]$IDE = "vs2022",
    [string]$MCCPath = "C:\Program Files (x86)\Steam\steamapps\common\Halo The Master Chief Collection"
)

# /README.md
# /halo1/maps/{a10,a30,a50,b30,b40,c10,c20,c40,d20,d40}.map  ; copied from $MCCPath\Halo1\maps
# /MCC/Binaries/Win64/
#                    /mods/smc64.dll
#                    /spark.dll
#                    /sm64.dll
#                    /xaudio2_9redist.dll   (import-patched by setdll)
#                    /xaudio2_9redist.dll~  (original backup left by setdll)

# Build the project.
& "./scripts/build.ps1" -Config $Config -IDE $IDE

# Create fresh package directory.
$PackagePath = "bin\$Config-Win64\package"
if (Test-Path $PackagePath) { Remove-Item -Path $PackagePath -Recurse -Force }
New-Item -Path $PackagePath -ItemType Directory -Force

# Root-level README.
Copy-Item -Path smc64\shipfiles\README.md -Destination $PackagePath\README.md -Force

# Campaign maps.
$MapsPackagePath = "$PackagePath\halo1\maps"
New-Item -Path $MapsPackagePath -ItemType Directory -Force | Out-Null
@("a10","a30","a50","b30","b40","c10","c20","c40","d20","d40") | ForEach-Object {
    Copy-Item -Path "$MCCPath\Halo1\maps\$_.map" -Destination $MapsPackagePath -Force
}

# MCC/Binaries/Win64 layout.
$Win64Path = "$PackagePath\MCC\Binaries\Win64"
New-Item -Path $Win64Path -ItemType Directory -Force | Out-Null

Copy-Item -Path vendor\spark\bin\$Config-Win64\spark\spark.dll -Destination $Win64Path\spark.dll -Force
Copy-Item -Path vendor\libsm64\dist\sm64.dll -Destination $Win64Path\sm64.dll -Force
Copy-Item -Path "$MCCPath\MCC\Binaries\Win64\xaudio2_9redist.dll" -Destination $Win64Path\xaudio2_9redist.dll -Force

# Stage mod DLLs into mods/ so Spark's ModLoader picks them up on the end user's machine.
$ModsPackagePath = "$Win64Path\mods"
New-Item -Path $ModsPackagePath -ItemType Directory -Force | Out-Null
Copy-Item -Path vendor\spark\spark\shipfiles\mods\* -Destination $ModsPackagePath -Recurse -Force
Copy-Item -Path bin\$Config-Win64\smc64\smc64.dll -Destination $ModsPackagePath\smc64.dll -Force

# Add spark.dll to xaudio2_9redist.dll's import table (must run from Win64Path to keep the
# import record as a bare filename rather than an absolute path).
Push-Location $Win64Path
& "..\..\..\..\..\..\vendor\spark\vendor\Detours\bin.X64\setdll.exe" "-d:spark.dll" "xaudio2_9redist.dll"
Pop-Location

# Zip the package directory.
$ConfigLower = $Config.ToLower()
$ZipPath = "bin\$Config-Win64\smc64-$ConfigLower.zip"
Compress-Archive -Path $PackagePath\* -DestinationPath $ZipPath -Force

# Cleanup package directory.
Remove-Item -Path $PackagePath -Recurse -Force
