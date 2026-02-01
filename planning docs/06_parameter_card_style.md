# Neon Parameter Card Style Definition

**Version:** 1.0  
**Date:** January 27, 2026  
**Status:** Definitive Standard

## Overview

The Neon Parameter Card is the definitive UI pattern for displaying and editing synthesizer parameters. Each parameter is represented as a horizontal card with consistent layout, interaction, and visual feedback.

## Visual Layout

```
┌─────────────────────────────────────┐
│         PARAMETER NAME              │  ← Label (centered, uppercase)
│                                     │
│  ╔═══════════════════════╗          │  ← Value bar (45px height)
│  ║█████████████░░░░░░░░░░║          │     Filled portion shows value
│  ║         ●             ║          │     White circle indicator (10px)
│  ╚═══════════════════════╝          │
│                                     │
│         [ 1 2 8 . 0 ]               │  ← 7-Segment Pseudo Display
└─────────────────────────────────────┘

Card Width: ~1/4 of panel width (flexible, horizontal layout)
Card Height: Auto-fit content
```

## Dimensions & Spacing

### Card Structure
- **Global Rule:** ALL UI components (Knobs, Buttons, Sliders) must reside within this Card structure.
- **Total card width:** Approximately 1/4 of available panel width
- **Internal padding:** 5px all sides
- **Corner radius:** 5px
- **Background:** Semi-transparent dark (0xff1a1a1a)

### Parameter Name
- **Position:** Top of card, centered
- **Font size:** 14px
- **Font:** Bold, uppercase
- **Color:** White (0xffffffff)
- **Margin bottom:** 8px

### Value Bar Area
- **Total height:** 60px (vertical space reserved)
- **Bar background height:** 45px (actual drawable bar)
- **Vertical offset:** 20px reserved at bottom for numeric display
- **Corner radius:** 3px
- **Background color:** Dark gray (0xff404040)
- **Fill color:** Parameter-specific (bright, saturated colors)

### Value Indicator
- **Type:** Filled circle
- **Diameter:** 10px (increased from 6px for better visibility)
- **Color:** White (0xffffffff)
- **Position:** Centered vertically on bar, positioned horizontally at normalized value

### Value Text (7-Segment Display)
- **Position:** Below bar, centered
- **Font size:** 16px
- **Style:** Pseudo 7-segment LED display
- **Color:** Light gray (0xffcccccc) or dim version of parameter color
- **Format:** 
  - **Linear:** Parameter-specific (e.g., "45.0", "1.20 ms", "0.75")
  - **Binary:** " ON " / " OFF " or " YES " / " NO " (Fixed-width string for 7-seg alignment)
- **Layout:** Contained within a subtle "inset" box at the bottom of the card.

## Implementation Philosophy
- **Stay in the JUCE Ecosystem:** Use `juce::Slider` with `SliderStyle::LinearHorizontal` and `juce::Label`. 
- **Minimum Custom Code:** Achieved by overriding specific drawing functions in a single `NeonLookAndFeel` class.
- **No Complex Canvas Drawing:** Leverage JUCE's built-in components to handle accessibility, automation, and touch events automatically.

## Color Palette

Use bright, saturated colors for parameter bars to create visual distinction:

```cpp
// Recommended colors (JUCE Colour format)
Envelope:   0xff00ff00  // Bright green
Oscillator: 0xff00aaff  // Cyan blue
LFO:        0xffaa00ff  // Purple/magenta
Filter:     0xff00ffaa  // Teal/cyan
Delay:      0xff0088ff  // Sky blue
Reverb:     0xff0088ff  // Sky blue
Modulation: 0xffffaa00  // Orange/amber
Level/Mix:  0xffffaa00  // Orange/amber
Pan:        0xffff00aa  // Pink/magenta
Pitch:      0xff00ccff  // Light cyan
```

**Color Selection Rules:**
- Use colors from different parts of spectrum for adjacent parameters
- Maintain minimum 50% saturation for visibility
- Keep value (brightness) above 50% for dark backgrounds
- Avoid pure red (0xff0000) - reserve for errors/warnings

## Interaction Model

### Mouse Events

**Click:**
- **Linear Parameters:** Click anywhere on bar area to immediately jump to that value. Value updates instantly. Stores click position for relative dragging.
- **Binary Parameters (On/Off):** Click anywhere on the bar to toggle between 0.0 and 1.0. No dragging is initiated.
- Value updates instantly.

**Drag:**
- **Linear Parameters:** After clicking, drag left/right for relative adjustment. Movement is relative to initial click position. Smooth continuous value updates during drag.
- **Binary Parameters:** Dragging is disabled.

**Mouse Wheel:**
- Scroll wheel to adjust value (5% per tick for linear, toggle state for binary).
- Shift + scroll wheel for fine control (0.5% per tick).
- Works when hovering anywhere over the parameter card

**Visual Feedback:**
- No hover state (keep clean)
- Immediate value change is the feedback
- Smooth indicator movement (no animation, instant)

### Value Calculation

```cpp
void mouseDown(const juce::MouseEvent& event) override
{
    if (isBinary) // Toggle behavior
    {
        setNormalizedValue(normalizedValue > 0.5f ? 0.0f : 1.0f);
        if (onValueChanged) onValueChanged(normalizedValue);
        return;
    }

    // Linear behavior (Click & Drag)
    auto bounds = getLocalBounds().toFloat().reduced(8.0f);
    bounds.removeFromTop(bounds.getHeight() - 60.0f);
    auto barArea = bounds.removeFromBottom(45.0f);
    
    // Set value based on click position
    float clickValue = (event.position.x - barArea.getX()) / barArea.getWidth();
    clickValue = juce::jlimit(0.0f, 1.0f, clickValue);
    
    setNormalizedValue(clickValue);
    
    if (onValueChanged)
        onValueChanged(normalizedValue);
    
    // Store starting position for dragging
    dragStartValue = normalizedValue;
    dragStartX = event.position.x;
}

void mouseDrag(const juce::MouseEvent& event) override
{
    if (isBinary) return; // No dragging for toggle switches

    // Calculate relative movement from drag start
    auto bounds = getLocalBounds().toFloat().reduced(8.0f);
    bounds.removeFromTop(bounds.getHeight() - 60.0f);
    auto barArea = bounds.removeFromBottom(45.0f);
    
    float deltaX = event.position.x - dragStartX;
    float deltaNormalized = deltaX / barArea.getWidth();
    
    float newValue = juce::jlimit(0.0f, 1.0f, dragStartValue + deltaNormalized);
    
    setNormalizedValue(newValue);
    
    if (onValueChanged)
        onValueChanged(normalizedValue);
}
```

### Callback Pattern

```cpp
card->onValueChanged = [this](float normalized) 
{
    // Update parameter data model
    parameters->someParameter = normalized * maxValue;
    
    // Update display (value text)
    updateCardDisplay(cardIndex);
    
    // Optional: Update visualization
    visualization.setParameters(*parameters);
};
```

## Drawing Implementation

```cpp
void paint(juce::Graphics& g) override
{
    auto bounds = getLocalBounds().toFloat();
    
    // Card background
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRoundedRectangle(bounds.reduced(2.0f), 4.0f);
    
    // Calculate bar area (45px bar within 60px total space)
    auto barArea = bounds.reduced(8.0f);
    barArea.removeFromTop(barArea.getHeight() - 60.0f); 
    barArea.removeFromBottom(15.0f); 
    
    // Bar background
    g.setColour(juce::Colour(0xff404040));
    g.fillRoundedRectangle(barArea, 3.0f);
    
    // Filled portion
    float displayValue = normalizedValue;
    if (isBinary) {
        displayValue = (normalizedValue > 0.5f) ? 1.0f : 0.0f;
    }

    auto filledBar = barArea.withWidth(barArea.getWidth() * displayValue);
    g.setColour(barColour);
    g.fillRoundedRectangle(filledBar, 3.0f);
    
    // White circle indicator (10px diameter)
    float indicatorX = barArea.getX() + 
                       (barArea.getWidth() * displayValue);
    g.setColour(juce::Colours::white);
    g.fillEllipse(indicatorX - 5.0f, barArea.getCentreY() - 5.0f, 
                  10.0f, 10.0f);
}
```

## Usage Pattern

### Creating a Parameter Card

```cpp
// In module panel createParameterCards() method:
auto* card = addParameterCard("ATTACK", juce::Colour(0xff00ff00));
card->onValueChanged = [this](float normalized) 
{
    parameters->attack = normalized * 10000.0f; // 0-10000ms
    updateCardDisplay(0);
};
```

### Updating Card Display

```cpp
void updateCardDisplay(int cardIndex)
{
    auto* card = getCard(cardIndex);
    if (!card) return;
    
    // Example for attack parameter
    float normalized = parameters->attack / 10000.0f;
    card->setNormalizedValue(normalized);
    card->setValueText(juce::String((int)parameters->attack) + " ms");
}
```

### Panel Layout

Cards are arranged horizontally in a flex-box style layout:

```cpp
void resized() override
{
    auto bounds = getLocalBounds();
    
    // Display area (top section - envelope graph, waveform, etc.)
    auto displayArea = bounds.removeFromTop(getDisplayHeight());
    if (getDisplayComponent())
        getDisplayComponent()->setBounds(displayArea);
    
    // Parameter cards area (remaining space)
    auto cardsArea = bounds.reduced(10);
    
    int numCards = parameterCards.size();
    if (numCards > 0)
    {
        int cardWidth = cardsArea.getWidth() / numCards;
        
        for (auto& card : parameterCards)
        {
            card->setBounds(cardsArea.removeFromLeft(cardWidth)
                                    .reduced(5)); // 5px gap
        }
    }
}
```

## Complete Component Class

See reference implementation:
- **File:** `LeviaSynth/source/ParameterCard.h` (single template used by ALL parameters)
- **Base class:** `modulePanel.h` (provides addParameterCard() method)
- **Examples:** 
  - `EnvelopemodulePanel.h` (6 cards, 140px display height)
  - `LFOmodulePanel.h` (6 cards, 140px display height)
  - `OscillatormodulePanel.h` (6 cards, 180px display height)
  - `FiltermodulePanel.h` (5 cards, 180px display height)
  - `DelaymodulePanel.h` (4 cards, 150px display height)
  - `ReverbmodulePanel.h` (5 cards, 140px display height)

**Note:** All module panels use the exact same ParameterCard component. The only differences are the display visualization height and the colors assigned to each parameter for visual distinction.

## Design Rationale

1. **Horizontal layout:** Maximizes screen width usage, allows more parameters visible
2. **Thick bars (45px actual height):** Easy to click/drag on all devices, within 60px reserved space
3. **Large indicator (10px):** Clearly visible current position
4. **Direct manipulation:** Click anywhere on bar = instant value change, drag for relative adjustment
5. **Real-time feedback:** Value text updates during drag
6. **Visual hierarchy:** Name → Bar → Value (top to bottom reading)
7. **Color coding:** Bright saturated colors for instant parameter recognition
8. **Normalized values:** All parameters use 0.0-1.0 internally for consistency
9. **Standardized display heights:** Most module panels use 140px for visualization (envelope, LFO, reverb, delay), some use 180px (oscillator, filter) for more complex displays

## Accessibility

- **Minimum touch target:** 45px height meets WCAG AAA standards
- **High contrast:** White text/indicators on dark backgrounds
- **Color independence:** Parameter names always visible regardless of color blindness
- **Clear value display:** Always show numeric value, not just position

## Future Enhancements

Potential additions (not in current version):

- Right-click context menu for fine adjustment
- Shift+drag for fine control (10x slower)
- Double-click to reset to default
- MIDI learn indicator
- Modulation visualization (second bar showing modulated value)
- Animation for automated parameter changes

---

**© 2026 Neon H2O | All Rights Reserved**

