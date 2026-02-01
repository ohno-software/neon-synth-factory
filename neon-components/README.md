# Neon UI Component Library

The **Neon UI Component Library** is a custom JUCE module containing the atomic, molecular, and organic UI elements for the **Neon Synth Factory** ecosystem.

## Structure
- **core/**: LookAndFeel overrides and centralized color/typography systems.
- **widgets/**: Individual UI atoms like the `NeonBar` and `NeonParameterCard`.
- **modules/**: Composite containers for synth sections (Oscillators, Filters, etc.) and the navigational block diagram.

## Design Inspiration
- **Hydrasynth**: Interaction models and block-diagram navigation.
- **Neon Retro Aesthetics**: High-contrast saturated colors (Cyan, Teal, Orange) on dark backgrounds.
- **Tactile Feedback**: Large click areas (45px bars) and digital 7-segment displays.

## Integration
This library is provided as a standard JUCE Module. To use it in your project:
1. Add the `neon_ui_components` module path to your Projucer or CMake project.
2. Include the module: `#include <neon_ui_components/neon_ui_components.h>`
3. Apply the global styling: `juce::LookAndFeel::setDefaultLookAndFeel (&neonLookAndFeel);`
