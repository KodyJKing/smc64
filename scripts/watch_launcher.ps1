param(
    [string]$Config = "Debug",
    [string]$IDE = "vs2022",
    [string]$Arguments = ""
)

& "./scripts/watch.ps1" "." "& ./scripts/build.ps1 $Config $IDE; & ./scripts/run_launcher.ps1 $Config '$Arguments'"
