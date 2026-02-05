param (
    [Parameter(Mandatory=$false)]
    [string]$Synth = "neon-jr",

    [Parameter(Mandatory=$false)]
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Debug",

    [Parameter(Mandatory=$false)]
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

$RootDir = $PSScriptRoot
$SynthDir = Join-Path $RootDir $Synth
$BuildDir = Join-Path $RootDir "build\$Synth"
$ArtifactsDir = Join-Path $RootDir "artifacts\$Synth"

if (-not (Test-Path $SynthDir)) {
    Write-Error "Synth directory $SynthDir not found at $SynthDir"
}

# 1. Clean if requested
if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host "Cleaning build directory: $BuildDir"
    Remove-Item -Recurse -Force $BuildDir
}

# 2. Configure
Write-Host "--- Configuring $Synth ($Config) ---" -ForegroundColor Cyan
cmake -S $SynthDir -B $BuildDir -G Ninja "-DCMAKE_BUILD_TYPE=$Config"

# 3. Build
Write-Host "--- Building $Synth ---" -ForegroundColor Cyan
cmake --build $BuildDir --config $Config

# 4. Collect Artifacts
Write-Host "--- Collecting Artifacts to $ArtifactsDir ---" -ForegroundColor Cyan
if (-not (Test-Path $ArtifactsDir)) {
    New-Item -ItemType Directory -Path $ArtifactsDir -Force | Out-Null
}

# Helper to find artifacts. JUCE puts them in <ProjectId>_artefacts/<Config>/
# We will look for .exe and .vst3
$ProjectName = (Get-Content (Join-Path $SynthDir "CMakeLists.txt") | Select-String "project\(([^ ]+)" | ForEach-Object { $_.Matches.Groups[1].Value })
if (-not $ProjectName) {
    # Fallback if regex fails, though usually projects are named same as folders
    $ProjectName = $Synth
}

$BinarySourceDir = Join-Path $BuildDir "$($ProjectName)_artefacts\$Config"

if (Test-Path $BinarySourceDir) {
    # Copy Standalone (.exe on Windows, .app on Mac)
    $StandaloneTargetPath = Join-Path $ArtifactsDir "Standalone"
    if (-not (Test-Path $StandaloneTargetPath)) {
        New-Item -ItemType Directory -Path $StandaloneTargetPath -Force | Out-Null
    }

    $Ext = if ($IsWindows) { "*.exe" } else { "*.app" }
    Get-ChildItem -Path $BinarySourceDir -Filter $Ext | Copy-Item -Destination $StandaloneTargetPath -Force -Recurse
    $StandaloneSubDir = Join-Path $BinarySourceDir "Standalone"
    if (Test-Path $StandaloneSubDir) {
        Get-ChildItem -Path $StandaloneSubDir -Filter $Ext | Copy-Item -Destination $StandaloneTargetPath -Force -Recurse
    }
    
    # Copy VST3 folder
    $Vst3Path = Join-Path $BinarySourceDir "VST3"
    if (Test-Path $Vst3Path) {
        Copy-Item -Path $Vst3Path -Destination $ArtifactsDir -Recurse -Force
    }

    # Copy Wavetables for distribution
    $WavesSrc = Join-Path $SynthDir "waves-1"
    if (Test-Path $WavesSrc) {
        Write-Host "Copying wavetables..."
        $WavesDest = Join-Path $ArtifactsDir "waves-1"
        if (-not (Test-Path $WavesDest)) {
            New-Item -ItemType Directory -Path $WavesDest -Force | Out-Null
        }
        Copy-Item -Path $WavesSrc -Destination $ArtifactsDir -Recurse -Force
    }
    
    Write-Host "Success! Artifacts available in: $ArtifactsDir" -ForegroundColor Green
} else {
    Write-Warning "Could not find artifact source directory: $BinarySourceDir"
}
