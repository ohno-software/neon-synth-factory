#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "NeonDebugPanel.h"

namespace neon
{
    /**
     * NeonDebugLauncher
     * A small "DBG" toggle button that can be parented to any editor.
     * Clicking it opens (or closes) a pop-out DocumentWindow containing a
     * live NeonDebugPanel reflecting the global ParameterRegistry.
     *
     * Usage in an editor:
     *     NeonDebugLauncher debugLauncher;
     *     // in constructor:
     *     addAndMakeVisible (debugLauncher);
     *     // in resized():
     *     debugLauncher.setBounds (getWidth() - 56, 8, 48, 22);
     */
    class NeonDebugLauncher : public juce::Component
    {
    public:
        NeonDebugLauncher()
        {
            button.setButtonText ("DBG");
            button.setClickingTogglesState (true);
            button.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xFF202020));
            button.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xFFFFCC00));
            button.setColour (juce::TextButton::textColourOffId,  juce::Colours::yellow);
            button.setColour (juce::TextButton::textColourOnId,   juce::Colours::black);
            button.setTooltip ("Toggle parameter debug panel");
            button.onClick = [this]
            {
                if (button.getToggleState())
                    showWindow();
                else
                    hideWindow();
            };

            addAndMakeVisible (button);
        }

        ~NeonDebugLauncher() override
        {
            hideWindow();
        }

        void resized() override
        {
            button.setBounds (getLocalBounds());
        }

        /** Programmatically open the pop-out window. */
        void showWindow()
        {
            if (debugWindow == nullptr)
                debugWindow.reset (new DebugWindow ([this] { onWindowClosed(); }));

            debugWindow->setVisible (true);
            debugWindow->toFront (true);
            button.setToggleState (true, juce::dontSendNotification);
        }

        /** Close and destroy the pop-out window. */
        void hideWindow()
        {
            debugWindow.reset();
            button.setToggleState (false, juce::dontSendNotification);
        }

    private:
        // Internal pop-out window that owns the panel.
        class DebugWindow : public juce::DocumentWindow
        {
        public:
            explicit DebugWindow (std::function<void()> onClose)
                : juce::DocumentWindow ("Neon Debug Parameters",
                                        juce::Colours::black,
                                        juce::DocumentWindow::closeButton),
                  onCloseCallback (std::move (onClose))
            {
                setUsingNativeTitleBar (true);
                setResizable (true, true);
                setContentOwned (new NeonDebugPanel(), false);
                centreWithSize (720, 540);
            }

            void closeButtonPressed() override
            {
                if (onCloseCallback)
                    onCloseCallback();
            }

        private:
            std::function<void()> onCloseCallback;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DebugWindow)
        };

        void onWindowClosed()
        {
            // Defer destruction so we don't delete the window from inside its own callback.
            juce::MessageManager::callAsync ([safe = juce::Component::SafePointer<NeonDebugLauncher> (this)]
            {
                if (safe != nullptr)
                    safe->hideWindow();
            });
        }

        juce::TextButton button;
        std::unique_ptr<DebugWindow> debugWindow;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeonDebugLauncher)
    };
}
