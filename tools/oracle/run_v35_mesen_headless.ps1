param(
    [Parameter(Mandatory = $true)] [string] $Mesen,
    [Parameter(Mandatory = $true)] [string] $Rom,
    [Parameter(Mandatory = $true)] [string] $Lua,
    [Parameter(Mandatory = $true)] [string] $Output
)

$ErrorActionPreference = 'Stop'
$expectedRom = 'DE2D5A952096C5F50368B9270D342AA6E7A39007FFBEC27117E182E30EF4CF32'
$expectedMesen = '8EF403D6B9AF32075416193914D7993CE8480B347FE1F20A0CFD9815374933E7'

$mesenPath = (Resolve-Path -LiteralPath $Mesen).Path
$romPath = (Resolve-Path -LiteralPath $Rom).Path
$luaPath = (Resolve-Path -LiteralPath $Lua).Path
if ((Get-FileHash -Algorithm SHA256 -LiteralPath $mesenPath).Hash -ne $expectedMesen) {
    throw 'Mesen executable SHA-256 does not match the pinned V35 oracle.'
}
if ((Get-FileHash -Algorithm SHA256 -LiteralPath $romPath).Hash -ne $expectedRom) {
    throw 'ROM SHA-256 does not match Civilization (USA).'
}

$outputPath = [IO.Path]::GetFullPath($Output)
$stderrPath = $outputPath + '.stderr.txt'
$process = Start-Process -FilePath $mesenPath -ArgumentList @(
    '--testRunner', '--enableStdout', '--timeout=30',
    ('"' + $luaPath + '"'), ('"' + $romPath + '"')
) -WindowStyle Hidden -Wait -PassThru -RedirectStandardOutput $outputPath -RedirectStandardError $stderrPath
$lines = @()
if (Test-Path -LiteralPath $outputPath) { $lines += Get-Content -LiteralPath $outputPath }
if (Test-Path -LiteralPath $stderrPath) { $lines += Get-Content -LiteralPath $stderrPath }
$exitCode = $process.ExitCode
$lines | Set-Content -LiteralPath $outputPath -Encoding utf8
$summary = $lines | Where-Object { $_ -match '^V35_HEADLESS_ORACLE reason=' } | Select-Object -Last 1
if ($exitCode -ne 0) { throw "Mesen headless oracle failed with exit code $exitCode." }
if (-not $summary) { throw 'Mesen did not emit a V35 headless-oracle summary.' }
$summary
