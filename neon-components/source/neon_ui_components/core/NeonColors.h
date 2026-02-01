#pragma once

#include <juce_graphics/juce_graphics.h>

namespace neon
{
    /**
     * Theme structure for the Neon brand.
     * These values are typically populated from .toml files in the themes/ directory.
     * See planning docs / 08_themes.md
     */
    struct Theme
    {
        // Global
        juce::Colour background;
        juce::Colour cardBackground;
        juce::Colour barBackground;
        juce::Colour textDim;
        juce::Colour textBright;

        // Module Accents
        juce::Colour oscillator;
        juce::Colour mutator;
        juce::Colour filter;
        juce::Colour envelope;
        juce::Colour amplifier;
        juce::Colour modulation;
        juce::Colour effects;
        
        juce::Colour indicator;
    };
    
    // Default fallback colors (Classic Neon)
    struct Colors
    {
        static inline const juce::Colour background      { 0xff1a1a1a };
        static inline const juce::Colour cardBackground  { 0xff2a2a2a };
        static inline const juce::Colour barBackground   { 0xff404040 };
        static inline const juce::Colour textDim         { 0xffcccccc };
        static inline const juce::Colour textBright      { 0xffffffff };

        // Module Accents
        static inline const juce::Colour oscillator      { 0xff00aaff }; // Cyan
        static inline const juce::Colour mutator         { 0xff00aaff }; // Same as Osc
        static inline const juce::Colour filter          { 0xff00ffaa }; // Teal
        static inline const juce::Colour envelope        { 0xff00ff00 }; // Bright Green
        static inline const juce::Colour amplifier       { 0xffff4d00 }; // Volcanic Orange
        static inline const juce::Colour modulation      { 0xffffaa00 }; // Amber/Orange
        static inline const juce::Colour effects         { 0xff0088ff }; // Sky Blue
        
        static inline const juce::Colour indicator       { 0xffffffff }; // White
    };
}
