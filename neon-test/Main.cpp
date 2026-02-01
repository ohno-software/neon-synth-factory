#include <juce_gui_basics/juce_gui_basics.h>
#include <neon_ui_components/neon_ui_components.h>

class MainComponent : public juce::Component
{
public:
    MainComponent()
    {
        setLookAndFeel(&lookAndFeel);

        // 1. Setup Selection Panel (at the top)
        addAndMakeVisible (selectionPanel);
        selectionPanel.onModuleChanged = [this] (int index) { setActiveModule (index); };

        // 2. Setup Modules
        auto osc1 = std::make_unique<neon::OscillatorModule> ("Oscillator 1");
        auto env1 = std::make_unique<neon::DahdsrModule> ("Envelope 1");
        
        modules.add (std::move (osc1));
        modules.add (std::move (env1));

        for (auto* m : modules)
            addChildComponent (m);

        setActiveModule (0);
        
        setSize(1200, 850); // Slightly taller for the top bar
    }

    ~MainComponent() override
    {
        setLookAndFeel(nullptr);
    }

    void setActiveModule (int index)
    {
        for (int i = 0; i < modules.size(); ++i)
            modules[i]->setVisible (i == index);
            
        resized();
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(neon::Colors::background);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        
        // Top bar for selection (50px)
        selectionPanel.setBounds (bounds.removeFromTop (50));
        
        // Active module takes the rest
        for (auto* m : modules)
        {
            if (m->isVisible())
                m->setBounds (bounds);
        }
    }

private:
    neon::LookAndFeel lookAndFeel;
    neon::ModuleSelectionPanel selectionPanel;
    juce::OwnedArray<neon::ModuleBase> modules;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow(juce::String name)
        : DocumentWindow(name,
                         juce::Desktop::getInstance().getDefaultLookAndFeel()
                         .findColour(ResizableWindow::backgroundColourId),
                         DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setContentOwned(new MainComponent(), true);

        #if JUCE_IOS || JUCE_ANDROID
         setFullScreen(true);
        #else
         setResizable(true, true);
         centreWithSize(getWidth(), getHeight());
        #endif

        setVisible(true);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

class NeonTestApplication : public juce::JUCEApplication
{
public:
    NeonTestApplication() {}

    const juce::String getApplicationName() override { return "Neon Test"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& commandLine) override
    {
        mainWindow.reset(new MainWindow(getApplicationName()));
    }

    void shutdown() override
    {
        mainWindow.reset();
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(const juce::String& commandLine) override {}

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(NeonTestApplication)
