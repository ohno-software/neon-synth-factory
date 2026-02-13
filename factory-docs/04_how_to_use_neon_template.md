# How to Use neon-template as a Base for New Synth Projects

## Purpose
`neon-template` is the official UI blueprint for all new synths in the Neon Synth Factory. It provides:

- A standardized 8x2 button layout (top: categories, bottom: modules)
- Consistent theming (magenta accents, dark background)
- JUCE-based structure compatible with VST3 and Standalone formats
- No audio engine — allowing you to implement your own DSP from scratch

Use this template to ensure visual and interaction consistency across all Neon synths.

---

## Prerequisites

Before starting, ensure you have:

- **Visual Studio 2022** with C++ development tools installed
- **JUCE 7.0+** (already included in the workspace under `../JUCE`)
- **neon-components** library built and available (`../neon-components`)
- Access to the `build.ps1` script for building projects

> 💡 Tip: If you haven’t built any synth yet, run `.\build.ps1 -Project neon-split -Config Debug` first to verify your environment.

---

## Step 1: Clone neon-template into a New Project Folder

1. In Windows Explorer or PowerShell, navigate to the workspace root:
   ```powershell
   cd d:/l/neon-synth-factory
   ```

2. Copy the entire `neon-template` folder and rename it to your new synth name (e.g., `neon-osc`):
   ```powershell
   cp -Recurse neon-template neon-osc
   ```

3. Rename all internal references:

   | File | Change |
   |------|--------|
   | `neon-osc/CMakeLists.txt` | Replace `NeonTemplate` → `NeonOsc` (case-sensitive) |
   | `neon-osc/source/PluginProcessor.h/cpp` | Rename class from `NeonTemplateAudioProcessor` → `NeonOscAudioProcessor` |
   | `neon-osc/source/PluginEditor.h/cpp` | Rename class from `NeonTemplateAudioProcessorEditor` → `NeonOscAudioProcessorEditor` |

4. Update the CMake project name in `CMakeLists.txt`:
   ```cmake
   project(NeonOsc VERSION 0.1.0)
   ```

5. Update plugin identifiers (in `CMakeLists.txt`):
   ```cmake
   PLUGIN_MANUFACTURER_CODE NeAu    # Keep unchanged if same manufacturer
   PLUGIN_CODE NeOs                 # New unique 4-letter code (e.g., NeOs for Neon Osc)
   BUNDLE_ID com.neonaudio.neonosc  # Update bundle ID
   PRODUCT_NAME "Neon Osc"          # Update display name
   ```

> ✅ Use [JUCE Plugin Identifier Guide](https://juce.com/learn/plugin-identifiers) to ensure uniqueness.

---

## Step 2: Customize the UI (Optional)

The template includes a `TemplateSelectionPanel` that displays:

- **Top row**: 5 labeled buttons (`BASS`, `PAD`, etc.) + 3 empty slots
- **Bottom row**: 8 empty button placeholders

### To change category labels:

Edit `neon-osc/source/PluginEditor.h`, inside the `TemplateSelectionPanel` constructor:

```cpp
// Set category button labels (matching neon-split)
categoryButtons[0]->setButtonText("BASS");
categoryButtons[1]->setButtonText("PAD");
categoryButtons[2]->setButtonText("ARP");
categoryButtons[3]->setButtonText("DRUM");
categoryButtons[4]->setButtonText("MAIN");
```

Replace with your synth’s categories (e.g., `OSC`, `FILTER`, `LFO`, `EFFECTS`, `GLOBAL`).

### To add module buttons:

If your synth has 8 distinct modules, you can make all bottom buttons visible and assign names. For example:

```cpp
// In TemplateSelectionPanel constructor
moduleButtons[0]->setButtonText("OSC1");
moduleButtons[1]->setButtonText("OSC2");
moduleButtons[2]->setButtonText("FILTER");
moduleButtons[3]->setButtonText("ENV");
moduleButtons[4]->setButtonText("LFO");
moduleButtons[5]->setButtonText("FX");
moduleButtons[6]->setButtonText("MIXER");
moduleButtons[7]->setButtonText("MASTER");
```

> 🔔 Keep the 8-button grid. Do not change layout dimensions — this ensures consistency across all Neon synths.

---

## Step 3: Implement Your Audio Engine

The template has no audio engine by design. You must implement:

1. **Audio Processor** (`PluginProcessor.h/cpp`)
   - Override `processBlock()` to generate or process audio
   - Add your DSP modules (oscillators, filters, envelopes)
   - Connect them via a signal path similar to `SplitSignalPath`

2. **Module Components** (optional but recommended)
   - Create custom module classes inheriting from `NeonSynthModule` in `neon-ui-components/modules/`
   - Example: `OscillatorModule`, `FilterModule`
   - Register them in your editor’s `createEditor()` and show/hide based on button state

> 💡 Reference `neon-split/source/SplitSignalPath.cpp` or `neon-jr/source/SignalPath.cpp` for signal path patterns.

---

## Step 4: Build and Test

1. Build your project using the build script:
   ```powershell
   .\build.ps1 -Project neon-osc -Config Release
   ```

2. Locate the executable:
   ```
   build/neon-osc/NeonOsc_artefacts/Release/Standalone/Neon Osc.exe
   ```

3. Launch it and verify:
   - UI matches Neon brand (colors, layout)
   - Buttons respond to clicks
   - No crashes or missing assets

4. Test in your DAW as a VST3 plugin if needed.

---

## Step 5: Document Your Synth

After building, create documentation:

1. Add a new file in `planning docs/` describing your synth’s architecture (e.g., `05_neon_osc_architecture.md`).
2. Reference this guide and link to `neon-template` as the UI foundation.
3. Update `FACTORY_README.md` if adding a new member to the Neon family.

---

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Build fails with "CMake Error: CMAKE_CXX_COMPILER not set" | Run `build.ps1` from an **Administrator PowerShell** or ensure Visual Studio 2022 is installed with C++ workload. |
| UI buttons don’t respond | Ensure `setClickingTogglesState(true)` and `setRadioGroupId()` are called in the panel constructor. |
| Theme colors wrong | Verify `Colors::background` is defined in `neon-ui-components/core/NeonColors.h`. If not, use `juce::Colours::grey10` as fallback. |
| Plugin doesn’t appear in DAW | Ensure `FORMATS VST3 Standalone` is set in CMakeLists.txt and you’re scanning the correct plugin folder (e.g., `C:\Program Files\VST3`). |

---

## Final Notes

- **Never modify neon-template directly** — always clone it.
- **Always match the 8x2 button layout** — this is a brand requirement.
- Use `neon-ui-components` for consistent widgets (parameter cards, toggles, ribbons).
- When in doubt, copy patterns from `neon-split`, `neon-jr`, or `neon-fm`.

> 🌟 Your synth will be part of a unified family. Consistency = trust. Consistency = professionalism.