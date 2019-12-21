#pragma once
#include <cstdint>
#include <type_traits>
#include <cstddef>

namespace utils {
    using Byte = std::uint8_t;
    using HalfWord = std::uint16_t;
    using Word = std::uint32_t;
    using DoubleWord = std::uint64_t;

    namespace detail {
        template <bool Cond, class Type>
        struct add_const_if final {
            using this_type = add_const_if;
            using type = std::add_const_t<Type>;
        }; // END of struct add_const_if

        template <class Type>
        struct add_const_if<false, Type> final {
            using this_type = add_const_if;
            using type = Type;
        }; // END of struct add_const_if

        template <bool Cond, class Type>
        using add_const_if_t = typename add_const_if<Cond, Type>::type;

    } // END of namespace detail

    template <class Pointer>
    void advancePtr(Pointer &pointer, std::size_t advanceBy) {
        static_assert(std::is_pointer_v<std::remove_reference_t<Pointer>>, "Pointer in advancePtr was not a pointer");
        auto bytePtr = reinterpret_cast<detail::add_const_if_t<std::is_const<std::remove_pointer_t<Pointer>>::value,
                                                  Byte> *>(pointer);
        bytePtr += advanceBy;
        pointer = reinterpret_cast<Pointer>(bytePtr);
    }

    template <class TypeToRead, class Pointer>
    TypeToRead readFromAddress(Pointer &&pointer) {
        static_assert(std::is_pointer_v<std::remove_reference_t<Pointer>>, "Pointer in readFromAddress was not a pointer");
        auto p = reinterpret_cast<TypeToRead const *>(pointer);
        return *p;
    }

    template <class TypeToWrite, class Pointer>
    void writeToAddress(Pointer pointer, TypeToWrite value) {
        static_assert(std::is_pointer_v<std::remove_reference_t<Pointer>>, "Pointer in writeToAddress was not a pointer");
        auto p = reinterpret_cast<TypeToWrite *>(pointer);
        *p = value;
    }
    
} // END of namespace utils
