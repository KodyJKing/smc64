param(
    [string]$Config = "Debug",
    [string]$Arguments = ""
)

$ArgumentList = $Arguments -split " "

& "./vendor/spark/bin/$Config-Win64/spark-launcher/spark-launcher.exe" $ArgumentList | Out-Default
