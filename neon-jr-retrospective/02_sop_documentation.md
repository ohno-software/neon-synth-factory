# Neon Jr. - Standard Operating Procedure (SOP)

## Table of Contents

1. [Project Setup](#project-setup)
2. [File Organization](#file-organization)
3. [Coding Standards](#coding-standards)
4. [Component Development](#component-development)
5. [Audio Processing](#audio-processing)
6. [UI Development](#ui-development)
7. [Build System](#build-system)
8. [Testing Guidelines](#testing-guidelines)

---

## Project Setup

### Initial Project Structure

```cpp
// Recommended directory structure
project-root/
├── CMakeLists.txt
├── source/
│   ├── PluginProcessor.h
│   ├── PluginProcessor.cpp
│   ├── PluginEditor.h
│   ├── PluginEditor.cpp
│   └── SignalPath.h
│       └── SignalPath.cpp
├── neon-components/
│   ├── source/
│   │   ├── neon_ui_components.h
│   │   ├── neon_ui_components.cpp
│   │   ├── core/
│   │   ├── modules/
│   │   └── widgets/
└── waves-1/
    └── *.wav
```

### CMake Configuration Template

```cmake
cmake_minimum_required(VERSION 3.22)

project(YourProjectName VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Paths to dependencies
set(JUCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../JUCE)
set(COMPONENTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../neon-components)

# Add JUCE
add_subdirectory(${JUCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/JUCE)

# Add custom components
add_subdirectory(${COMPONENTS_DIR} ${CMAKE_CURRENT_BINARY_DIR}/neon-components)

# Create the Plugin
juce_add_plugin(YourPluginName
    PRODUCT_NAME "Your Plugin"
    COMPANY_NAME "Your Company"
    BUNDLE_ID com.yourcompany.yourplugin
    PLUGIN_MANUFACTURER_CODE YC
    PLUGIN_CODE YP
    VST2_ID YP2
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_RESIZABLE TRUE
    FORMATS VST3 Standalone
    STANDALONE_MENU_BAR TRUE
)

target_sources(YourPluginName
    PRIVATE
        source/PluginProcessor.cpp
        source/PluginEditor.cpp
        source/SignalPath.cpp
)

target_link_libraries(YourPluginName
    PRIVATE
        neon_ui_components
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_graphics
        juce::juce_audio_basics
        juce::juce_audio_formats
        juce::juce_audio_devices
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
)

target_compile_definitions(YourPluginName
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_STANDALONE_FILTER_WINDOW_USE_NAVIGATIONS_BUTTONS=1
)

# Standard Juce aide setup
juce_generate_juce_header(YourPluginName)
```

---

## File Organization

### Header File Structure

```cpp
#pragma once

// Include order: JUCE headers → project headers → local headers

#include <juce_audio_processors/juce_audio_processors.h>
#include "SignalPath.h"

namespace neon
{
    class NeonJrAudioProcessor : public juce::AudioProcessor
    {
    public:
        NeonJrAudioProcessor();
        ~NeonJrAudioProcessor() override;

        // Required overrides
        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void releaseResources() override;
        void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
        juce::AudioProcessorEditor* createEditor() override;
        bool hasEditor() const override { return true; }

        // Optional overrides
        const juce::String getName() const override { return "Your Plugin"; }
        bool acceptsMidi() const override { return true; }
        bool producesMidi() const override { return false; }
        bool isMidiEffect() const override { return false; }
        double getTailLengthSeconds() const override { return 0.0; }

        // Custom methods
        SignalPath& getSignalPath() { return signalPath; }
        juce::MidiKeyboardState& getKeyboardState() { return keyboardState; }

        std::atomic<bool> midiActivity{ false };

    private:
        SignalPath signalPath;
        juce::MidiKeyboardState keyboardState;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeonJrAudioProcessor)
    };
}
```

### Implementation File Structure

```cpp
#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace neon
{
    NeonJrAudioProcessor::NeonJrAudioProcessor()
        : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
    {
    }

    NeonJrAudioProcessor::~NeonJrAudioProcessor()
    {
    }

    void NeonJrAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        signalPath.prepareToPlay(samplesPerBlock, sampleRate);
    }

    void NeonJrAudioProcessor::releaseResources()
    {
        signalPath.releaseResources();
    }

    void NeonJrAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
    {
        juce::ScopedNoDenormals noDenormals;
        auto totalNumInputChannels = getTotalNumInputChannels();
        auto totalNumOutputChannels = getTotalNumOutputChannels();

        for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
            buffer.clear(i, 0, buffer.getNumSamples());

        // Sync keyboard state to midi buffer
        keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

        // Get Playhead info for LFO sync
        if (auto* ph = getPlayHead())
        {
            if (auto pos = ph->getPosition())
            {
                if (auto bpmValue = pos->getBpm())
                    signalPath.setBpm(*bpmValue);
            }
        }

        // Process MIDI
        if (!midiMessages.isEmpty())
            midiActivity = true;

        for (const auto metadata : midiMessages)
        {
            const auto msg = metadata.getMessage();
            if (msg.isNoteOn())
                signalPath.noteOn(msg.getNoteNumber(), msg.getFloatVelocity());
            else if (msg.isNoteOff())
                signalPath.noteOff(msg.getNoteNumber());
            else if (msg.isPitchWheel())
                signalPath.setPitchWheel(juce::jmap((float)msg.getPitchWheelValue(), 0.0f, 16383.0f, -1.0f, 1.0f));
            else if (msg.isAftertouch())
                signalPath.setAftertouch(msg.getAfterTouchValue() / 127.0f);
            else if (msg.isChannelPressure())
                signalPath.setAftertouch(msg.getChannelPressureValue() / 127.0f);
            else if (msg.isController())
            {
                if (msg.getControllerNumber() == 1) // Mod Wheel
                    signalPath.setModWheel(msg.getControllerValue() / 127.0f);
            }
        }

        juce::AudioSourceChannelInfo info(&buffer, 0, buffer.getNumSamples());
        signalPath.getNextAudioBlock(info);
    }

    juce::AudioProcessorEditor* NeonJrAudioProcessor::createEditor()
    {
        return new NeonJrAudioProcessorEditor(*this);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new neon::NeonJrAudioProcessor();
}
```

---

## Coding Standards

### Naming Conventions

| Type | Convention | Example |
|------|------------|---------|
| Classes | PascalCase | `NeonJrAudioProcessor` |
| Functions | camelCase | `prepareToPlay` |
| Variables | camelCase | `sampleRate` |
| Constants | UPPER_SNAKE_CASE | `numVoices` |
| Namespaces | lowercase | `namespace neon` |
| Private members | camelCase | `signalPath` |
| Protected members | camelCase | `audioProcessor` |

### Memory Management Rules

1. **Use smart pointers for ownership**
   ```cpp
   // Good
   auto module = std::make_unique<MyModule>();
   modules.add(std::move(module));
   
   // Bad
   MyModule* module = new MyModule();
   modules.add(module);
   ```

2. **Use std::move for transferring ownership**
   ```cpp
   auto module = std::make_unique<MyModule>();
   modules.add(std::move(module));
   ```

3. **Avoid raw pointers when possible**
   ```cpp
   // Good
   ManagedParameter* param = registry.getParameter("Path");
   
   // Bad
   ManagedParameter* param = new ManagedParameter(...);
   ```

4. **Use JUCE's memory management helpers**
   ```cpp
   // Good
   juce::AudioBuffer<float> buffer(2, 512);
   
   // Bad
   float* buffer = new float[1024];
   ```

### JUCE-Specific Patterns

1. **Use JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR for classes with virtual methods**
   ```cpp
   class MyClass : public juce::Component {
       JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MyClass)
   };
   ```

2. **Use JUCE_DECLARE_NON_COPYABLE for non-virtual classes**
   ```cpp
   class MyClass {
       JUCE_DECLARE_NON_COPYABLE(MyClass)
   };
   ```

3. **Use juce::ScopedNoDenormals in audio processing**
   ```cpp
   void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override
   {
       juce::ScopedNoDenormals noDenormals;
       // ... processing code
   }
   ```

4. **Use juce::AudioBuffer<float> for audio data**
   ```cpp
   void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
   {
       auto* mainOutL = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
       auto* mainOutR = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
   }
   ```

5. **Use juce::MidiBuffer for MIDI data**
   ```cpp
   void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override
   {
       for (const auto metadata : midi)
       {
           const auto msg = metadata.getMessage();
           if (msg.isNoteOn())
               // ... handle note on
       }
   }
   ```

---

## Component Development

### ModuleBase Pattern

```cpp
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../widgets/NeonParameterCard.h"
#include "../core/NeonManagedParameter.h"
#include "../core/NeonColors.h"

namespace neon
{
    class ModuleBase : public juce::Component, public juce::Timer
    {
    public:
        ModuleBase(const juce::String& moduleName, const juce::Colour& accentColor);
        ~ModuleBase() override;

        void paint(juce::Graphics& g) override;
        void resized() override;
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
        void timerCallback() override;

        void setPage(int newPage);
        void addParameter(const juce::String& name, float min, float max, float def, bool isBool = false, float interval = 0.0f);
        void addChoiceParameter(const juce::String& name, const std::vector<juce::String>& choices, int defaultIndex);
        void updateChoiceParameter(const juce::String& name, const std::vector<juce::String>& choices);
        void addSpacer();
        
        void setMidiActive(bool active) { midiActive = active; }
        void setVoiceCount(int count) { voiceCountCount = count; }

        void loadPatch(const std::map<juce::String, float>& patchData);

    protected:
        virtual void paintVisualization(juce::Graphics& g, juce::Rectangle<int> area) {}
        virtual void handleVisualizationInteraction(const juce::MouseEvent& e, bool isDrag) {}

        juce::String moduleName;
        juce::Colour accentColor;
        bool midiActive = false;
        int voiceCountCount = 0;
        
        std::vector<ManagedParameter*> parameters;
        std::vector<std::unique_ptr<ParameterCard>> cards;
        
        juce::Label unitDisplay;
        juce::Label midiIndicator;
        juce::Label activeParamNameDisplay;
        juce::Label activeParamValueDisplay;
        juce::Label moduleNameDisplay;
        
        juce::TextButton prevButton, nextButton;
        int currentPage = 0;
        int lastAdjustedIndex = 0;
        int numPages = 1;

    private:
        void updatePageVisibility();
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModuleBase)
    };
}
```

### ManagedParameter Pattern

```cpp
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace neon
{
    class ManagedParameter
    {
    public:
        ManagedParameter(const juce::String& name, float min, float max, float def, bool isBool = false)
            : name(name), range(min, max), value(def), isBoolean(isBool)
        {}

        void setBinaryLabels(const juce::String& offLabel, const juce::String& onLabel)
        {
            binaryOffLabel = offLabel;
            binaryOnLabel = onLabel;
        }

        void setInterval(float newInterval)
        {
            range.interval = newInterval;
        }

        void setChoices(const std::vector<juce::String>& newChoices)
        {
            choices = newChoices;
            if (!choices.empty())
            {
                range.interval = 1.0f;
                range.start = 0.0f;
                range.end = (float)(choices.size() - 1);
            }
        }

        juce::String getChoiceLabel(int index) const
        {
            if (index >= 0 && index < (int)choices.size())
                return choices[(size_t)index];
            return juce::String(index);
        }

        bool hasChoices() const { return !choices.empty(); }
        const std::vector<juce::String>& getChoices() const { return choices; }

        juce::String getBinaryLabel(bool state) const
        {
            if (state) return binaryOnLabel.isNotEmpty() ? binaryOnLabel : "ON";
            return binaryOffLabel.isNotEmpty() ? binaryOffLabel : "OFF";
        }

        float getValue() const { return value; }
        void setValue(float newValue) { value = juce::jlimit(range.start, range.end, newValue); }
        
        juce::NormalisableRange<float> getRange() const { return range; }
        juce::String getName() const { return name; }
        bool getIsBoolean() const { return isBoolean; }

    private:
        juce::String name;
        juce::NormalisableRange<float> range;
        float value;
        bool isBoolean;
        juce::String binaryOffLabel, binaryOnLabel;
        std::vector<juce::String> choices;
    };
}
```

### ParameterRegistry Pattern

```cpp
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <map>
#include "NeonManagedParameter.h"

namespace neon
{
    class ParameterRegistry
    {
    public:
        static ParameterRegistry& getInstance();

        ManagedParameter* getOrCreateParameter(const juce::String& modulePath, const juce::String& name, float min, float max, float def, bool isBool = false, float interval = 0.0f)
        {
            auto fullPath = modulePath + "/" + name;
            if (parameters.count(fullPath))
                return parameters[fullPath].get();
            
            auto param = std::make_unique<ManagedParameter>(name, min, max, def, isBool);
            param->setInterval(interval);
            auto* ptr = param.get();
            parameters[fullPath] = std::move(param);
            return ptr;
        }

        ManagedParameter* getOrCreateChoiceParameter(const juce::String& modulePath, const juce::String& name, const std::vector<juce::String>& choices, int defaultIndex)
        {
            auto fullPath = modulePath + "/" + name;
            
            if (parameters.count(fullPath))
            {
                auto* existing = parameters[fullPath].get();
                if (name.contains("Target"))
                    existing->setChoices(choices);
                return existing;
            }
            
            auto param = std::make_unique<ManagedParameter>(name, 0.0f, (float)(choices.size() - 1), (float)defaultIndex, false);
            param->setChoices(choices);
            auto* ptr = param.get();
            parameters[fullPath] = std::move(param);
            return ptr;
        }

        ManagedParameter* getParameter(const juce::String& fullPath)
        {
            if (parameters.count(fullPath))
                return parameters[fullPath].get();
            return nullptr;
        }

        const std::map<juce::String, std::unique_ptr<ManagedParameter>>& getParameters() const
        {
            return parameters;
        }

    private:
        ParameterRegistry() = default;
        std::map<juce::String, std::unique_ptr<ManagedParameter>> parameters;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterRegistry)
    };
}
```

---

## Audio Processing

### SignalPath Pattern

```cpp
#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_dsp/juce_dsp.h>
#include <neon_ui_components/neon_ui_components.h>

namespace neon
{
    class SignalPath : public juce::AudioSource
    {
    public:
        SignalPath();
        ~SignalPath() override = default;

        void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
        void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
        void releaseResources() override;

        void noteOn(int midiNote, float velocity);
        void noteOff(int midiNote);
        void setPitchWheel(float value) { pitchWheel = value; }
        void setModWheel(float value) { modWheel = value; }
        void setAftertouch(float value) { aftertouch = value; }
        void setBpm(double newBpm) { bpm = newBpm; }

        float getPitchWheel() const { return pitchWheel; }
        float getModWheel() const { return modWheel; }
        float getAftertouch() const { return aftertouch; }
        
        int getActiveVoicesCount() const;

    private:
        void updateParams();
        void updateWavetables();
        float renderOscSample(float& phase, float actualFreq, const OscState& settings, const juce::AudioBuffer<float>& table);
        
        double sampleRate = 44100.0;
        int samplesPerBlock = 512;
        double bpm = 120.0;
        
        juce::AudioFormatManager formatManager;
        juce::Array<juce::File> waveformFiles;
        std::vector<juce::AudioBuffer<float>> wavetables;
        
        static constexpr int numVoices = 16;
        std::array<Voice, numVoices> voices;
        
        // Global Parameter Cache
        OscState globalOsc1, globalOsc2;
        std::array<LfoSettings, 3> globalLfos;
        int filterType = 0;
        float baseFilterCutoff = 20000.0f;
        float baseFilterRes = 0.0f;
        float baseFilterDrive = 1.0f;
        float filterKeyTrack = 0.5f;
        bool filterIs24dB = true;
        
        float filterEnvAmount = 0.0f;
        float pitchEnvAmount = 0.0f;
        float ampLevel = 0.8f;

        float pitchWheel = 0.0f;
        float modWheel = 0.0f;
        float aftertouch = 0.0f;
        float pbRange = 2.0f;
        
        float ampEnvMode = 0.0f;
        float filterEnvMode = 0.0f;
        float pitchEnvMode = 0.0f;
        float modEnvMode = 0.0f;
        
        juce::ADSR::Parameters ampParams, filterParams, pitchParams, modParams;

        struct ModSlot { float target = 0.0f; float amount = 0.0f; };
        ModSlot modSlots[4];

        struct CtrlSlot { float source = 0.0f; float target = 0.0f; float amount = 0.0f; };
        std::array<CtrlSlot, 8> ctrlSlots;
        
        ParameterRegistry& registry;
        juce::AudioBuffer<float> tempBuffer;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SignalPath)
    };
}
```

### Voice Management Pattern

```cpp
struct Voice
{
    int midiNote = -1;
    float velocity = 0.0f;
    bool isActive = false;
    double noteOnTime = 0.0;

    OscState osc1, osc2;
    std::array<LfoState, 3> lfos;
    juce::ADSR ampEnv, filterEnv, pitchEnv, modEnv;
    juce::dsp::StateVariableTPTFilter<float> filter1;
    juce::dsp::StateVariableTPTFilter<float> filter2;

    // Decimation cache for CPU optimization
    float u1Freqs[8], u1GainsL[8], u1GainsR[8];
    float u2Freqs[8], u2GainsL[8], u2GainsR[8];
    float mOsc1Pitch = 0, mOsc2Pitch = 0;
    float mOsc1Sym = 0, mOsc1Fold = 0, mOsc1Drive = 0, mOsc1Bit = 0, mOsc1Lvl = 0, mOsc1Pan = 0;
    float mOsc2Sym = 0, mOsc2Fold = 0, mOsc2Drive = 0, mOsc2Bit = 0, mOsc2Lvl = 0, mOsc2Pan = 0;
    float mOsc1Det = 0, mOsc2Det = 0;
    float mFiltCut = 0, mFiltRes = 0;
    float mLfoAmts[3][4] = {{0.0f}};
    int decimationCounter = 0;

    void reset()
    {
        for (int i = 0; i < 8; ++i)
        {
            osc1.phases[i] = 0.0f;
            osc2.phases[i] = 0.0f;
        }
        for (auto& l : lfos) l.reset(0.0f);
        
        ampEnv.reset();
        filterEnv.reset();
        pitchEnv.reset();
        modEnv.reset();
        filter1.reset();
        filter2.reset();
        
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 4; ++j)
                mLfoAmts[i][j] = 0.0f;

        isActive = false;
        midiNote = -1;
    }
};
```

---

## UI Development

### PluginEditor Pattern

```cpp
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <neon_ui_components/neon_ui_components.h>
#include "PluginProcessor.h"

namespace neon
{
    class NeonJrAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
    {
    public:
        NeonJrAudioProcessorEditor(NeonJrAudioProcessor&);
        ~NeonJrAudioProcessorEditor() override;

        void paint(juce::Graphics& g) override;
        void resized() override;
        void timerCallback() override;

        void setActiveModule(int index);

    private:
        NeonJrAudioProcessor& audioProcessor;

        LookAndFeel lookAndFeel;
        
        NeonDebugPanel debugPanel;
        ModuleSelectionPanel topSelectionPanel;
        ModuleSelectionPanel bottomSelectionPanel;
        juce::OwnedArray<ModuleBase> modules;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeonJrAudioProcessorEditor)
    };
}
```

### Module Creation Pattern

```cpp
NeonJrAudioProcessorEditor::NeonJrAudioProcessorEditor(NeonJrAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&lookAndFeel);

    addAndMakeVisible(topSelectionPanel);
    addAndMakeVisible(bottomSelectionPanel);
    
    // Modules
    auto osc1 = std::make_unique<OscillatorModule>("Oscillator 1", juce::Colours::orange);
    auto osc2 = std::make_unique<OscillatorModule>("Oscillator 2", juce::Colours::orange);
    auto filter = std::make_unique<LadderFilterModule>("Ladder Filter", juce::Colours::limegreen);
    auto amp = std::make_unique<AmpModule>("Amp Output", juce::Colours::indianred);
    
    auto envPitch = std::make_unique<DahdsrModule>("Pitch Env", juce::Colours::magenta, true);
    auto envFilter = std::make_unique<DahdsrModule>("Filter Env", juce::Colours::limegreen, true);
    auto envMod = std::make_unique<DahdsrModule>("Mod Env", juce::Colours::yellow, false);
    auto envAmp = std::make_unique<DahdsrModule>("Amp Env", juce::Colours::cyan, false);

    auto lfo1 = std::make_unique<LfoModule>("LFO 1", juce::Colours::yellow);
    auto lfo2 = std::make_unique<LfoModule>("LFO 2", juce::Colours::yellow);
    auto lfo3 = std::make_unique<LfoModule>("LFO 3", juce::Colours::yellow);

    auto modMatrix = std::make_unique<ModMatrixModule>("Mod", juce::Colours::white);
    auto ctrlSources = getCtrlSourceNames();
    auto targets = getModTargetNames();

    for (int i = 1; i <= 8; ++i)
    {
        modMatrix->addChoiceParameter("Slot " + juce::String(i) + " Source", ctrlSources, 0);
        modMatrix->addChoiceParameter("Slot " + juce::String(i) + " Target", targets, 0);
        modMatrix->addParameter("Slot " + juce::String(i) + " Amount", -100.0f, 100.0f, 0.0f);
        modMatrix->addSpacer();
    }

    auto ctrlModule = std::make_unique<ControlModule>("Control", juce::Colours::cyan);

    modules.add(std::move(osc1));
    modules.add(std::move(osc2));
    modules.add(std::move(filter));
    modules.add(std::move(amp));
    modules.add(std::move(envPitch));
    modules.add(std::move(envFilter));
    modules.add(std::move(envMod));
    modules.add(std::move(envAmp));
    modules.add(std::move(lfo1));
    modules.add(std::move(lfo2));
    modules.add(std::move(lfo3));
    modules.add(std::move(modMatrix));
    modules.add(std::move(ctrlModule));

    topSelectionPanel.setModuleNames({
        "OSC 1", "OSC 2", "FILTER", "AMP", "P-ENV", "F-ENV", "M-ENV", "A-ENV"
    });

    bottomSelectionPanel.setModuleNames({
        "LFO 1", "LFO 2", "LFO 3", "MOD", "CTRL"
    });

    for (auto* m : modules)
        addChildComponent(m);

    topSelectionPanel.onModuleChanged = [this](int index) { setActiveModule(index); };
    bottomSelectionPanel.onModuleChanged = [this](int index) { setActiveModule(index + 8); };

    setActiveModule(0);
    setSize(940, 840);
    startTimerHz(30);
}
```

---

## Build System

### CMake Best Practices

1. **Use juce_add_plugin for plugin creation**
   ```cmake
   juce_add_plugin(YourPluginName
       PRODUCT_NAME "Your Plugin"
       COMPANY_NAME "Your Company"
       BUNDLE_ID com.yourcompany.yourplugin
       PLUGIN_MANUFACTURER_CODE YC
       PLUGIN_CODE YP
       VST2_ID YP2
       IS_SYNTH TRUE
       NEEDS_MIDI_INPUT TRUE
       NEEDS_MIDI_OUTPUT FALSE
       IS_MIDI_EFFECT FALSE
       EDITOR_RESIZABLE TRUE
       FORMATS VST3 Standalone
       STANDALONE_MENU_BAR TRUE
   )
   ```

2. **Properly link required JUCE modules**
   ```cmake
   target_link_libraries(YourPluginName
       PRIVATE
           neon_ui_components
           juce::juce_gui_basics
           juce::juce_gui_extra
           juce::juce_graphics
           juce::juce_audio_basics
           juce::juce_audio_formats
           juce::juce_audio_devices
           juce::juce_audio_utils
           juce::juce_audio_processors
           juce::juce_dsp
   )
   ```

3. **Set compile definitions for platform-specific features**
   ```cmake
   target_compile_definitions(YourPluginName
       PUBLIC
           JUCE_VST3_CAN_REPLACE_VST2=0
           JUCE_STANDALONE_FILTER_WINDOW_USE_NAVIGATIONS_BUTTONS=1
   )
   ```

4. **Use juce_generate_juce_header for JUCE-generated code**
   ```cmake
   juce_generate_juce_header(YourPluginName)
   ```

---

## Testing Guidelines

### Unit Testing

1. **Test individual components in isolation**
   ```cpp
   TEST_CASE("ManagedParameter", "[Parameter]")
   {
       ManagedParameter param("Test", 0.0f, 1.0f, 0.5f);
       CHECK(param.getValue() == 0.5f);
       param.setValue(1.0f);
       CHECK(param.getValue() == 1.0f);
   }
   ```

2. **Use parameter registry for testing**
   ```cpp
   auto& registry = ParameterRegistry::getInstance();
   auto* param = registry.getOrCreateParameter("TestModule", "TestParam", 0.0f, 1.0f, 0.5f);
   ```

3. **Mock external dependencies**
   ```cpp
   // Use mocks for external dependencies
   ```

4. **Test edge cases and error conditions**
   ```cpp
   // Test boundary conditions
   // Test invalid inputs
   // Test error handling
   ```

### Integration Testing

1. **Test module interactions**
   ```cpp
   // Test parameter updates
   // Test voice management
   // Test audio processing pipeline
   ```

2. **Test parameter updates**
   ```cpp
   // Test parameter changes propagate correctly
   // Test parameter validation
   ```

3. **Test voice management**
   ```cpp
   // Test voice stealing
   // Test voice lifecycle
   // Test polyphony
   ```

4. **Test audio processing pipeline**
   ```cpp
   // Test audio output
   // Test MIDI input
   // Test real-time performance
   ```

### Performance Testing

1. **Monitor CPU usage**
   ```cpp
   // Profile critical sections
   // Optimize hot paths
   ```

2. **Test with maximum polyphony**
   ```cpp
   // Test voice count limits
   // Test memory usage
   ```

3. **Profile critical sections**
   ```cpp
   // Use profiling tools
   // Identify bottlenecks
   // Optimize accordingly
   ```

4. **Optimize hot paths**
   ```cpp
   // Use decimation for expensive operations
   // Cache frequently accessed data
   // Use efficient algorithms
   ```

---

## Version History

- **v1.0** (2026-01-31): Initial SOP documentation based on neon-jr project