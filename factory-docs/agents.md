# Neon Synth Factory — Agent Guide

> _"Consistency is the soul of brand. Clarity, the soul of code."_ 

This guide provides essential commands and patterns for creating virtual instruments in the Neon Synth Factory using the `neon-template` paradigm.

---

## 🎯 Philosophy

All synths in this family must:

- Share a **visual language**: 8x2 button layout, neon theme (magenta/grey)
- Use **modular DSP**: Separate engines (oscillators, filters, etc.) connected via signal path
- Expose parameters through `NeonRegistry` for unified modulation and UI binding
- Be built with **JUCE** and packaged as **VST3 + Standalone**
- Never modify `neon-template` directly — always clone it

---

## 🚀 Quick Start Commands

Run these in PowerShell from the workspace root (`d:/l/neon-synth-factory`).

### 1. Clone Template into New Synth (Recommended)

```powershell
# Creates and configures a new synth project automatically
.\clone-template.ps1 -Name "NeonOsc"
```

> ✅ Automatically:
> - Clones `neon-template` → `NeonOsc`
> - Renames all classes (`NeonTemplateAudioProcessor` → `NeonOscAudioProcessor`)
> - Sets unique `PLUGIN_CODE` (e.g., `NeOs`) and `BUNDLE_ID` (e.g., `com.neonaudio.neonosc`)
> - Updates CMakeLists.txt
>
> **Do not manually rename files after this — the script does it all.**

### 2. Build Your Synth (Release Mode)

```powershell
# Builds VST3 and Standalone executable
.\build.ps1 -Project NeonOsc -Config Release
```

> ✅ Output: `build/NeonOsc/NeonOsc_artefacts/Release/Standalone/Neon Osc.exe`

### 3. Build All Synths (Debug)

```powershell
# Useful for testing the entire suite
.\build.ps1 -Config Debug
```

### 4. List All Available Synth Projects

```powershell
# Shows all folders with CMakeLists.txt
Get-ChildItem -Directory | Where-Object { Test-Path "$($_.Name)/CMakeLists.txt" }
```

### 5. Open Your New Project in VS Code

```powershell
# Opens the folder in current VS Code instance
code NeonOsc
```

---

## 📚 Key Documentation

| Topic | File |
|-------|------|
| How to use `neon-template` | [04_how_to_use_neon_template.md](./04_how_to_use_neon_template.md) |
| Neon Split Architecture | [01_neon_split_architecture.md](./01_neon_split_architecture.md) |
| Design System & UI Standards | [02_design_system.md](./02_design_system.md) |
| Component Inventory (neon-ui-components) | [03_component_inventory.md](./03_component_inventory.md) |

---

## ✅ Best Practices

- **Always** use `NeonRegistry::setTargetNames()` and `NeonRegistry::setSourceNames()` to expose modulation targets.
- **Never** hardcode UI labels — use enums from `ModulationTargets.h` or equivalent in your project.
- **Use** `neon-ui-components/widgets/NeonParameterCard.cpp` for consistent parameter displays.
- **Reference** existing synths (`neon-split`, `neon-jr`) for signal path patterns.
- **Document** your synth’s architecture in `planning docs/0X_your_synth_name.md`.

---

## ❌ Anti-Patterns (Avoid These)

| Mistake | Consequence |
|---------|-------------|
| Modifying `neon-template` directly | Breaks future clones and causes merge conflicts |
| Using custom button layouts or colors | Violates brand consistency across synths |
| Hardcoding parameter IDs instead of using enums | Makes modulation mapping fragile and unscalable |
| Skipping CMake configuration (`PLUGIN_CODE`, `BUNDLE_ID`) | Plugin won’t load in DAWs or causes conflicts |
| Not testing as VST3 | May work standalone but fail in host applications |

---

## 💡 Pro Tips

- Use `NeonLookAndFeel` from `neon-ui-components/core/NeonLookAndFeel.h` for theme consistency.
- If you need a new widget (e.g., custom ribbon, XY pad), add it to `neon-ui-components/widgets/` and reuse across projects.
- For complex modulation routing, extend `SplitModTarget` in your project’s `ModulationTargets.h` — keep naming consistent (`[Engine][Parameter]`).
- Always run `.\build.ps1 -Project neon-template -Config Release` first to verify your build environment is healthy.

---

## 📬 Support

For help with:

| Issue | Contact |
|-------|---------|
| Build failures | `#dev-builds` in Slack |
| UI/UX design questions | `#design-system` in Slack |
| JUCE or plugin architecture | `#juce-devs` in Slack |

> 🌟 You’re not just building a synth — you’re expanding the Neon family. Stay consistent. Stay creative.