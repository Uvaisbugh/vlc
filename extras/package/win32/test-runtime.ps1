param(
    [Parameter(Mandatory = $true)][string]$RuntimeDirectory,
    [Parameter(Mandatory = $true)][string]$EvidenceDirectory
)

$ErrorActionPreference = 'Stop'
$vlcRuntimePath = (Resolve-Path -LiteralPath $RuntimeDirectory).Path
$vlcExecutable = Join-Path $vlcRuntimePath 'vlc.exe'
if (-not (Test-Path -LiteralPath $vlcExecutable)) { throw 'Packaged vlc.exe is missing.' }
New-Item -ItemType Directory -Path $EvidenceDirectory -Force | Out-Null
$vlcEvidencePath = (Resolve-Path -LiteralPath $EvidenceDirectory).Path
$vlcFixture = Join-Path $vlcEvidencePath 'silence.wav'

# Generate an eight-second PCM fixture locally. No media download or device is needed.
$vlcSampleRate = 8000
$vlcData = [byte[]]::new($vlcSampleRate * 2 * 8)
$vlcWriter = [System.IO.BinaryWriter]::new([System.IO.File]::Create($vlcFixture))
try {
    $vlcWriter.Write([System.Text.Encoding]::ASCII.GetBytes('RIFF'))
    $vlcWriter.Write([uint32](36 + $vlcData.Length))
    $vlcWriter.Write([System.Text.Encoding]::ASCII.GetBytes('WAVEfmt '))
    $vlcWriter.Write([uint32]16)
    $vlcWriter.Write([uint16]1)
    $vlcWriter.Write([uint16]1)
    $vlcWriter.Write([uint32]$vlcSampleRate)
    $vlcWriter.Write([uint32]($vlcSampleRate * 2))
    $vlcWriter.Write([uint16]2)
    $vlcWriter.Write([uint16]16)
    $vlcWriter.Write([System.Text.Encoding]::ASCII.GetBytes('data'))
    $vlcWriter.Write([uint32]$vlcData.Length)
    $vlcWriter.Write($vlcData)
} finally {
    $vlcWriter.Dispose()
}

foreach ($vlcInterface in @('dummy', 'qt')) {
    $vlcLogPath = Join-Path $vlcEvidencePath "$vlcInterface.log"
    $vlcOutputPath = Join-Path $vlcEvidencePath "$vlcInterface.stdout.txt"
    $vlcErrorPath = Join-Path $vlcEvidencePath "$vlcInterface.stderr.txt"
    $vlcArguments = @(
        '--ignore-config', '--no-one-instance', '--no-media-library', '--no-metadata-network-access',
        '--no-qt-privacy-ask', '--no-qt-updates-notif', '--no-qt-error-dialogs',
        '--intf', $vlcInterface, '--aout', 'dummy', '--play-and-exit',
        '--verbose', '2', '--file-logging', '--logfile', ('"' + $vlcLogPath + '"'),
        ('"' + $vlcFixture + '"')
    )
    $vlcProcess = Start-Process -FilePath $vlcExecutable -WorkingDirectory $vlcRuntimePath `
        -ArgumentList $vlcArguments -PassThru -WindowStyle Hidden `
        -RedirectStandardOutput $vlcOutputPath -RedirectStandardError $vlcErrorPath
    try {
        if (-not $vlcProcess.WaitForExit(60000)) {
            $vlcProcess.Kill($true)
            throw "$vlcInterface interface did not exit after the playback fixture."
        }
        if ($vlcProcess.ExitCode -ne 0) {
            throw "$vlcInterface interface exited with code $($vlcProcess.ExitCode)."
        }
    } finally {
        $vlcProcess.Dispose()
    }
    if (-not (Test-Path -LiteralPath $vlcLogPath)) { throw "$vlcInterface did not produce a runtime log." }
    $vlcLog = Get-Content -LiteralPath $vlcLogPath -Raw
    if ($vlcLog -notmatch 'EOF reached') { throw "$vlcInterface did not reach the end of the audio fixture." }
    if ($vlcInterface -eq 'qt' -and $vlcLog -notmatch 'using interface module "qt"') {
        throw 'The native Qt interface was not loaded.'
    }
    $vlcAllLogs = $vlcLog + (Get-Content -LiteralPath $vlcErrorPath -Raw)
    if ($vlcAllLogs -match 'failed to load component|Component is not ready|no suitable interface module') {
        throw "$vlcInterface could not initialize the application interface."
    }
    $vlcUiErrors = $vlcAllLogs -split "`n" | Where-Object {
        $_ -match 'PlayButton.qml|VLCStyle.qml|Redesign[^\s:]*\.qml|qrc:/redesign/' -and
        $_ -match 'ReferenceError|TypeError|Cannot open|is not a type|Unable to assign'
    }
    if ($vlcUiErrors) { throw "Redesign QML/resource errors: $($vlcUiErrors -join ' | ')" }
    Write-Output "${vlcInterface}: native startup and PCM playback passed."
}

'Native startup, Qt loading, and PCM playback passed. Visual and interaction QA are separate.' |
    Set-Content -LiteralPath (Join-Path $vlcEvidencePath 'result.txt')
