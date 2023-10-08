#pragma once

#ifndef DEBUG
#define ASSERT(Msg) IKIGAI::UTILS::__Assert("", false, __FILE__, __LINE__, Msg)
#define ASSERT_IF(Expr, Msg) IKIGAI::UTILS::__Assert(#Expr, Expr, __FILE__, __LINE__, Msg)
#else
#   define M_Assert(Expr, Msg) ;
#endif

namespace IKIGAI::UTILS
{
    void __Assert(const char* expr_str, bool expr, const char* file, int line, const char* msg);
}
