#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace neon
{
    /**
     * NeonBar
     * A custom slider subclass for the 45px tall horizontal parameter bars.
     * Handles the specific click-toggle logic for binary parameters.
     */
    class NeonBar : public juce::Slider
    {
    public:
        NeonBar (bool isBinary = false);
        ~NeonBar() override = default;

        void mouseDown (const juce::MouseEvent& e) override;
        void mouseDrag (const juce::MouseEvent& e) override;

    private:
        bool isBinaryMode;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeonBar)
    };
}
