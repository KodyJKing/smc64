# See https://powershell.one/tricks/filesystem/filesystemwatcher

param(
    [string]$watchPath,
    [string]$command,
    [string]$extensions = ".cpp, .hpp, .c, .h, .lua, .ps1, .bat, .rc",
    [string]$ignorePaths = "bin, obj, .git"
)

$global:process = $null
$global:needsRestart = $false
$global:extensions = $extensions -split "," | ForEach-Object { $_.Trim() }
$global:ignorePath = $ignorePaths -split "," | ForEach-Object { $_.Trim() }

function Run-Command {
    $global:process = Start-Process pwsh -NoNewWindow -PassThru -ArgumentList "-Command", $command
}

function Kill-Tree {
    Param([int]$ppid)
    Get-CimInstance Win32_Process | Where-Object { $_.ParentProcessId -eq $ppid } | ForEach-Object { Kill-Tree $_.ProcessId }
    Stop-Process -Id $ppid -Force
}

function On-Exit {
    Write-Host "Unwatching file system." -ForegroundColor Blue
    $watcher.EnableRaisingEvents = $false
    $handlers | ForEach-Object { Unregister-Event -SourceIdentifier $_.Name }
    $handlers | Remove-Job
    $watcher.Dispose()

    if ($global:process) {
        Write-Host "Cleaning up process." -ForegroundColor Blue
        Kill-Tree $global:process.Id
    } else {
        Write-Host "No process to clean up." -ForegroundColor Blue
    }
}

try {
    Write-Host "Watching $watchPath for changes" -ForegroundColor Green

    Run-Command

    $watcher = New-Object System.IO.FileSystemWatcher
    $watcher.Path = (Resolve-Path $watchPath).Path
    $watcher.IncludeSubdirectories = $true
    $watcher.EnableRaisingEvents = $true

    $action = {
        if ($needsRestart) { return }
        $path = $Event.SourceEventArgs.FullPath

        $matchesExtension = $false
        foreach ($ext in $global:extensions) {
            if ($path.EndsWith($ext)) {
                $matchesExtension = $true
                break
            }
        }
        if (-not $matchesExtension) { return }

        $matchesIgnore = $false
        foreach ($ignore in $ignorePath -split ",") {
            if ($path.StartsWith($ignore)) {
                $matchesIgnore = $true
                break
            }
        }
        if ($matchesIgnore) { return }

        $changeType = $Event.SourceEventArgs.ChangeType
        $changeType = $changeType.ToString().ToLower()
        Write-Host ""
        Write-Host "File $path $changeType." -ForegroundColor Yellow
        Write-Host ""
        $global:needsRestart = $true
    }

    $handlers = . {
        Register-ObjectEvent -InputObject $watcher -EventName "Created" -Action $action
        Register-ObjectEvent -InputObject $watcher -EventName "Changed" -Action $action
        Register-ObjectEvent -InputObject $watcher -EventName "Deleted" -Action $action
        Register-ObjectEvent -InputObject $watcher -EventName "Renamed" -Action $action
    }

    do {
        Start-Sleep -Milliseconds 20

        # Press R to manually restart the process
        if ([Console]::KeyAvailable) {
            $key = [Console]::ReadKey($true)
            if ($key.Key -eq "R") {
                $global:needsRestart = $true
            }
        }

        if ($global:needsRestart) {
            $global:needsRestart = $false
            if ($global:process) {
                Kill-Tree $global:process.Id
            }
            Run-Command
        }

    } while ($true)

} finally {
    On-Exit
}
