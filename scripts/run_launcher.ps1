param(
    [string]$Config = "Debug",
    [string]$Arguments = ""
)

$ArgumentList = $Arguments -split " "

& "./bin/$Config-Win64/smc64-launcher/smc64-launcher.exe" $ArgumentList | Out-Default
