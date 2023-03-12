#include "serdepp/serde.hpp"
#include "serdepp/adaptor/fmt.hpp"

namespace serde::attribute {
    namespace detail {
        struct tttt {
            template <typename T, typename serde_ctx, typename Next, typename... Attributes>
            constexpr inline void from(serde_ctx& ctx, T& data, std::string_view key,
                                       Next&& next_attr, Attributes&& ...remains) const{
                fmt::print("from\n");
                next_attr.from(ctx, data, key, remains...);
            }

            template <typename T, typename serde_ctx, typename Next, typename... Attributes>
            constexpr inline void into(serde_ctx& ctx, T& data, std::string_view key,
                                       Next&& next_attr, Attributes&& ...remains) const{
                fmt::print("into\n");
                next_attr.into(ctx, data, key, remains...);
            }
        };
    }
    [[maybe_unused]] constexpr static auto tttt = detail::tttt{};
}

struct test_{
    DERIVE_SERDE(test_, .attributes(tttt,tttt)
                 (&Self::test, "test")
                 [attributes(make_optional)]
                 (&Self::a, "a")
                 )
    int test;
    bool a;
};

int main(int argc, char *argv[])
{
    fmt::print("{}\n",test_{1, false});
    
    return 0;
}
