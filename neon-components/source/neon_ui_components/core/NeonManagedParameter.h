#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace neon
{
    /**
     * ManagedParameter
     * A lightweight abstraction for a parameter value.
     * Separates the value logic from the UI representation.
     */
    class ManagedParameter
    {
    public:
        ManagedParameter (const juce::String& name, float min, float max, float def, bool isBool = false, bool isLinear = false)
            : name (name), range ((double)min, (double)max), value (def), defaultValue (def), isBoolean (isBool)
        {
            if (!isBool && min < max)
            {
                if (isLinear)
                {
                    range.setSkewForCentre ((double)(min + (max - min) * 0.5f));
                    range.interval = 0.0; // Allow smooth linear
                }
                else if (min < 0.0f && max > 0.0f)
                {
                    // Symmetric or nearly symmetric bipolar range: map [0, 1] -> [min, max] with center (0.5) at 0.
                    // Skewed exponentially away from the center for finer control near zero.
                    range = juce::NormalisableRange<double> (
                        (double)min, (double)max,
                        [min, max] (double s, double e, double n) {
                            double u = n * 2.0 - 1.0;
                            double skewedU = std::abs (u) * std::abs (u) * (u < 0 ? -1.0 : 1.0);
                            if (skewedU < 0) return std::abs (skewedU) * (double)min;
                            return skewedU * (double)max;
                        },
                        [min, max] (double s, double e, double v) {
                            double u = (v < 0) ? -(v / (double)min) : (v / (double)max);
                            double nU = std::sqrt (std::abs (u)) * (u < 0 ? -1.0 : 1.0);
                            return (nU + 1.0) * 0.5;
                        }
                    );
                }
                else
                {
                    range.setSkewForCentre ((double)(min + (max - min) * 0.25f));
                }
            }
        }

        void setBinaryLabels (const juce::String& offLabel, const juce::String& onLabel)
        {
            binaryOffLabel = offLabel;
            binaryOnLabel = onLabel;
        }

        void setInterval (float newInterval)
        {
            range.interval = (double)newInterval;
        }

        void setChoices (const std::vector<juce::String>& newChoices)
        {
            choices = newChoices;
            if (!choices.empty())
            {
                range.interval = 1.0;
                range.start = 0.0;
                range.end = (double)(choices.size() - 1);
            }
        }

        juce::String getChoiceLabel (int index) const
        {
            if (index >= 0 && index < (int)choices.size())
                return choices[(size_t)index];
            return juce::String (index);
        }

        bool hasChoices() const { return !choices.empty(); }
        const std::vector<juce::String>& getChoices() const { return choices; }

        juce::String getBinaryLabel (bool state) const
        {
            if (state) return binaryOnLabel.isNotEmpty() ? binaryOnLabel : "ON";
            return binaryOffLabel.isNotEmpty() ? binaryOffLabel : "OFF";
        }

        float getValue() const { return value; }
        float getDefaultValue() const { return defaultValue; }
        void setValue (float newValue) { value = juce::jlimit ((float)range.start, (float)range.end, newValue); }
        
        void setIsMomentary (bool momentary) { isMomentary = momentary; }
        bool getIsMomentary() const { return isMomentary; }

        juce::NormalisableRange<double> getRange() const { return range; }
        juce::String getName() const { return name; }
        bool getIsBoolean() const { return isBoolean; }

    private:
        juce::String name;
        juce::NormalisableRange<double> range;
        float value;
        float defaultValue;
        bool isBoolean;
        bool isMomentary = false;
        juce::String binaryOffLabel, binaryOnLabel;
        std::vector<juce::String> choices;
    };
}
