# Neon Synth Factory - Themes

## Overview
The "Neon" brand supports human-readable, hot-swappable **Themes**. These are stored as `.toml` files in the `themes/` directory, allowing for rapid iteration on the visual identity without touching the C++ logic.

## Theme Structure (TOML)

### [metadata]
- `name`: Display name of the theme.
- `author`: For attribution.

### [global]
Base colors for the application shell and card containers.
- `background`: The main window color.
- `card_background`: The color of the individual [Parameter Cards](./06_parameter_card_style.md).
- `bar_background`: The "well" or empty part of the parameter bar.

### [accents]
Categorical colors for different [Synth Modules](./04_synth_modules.md).
- `oscillator / mutator`: Cyan/Blue tones.
- `filter`: Teal/Green tones.
- `envelope`: Green tones.
- `amplifier`: Warm/Orange tones.
- `modulation`: Amber/Yellow tones.
- `effects`: Sky blue tones.

## File Locations

The Neon Factory uses a two-tier theme system:

1.  **Global Themes:** Located in `neon-components/themes/`. These are the "factory standard" skins available to all instruments (e.g., *Classic Neon*, *Deep Sea*).
2.  **Synth-Specific Themes:** Located in `[synth-directory]/themes/`. These are custom skins unique to a specific instrument (e.g., `neon-h2o/themes/h2o_default.toml`).

## Workflow
1. **Fiddle:** Open the `.toml` file in the specific synth's `themes/` folder.
2. **Hex Codes:** Adjust the hex strings (e.g., `#00AAFF`).
3. **Commit:** Save the file.
4. **Implementation:** The `neon::ThemeManager` class (TODO) parses these at runtime and updates the `NeonLookAndFeel` instances across the app.

## Included Themes
- **[classic_neon.toml](../../neon-components/themes/classic_neon.toml)**: The signature Brand Look.
- **[deep_sea.toml](../../neon-components/themes/deep_sea.toml)**: A darker, low-fatigue night mode.
- **[h2o_default.toml](../../neon-h2o/themes/h2o_default.toml)**: The specific liquid identity for the H2O synth.
