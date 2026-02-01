# Neon Jr. - DRY Principles & JUCE Working Patterns

## Overview

This document captures the Design, Refactoring, and Yields (DRY) principles and working patterns established in the neon-jr project. These patterns serve as the foundation for the Neon Synth Factory project's SOP (Standard Operating Procedure).

## Project Structure

```
neon-jr/
├── CMakeLists.txt          # Build configuration
├── source/
│   ├── PluginProcessor.h/cpp    # Audio processing core
│   ├── PluginEditor.h/cpp       # UI implementation
│   └── SignalPath.h/cpp         # Signal routing & DSP
└── waves-1/                  # Wavetable samples
```

## Core Principles

### 1. Namespace Organization
- Use `namespace neon` consistently for all custom classes
- Keep JUCE classes in their original namespaces
- Prevents naming conflicts and improves code organization

### 2. Singleton Pattern for Global State
- Use singletons for globally accessible resources
- Example: `ParameterRegistry` for centralized parameter management
- Ensures consistent access across components

### 3. Component-Based Architecture
- Clear separation between concerns
- Reusable base classes for common functionality
- Consistent inheritance hierarchies

### 4. Memory Management
- Use `std::unique_ptr` for ownership semantics
- Avoid manual memory management
- Leverage RAII principles

### 5. JUCE Best Practices
- Use JUCE's built-in memory management helpers
- Proper use of JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR
- Follow JUCE coding conventions

## Key Patterns

### Pattern 1: Parameter Registry Pattern

**Purpose**: Centralized parameter management for UI and DSP

**Implementation**:
```cpp
// Core concept
class ParameterRegistry {
    static ParameterRegistry& getInstance();
    ManagedParameter* getOrCreateParameter(...);
    ManagedParameter* getParameter(...);
};
```

**Benefits**:
- Single source of truth for parameters
- Easy access from any component
- Centralized parameter updates
- Debugging support

### Pattern 2: Module Base Class

**Purpose**: Consistent module structure with common functionality

**Implementation**:
```cpp
class ModuleBase : public juce::Component, public juce::Timer {
    // Common functionality
    // Virtual methods for customization
};
```

**Benefits**:
- Consistent UI behavior across modules
- Shared functionality (timer, parameter management)
- Easy to extend with new modules
- Clear interface for derived classes

### Pattern 3: Managed Parameter

**Purpose**: Lightweight parameter abstraction

**Implementation**:
```cpp
class ManagedParameter {
    juce::NormalisableRange<float> range;
    float value;
    std::vector<juce::String> choices;
    // ... getters and setters
};
```

**Benefits**:
- Separates value logic from UI representation
- Type-safe parameter handling
- Easy to extend with new features
- Supports both continuous and discrete parameters

### Pattern 4: Look and Feel Customization

**Purpose**: Consistent visual styling

**Implementation**:
```cpp
class NeonLookAndFeel : public juce::LookAndFeel_V4 {
    // Custom drawing methods
};
```

**Benefits**:
- Consistent visual identity
- Easy to maintain and update
- Reusable across components

### Pattern 5: Voice Management

**Purpose**: Polyphonic audio processing

**Implementation**:
```cpp
class SignalPath {
    static constexpr int numVoices = 16;
    std::array<Voice, numVoices> voices;
    // Voice management methods
};
```

**Benefits**:
- Clear voice lifecycle management
- Efficient voice stealing
- Consistent voice state
- Easy to scale

### Pattern 6: Audio Processing Pipeline

**Purpose**: Clean audio signal flow

**Implementation**:
```cpp
class SignalPath : public juce::AudioSource {
    void prepareToPlay(...) override;
    void getNextAudioBlock(...) override;
    void releaseResources() override;
};
```

**Benefits**:
- Clear separation of concerns
- Proper resource management
- Consistent audio processing pattern
- Easy to debug and optimize

## Coding Standards

### File Organization
1. Header files (.h) contain declarations
2. Implementation files (.cpp) contain implementations
3. Use `#pragma once` for header guards
4. Include order: JUCE headers → project headers

### Naming Conventions
1. Classes: PascalCase (e.g., `NeonJrAudioProcessor`)
2. Functions: camelCase (e.g., `prepareToPlay`)
3. Variables: camelCase (e.g., `sampleRate`)
4. Constants: UPPER_SNAKE_CASE (e.g., `numVoices`)
5. Namespaces: lowercase (e.g., `namespace neon`)

### Memory Management
1. Use `std::unique_ptr` for ownership
2. Use `std::make_unique` for allocation
3. Use `std::move` for transferring ownership
4. Avoid raw pointers when possible

### JUCE-Specific Patterns
1. Use `JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR` for classes with virtual methods
2. Use `JUCE_DECLARE_NON_COPYABLE` for non-virtual classes
3. Use `juce::ScopedNoDenormals` in audio processing
4. Use `juce::AudioBuffer<float>` for audio data
5. Use `juce::MidiBuffer` for MIDI data

## Build System

### CMake Configuration
1. Use `juce_add_plugin` for plugin creation
2. Properly link required JUCE modules
3. Set compile definitions for platform-specific features
4. Use `juce_generate_juce_header` for JUCE-generated code

### Module Dependencies
1. Clearly define module dependencies
2. Use relative paths for subdirectories
3. Properly set up include directories
4. Link against required libraries

## Testing Strategy

### Unit Testing
1. Test individual components in isolation
2. Use parameter registry for testing
3. Mock external dependencies
4. Test edge cases and error conditions

### Integration Testing
1. Test module interactions
2. Test parameter updates
3. Test voice management
4. Test audio processing pipeline

### Performance Testing
1. Monitor CPU usage
2. Test with maximum polyphony
3. Profile critical sections
4. Optimize hot paths

## Documentation Standards

### Code Comments
1. Use Doxygen-style comments for public APIs
2. Explain complex algorithms
3. Document parameter meanings
4. Include usage examples

### Inline Documentation
1. Explain non-obvious logic
2. Document parameter ranges
3. Explain design decisions
4. Note performance considerations

## Future Improvements

### Potential Enhancements
1. Add more comprehensive error handling
2. Implement parameter undo/redo
3. Add more advanced modulation options
4. Improve voice stealing algorithm
5. Add more visualization options

### Refactoring Opportunities
1. Extract common parameter handling code
2. Consolidate similar module types
3. Improve code reusability
4. Enhance type safety
5. Add more comprehensive tests

## References

- JUCE Documentation: https://juce.com/doc/
- JUCE Module Format: https://juce.com/doc/JUCEModuleFormat
- JUCE Audio Processing: https://juce.com/doc/JUCEAudioProcessing
- JUCE GUI Components: https://juce.com/doc/JUCEGUIComponents

## Version History

- **v1.0** (2026-01-31): Initial documentation of DRY principles and JUCE patterns