/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION

  ID:               neon_ui_components
  vendor:           Neon H2O
  version:          0.1.0
  name:             Neon UI Component Library
  description:      Cyber-retro UI framework for the Neon Synth Factory.
  website:          http://neonh2o.com
  license:          Proprietary

  dependencies:     juce_gui_basics, juce_gui_extra, juce_graphics

 END_JUCE_MODULE_DECLARATION
*******************************************************************************/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_graphics/juce_graphics.h>

/** Config: NEON_USE_7SEG_FONT
    Whether to use the specialized 7-segment font for value displays.
*/
#ifndef NEON_USE_7SEG_FONT
 #define NEON_USE_7SEG_FONT 1
#endif

// Core Design System
#include "core/NeonColors.h"
#include "core/NeonLookAndFeel.h"
#include "core/NeonParameterTemplate.h"
#include "core/NeonManagedParameter.h"
#include "core/NeonParameterRegistry.h"
#include "core/NeonRegistry.h"
#include "core/NeonPatchManager.h"

// Atoms (Individual Widgets)
#include "widgets/NeonBar.h"
#include "widgets/NeonToggle.h"
#include "widgets/NeonVowelMap.h"
#include "widgets/NeonXYPad.h"
#include "widgets/NeonRibbon.h"
#include "widgets/NeonDebugPanel.h"

// Molecules (Compound Components)
#include "widgets/NeonParameterCard.h"

// Organisms (Synth Modules)
#include "modules/ModuleBase.h"
#include "modules/OscillatorModule.h"
#include "modules/DahdsrModule.h"
#include "modules/AdsrModule.h"
#include "modules/FxModule.h"
#include "modules/LibrarianModule.h"
#include "modules/LadderFilterModule.h"
#include "modules/AmpModule.h"
#include "modules/ModuleSelectionPanel.h"
#include "modules/NeonSynthModule.h"
#include "modules/NeonSelectionPanel.h"
