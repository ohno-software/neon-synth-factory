# Neon Synth Factory - Component Inventory

## 1. Atoms (Basic Controls)
| Component | Base Class | Description |
| :--- | :--- | :--- |
| **NeonKnob** | `juce::Slider` | Standard rotary control for master or macro parameters. |
| **NeonBar** | `juce::Slider` | Horizontal bar component used within Parameter Cards (45px high). Supports click-drag (linear) or click-toggle (binary). |
| **NeonIndicator** | `juce::Component` | 10px white circle indicator for current bar value. |
| **NeonMomentaryButton**| `juce::Button` | High-contrast trigger/gate button. |
| **NeonToggleButton** | `juce::ToggleButton` | Latched state button with neon LED indicator. |
| **NeonPagerDot** | `juce::Button` | Small circle button for direct page access. Filled when active. |
| **NeonArrowButton** | `juce::Button` | Stylized triangle button (< or >) for page navigation. |

## 2. Molecules (Visualizers & Displays)
| Component | Description |
| :--- | :--- |
| **NeonParameterCard** | **The definitive interaction module.** Combines Label (14px Bold), NeonBar (Drag or Toggle), and pseudo 7-segment Value (16px). |
| **NeonModuleDisplay** | Base class for the visual representations (Waveform, Curve, etc.) in the top 2/3 of a module. |
| **NeonModuleSelectionButton** | Tactile block-diagram button used in the selection panel. |
| **NeonValueDisplay** | A numeric readout component that handles module formatting (Hz, ms, %, etc.). |
| **NeonLevelMeter** | Vertical amplitude meter with neon segments and peak holding. |
| **NeonMiniScope** | A real-time waveform visualizer styled with Neon accent colors. |
| **NeonXYPad** | A 2D XY-pad with optional physics (Spring/Inertia). |
| **NeonRibbon** | A horizontal touch-strip for expressive modulation. |
| **NeonDataGrid** | A high-performance list view for patches and MIDI maps. |
| **NeonPropertyField** | Standardized input for configuration (Dropdowns, Text, Sliders). |

## 3. Organisms (Synth Modules)
| Module | Components Included |
| :--- | :--- |
| **NeonSynthModule** | **The Master Module Container.** Layout: Top 2/3 NeonModuleDisplay, Bottom 1/3 2x4 Grid of Parameter Cards, Footer with Paging Controls. |
| **NeonSelectionPanel** | **The Navigation Hub.** Block diagram of the synth's signal flow with `NeonModuleSelectionButton` elements. |
| **Oscillator Module** | Custom `NeonSynthModule` with Waveform Display. |
| **Mutator Module** | Custom `NeonSynthModule` for FM/Sync/Warp with Comparison Display. |
| **Mixer Module** | Custom `NeonSynthModule` for source balancing and pre-filter Drive. |
| **Filter Module** | Custom `NeonSynthModule` with Curve Display. |
| **Envelope Module** | Custom `NeonSynthModule` with ADSR Graph Display. |
| **Amplifier Module** | Custom `NeonSynthModule` with Level/Vector Scope Display. |
| **Matrix Router** | Custom `NeonSynthModule` with Modulation Flow visualization. |
| **Effect Module** | Custom `NeonSynthModule` with procedural Effect visualization. |

## 4. Layout Templates
- **Sidebar Navigation:** Preset browser and global settings.
- **Main Rack:** Scrollable area for synth modules.
- **Footer:** Virtual keyboard, master volume, and CPU meter.

