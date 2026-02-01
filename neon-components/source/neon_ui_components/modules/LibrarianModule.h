#pragma once

#include "ModuleBase.h"
#include "../core/NeonPatchManager.h"

namespace neon
{
    class LibrarianModule : public ModuleBase, public juce::ListBoxModel
    {
    public:
        LibrarianModule (const juce::String& name, const juce::Colour& color) 
            : ModuleBase (name, color)
        {
            // Patch controls
            patchNameEditor.setJustification (juce::Justification::centred);
            patchNameEditor.setFont (juce::FontOptions ().withHeight (30.0f).withStyle ("Bold"));
            patchNameEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colours::black.withAlpha (0.3f));
            patchNameEditor.setColour (juce::TextEditor::textColourId, juce::Colours::white);
            patchNameEditor.setText (NeonRegistry::getCurrentPatchName());
            patchNameEditor.onReturnKey = [this] { performSave (false); };
            addAndMakeVisible (patchNameEditor);

            loadBtn.setButtonText ("LOAD");
            loadBtn.onClick = [this] { 
                PatchManager::getInstance().loadPatch (patchList.getSelectedRow()); 
                updateUI();
            };
            addAndMakeVisible (loadBtn);

            saveBtn.setButtonText ("SAVE");
            saveBtn.onClick = [this] { performSave (false); };
            addAndMakeVisible (saveBtn);

            saveAsBtn.setButtonText ("SAVE AS");
            saveAsBtn.onClick = [this] { performSave (true); };
            addAndMakeVisible (saveAsBtn);

            renameBtn.setButtonText ("RENAME");
            renameBtn.onClick = [this] { patchNameEditor.grabKeyboardFocus(); };
            addAndMakeVisible (renameBtn);

            initBtn.setButtonText ("INIT");
            initBtn.onClick = [this] { 
                PatchManager::getInstance().initPatch(); 
                updateUI();
            };
            addAndMakeVisible (initBtn);

            // Bank controls
            bankSelector.onChange = [this] { 
                PatchManager::getInstance().selectBank (bankSelector.getSelectedItemIndex());
                updateUI();
            };
            addAndMakeVisible (bankSelector);

            newBankBtn.setButtonText ("NEW BANK");
            newBankBtn.onClick = [this] {
                // Temporary name, user can rename
                PatchManager::getInstance().createNewBank ("New Bank");
                updateUI();
            };
            addAndMakeVisible (newBankBtn);

            renameBankBtn.setButtonText ("RENAME BANK");
            renameBankBtn.onClick = [this] {
                // Simplified rename for bank
                PatchManager::getInstance().renameBank (bankSelector.getText());
                updateUI();
            };
            addAndMakeVisible (renameBankBtn);

            patchList.setModel (this);
            patchList.setColour (juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
            patchList.setRowHeight (30);
            addAndMakeVisible (patchList);

            messageLabel.setJustificationType (juce::Justification::centred);
            messageLabel.setFont (juce::FontOptions ().withHeight (24.0f));
            messageLabel.setColour (juce::Label::textColourId, juce::Colours::white);
            addChildComponent (messageLabel);

            // Add navigation parameters using the new Momentary flag
            addParameter ("PREV BANK", 0.0f, 1.0f, 0.0f, true, 0.0f, true);
            addParameter ("NEXT BANK", 0.0f, 1.0f, 0.0f, true, 0.0f, true);
            addParameter ("PREV PATCH", 0.0f, 1.0f, 0.0f, true, 0.0f, true);
            addParameter ("NEXT PATCH", 0.0f, 1.0f, 0.0f, true, 0.0f, true);

            updateUI();
        }

        void timerCallback() override
        {
            ModuleBase::timerCallback();
            
            auto checkTrigger = [this](const juce::String& name, bool& lastState, std::function<void()> action) {
                for (auto* p : parameters) {
                    if (p->getName() == name) {
                        bool currentState = p->getValue() > 0.5f;
                        if (currentState && !lastState) {
                            action();
                        }
                        lastState = currentState;
                        break;
                    }
                }
            };

            checkTrigger ("PREV BANK", lastPrevBank, [this]{ PatchManager::getInstance().prevBank(); updateUI(); });
            checkTrigger ("NEXT BANK", lastNextBank, [this]{ PatchManager::getInstance().nextBank(); updateUI(); });
            checkTrigger ("PREV PATCH", lastPrevPatch, [this]{ PatchManager::getInstance().prevPatch(); updateUI(); });
            checkTrigger ("NEXT PATCH", lastNextPatch, [this]{ PatchManager::getInstance().nextPatch(); updateUI(); });
        }

        void updateUI()
        {
            auto& pm = PatchManager::getInstance();
            patchNameEditor.setText (NeonRegistry::getCurrentPatchName(), juce::dontSendNotification);
            
            bankSelector.clear (juce::dontSendNotification);
            auto banks = pm.getBankNames();
            for (int i = 0; i < banks.size(); ++i)
                bankSelector.addItem (banks[i], i + 1);
            bankSelector.setSelectedItemIndex (pm.getCurrentBankIndex(), juce::dontSendNotification);
            
            patchList.updateContent();
            patchList.selectRow (pm.getCurrentPatchIndex());
            repaint();
        }

        void performSave (bool isSaveAs)
        {
            auto& pm = PatchManager::getInstance();
            int targetIndex = pm.getCurrentPatchIndex();

            if (isSaveAs)
            {
                targetIndex = pm.findFirstEmptySlot();
                if (targetIndex == -1)
                {
                    showMomentaryMessage ("BANK FULL!");
                    return;
                }
            }

            pm.savePatch (patchNameEditor.getText(), targetIndex);
            
            if (isSaveAs)
                showMomentaryMessage ("SAVED AS #" + juce::String (targetIndex + 1));
            else
                showMomentaryMessage ("PATCH SAVED");
                
            updateUI();
        }

        void showMomentaryMessage (const juce::String& msg)
        {
            messageLabel.setText (msg, juce::dontSendNotification);
            messageLabel.setVisible (true);
            juce::Timer::callAfterDelay (2000, [this] { messageLabel.setVisible (false); });
        }

        // ListBoxModel overrides
        int getNumRows() override { return 128; }
        void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override
        {
            if (rowIsSelected)
                g.fillAll (accentColor.withAlpha (0.3f));
            
            g.setColour (rowIsSelected ? juce::Colours::white : accentColor.withAlpha (0.7f));
            g.setFont (18.0f);
            
            auto names = PatchManager::getInstance().getPatchNames();
            juce::String text = juce::String (rowNumber + 1).paddedLeft (' ', 3) + ": ";
            if (rowNumber < names.size())
                text += names[rowNumber];
            else
                text += "Empty";
                
            g.drawText (text, 10, 0, width - 20, height, juce::Justification::centredLeft);
        }

        void listBoxItemDoubleClicked (int row, const juce::MouseEvent&) override
        {
            PatchManager::getInstance().loadPatch (row);
            updateUI();
        }

        void resized() override
        {
            ModuleBase::resized();
            auto bounds = getLocalBounds();
            float unitH = (float)bounds.getHeight() / 10.0f;
            auto area = bounds.removeFromTop ((int)(unitH * 7)).reduced (40);
            
            // Header room for patch name display (already handled by ModuleBase)
            area.removeFromTop (50);

            auto topRow = area.removeFromTop (40);
            patchNameEditor.setBounds (topRow.removeFromLeft (area.getWidth() * 0.4f));
            topRow.removeFromLeft (10);
            float btnW = topRow.getWidth() / 5.0f - 4.0f;
            loadBtn.setBounds (topRow.removeFromLeft (btnW));
            topRow.removeFromLeft (4);
            saveBtn.setBounds (topRow.removeFromLeft (btnW));
            topRow.removeFromLeft (4);
            saveAsBtn.setBounds (topRow.removeFromLeft (btnW));
            topRow.removeFromLeft (4);
            renameBtn.setBounds (topRow.removeFromLeft (btnW));
            topRow.removeFromLeft (4);
            initBtn.setBounds (topRow);

            area.removeFromTop (10);

            auto nextRow = area.removeFromTop (40);
            bankSelector.setBounds (nextRow.removeFromLeft (area.getWidth() * 0.4f));
            nextRow.removeFromLeft (10);
            btnW = nextRow.getWidth() / 2.0f - 5.0f;
            newBankBtn.setBounds (nextRow.removeFromLeft (btnW));
            nextRow.removeFromLeft (5);
            renameBankBtn.setBounds (nextRow);

            area.removeFromTop (20);
            patchList.setBounds (area);
            messageLabel.setBounds (getLocalBounds().withSize(400, 100).withCentre(getLocalBounds().getCentre()));
        }

    private:
        juce::TextEditor patchNameEditor;
        juce::TextButton loadBtn, saveBtn, saveAsBtn, renameBtn, initBtn;
        
        juce::ComboBox bankSelector;
        juce::TextButton newBankBtn, renameBankBtn;

        juce::ListBox patchList;
        juce::Label messageLabel;

        bool lastPrevBank = false;
        bool lastNextBank = false;
        bool lastPrevPatch = false;
        bool lastNextPatch = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LibrarianModule)
    };
}
