# Neon Synth Factory - Global Configuration Panel

## Overview
The **Global Configuration Panel** is the administrative Hub of the instrument. Unlike the tactile, performance-focused Synth Modules, this panel uses a "Productivity" layout: DataGrids, lists, and form fields designed for efficient management of patches, settings, and themes.

## 1. Patch Librarian (DataGrid View)
A centralized database for managing the instrument's sound library.

### Features:
- **Search & Filter:** Real-time text search and category tagging (Basses, Pads, Leads).
- **Patch List (DataGrid):**
    - Columns: Name, Author, Category, Date Modified, Rating.
    - Behavior: Single-click to preview, double-click to load. Drag-and-drop to move between folders.
- **Actions:** Save, Save As, Delete, Export (as `.neonpatch`), and "Initialize Patch."

## 2. Theme Editor (Form Fields)
A visual interface for live-editing the [Theme TOML files](./08_themes.md).

### Features:
- **Color Pickers:** Standard hex input or "Neon Sliders" to adjust RGB/HSB values for Global and Accent colors.
- **Real-Time Preview:** Changes reflect instantly across the entire UI.
- **Theme Management:**
    - Dropdown to switch between `Global` and `Synth-Specific` themes.
    - "Clone Theme" to create new variants.
    - "Export Theme" to share `.toml` files.

## 3. MIDI & Expression Hub
Detailed control over how the synth interacts with external hardware.

### Settings:
- **MIDI Input/Output:** Device selection and channel filtering.
- **MPE (Multidimensional Polyphonic Expression):** Enable/Disable MPE, set Pitch Bend range, and calibrate Pressure/Timbre curves.
- **MIDI CC Map (DataGrid):**
    - A list of all MIDI CC mappings.
    - Columns: MIDI CC #, Destination Parameter, Min/Max Range, Smoothing.
    - "MIDI Learn" button to quickly assign hardware knobs.

## 4. System & Global Settings (Form Fields)
Core instrument behavior.

### Settings:
- **UI Scaling:** A dropdown or slider to scale the entire vector UI (80% to 200%). Uses JUCE's `Desktop::setGlobalScaleFactor`.
- **Global Tune:** Master tuning offset (Default 440.0 Hz).
- **Voices:** Global polyphony limit (1 to 64 voices).
- **Audio Setup:** Shortcut button to launch the JUCE standard Audio/MIDI device dialog.
- **About/Credits:** Version info and legal text.

## Visual Language
While the Synth Modules are "Cyberpunk Tactile," the Config Panel is **"Cyberpunk Functional."**

- **Typography:** 13px Regular for body text, 14px Bold for Headers.
- **Controls:**
    - **NeonListBox:** Custom JUCE `ListBox` with rows highlighted in the theme's `indicator` color.
    - **NeonComboBox:** Minimalist dropdowns with a neon border.
    - **NeonTextEditor:** Inset boxes with mono-spaced fonts.
- **Layout:** A vertical sidebar on the left for category selection (Library, Theme, MIDI, System) and a main content area on the right.

## Implementation Details
- **JUCE Component:** `juce::SidePanel` or a dedicated modal `juce::Viewport`.
- **Data Persistence:** Settings are stored in a global `config.toml` file in the User's Application Data folder.
- **Scaling:** Uses `juce::AffineTransform` or global scaling factors to maintain crisp vector graphics at any size.
