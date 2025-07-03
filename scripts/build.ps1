param(
    [string]$Config = "Debug",
    [string]$IDE = "vs2022"
)

if ($IDE -ne "vs2022") {
    Write-Host "Warning: IDE $IDE is not standard. May not work as expected."
}

& "./scripts/kill_injected_instances.ps1"
# & "./scripts/vendor/premake/premake5.exe" $IDE
& "premake5.exe" $IDE
& "MSBuild.exe" "smc64.sln" "/t:Build" "/p:Configuration=$Config" "/p:Platform=Win64"
