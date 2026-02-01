#include "NeonParameterRegistry.h"

namespace neon
{
    ParameterRegistry& ParameterRegistry::getInstance()
    {
        static ParameterRegistry instance;
        return instance;
    }
}
