# Neon Synth Factory: Introduction 🎹🧪

Welcome to the **Neon Synth Factory**, a centralized ecosystem for developing high-performance JUCE-based synthesizers. This environment is designed for rapid iteration, shared component usage, and a unified building experience.

## The Vision
The factory serves as a laboratory where multiple "Neon" synthesizer variations can be developed in parallel while sharing a common UI and DSP foundation. By treating each synthesizer as a module within the larger factory, we ensure consistency and efficiency.

## Core Projects
*   **[neon-jr](../neon-jr)**: The core reference synth. Stable, reliable, and the blueprint for the factory standards.
*   **[neon-qwen](../neon-qwen)**: A modern synth prototype demonstrating the factory's sparse-folder build capability.
*   **[neon-components](../neon-components)**: The central library of UI elements and themes shared across all factory synths.

## Ecosystem Overview
The workspace is organized to separate the engine (JUCE), the tools (Build Script), the components (neon-components), and the products (the individual synths). This modularity allows us to update the UI globally without rebuilding every synth from scratch manually.
