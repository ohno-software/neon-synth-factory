#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "NeonParameterRegistry.h"
#include "NeonRegistry.h"

namespace neon
{
    /**
     * PatchManager
     * Handles filesystem operations for banks and patches.
     * Manages current patch state and MIDI integration.
     */
    class PatchManager
    {
    public:
        static PatchManager& getInstance()
        {
            static PatchManager instance;
            return instance;
        }

        void initialize (const juce::String& appName)
        {
            // 1. Determine Root Directory (Portable vs Documents)
            auto exeDir = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();
            auto portableFile = exeDir.getChildFile ("portable.txt");

            if (portableFile.exists())
            {
                rootDir = exeDir.getChildFile ("Patches");
            }
            else
            {
                rootDir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                            .getChildFile ("NeonSynth")
                            .getChildFile (appName)
                            .getChildFile ("Banks");
            }
            
            if (!rootDir.exists())
                rootDir.createDirectory();

            scanBanks();
            
            // 2. Deployment Safety
            // If "Factory" bank is missing, create it. 
            // If it exists, we DON'T touch it to avoid overwriting user changes to factory sounds.
            if (!rootDir.getChildFile ("Factory").exists())
                createNewBank ("Factory");
            
            if (bankNames.isEmpty())
                scanBanks();

            selectBank (0);
        }

        void scanBanks()
        {
            bankNames.clear();
            if (!rootDir.exists()) return;

            auto banks = rootDir.findChildFiles (juce::File::findDirectories, false);
            for (auto& b : banks)
                bankNames.add (b.getFileName());
            bankNames.sort (true);
        }

        void createNewBank (const juce::String& bankName)
        {
            auto bankDir = rootDir.getChildFile (bankName);
            if (!bankDir.exists())
                bankDir.createDirectory();

            ensureInitPatches (bankDir);
            scanBanks();
            
            int newIdx = bankNames.indexOf (bankName);
            if (newIdx >= 0) selectBank (newIdx);
        }

        void selectBank (int index)
        {
            if (index < 0 || index >= bankNames.size()) return;
            currentBankIndex = index;
            currentBank = bankNames[index];
            scanPatches();
            loadPatch (0);
        }

        void scanPatches()
        {
            patchNames.clear();
            auto bankDir = rootDir.getChildFile (currentBank);
            
            auto indexFile = bankDir.getChildFile ("index.txt");
            if (indexFile.exists())
            {
                patchNames.addLines (indexFile.loadFileAsString());
            }
            
            // Ensure we have 128 entries
            while (patchNames.size() < 128)
                patchNames.add ("INIT PATCH");
            
            if (patchNames.size() > 128)
                patchNames.removeRange (128, patchNames.size() - 128);
        }

        void updateIndexFile()
        {
            auto bankDir = rootDir.getChildFile (currentBank);
            auto indexFile = bankDir.getChildFile ("index.txt");
            indexFile.replaceWithText (patchNames.joinIntoString ("\n"));
        }

        void loadPatch (int index)
        {
            if (index < 0 || index >= 128) return;
            currentPatchIndex = index;
            
            juce::String filename = juce::String ("patch_") + juce::String (index + 1) + ".neon";
            auto file = rootDir.getChildFile (currentBank).getChildFile (filename);
            
            if (file.exists())
            {
                auto json = juce::JSON::parse (file);
                if (json.isObject())
                {
                    auto patchName = json.getProperty ("name", "INIT PATCH").toString();
                    NeonRegistry::setCurrentPatchName (patchName);

                    auto paramsObj = json.getProperty ("parameters", juce::var());
                    if (paramsObj.isObject())
                    {
                        auto& registry = ParameterRegistry::getInstance();
                        auto* dynamicObj = paramsObj.getDynamicObject();
                        for (auto& it : dynamicObj->getProperties())
                        {
                            if (auto* p = registry.getParameter (it.name.toString()))
                                p->setValue ((float)it.value);
                        }
                    }
                }
            }
            else
            {
                // If file doesn't exist, it's effectively an INIT patch
                initPatch();
                NeonRegistry::setCurrentPatchName ("INIT PATCH");
            }
        }

        void savePatch (const juce::String& name, int index = -1)
        {
            int targetIndex = (index == -1) ? currentPatchIndex : index;
            if (targetIndex < 0 || targetIndex >= 128) return;

            juce::String filename = juce::String ("patch_") + juce::String (targetIndex + 1) + ".neon";
            auto bankDir = rootDir.getChildFile (currentBank);
            if (!bankDir.exists()) bankDir.createDirectory();
            
            auto file = bankDir.getChildFile (filename);

            juce::DynamicObject::Ptr obj = new juce::DynamicObject();
            obj->setProperty ("name", name);
            
            juce::DynamicObject::Ptr params = new juce::DynamicObject();
            auto& registry = ParameterRegistry::getInstance();
            for (auto& pair : registry.getParameters())
            {
                // Don't save Librarian navigation parameters into the patch
                if (pair.first.contains ("Librarian"))
                    continue;

                params->setProperty (pair.first, pair.second->getValue());
            }
            obj->setProperty ("parameters", juce::var(params));

            file.replaceWithText (juce::JSON::toString (juce::var(obj)));
            
            if (targetIndex == currentPatchIndex)
                NeonRegistry::setCurrentPatchName (name);

            patchNames.set (targetIndex, name);
            updateIndexFile();
        }

        int findFirstEmptySlot()
        {
            for (int i = 0; i < patchNames.size(); ++i)
            {
                if (patchNames[i].equalsIgnoreCase ("INIT PATCH"))
                    return i;
            }
            return -1;
        }

        void initPatch()
        {
            auto& registry = ParameterRegistry::getInstance();
            for (auto& pair : registry.getParameters())
            {
                pair.second->setValue (pair.second->getDefaultValue());
            }
            NeonRegistry::setCurrentPatchName ("INIT PATCH");
        }

        void nextBank()
        {
            int nextIndex = currentBankIndex + 1;
            if (nextIndex >= bankNames.size()) nextIndex = 0;
            selectBank (nextIndex);
        }

        void prevBank()
        {
            int prevIndex = currentBankIndex - 1;
            if (prevIndex < 0) prevIndex = bankNames.size() - 1;
            selectBank (prevIndex);
        }

        void nextPatch()
        {
            int nextIndex = currentPatchIndex + 1;
            if (nextIndex >= 128) nextIndex = 0;
            loadPatch (nextIndex);
        }

        void prevPatch()
        {
            int prevIndex = currentPatchIndex - 1;
            if (prevIndex < 0) prevIndex = 127;
            loadPatch (prevIndex);
        }

        void renameBank (const juce::String& newName)
        {
            auto oldDir = rootDir.getChildFile (currentBank);
            auto newDir = rootDir.getChildFile (newName);
            if (oldDir.moveFileTo (newDir))
            {
                scanBanks();
                selectBank (bankNames.indexOf (newName));
            }
        }

        juce::StringArray getBankNames() const { return bankNames; }
        juce::StringArray getPatchNames() const { return patchNames; }
        juce::String getCurrentBankName() const { return currentBank; }
        int getCurrentBankIndex() const { return currentBankIndex; }
        int getCurrentPatchIndex() const { return currentPatchIndex; }

    private:
        PatchManager() = default;

        void ensureInitPatches (const juce::File& bankDir)
        {
            juce::StringArray names;
            for (int i = 0; i < 128; ++i)
            {
                juce::String filename = juce::String ("patch_") + juce::String (i + 1) + ".neon";
                auto file = bankDir.getChildFile (filename);
                if (!file.exists())
                {
                    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
                    obj->setProperty ("name", "INIT PATCH");
                    file.replaceWithText (juce::JSON::toString (juce::var(obj)));
                }
                names.add (getPatchNameFromFile(file));
            }

            auto indexFile = bankDir.getChildFile ("index.txt");
            if (!indexFile.exists())
                indexFile.replaceWithText (names.joinIntoString ("\n"));
        }

        juce::String getPatchNameFromFile (const juce::File& file)
        {
            auto json = juce::JSON::parse (file);
            return json.getProperty ("name", "INIT PATCH").toString();
        }

        juce::File rootDir;
        juce::String currentBank;
        int currentBankIndex = 0;
        int currentPatchIndex = 0;
        juce::StringArray bankNames;
        juce::StringArray patchNames;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PatchManager)
    };
}
