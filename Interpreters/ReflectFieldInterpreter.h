#pragma once

#include "Inspect/API.h"

#include "Inspect/Canvas.h"
#include "Inspect/Interpreter.h"

namespace Helium
{
    namespace Inspect
    {
        class HELIUM_INSPECT_API ReflectFieldInterpreter : public Interpreter
        {
        public:
            ReflectFieldInterpreter (Container* container);

            virtual void InterpretField(const Reflect::Field* field, const std::vector<Reflect::Object*>& instances, Container* parent) = 0;
        };

        typedef Helium::StrongPtr<ReflectFieldInterpreter> ReflectFieldInterpreterPtr;
    }
}