param (
    [Parameter(Mandatory=$false)]
    [string]$Synth = "neon-jr",

    [Parameter(Mandatory=$false)]
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Release",

    [Parameter(Mandatory=$false)]
    [string]$AndroidNdkPath,

    [Parameter(Mandatory=$false)]
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

# Detect NDK path
if (-not $AndroidNdkPath) {
    if ($env:ANDROID_NDK_ROOT) {
        $AndroidNdkPath = $env:ANDROID_NDK_ROOT
    } elseif ($env:ANDROID_NDK_HOME) {
        $AndroidNdkPath = $env:ANDROID_NDK_HOME
    } elseif ($env:NDK_HOME) {
        $AndroidNdkPath = $env:NDK_HOME
    } else {
        Write-Error "Android NDK path not found. Please set ANDROID_NDK_ROOT or provide -AndroidNdkPath"
    }
}

if (-not (Test-Path $AndroidNdkPath)) {
    Write-Error "Android NDK path does not exist: $AndroidNdkPath"
}

Write-Host "Using Android NDK: $AndroidNdkPath" -ForegroundColor Green

$RootDir = $PSScriptRoot
$SynthDir = Join-Path $RootDir $Synth
$BuildDir = Join-Path $RootDir "build\$Synth-android"
$ArtifactsDir = Join-Path $RootDir "artifacts\$Synth\Android"

if (-not (Test-Path $SynthDir)) {
    Write-Error "Synth directory not found at $SynthDir"
}

# 1. Clean if requested
if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host "Cleaning build directory: $BuildDir" -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}

# 2. Configure for Android
Write-Host "--- Configuring $Synth for Android ($Config) ---" -ForegroundColor Cyan

$AndroidToolchainFile = Join-Path $AndroidNdkPath "build/cmake/android.toolchain.cmake"

if (-not (Test-Path $AndroidToolchainFile)) {
    Write-Error "Android toolchain file not found: $AndroidToolchainFile"
}

Write-Host "Toolchain file: $AndroidToolchainFile" -ForegroundColor Green

# Create AndroidStudio project first, then we'll use Gradle to build
# Use Unix-style paths for cross-platform compatibility
$ToolchainUnix = $AndroidToolchainFile -replace '\\', '/'

cmake -S $SynthDir -B $BuildDir `
    -DCMAKE_BUILD_TYPE=$Config `
    "-DCMAKE_TOOLCHAIN_FILE=$ToolchainUnix" `
    -DANDROID_ABI=arm64-v8a `
    -DANDROID_PLATFORM=android-24 `
    -DANDROID_STL=c++_shared

if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configuration failed"
}

# 3. Build Android Studio project
Write-Host "--- Building Android Studio project ---" -ForegroundColor Cyan

# Detect project name from CMakeLists.txt (e.g. NeonJr, NeonFM)
$ProjectName = (Get-Content (Join-Path $SynthDir "CMakeLists.txt") | Select-String "project\(([^ ]+)" | ForEach-Object { $_.Matches.Groups[1].Value })
if (-not $ProjectName) { $ProjectName = $Synth }
$ArtefactsDirName = "${ProjectName}_artefacts"
Write-Host "Looking for artefacts directory: $ArtefactsDirName" -ForegroundColor Green

# Find the generated Android Studio project
$AndroidBuildDir = Get-ChildItem -Path $BuildDir -Recurse -Directory -Filter $ArtefactsDirName | Select-Object -First 1

if (-not $AndroidBuildDir) {
    Write-Warning "Android Studio artifacts not found directly. Building with cmake..."
    cmake --build $BuildDir --config $Config
    $AndroidBuildDir = Get-ChildItem -Path $BuildDir -Recurse -Directory -Filter $ArtefactsDirName | Select-Object -First 1
}

if ($AndroidBuildDir) {
    $AndroidProjectDir = Join-Path $AndroidBuildDir.FullName "AndroidStudio"
    
    if (Test-Path $AndroidProjectDir) {
        Write-Host "Found Android Studio project at: $AndroidProjectDir" -ForegroundColor Green
        
        # Build using Gradle wrapper if available
        $GradleWrapper = Join-Path $AndroidProjectDir "gradlew"
        if ($IsWindows) {
            $GradleWrapper = "$GradleWrapper.bat"
        }
        
        if (Test-Path $GradleWrapper) {
            Write-Host "Building APK with Gradle..." -ForegroundColor Cyan
            Push-Location $AndroidProjectDir
            try {
                if ($IsWindows) {
                    & cmd /c "$GradleWrapper assembleRelease"
                } else {
                    & bash -c "chmod +x gradlew && ./gradlew assembleRelease"
                }
            } finally {
                Pop-Location
            }
        }
    }
}

# 4. Collect Artifacts
Write-Host "--- Collecting Android Artifacts to $ArtifactsDir ---" -ForegroundColor Cyan

if (-not (Test-Path $ArtifactsDir)) {
    New-Item -ItemType Directory -Path $ArtifactsDir -Force | Out-Null
}

# Find all APK files
$ApkFiles = Get-ChildItem -Path $BuildDir -Recurse -Filter "*.apk" -File

if ($ApkFiles) {
    foreach ($apk in $ApkFiles) {
        Write-Host "Found APK: $($apk.Name)" -ForegroundColor Green
        Copy-Item $apk.FullName -Destination $ArtifactsDir -Force
    }
} else {
    Write-Warning "No APK files found in build directory"
}

# Copy wavetables
$WavesSource = Join-Path $SynthDir "waves-1"
if (Test-Path $WavesSource) {
    $WavesDest = Join-Path $ArtifactsDir "waves-1"
    Copy-Item -Path $WavesSource -Destination $WavesDest -Recurse -Force
    Write-Host "Copied wavetables to artifacts" -ForegroundColor Green
}

Write-Host "--- Android Build Complete ---" -ForegroundColor Green
Write-Host "Artifacts location: $ArtifactsDir" -ForegroundColor Cyan
