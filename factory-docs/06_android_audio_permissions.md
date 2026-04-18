# Android Audio & Permissions Reference

> Technical details about audio APIs, permissions, and MIDI on Android.

---

## 🔐 Required Permissions

The Neon Synth Factory Android APKs declare the following permissions in `AndroidManifest.xml`:

### Audio Permissions
| Permission | Purpose | Required |
|-----------|---------|----------|
| `RECORD_AUDIO` | Capture microphone input for MIDI/audio processing | **YES** |
| `MODIFY_AUDIO_SETTINGS` | Allow app to modify audio routing and effects | YES |
| `ACCESS_NOTIFICATION_POLICY` | Allow audio to play during Do Not Disturb | NO |

### Storage Permissions
| Permission | API Level | Purpose |
|-----------|-----------|---------|
| `READ_EXTERNAL_STORAGE` | ≤ 32 | Load custom waveforms/samples from storage | YES* |
| `WRITE_EXTERNAL_STORAGE` | ≤ 28 | Save patches and recordings | YES* |
| `READ_MEDIA_AUDIO` | ≥ 33 | Modern scoped audio access | YES* |
| `READ_MEDIA_IMAGES` | ≥ 33 | Access UI icons/images | NO |

*Only needed if loading/saving custom files; optional for built-in presets.

### Network & Bluetooth
| Permission | Purpose |
|-----------|---------|
| `BLUETOOTH_CONNECT` | Connect to Bluetooth MIDI devices |
| `BLUETOOTH_SCAN` | Discover nearby Bluetooth devices |
| `INTERNET` | (Optional) For future cloud features |

---

## 🎵 Audio APIs Used

### OpenSL ES (Primary)
- **API Level:** 9+ (Universal support)
- **Latency:** ~50-200ms (device-dependent)
- **Channels:** Mono/Stereo/Multichannel
- **Sample Rate:** 8kHz - 192kHz
- **Bit Depth:** 16-bit PCM (primary), 32-bit float (secondary)
- **JUCE Implementation:** `OpenSLAudioDevice.cpp`

**Pros:**
- Low-latency audio
- Works on all Android devices
- Good for real-time audio

**Cons:**
- No access to system audio effects
- Limited parameter control

### AAudio (Modern)
- **API Level:** 26+ (Android 8.0+)
- **Latency:** ~10-50ms (significantly lower)
- **Features:** Session ID, sharing mode, spatialization
- **JUCE Fallback:** Used if available, falls back to OpenSL ES

**Activation:**
- Automatically used on Android 8.0+
- Requires `android.permission.MODIFY_AUDIO_SETTINGS`

---

## 🎹 MIDI Support

### Bluetooth MIDI (BLE)
- **Standard:** Bluetooth MIDI over BLE
- **Latency:** 5-20ms typical
- **Devices:** Alesis, Korg, CME, Native Instruments, etc.
- **Pairing:** Standard Bluetooth > Settings > Pair

**Tested Devices:**
- ✓ Alesis VortexWireless 2
- ✓ Korg Volca Keys
- ✓ Native Instruments Kontrol M1
- ⚠️ Timing can drift; reset connection if lag noticed

### USB MIDI (OTG)
- **Standard:** USB MIDI Class Compliant
- **Requirements:** USB-C OTG adapter (most phones) or USB-A to USB-C hub
- **Latency:** 1-5ms (fastest option)
- **Devices:** Most keyboards, controllers, iOS devices with adapter

**OTG Adapter Recommendations:**
- Belkin USB-C to USB-A Adapter
- UGREEN USB-C OTG Cable
- Generic USB-C Hub with USB-A ports

### Touch Keyboard (Fallback)
- Built-in touch keyboard for devices without external MIDI
- Polyphonic finger tracking
- Velocity sensitivity via finger pressure (on supported devices)

---

## 🔧 Configuration & Tuning

### Audio Buffer Settings

**Located in:** App Settings > Audio > Buffer Size

| Buffer Size | Latency | CPU | Best For |
|------------|---------|-----|----------|
| 128 | ~7ms | High | Performance, low latency |
| 256 | ~14ms | High | Responsive control |
| 512 | ~28ms | Medium | Balanced |
| 1024+ | ~56ms+ | Low | Older devices, battery |

**Recommendation for old tablets/phones:** Start with 512-1024, reduce if stable.

### Sample Rate

| Rate | Use Case | Device Requirement |
|------|----------|-------------------|
| 44100 Hz | Standard (CD quality) | All devices |
| 48000 Hz | Professional | All modern devices |
| 96000 Hz+ | High-fidelity | Recent flagship devices |

**Note:** Higher rates require more CPU; use 44.1-48kHz for older devices.

### Audio Focus & Routing

Android allows multiple apps to play audio simultaneously. Neon Synth can request:

- **AUDIOFOCUS_GAIN** — Exclusive audio (pauses other apps)
- **AUDIOFOCUS_GAIN_TRANSIENT** — Temporary focus (for notifications)

**Current behavior:** Neon Synth requests `AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK` (shares audio, slightly reduces other apps' volume).

---

## 🎯 Permission Request Flow

### Android 6.0+ (Runtime Permissions)

When you first launch Neon Synth:

1. **Initial prompt:** "Neon Jr needs permission to record audio"
   - Tap **Allow** to enable microphone
2. **Storage prompt:** (if loading custom files)
   - Tap **Allow** to access your music library
3. **Bluetooth prompt:** (if connecting MIDI device)
   - Tap **Allow** to pair devices

### Granting Permissions Later

If you denied permissions:

1. Open **Settings** > **Apps** > **Neon Jr**
2. Tap **Permissions**
3. Toggle each permission ON

**Note:** Some permissions are system-level and cannot be revoked by the app (audio focus, notification policy).

---

## 🚨 Permission Denial Scenarios

| Denied Permission | Impact | Workaround |
|-----------------|--------|-----------|
| `RECORD_AUDIO` | No audio input/output | Grant permission; restart app |
| `READ_MEDIA_AUDIO` | Can't load custom samples | Use built-in presets only |
| `BLUETOOTH_CONNECT` | Can't pair MIDI devices | Use USB MIDI or touch keyboard |
| `MODIFY_AUDIO_SETTINGS` | Limited audio control | Usually not critical |

---

## 🔗 Audio Routing on Android

### Output Routing Priority

When audio is playing, Android prioritizes output to:

1. **Bluetooth Speaker** (if connected and actively playing)
2. **Headphones** (if plugged in)
3. **Speaker** (built-in device speaker)

**To force output:**
- Unplug headphones
- Disconnect Bluetooth
- Or use app's audio output settings (if available)

### Input Routing Priority

When recording/listening:

1. **USB Microphone** (if OTG adapter with mic attached)
2. **Bluetooth Headset** (if paired and active)
3. **Built-in Microphone** (default)

---

## 🔇 Audio in Different States

### Foreground (App Active)
- ✓ Full audio I/O
- ✓ Lowest latency
- ✓ Full MIDI responsiveness

### Background (App Minimized)
- ⚠️ Audio may pause (Android 8.0+)
- ⚠️ Reduced CPU allocation
- ⚠️ MIDI may disconnect

### Screen Off
- ⚠️ Audio pauses on some devices
- ⚠️ Maximum CPU throttling
- ✓ Can be re-enabled in settings (battery impact)

**Best Practice:** Keep app in foreground during performance, or enable "Audio While Locked" in settings (if available).

---

## 🔋 Battery Impact

### Audio Components & Power Draw

| Component | Impact | Notes |
|-----------|--------|-------|
| OpenSL ES @ 48kHz | +15-20% CPU | Minimal battery drain |
| Bluetooth MIDI | +10% per device | Connected device search: +5% |
| Screen at full brightness | +30-40% | Turn down brightness |
| Wi-Fi on (no data) | +5% | Can be disabled |
| USB OTG device | +10-15% | Hub or powered adapter recommended |

**Power optimization:**
- Use Bluetooth instead of USB OTG (lower power)
- Reduce screen brightness
- Disable Wi-Fi if not needed
- Close background apps
- Use external power for long sessions

---

## 🧪 Testing Audio Permissions

### Check Current Permissions (ADB)

```powershell
adb shell dumpsys package com.neonaudio.neonjr | grep permissions
```

### Grant/Revoke Permissions (ADB)

```powershell
# Grant RECORD_AUDIO
adb shell pm grant com.neonaudio.neonjr android.permission.RECORD_AUDIO

# Revoke RECORD_AUDIO
adb shell pm revoke com.neonaudio.neonjr android.permission.RECORD_AUDIO
```

### Check Audio Device State

```powershell
# List audio devices
adb shell dumpsys audio | grep -i "device"

# Check Bluetooth status
adb shell dumpsys bluetooth_manager | grep -i "state"
```

---

## 🐛 Common Issues

### "No audio input detected"
**Check:**
1. `RECORD_AUDIO` permission granted
2. Microphone not muted (physical button)
3. Built-in mic not damaged (try USB headset)
4. App not in background (bring to foreground)

**Fix:**
```powershell
adb shell pm grant com.neonaudio.neonjr android.permission.RECORD_AUDIO
adb shell am start -n com.neonaudio.neonjr/.MainActivity
```

### "MIDI device not showing up"
**Check:**
1. Device paired in Bluetooth
2. Device is NOT connected to another app
3. USB OTG adapter seated properly (if USB)
4. Correct USB mode selected (File Transfer, not Charge Only)

**Fix:**
- Unpair/re-pair device
- Restart app
- Restart phone (last resort)

### "Audio crackling or stuttering"
**Check:**
1. Buffer size too small (increase to 512)
2. Sample rate mismatch (try 44.1kHz)
3. CPU overloaded (close other apps)
4. Bluetooth interference (move away from other devices)

**Fix:**
```
Settings > Audio > Buffer Size: 512 (or higher)
Settings > Audio > Sample Rate: 44100 Hz
```

---

## 📚 References

- [Android Audio Documentation](https://developer.android.com/guide/topics/media/audio)
- [OpenSL ES Specification](https://www.khronos.org/opensles/)
- [AAudio Best Practices](https://developer.android.com/ndk/guides/audio/aaudio/best-practices)
- [Bluetooth MIDI on Android](https://developer.android.com/guide/topics/media-framework/midi)
- [JUCE Android Documentation](https://docs.juce.com/master/classAndroidAudioIODevice.html)

---

**Questions?** Open an issue on [GitHub](https://github.com/ohno-software/neon-synth-factory/issues).
