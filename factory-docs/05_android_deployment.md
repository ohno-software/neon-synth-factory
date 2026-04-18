# Android Deployment Guide

> Repurpose old Android phones and tablets as standalone music instruments with Neon Synth Factory.

---

## 📋 Prerequisites

### On Your Computer
- **Android SDK Platform Tools** (includes `adb`)
  - Download from [Android Developer Tools](https://developer.android.com/tools/releases/platform-tools)
  - Or install via Android Studio
- **USB Cable** to connect phone/tablet to computer
- **Built APK file** (from `build-android.ps1`)

### On Your Android Device
- **Android 10+** (API level 29+) recommended for best audio support
- **USB Debugging enabled** (Settings > Developer Options > Enable "USB Debugging")
- **File storage permissions** enabled (Settings > Apps > Neon Synth > Permissions > Files)
- **Microphone/audio permissions** enabled

---

## 🔧 Enable Developer Mode

**On your Android device:**

1. Open **Settings**
2. Go to **About Phone** (or **About Device**)
3. Tap **Build Number** 7 times rapidly
   - You'll see: "You are now a developer!"
4. Go back to **Settings**
5. Look for **Developer Options** (usually at the bottom)
6. Enable **USB Debugging**
7. When prompted on device: "Allow USB debugging for this computer?" → **Allow**

---

## 📱 Installation Methods

### Method 1: ADB (Recommended)

**Step 1: Connect device via USB**
```powershell
adb devices
```
You should see your device listed (e.g., `emulator-5554` or `device_id`).

**Step 2: Install APK**
```powershell
adb install -r artifacts/neon-jr/Android/NeonJr-Release.apk
```

The `-r` flag allows reinstalling over an existing version.

**Step 3: Verify installation**
```powershell
adb shell pm list packages | grep neonaudio
```

**Step 4: Launch app**
```powershell
adb shell am start -n com.neonaudio.neonjr/.MainActivity
```

Or simply tap the app icon on your device.

### Method 2: Direct File Transfer (Sideload)

**Step 1: Copy APK to phone**
- Connect phone via USB (File Transfer mode)
- Drag `NeonJr-Release.apk` to `Downloads/` folder

**Step 2: Install using file manager**
- Open file manager on phone
- Navigate to Downloads/
- Tap `NeonJr-Release.apk`
- Tap **Install**
- If prompted about unknown source: **Settings > Allow from this source**

### Method 3: Play Store (Future)

Once published to Google Play, installation is one tap.

---

## 🔊 Audio Configuration

### Enable Microphone Permission

1. Open **Settings** on device
2. Go to **Apps** > **Neon Jr** (or your synth)
3. Tap **Permissions**
4. Enable:
   - ✓ **Microphone** (for MIDI input from devices)
   - ✓ **Storage** (for loading custom patches/waveforms)

### Audio I/O on Android

- **Headphone Jack / USB-C Audio** — Most reliable for external audio
- **Bluetooth Audio** — Works but introduces latency; use for practice only
- **Internal Speaker** — Limited; not recommended for production

**Recommended setup:**
- Use USB-C audio adapter or Bluetooth speaker for output
- Use Bluetooth MIDI keyboard (e.g., Alesis, Korg Volca) for input
- Or use headphone jack with inline MIDI controller

---

## 🎵 First Launch

1. **Tap app icon** to launch Neon Synth
2. **Allow permissions** as prompted
3. **Wait for initialization** (first launch may take 10-15 seconds)
4. **Tap the main module area** to see parameter cards
5. **Use on-screen buttons** or **external MIDI controller** to play notes

### MIDI Input

- **Bluetooth MIDI devices** pair automatically
- **USB MIDI** requires USB On-The-Go (OTG) adapter
- **Touch keyboard** (built into app) available as fallback

---

## 🐛 Troubleshooting

### App won't install ("Unknown sources" error)
- **Solution:** Settings > Apps > Special app access > Install unknown apps > Select file manager > Allow

### App crashes on launch
- **Solution:** 
  1. Uninstall: `adb uninstall com.neonaudio.neonjr`
  2. Reinstall with fresh APK
  3. Check logcat: `adb logcat | grep neonaudio`

### No sound coming out
- **Check:**
  1. Volume not muted (check physical buttons)
  2. Audio output device selected (Bluetooth speaker, headphones)
  3. App has microphone permission enabled
  4. App not in background (may suspend audio)

### Audio input (MIDI) not detected
- **Bluetooth:** 
  - Pair device in Settings > Bluetooth
  - Relaunch app (some devices need app restart)
- **USB MIDI:**
  - Requires USB-C OTG adapter
  - Check device Settings > USB for connection mode

### Lag/latency issues
- **Cause:** Audio buffer too small, Bluetooth lag
- **Solution:**
  1. Use wired audio (USB-C or headphone jack)
  2. Reduce audio quality in settings (if available)
  3. Close background apps
  4. Use external buffer: set buffer size to 512-1024 samples

### ADB device not recognized
```powershell
# Restart ADB daemon
adb kill-server
adb start-server

# If still not recognized:
adb devices -l

# Check drivers (Windows)
# Download from phone manufacturer's website
```

---

## 📂 File Organization

### Patches
- Location: `Documents/Neon Synth/Patches/`
- Format: `.neon_patch` or manufacturer-specific

### Waveforms (if applicable)
- Location: `Documents/Neon Synth/Waveforms/`
- Format: `.wav` (44.1 kHz, mono or stereo)

### Settings
- Location: `Android/data/com.neonaudio.neonjr/cache/`
- Backed up on app uninstall: NO (save custom settings manually!)

---

## 🎛️ Tips & Tricks

### Optimize battery life
- Reduce screen brightness (audio runs independent of display)
- Enable Battery Saver mode
- Use external power during long sessions

### Multi-device setup
- Install same APK on multiple phones/tablets
- Sync via Bluetooth MIDI for polyphonic performances
- Each device can have different patches

### External controllers
- **Bluetooth MIDI keyboards:** Alesis, Korg Volca, CME Widi
- **USB MIDI adapters:** Rock Band Midi Adapter, custom DIY controllers
- **Touch pads:** KAOSSILATOR-style apps (via network MIDI OSC)

---

## 📊 Supported Devices

### Tested Configurations
| Device | OS | Status | Notes |
|--------|----|---------|----|
| Samsung Galaxy A51 | Android 11 | ✓ Working | Good audio quality |
| OnePlus 7T | Android 10 | ✓ Working | Excellent latency |
| Google Pixel 4a | Android 12 | ✓ Working | Stable, reliable |
| iPad (via DEX) | Android emulation | ⚠️ Untested | Requires Android container |
| Older tablets (2016+) | Android 9-10 | ✓ Expected | Test before relying |

**Not supported:**
- Android 9 and below (may still work, not guaranteed)
- Devices without audio hardware
- Tablets with custom ROMs (YMMV)

---

## 📦 Build Your Own APK

### Rebuild with custom settings
```powershell
# From repository root
.\build-android.ps1 -Synth neon-jr -Config Release

# Output
# artifacts/neon-jr/Android/NeonJr-Release.apk
```

### Customize app name/icon (advanced)
Edit [`build-android.ps1`](../../build-android.ps1) and modify:
- `$ApplicationId` (e.g., `com.neonaudio.neonjr`)
- `$ProjectDisplayName` (e.g., "Neon Jr")
- Icon file location (in build-android.ps1 APK template section)

---

## 🔐 Security & Privacy

- **No internet required** — Neon Synth works offline
- **No telemetry** — Your music data stays on device
- **Permissions used:**
  - `RECORD_AUDIO` — For audio I/O only
  - `READ_MEDIA_AUDIO` — For loading custom samples
  - `BLUETOOTH_*` — For MIDI controller pairing only

---

## 🤝 Support

- **Issues/Crashes:** Check logcat output and share with developers
  ```powershell
  adb logcat -c
  # [reproduce crash]
  adb logcat > crash_log.txt
  ```
- **Feature Requests:** Post to GitHub Issues
- **Community:** Join the Neon Synth Factory Discord

---

**Happy music-making on your Android instruments! 🎵**
