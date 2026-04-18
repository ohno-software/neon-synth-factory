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
     * DX100-style algorithms for 4 operators with non-overlapping visual layout:
     * 
     *  0: Serial Chain    1 → 2 → 3 → 4 → out
     *  1: Branch          (1 → 2 → 4) + (3 → 4) → out
     *  2: Dual Stack      (1 → 2) + (3 → 4) → out
     *  3: Triple Carrier  1 + 2 + 3 → 4 → out (3 modulators, 1 carrier)
     *  4: One to Many     1 → (2, 3, 4) → out (1 modulator, 3 carriers)
     *  5: Parallel Dual   (1 → 2) + 3 + 4 → out
     *  6: Complex Fork    (1 → 2, 1 → 3) + (2, 3 → 4) → out
     *  7: Full Parallel   1 + 2 + 3 + 4 → out (additive)
     */
    enum class FmAlgorithmType
    {
        SerialChain = 0,    // 1→2→3→4→out
        Branch,             // 1→2→4, 3→4→out
        DualStack,          // (1→2) + (3→4)→out
        TripleCarrier,      // 1,2,3→4→out
        OneToMany,          // 1→(2,3,4)→out
        ParallelDual,       // (1→2) + 3 + 4→out
        ComplexFork,        // 1→2,3 then 2,3→4→out
        FullParallel,       // 1+2+3+4→out
        Count
    };

    /**
     * getAlgorithmName - Returns the display name of an algorithm
     */
    static inline juce::String getAlgorithmName (int index)
    {
        static const juce::String names[] = {
            "1: Serial",      "2: Branch",       "3: Dual Stack",
            "4: Triple Mod",  "5: One→Many",     "6: Parallel Dual",
            "7: Complex",     "8: Full Parallel"
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

                case FmAlgorithmType::OneToMany:
                {
                    // 1 → (2, 3, 4) → out
                    float o1 = ops[0].processSample (baseFreq);
                    float o2 = ops[1].processSample (baseFreq, o1 * modDepth);
                    float o3 = ops[2].processSample (baseFreq, o1 * modDepth);
                    float o4 = ops[3].processSample (baseFreq, o1 * modDepth);
                    out = (o2 + o3 + o4) * 0.333f;
                    break;
                }

                case FmAlgorithmType::ParallelDual:
                {
                    // (1→2) + 3 + 4 → out
                    float o1 = ops[0].processSample (baseFreq);
                    float o2 = ops[1].processSample (baseFreq, o1 * modDepth);
                    float o3 = ops[2].processSample (baseFreq);
                    float o4 = ops[3].processSample (baseFreq);
                    out = (o2 + o3 + o4) * 0.333f;
                    break;
                }

                case FmAlgorithmType::ComplexFork:
                {
                    // 1→2, 1→3, 2,3→4→out
                    float o1 = ops[0].processSample (baseFreq);
                    float o2 = ops[1].processSample (baseFreq, o1 * modDepth);
                    float o3 = ops[2].processSample (baseFreq, o1 * modDepth);
                    float o4 = ops[3].processSample (baseFreq, (o2 + o3) * modDepth * 0.5f);
                    out = o4;
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

                default:
                    out = ops[3].processSample (baseFreq);
                    break;
            }

            return out;
        }

        static constexpr float modDepth = juce::MathConstants<float>::twoPi;
    };

} // namespace neon
