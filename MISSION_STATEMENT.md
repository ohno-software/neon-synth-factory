# Neon Synth Factory — Mission Statement

## Mission
Neon Synth Factory exists to rapidly create novel, high-quality synthesizers with a unified Neon identity by combining a reusable product template (`neon-template`), shared UI infrastructure (`neon-components`), and a consistent JUCE-based build and release workflow.

## Primary Architecture Directive (Most Important)
- All synth controls must write to a central parameter state (JUCE parameter system / equivalent parameter table object).
- Signal flow / DSP must read parameter values from that central state during audio processing.
- Controls must not modify DSP blocks or signal flow directly.
- This rule is mandatory for all current and future synths.

## Core Intent
- Build a family of distinct instruments that share one recognizable look, feel, and interaction model.
- Preserve branding coherence across all products while allowing each synth to implement its own engine architecture, DSP topology, and feature set.
- Maintain fast iteration through standardized project structure, cloning workflow, and centralized artifacts.

## Product and Platform Requirements (Non-Negotiable)
- Every synth must ship in both formats:
  - **VST3** (plugin for DAWs)
  - **Standalone** using the **JUCE native standalone wrapper**
- All synths must be buildable from the factory workspace using the shared build system.
- New synth projects are created by cloning `neon-template` (the template is not modified directly for product-specific work).

## Reusable Component Strategy
`neon-components` is the shared component library for all Neon Synth Factory instruments and serves as the primary mechanism for:
- Consistent visual language and behavior
- Shared themes/look-and-feel
- Reusable widgets and module building blocks
- Reduced duplication and easier cross-product evolution

## Architectural Flexibility
Within the shared UX/brand shell, each synth is free to define its own:
- Signal path and synthesis method
- Modulation architecture and routing
- Voice behavior, module composition, and control mapping

This balances **consistency** (brand/system) with **creative differentiation** (engine/design).

## Hardware Roadmap Commitment
Neon Synth Factory is designed with an eventual hardware path in mind, including Raspberry Pi and other SBC targets. The direction includes:
- Touchscreen-capable UI behavior
- Physical button/knob integration (including MIDI learn and mapping workflows)
- Performance-conscious, portable JUCE/C++ architecture suitable for embedded deployment

## Quality and Evolution Principles
- Keep JUCE-native patterns wherever practical to maximize maintainability and portability.
- Treat shared components and themes as platform assets, not one-off product code.
- Keep build outputs organized and consistent for testing, distribution, and release readiness.
- Evolve the factory as a coherent ecosystem: one brand, many synths, shared standards.
- Enforce a strict UI→parameter-state→DSP flow contract across all implementations.

## Scope of This Statement
This mission consolidates direction from the factory documentation and planning set, including:
- Factory introduction and ecosystem overview
- Unified build and artifact standards
- `neon-template` usage and cloning model
- `neon-components` role in shared UX system
- Platform strategy for desktop now and SBC hardware later
