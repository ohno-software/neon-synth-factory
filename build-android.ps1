param (
    [Parameter(Mandatory=$false)]
    [string]$Synth = "neon-jr",

    [Parameter(Mandatory=$false)]
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Release",

    [Parameter(Mandatory=$false)]
    [string]$AndroidNdkPath,

    [Parameter(Mandatory=$false)]
    [int]$AndroidApiLevel = 29,

    [Parameter(Mandatory=$false)]
    [switch]$AllSynths,

    [Parameter(Mandatory=$false)]
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

$RootDir = $PSScriptRoot
$DefaultSynths = @(
    "neon-777",
    "neon-chip",
    "neon-fm",
    "neon-jr",
    "neon-sid",
    "neon-split",
    "neon-template"
)

if ($AllSynths) {
    Write-Host "--- Building Android APKs for all synths ---" -ForegroundColor Cyan

    $failedSynths = @()

    foreach ($synthName in $DefaultSynths) {
        Write-Host "\n=== [$synthName] Android build start ===" -ForegroundColor Yellow

        $scriptArgs = @(
            "-NoProfile",
            "-ExecutionPolicy", "Bypass",
            "-File", $PSCommandPath,
            "-Synth", $synthName,
            "-Config", $Config,
            "-AndroidApiLevel", $AndroidApiLevel
        )

        if ($AndroidNdkPath) {
            $scriptArgs += @("-AndroidNdkPath", $AndroidNdkPath)
        }

        if ($Clean) {
            $scriptArgs += "-Clean"
        }

        try {
            & pwsh @scriptArgs
            if ($LASTEXITCODE -ne 0) {
                $failedSynths += $synthName
            }
        } catch {
            Write-Warning "[$synthName] failed: $($_.Exception.Message)"
            $failedSynths += $synthName
        }
    }

    if ($failedSynths.Count -gt 0) {
        Write-Error ("Some synth Android builds failed: " + ($failedSynths -join ", "))
    }

    Write-Host "--- All synth Android builds completed successfully ---" -ForegroundColor Green
    exit 0
}

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

# Configure Android cross-compile using JUCE+CMake
# Use Unix-style paths for cross-platform compatibility
$ToolchainUnix = $AndroidToolchainFile -replace '\\', '/'

cmake -S $SynthDir -B $BuildDir `
    -G "Ninja Multi-Config" `
    -DCMAKE_BUILD_TYPE=$Config `
    "-DCMAKE_POLICY_VERSION_MINIMUM=3.5" `
    "-DCMAKE_TOOLCHAIN_FILE=$ToolchainUnix" `
    -DANDROID_ABI=arm64-v8a `
    "-DANDROID_PLATFORM=android-$AndroidApiLevel" `
    -DANDROID_STL=c++_shared

if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configuration failed"
}

# 3. Build Android targets
Write-Host "--- Building Android targets ---" -ForegroundColor Cyan

# Build CMake targets first
cmake --build $BuildDir --config $Config
if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake build failed"
}

# Detect project name from CMakeLists.txt (e.g. NeonJr, NeonFM)
$ProjectName = (Get-Content (Join-Path $SynthDir "CMakeLists.txt") | Select-String "project\(([^ ]+)" | ForEach-Object { $_.Matches.Groups[1].Value })
if (-not $ProjectName) { $ProjectName = $Synth }
$ArtefactsDirName = "${ProjectName}_artefacts"
Write-Host "Looking for artefacts directory: $ArtefactsDirName" -ForegroundColor Green

# Find the generated Android Studio project
$AndroidBuildDir = Get-ChildItem -Path $BuildDir -Recurse -Directory -Filter $ArtefactsDirName | Select-Object -First 1

if (-not $AndroidBuildDir) {
    Write-Warning "Android artefacts directory not found directly after CMake build"
}

# 4. Build installable APK from prebuilt JUCE standalone library
Write-Host "--- Packaging APK ---" -ForegroundColor Cyan

$JuceModulesDir = Join-Path $RootDir "third-party/JUCE/modules"
$ApkTemplateDir = Join-Path $RootDir "third-party/JUCE/extras/AudioPluginHost/Builds/Android"
$ApkProjectDir = Join-Path $BuildDir "apk-project"

if (-not (Test-Path $ApkTemplateDir)) {
    Write-Warning "JUCE Android template not found at $ApkTemplateDir; skipping APK packaging"
} elseif (-not (Test-Path $JuceModulesDir)) {
    Write-Warning "JUCE modules directory not found at $JuceModulesDir; skipping APK packaging"
} else {
    if (Test-Path $ApkProjectDir) {
        Remove-Item -Recurse -Force $ApkProjectDir
    }

    if (-not $env:JAVA_HOME) {
        if ($IsWindows) {
            $androidStudioJbr = "C:/Program Files/Android/Android Studio/jbr"
            if (Test-Path $androidStudioJbr) {
                $env:JAVA_HOME = $androidStudioJbr
                $env:PATH = "$($androidStudioJbr)/bin;$($env:PATH)"
                Write-Host "JAVA_HOME was unset; using Android Studio JBR at $androidStudioJbr" -ForegroundColor Yellow
            }
        }
    }

    New-Item -ItemType Directory -Path $ApkProjectDir -Force | Out-Null
    Copy-Item -Path (Join-Path $ApkTemplateDir "*") -Destination $ApkProjectDir -Recurse -Force

    $androidManifestPath = Join-Path $ApkProjectDir "app/src/main/AndroidManifest.xml"
    $gradleAppPath = Join-Path $ApkProjectDir "app/build.gradle"
    $gradleSettingsPath = Join-Path $ApkProjectDir "settings.gradle"

    $ApplicationId = "com.neonaudio." + (($Synth -replace "[^a-zA-Z0-9]", "").ToLower())
    $ProjectDisplayName = if ($Synth -eq "neon-fm") { "Neon FM" } else { ($Synth -replace "-", " ") }
    $DebugKeystorePath = if ($IsWindows) {
        (Join-Path $env:USERPROFILE ".android/debug.keystore") -replace "\\", "/"
    } else {
        "$HOME/.android/debug.keystore"
    }
    $SdkDir = if ($env:ANDROID_HOME) {
        $env:ANDROID_HOME
    } elseif ($env:ANDROID_SDK_ROOT) {
        $env:ANDROID_SDK_ROOT
    } else {
        try {
            (Split-Path (Split-Path $AndroidNdkPath -Parent) -Parent)
        } catch {
            $null
        }
    }

    $manifestContent = @"
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android" android:versionCode="1" android:versionName="1.0.0">
  <supports-screens android:smallScreens="true" android:normalScreens="true" android:largeScreens="true" android:anyDensity="true" android:xlargeScreens="true"/>
  <uses-permission android:name="android.permission.POST_NOTIFICATIONS"/>
  <uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" android:maxSdkVersion="28"/>
  <uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE" android:maxSdkVersion="32"/>
  <uses-permission android:name="android.permission.READ_MEDIA_VIDEO"/>
  <uses-permission android:name="android.permission.READ_MEDIA_IMAGES"/>
  <uses-permission android:name="android.permission.READ_MEDIA_AUDIO"/>
  <uses-permission android:name="android.permission.ACCESS_COARSE_LOCATION" android:maxSdkVersion="30"/>
  <uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" android:maxSdkVersion="30"/>
  <uses-permission android:name="android.permission.BLUETOOTH_ADMIN" android:maxSdkVersion="30"/>
  <uses-permission android:name="android.permission.BLUETOOTH" android:maxSdkVersion="30"/>
  <uses-permission android:name="android.permission.BLUETOOTH_CONNECT"/>
  <uses-permission android:name="android.permission.BLUETOOTH_ADVERTISE"/>
  <uses-permission android:name="android.permission.BLUETOOTH_SCAN" android:usesPermissionFlags="neverForLocation"/>
  <uses-permission android:name="android.permission.RECORD_AUDIO"/>
  <uses-permission android:name="android.permission.CHANGE_WIFI_MULTICAST_STATE"/>
  <uses-permission android:name="android.permission.INTERNET"/>
  <uses-feature android:glEsVersion="0x00030000" android:required="true"/>
  <application android:label="@string/app_name" android:name="com.rmsl.juce.JuceApp" android:icon="@drawable/icon" android:hardwareAccelerated="false">
    <receiver android:name="com.rmsl.juce.Receiver" android:exported="false"/>
    <activity android:name="com.rmsl.juce.JuceActivity" android:configChanges="keyboard|keyboardHidden|orientation|screenSize|navigation|smallestScreenSize|screenLayout|uiMode"
              android:launchMode="singleTask" android:hardwareAccelerated="true" android:exported="true">
      <intent-filter>
        <action android:name="android.intent.action.MAIN"/>
        <category android:name="android.intent.category.LAUNCHER"/>
      </intent-filter>
    </activity>
  </application>
</manifest>
"@

    $gradleAppContent = @"
apply plugin: 'com.android.application'

android {
    compileSdk 35
    ndkVersion "$((Split-Path $AndroidNdkPath -Leaf))"
    namespace "$ApplicationId"

    signingConfigs {
        juceSigning {
            storeFile     file("$DebugKeystorePath")
            storePassword "android"
            keyAlias      "androiddebugkey"
            keyPassword   "android"
            storeType     "jks"
        }
    }

    defaultConfig {
        applicationId "$ApplicationId"
        minSdkVersion $AndroidApiLevel
        targetSdkVersion 35
    }

    buildTypes {
         debug {
             initWith debug
             debuggable true
             signingConfig signingConfigs.juceSigning
         }
         release {
             initWith release
             debuggable false
             minifyEnabled false
             signingConfig signingConfigs.juceSigning
         }
    }

    sourceSets {
        main.java.srcDirs +=
            ["../../../../third-party/JUCE/modules/juce_core/native/javacore/init",
             "../../../../third-party/JUCE/modules/juce_core/native/javacore/app",
             "../../../../third-party/JUCE/modules/juce_gui_basics/native/javaopt/app"]

        main.res.srcDirs += []
        main.jniLibs.srcDirs = ["src/main/jniLibs"]
    }

    packagingOptions {
        jniLibs {
            useLegacyPackaging true
        }
    }
}

repositories {
    google()
    mavenCentral()
}

dependencies {
}
"@

    $settingsContent = @"
rootProject.name = '$ProjectDisplayName'
include ':app'
"@

    Set-Content -Path $androidManifestPath -Value $manifestContent -Encoding UTF8
    Set-Content -Path $gradleAppPath -Value $gradleAppContent -Encoding UTF8
    Set-Content -Path $gradleSettingsPath -Value $settingsContent -Encoding UTF8

    if ($SdkDir -and (Test-Path $SdkDir)) {
        $localPropsPath = Join-Path $ApkProjectDir "local.properties"
        $sdkDirEscaped = $SdkDir -replace "\\", "\\\\"
        Set-Content -Path $localPropsPath -Value "sdk.dir=$sdkDirEscaped" -Encoding UTF8
    } else {
        Write-Warning "Android SDK directory could not be resolved for Gradle local.properties"
    }

    $stringsPath = Join-Path $ApkProjectDir "app/src/main/res/values/strings.xml"
    if (Test-Path $stringsPath) {
        $stringsContent = @"
<?xml version="1.0" encoding="utf-8"?>
<resources>
    <string name="app_name">$ProjectDisplayName</string>
</resources>
"@
        Set-Content -Path $stringsPath -Value $stringsContent -Encoding UTF8
    }

    $StandaloneLibPath = Join-Path $BuildDir "$ArtefactsDirName/$Config/Standalone/lib${ProjectName}_Standalone.so"
    if (-not (Test-Path $StandaloneLibPath)) {
        $StandaloneLibPath = Get-ChildItem -Path $BuildDir -Recurse -File -Filter "lib*_Standalone.so" | Select-Object -First 1 -ExpandProperty FullName
    }

    if (-not $StandaloneLibPath -or -not (Test-Path $StandaloneLibPath)) {
        Write-Warning "Could not find JUCE standalone .so to package into APK"
    } else {
        $jniLibDir = Join-Path $ApkProjectDir "app/src/main/jniLibs/arm64-v8a"
        New-Item -ItemType Directory -Path $jniLibDir -Force | Out-Null

        Copy-Item -Path $StandaloneLibPath -Destination (Join-Path $jniLibDir "libjuce_jni.so") -Force
        Copy-Item -Path $StandaloneLibPath -Destination (Join-Path $jniLibDir (Split-Path $StandaloneLibPath -Leaf)) -Force

        $NdkHostTag = if ($IsWindows) {
            "windows-x86_64"
        } elseif ($IsLinux) {
            "linux-x86_64"
        } else {
            "darwin-x86_64"
        }

        $CxxSharedPath = Join-Path $AndroidNdkPath "toolchains/llvm/prebuilt/$NdkHostTag/sysroot/usr/lib/aarch64-linux-android/libc++_shared.so"
        if (Test-Path $CxxSharedPath) {
            Copy-Item -Path $CxxSharedPath -Destination (Join-Path $jniLibDir "libc++_shared.so") -Force
        } else {
            Write-Warning "libc++_shared.so not found at expected path: $CxxSharedPath"
        }

        $WrapperPath = Join-Path $ApkProjectDir "gradlew.bat"
        $WrapperUnixPath = Join-Path $ApkProjectDir "gradlew"

        if (-not $env:JAVA_HOME -and -not (Get-Command java -ErrorAction SilentlyContinue)) {
            Write-Warning "Java runtime not found (JAVA_HOME and java command missing); skipping APK packaging"
        } elseif ($IsWindows -and -not (Test-Path $WrapperPath)) {
            Write-Warning "Gradle wrapper not found in APK project; skipping APK packaging"
        } elseif (-not $IsWindows -and -not (Test-Path $WrapperUnixPath)) {
            Write-Warning "Gradle wrapper not found in APK project; skipping APK packaging"
        } else {
            Push-Location $ApkProjectDir
            try {
                if ($IsWindows) {
                    & cmd /c "$WrapperPath --no-daemon assembleRelease"
                } else {
                    & bash -c "chmod +x ./gradlew && ./gradlew --no-daemon assembleRelease"
                }
                if ($LASTEXITCODE -ne 0) {
                    Write-Warning "Gradle assembleRelease failed"
                }
            } finally {
                Pop-Location
            }
        }
    }
}

# 5. Collect Artifacts
Write-Host "--- Collecting Android Artifacts to $ArtifactsDir ---" -ForegroundColor Cyan

if (-not (Test-Path $ArtifactsDir)) {
    New-Item -ItemType Directory -Path $ArtifactsDir -Force | Out-Null
}

# Find all APK files
$ApkFiles = Get-ChildItem -Path $BuildDir -Recurse -Filter "*.apk" -File

if ($ApkFiles) {
    $primaryApk = $ApkFiles | Where-Object { $_.Name -match "release" } | Select-Object -First 1
    if (-not $primaryApk) {
        $primaryApk = $ApkFiles | Select-Object -First 1
    }

    foreach ($apk in $ApkFiles) {
        Write-Host "Found APK: $($apk.Name)" -ForegroundColor Green
        Copy-Item $apk.FullName -Destination $ArtifactsDir -Force
    }

    if ($primaryApk) {
        $namedApkPath = Join-Path $ArtifactsDir ("{0}-{1}.apk" -f $ProjectName, $Config)
        Copy-Item $primaryApk.FullName -Destination $namedApkPath -Force
        Write-Host "Canonical APK: $(Split-Path $namedApkPath -Leaf)" -ForegroundColor Green
    }
} else {
    Write-Warning "No APK files found in build directory"
}

# Find Android shared library outputs (JUCE standalone on CMake often produces .so only)
$SoFiles = Get-ChildItem -Path $BuildDir -Recurse -Filter "*.so" -File

if ($SoFiles) {
    $SoDest = Join-Path $ArtifactsDir "libs"
    if (-not (Test-Path $SoDest)) {
        New-Item -ItemType Directory -Path $SoDest -Force | Out-Null
    }

    foreach ($so in $SoFiles) {
        Write-Host "Found Android library: $($so.Name)" -ForegroundColor Green
        Copy-Item $so.FullName -Destination $SoDest -Force
    }
} else {
    Write-Warning "No Android .so libraries found in build directory"
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
