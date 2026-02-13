# Neon Synth Factory — Parameter Flow Compliance Audit

Date: 2026-02-13

## Scope
Synths reviewed:
- neon-split
- neon-777
- neon-chip
- neon-fm
- neon-jr
- neon-sid

Audit rule:
- UI controls must update central parameter state.
- DSP/signal flow must read from central parameter state in processing paths.
- UI must not directly mutate DSP engines or signal routing.

## Overall Result
All six synths are compliant with the parameter-flow architecture contract at the time of this audit.

## Common Architecture Evidence
- Central registry ownership and access:
  - `neon-components/source/neon_ui_components/core/NeonParameterRegistry.h`
- UI module parameter registration into the central registry:
  - `neon-components/source/neon_ui_components/modules/ModuleBase.cpp`
- UI control writes routed through managed parameter objects:
  - `neon-components/source/neon_ui_components/widgets/NeonParameterCard.cpp`

## Per-Synth Findings

### neon-split
- DSP polls and consumes parameter state:
  - `neon-split/source/SplitSignalPath.cpp` (`updateParams`, called from `processBlock`)
- Editor access to signal path is telemetry-only (no mutators found):
  - `neon-split/source/PluginEditor.cpp`
- Status: **Compliant**

### neon-777
- DSP polls and consumes parameter state:
  - `neon-777/source/SignalPath.cpp` (`updateParams`, called from audio block path)
- Editor access to signal path is telemetry-only (voice count / pitch / mod readouts):
  - `neon-777/source/PluginEditor.cpp`
- Status: **Compliant**

### neon-chip
- DSP polls and consumes parameter state:
  - `neon-chip/source/ChipSignalPath.cpp` (`updateParams`, called before/within render path)
- Editor access to signal path is telemetry-only:
  - `neon-chip/source/PluginEditor.cpp`
- Status: **Compliant**

### neon-fm
- DSP polls and consumes parameter state:
  - `neon-fm/source/FmSignalPath.cpp` (`updateParams`, called from render path)
- Editor access to signal path is telemetry-only:
  - `neon-fm/source/PluginEditor.cpp`
- Status: **Compliant**

### neon-jr
- DSP polls and consumes parameter state:
  - `neon-jr/source/SignalPath.cpp` (`updateParams`, called from render path)
- Editor access to signal path is telemetry-only:
  - `neon-jr/source/PluginEditor.cpp`
- Status: **Compliant**

### neon-sid
- DSP polls and consumes parameter state:
  - `neon-sid/source/SidSignalPath.cpp` (`updateParams`, called from `getNextAudioBlock`)
- No editor-side DSP mutation path identified.
- Status: **Compliant**

## Review Notes
- MIDI event handling in processors (note on/off, wheel, aftertouch) is expected transport/performance input and not a violation of the UI-parameter contract.
- This audit is static (code inspection) and reflects repository state on the audit date.

## Recommended Ongoing Check
For every PR touching synth UI or signal path, verify:
1. UI changes only parameter state.
2. Signal path reads parameter state in processing.
3. No direct editor-to-engine setter path is introduced for runtime control values.
