#pragma once

// These macros are empty for the C++ compiler.
// They are parsed by the IkigaiHeaderTool to generate reflection data.

namespace IKIGAI::UTILS {
    template<typename T>
    struct ReflectionReg;
}

#define IKI_CLASS(...)
#define IKI_PROPERTY(...)
#define IKI_FUNCTION(...)
#define IKI_ENUM(...)

#define IKI_GENERATED_BODY(ClassName) friend struct IKIGAI::UTILS::ReflectionReg<ClassName>;

