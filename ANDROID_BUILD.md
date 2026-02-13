# Neon Jr - Android Build Guide

This guide explains how to build Neon Jr for Android devices, including setup for GitHub Actions CI/CD and local development.

> Platform note: iOS/iPadOS targets are Standalone + AUv3. VST3 is desktop-only and not an iOS plugin format.

## Samsung Galaxy A9 Compatibility

Neon Jr is configured to build for ARM64 devices (arm64-v8a), which includes the Samsung Galaxy A9 and most modern Android devices.

- **Minimum Android Version**: Android 7.0 (API 24)
- **Architecture**: ARM64-v8a
- **Recommended**: Android 8.0+ for best performance

## Prerequisites

### For Local Builds

1. **Android NDK** (r26d or later)
   - Download from: https://developer.android.com/ndk/downloads
   - Set environment variable: `ANDROID_NDK_ROOT` or `ANDROID_NDK_HOME`

2. **Java Development Kit** (JDK 17 or later)
   - Required for Gradle builds
   - Download from: https://adoptium.net/

3. **CMake** (3.22 or later)
   - Download from: https://cmake.org/download/

4. **Ninja Build System**
   - Download from: https://github.com/ninja-build/ninja/releases

### For GitHub Actions (Automatic)

The CI/CD workflow automatically sets up all required dependencies when you push to the repository or create a release tag.

## Building Locally

### Quick Build (Windows/PowerShell)

```powershell
# Make sure ANDROID_NDK_ROOT is set
$env:ANDROID_NDK_ROOT = "C:\path\to\android-ndk-r26d"

# Build Neon Jr for Android
.\build-android.ps1 -Synth neon-jr -Config Release
```

### With Custom NDK Path

```powershell
.\build-android.ps1 -Synth neon-jr -Config Release -AndroidNdkPath "C:\android-ndk-r26d"
```

### Clean Build

```powershell
.\build-android.ps1 -Synth neon-jr -Config Release -Clean
```

### Build Output

The APK file will be generated in:
```
artifacts/neon-jr/Android/
```

Look for files like:
- `NeonJr-release.apk` - Release build (optimized)
- `NeonJr-debug.apk` - Debug build (if building with -Config Debug)

## Installing on Samsung Galaxy A9

### Method 1: Direct Install via USB (Recommended)

1. **Enable Developer Options** on your Galaxy A9:
   - Go to Settings → About phone
   - Tap "Build number" 7 times
   - Go back to Settings → Developer options
   - Enable "USB debugging"

2. **Install ADB** (Android Debug Bridge) on your PC:
   - Download Android Platform Tools: https://developer.android.com/tools/releases/platform-tools
   - Extract and add to PATH

3. **Connect and Install**:
   ```powershell
   # Connect your phone via USB
   adb devices  # Verify connection
   
   # Install the APK
   adb install artifacts/neon-jr/Android/NeonJr-release.apk
   
   # If updating an existing installation
   adb install -r artifacts/neon-jr/Android/NeonJr-release.apk
   ```

### Method 2: Manual Install

1. Copy the APK file to your phone (via USB, cloud storage, or email)
2. On your phone, enable "Install unknown apps" for your file manager:
   - Settings → Apps → Special access → Install unknown apps
3. Open the APK file in your file manager
4. Tap "Install"

### Method 3: Download from GitHub Releases

1. When you create a release tag (e.g., `v1.0.0`), GitHub Actions automatically builds and uploads the Android APK
2. Go to your repository's Releases page
3. Download `Neon-Jr-Android.zip`
4. Extract and install the APK using Method 1 or 2

## Project Wavetables on Android

The wavetables (`waves-1` folder) need to be accessible to the app:

### Option 1: Bundle in APK (Current Setup)
Wavetables are included in the APK during build. No additional steps needed.

### Option 2: Load from External Storage (Future Enhancement)
For dynamic wavetable loading, place files in:
```
/sdcard/Android/data/com.neonaudio.neonjr/files/waves-1/
```

## GitHub Actions CI/CD

The repository is configured to automatically build Android APKs when you:

1. **Push to main branch**: Creates development builds
2. **Create a pull request**: Builds and validates
3. **Manually trigger**: Use "Run workflow" on Actions tab
4. **Create a release tag**: Builds and publishes release artifacts

### Triggering a Release Build

```bash
git tag v1.0.0-alpha
git push origin v1.0.0-alpha
```

This will:
1. Build for Windows, macOS, and Android
2. Create a GitHub release
3. Upload all platform artifacts including `Neon-Jr-Android.zip`

## Architecture Details

### JUCE Android Configuration

The build is configured for:
- **Format**: AndroidStudio (generates Android Studio project)
- **ABI**: arm64-v8a (64-bit ARM)
- **Min SDK**: 24 (Android 7.0)
- **Target SDK**: Latest stable
- **STL**: c++_shared (LLVM libc++)

### Build Process

1. **CMake Configuration**: Generates Android Studio project using NDK toolchain
2. **Gradle Build**: Compiles native code and packages APK
3. **Artifact Collection**: Extracts APK from Gradle output

### File Structure

```
build/neon-jr-android/          # Build directory
├── NeonJr_artefacts/
│   └── AndroidStudio/          # Generated Android Studio project
│       ├── app/
│       ├── gradlew             # Gradle wrapper
│       └── build/              # APK output
└── ...

artifacts/neon-jr/Android/      # Final artifacts
├── NeonJr-release.apk
└── waves-1/                    # Wavetables
```

## Performance Tips for Mobile

1. **Use Release builds** - Debug builds have significant performance overhead
2. **Buffer size** - Use larger buffer sizes on mobile (256-512 samples)
3. **Voice count** - Consider reducing max polyphony on older devices
4. **Background audio** - Android may suspend audio when app is backgrounded

## Troubleshooting

### "No APK files found"

**Solution**: Ensure Gradle successfully built the APK
- Check that Java JDK is installed
- Verify NDK path is correct
- Look for build errors in the Gradle output

### "App not installed" on device

**Solutions**:
- Uninstall any previous versions first
- Check if "Install unknown apps" permission is enabled
- Verify the APK is for arm64-v8a architecture
- Make sure Android version is 7.0 or higher

### Build fails with "Android NDK path not found"

**Solution**: Set environment variable
```powershell
$env:ANDROID_NDK_ROOT = "C:\path\to\ndk"
# Or add to system environment variables permanently
```

### GitHub Actions fails on Android build

**Solutions**:
- Check that NDK version in workflow matches (currently r26d)
- Verify Java version is 17 or higher
- Look for CMake or Gradle errors in Actions logs

## Development Tips

### Testing on Emulator

You can test on Android Emulator (requires Android Studio):
```powershell
# Start emulator
emulator -avd Pixel_4_API_30

# Install APK
adb install artifacts/neon-jr/Android/NeonJr-release.apk
```

### Debugging

For debug builds with logging:
```powershell
.\build-android.ps1 -Synth neon-jr -Config Debug

# View logs
adb logcat | Select-String "NeonJr"
```

### Signing APK for Release

For production releases, you should sign the APK:
1. Generate a keystore
2. Configure signing in CMakeLists.txt
3. Use `jarsigner` or Android Studio's signing tools

## Resources

- [JUCE Android Documentation](https://docs.juce.com/master/tutorial_android_studio.html)
- [Android NDK Developer Guide](https://developer.android.com/ndk/guides)
- [ADB User Guide](https://developer.android.com/tools/adb)

## Support

For issues specific to:
- **Building**: Check build logs and verify prerequisites
- **Installation**: Ensure USB debugging and unknown apps permissions
- **Performance**: Try release build and adjust buffer sizes
- **Compatibility**: Verify device has Android 7.0+ and ARM64

---

**Note**: The first build may take longer as Gradle downloads dependencies. Subsequent builds will be faster.
