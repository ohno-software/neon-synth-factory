#pragma once

#include <juce_core/juce_core.h>
#include <array>
#include <vector>

namespace neon
{
    /**
     * FmAlgorithm
     * Defines the routing topology between 4 FM operators.
     * 
     * Classic DX-style algorithms adapted for 4 operators:
     * 
     *  0: Serial Chain    1 → 2 → 3 → 4 → out
     *  1: Branch          1 → 2 → 4 → out, 3 → 4
     *  2: Dual Stack      (1 → 2) + (3 → 4) → out
     *  3: Triple Carrier  1 → 4, 2 → 4, 3 → 4 → out (3 mods, 1 carrier)
     *  4: Dual Mod        (1 → 3) + (2 → 4) → out
     *  5: One to Three    1 → 2, 1 → 3, 1 → 4 → out (1 mod, 3 carriers)
     *  6: Triple Parallel (1 → 2) + 3 + 4 → out
     *  7: Full Parallel   1 + 2 + 3 + 4 → out (additive)
     *  8: Y-Shape         1 → 3 → 4 → out, 2 → 3
     *  9: Diamond         1 → 2 → 4, 1 → 3 → 4 → out
     * 10: Cascade Branch  1 → 2 → 3 → 4, 1 → 4 → out
     */
    enum class FmAlgorithmType
    {
        SerialChain = 0,    // 1→2→3→4→out
        Branch,             // 1→2→4, 3→4→out
        DualStack,          // (1→2) + (3→4)→out
        TripleCarrier,      // 1,2,3→4→out
        DualMod,            // (1→3) + (2→4)→out
        OneToThree,         // 1→(2+3+4)→out
        TripleParallel,     // (1→2) + 3 + 4→out
        FullParallel,       // 1+2+3+4→out
        YShape,             // 1→3→4, 2→3→out
        Diamond,            // 1→2→4, 1→3→4→out
        CascadeBranch,      // 1→2→3→4, 1→4→out
        Count
    };

    /**
     * getAlgorithmName - Returns the display name of an algorithm
     */
    static inline juce::String getAlgorithmName (int index)
    {
        static const juce::String names[] = {
            "1: Serial",      "2: Branch",       "3: Dual Stack",
            "4: Triple Mod",  "5: Dual Mod",     "6: One→Three",
            "7: Triple Par",  "8: Full Parallel", "9: Y-Shape",
            "10: Diamond",    "11: Cascade"
        };
        if (index >= 0 && index < (int)FmAlgorithmType::Count)
            return names[index];
        return "Unknown";
    }

    static inline std::vector<juce::String> getAlgorithmNames()
    {
        std::vector<juce::String> names;
        for (int i = 0; i < (int)FmAlgorithmType::Count; ++i)
            names.push_back (getAlgorithmName (i));
        return names;
    }

    /**
     * FmAlgorithmRouter
     * Given 4 operator outputs, routes them according to the selected algorithm.
     * 
     * Usage: call processAlgorithm() each sample. It takes the base frequency,
     * an array of 4 operator references, and returns the mixed output.
     */
    struct FmAlgorithmRouter
    {
        /**
         * Process one sample through the selected algorithm.
         * @param algo      The algorithm to use
         * @param ops       Array of 4 FmOperator references
         * @param baseFreq  The fundamental frequency for this voice
         * @return           The mixed output sample
         */
        template <typename OpArray>
        static float process (FmAlgorithmType algo, OpArray& ops, float baseFreq)
        {
            float out = 0.0f;

            switch (algo)
            {
                case FmAlgorithmType::SerialChain:
                {
                    // 1→2→3→4→out
                    float o1 = ops[0].processSample (baseFreq);
                    float o2 = ops[1].processSample (baseFreq, o1 * modDepth);
                    float o3 = ops[2].processSample (baseFreq, o2 * modDepth);
                    float o4 = ops[3].processSample (baseFreq, o3 * modDepth);
                    out = o4;
                    break;
                }

                case FmAlgorithmType::Branch:
                {
                    // 1→2→4, 3→4→out
                    float o1 = ops[0].processSample (baseFreq);
                    float o2 = ops[1].processSample (baseFreq, o1 * modDepth);
                    float o3 = ops[2].processSample (baseFreq);
                    float o4 = ops[3].processSample (baseFreq, (o2 + o3) * modDepth);
                    out = o4;
                    break;
                }

                case FmAlgorithmType::DualStack:
                {
                    // (1→2) + (3→4) → out
                    float o1 = ops[0].processSample (baseFreq);
                    float o2 = ops[1].processSample (baseFreq, o1 * modDepth);
                    float o3 = ops[2].processSample (baseFreq);
                    float o4 = ops[3].processSample (baseFreq, o3 * modDepth);
                    out = (o2 + o4) * 0.5f;
                    break;
                }

                case FmAlgorithmType::TripleCarrier:
                {
                    // 1,2,3 → 4 → out
                    float o1 = ops[0].processSample (baseFreq);
                    float o2 = ops[1].processSample (baseFreq);
                    float o3 = ops[2].processSample (baseFreq);
                    float o4 = ops[3].processSample (baseFreq, (o1 + o2 + o3) * modDepth * 0.333f);
                    out = o4;
                    break;
                }

                case FmAlgorithmType::DualMod:
                {
                    // (1→3) + (2→4) → out
                    float o1 = ops[0].processSample (baseFreq);
                    float o2 = ops[1].processSample (baseFreq);
                    float o3 = ops[2].processSample (baseFreq, o1 * modDepth);
                    float o4 = ops[3].processSample (baseFreq, o2 * modDepth);
                    out = (o3 + o4) * 0.5f;
                    break;
                }

                case FmAlgorithmType::OneToThree:
                {
                    // 1 → (2, 3, 4) → out
                    float o1 = ops[0].processSample (baseFreq);
                    float o2 = ops[1].processSample (baseFreq, o1 * modDepth);
                    float o3 = ops[2].processSample (baseFreq, o1 * modDepth);
                    float o4 = ops[3].processSample (baseFreq, o1 * modDepth);
                    out = (o2 + o3 + o4) * 0.333f;
                    break;
                }

                case FmAlgorithmType::TripleParallel:
                {
                    // (1→2) + 3 + 4 → out
                    float o1 = ops[0].processSample (baseFreq);
                    float o2 = ops[1].processSample (baseFreq, o1 * modDepth);
                    float o3 = ops[2].processSample (baseFreq);
                    float o4 = ops[3].processSample (baseFreq);
                    out = (o2 + o3 + o4) * 0.333f;
                    break;
                }

                case FmAlgorithmType::FullParallel:
                {
                    // 1 + 2 + 3 + 4 → out (additive)
                    float o1 = ops[0].processSample (baseFreq);
                    float o2 = ops[1].processSample (baseFreq);
                    float o3 = ops[2].processSample (baseFreq);
                    float o4 = ops[3].processSample (baseFreq);
                    out = (o1 + o2 + o3 + o4) * 0.25f;
                    break;
                }

                case FmAlgorithmType::YShape:
                {
                    // 1→3, 2→3, 3→4→out
                    float o1 = ops[0].processSample (baseFreq);
                    float o2 = ops[1].processSample (baseFreq);
                    float o3 = ops[2].processSample (baseFreq, (o1 + o2) * modDepth * 0.5f);
                    float o4 = ops[3].processSample (baseFreq, o3 * modDepth);
                    out = o4;
                    break;
                }

                case FmAlgorithmType::Diamond:
                {
                    // 1→2→4, 1→3→4→out
                    float o1 = ops[0].processSample (baseFreq);
                    float o2 = ops[1].processSample (baseFreq, o1 * modDepth);
                    float o3 = ops[2].processSample (baseFreq, o1 * modDepth);
                    float o4 = ops[3].processSample (baseFreq, (o2 + o3) * modDepth * 0.5f);
                    out = o4;
                    break;
                }

                case FmAlgorithmType::CascadeBranch:
                {
                    // 1→2→3→4, 1→4→out
                    float o1 = ops[0].processSample (baseFreq);
                    float o2 = ops[1].processSample (baseFreq, o1 * modDepth);
                    float o3 = ops[2].processSample (baseFreq, o2 * modDepth);
                    float o4 = ops[3].processSample (baseFreq, (o3 + o1) * modDepth * 0.5f);
                    out = o4;
                    break;
                }

                default:
                    out = ops[3].processSample (baseFreq);
                    break;
            }

            return out;
        }

        static constexpr float modDepth = juce::MathConstants<float>::twoPi;
    };

} // namespace neon
