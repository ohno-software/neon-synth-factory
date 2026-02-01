# Neon Synth Factory - Component Library Architecture

## Overview
The Neon Synth Factory component library is a custom JUCE-based UI framework designed to provide a consistent brand identity across a suite of synthesizers. It leverages JUCE's `LookAndFeel` system and custom `juce::Component` subclasses to create a unique "Neon" aesthetic.

## Core Principles
- **Card-Centric Design:** Every UI component is encapsulated in a "Neon Card" - there are no floating controls.
- **JUCE-Native / Low Custom Code:** Maximize the use of JUCE subclasses (`juce::Slider`, `juce::Button`) and the `juce::LookAndFeel` system. Avoid low-level custom drawing where JUCE's standard methods can be overridden to achieve the look.
- **Consistency:** All cards feature a centralized 7-segment style numeric display for value readouts.
- **Scalability:** Flexible card widths (typically 1/4 panel width) for responsive layouts.

## Architecture Layers

### 1. The Theme System (External Config)
A centralized location for brand assets that doesn't depend on JUCE logic:
- **TOML Themes:** Human-readable `.toml` files (e.g., `classic_neon.toml`) that define hex colors for all components.
- `NeonTheme`: A C++ struct that mirrors the TOML structure.
- `NeonLayout`: Standard padding, 5px card corner radius, 3px bar corner radius.

### 2. Custom LookAndFeel (`NeonLookAndFeel.h/cpp`)
Overrides JUCE's default drawing methods with a focus on Card styles:
- `drawLinearSlider()` (Primary drawing for Parameter Card bars)
- `drawRotarySlider()` (For specific accent knobs)
- `drawButtonBackground()`
- `drawTickBox()`

### 3. Component Library (Atom/Molecule Level)
Custom subclasses of `juce::Component` that wrap JUCE widgets with brand-specific behavior:
- **NeonParameterCard**: The primary interaction module (Wraps a horizontal slider).
- **NeonModuleSelectionButton**: Navigational blocks following Hydrasynth/ESQ-1 aesthetics.
- **NeonKnob**: Standard rotary control (used for specific master or macro controls).
- **NeonButton**: High-contrast trigger or toggle buttons.
- **NeonLevelMeter**: Custom visualization component.

### 4. Synth Module Base Classes
Reusable functional blocks combining multiple components:
- `NeonSelectionPanel` (The structural heart of the UI navigation)
- `NeonSynthModule` (Base class for all modules)
- `OscillatorModule`
- `FilterModule`
- `EnvelopeModule`
- `AmplifierModule`
- `EffectModule`
- `MatrixRouterModule`

## Module Structure (Planned)
The library will be structured as a JUCE Module to allow easy inclusion in multiple projects.
- `neon_ui_components/`
  - `neon_ui_components.h` (Module header)
  - `core/` (LookAndFeel and Design System)
  - `widgets/` (Individual controls)
  - `modules/` (Composite synth sections)

