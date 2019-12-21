#pragma once
#include <utility>
namespace utils {
    namespace detail {
        template <class Callable>
        struct FinalAction final {
            explicit FinalAction(Callable f) : clean{ std::move(f) } { }

            ~FinalAction() {
                clean();
            }

        private:
            Callable clean;
        }; // END of struct FinalAction
    } // END of namespace detail

    template <class Callable>
    detail::FinalAction<Callable> finally(Callable &&f) {
        return detail::FinalAction<Callable>(std::forward<Callable>(f));
    }
} // END of namespace utils
