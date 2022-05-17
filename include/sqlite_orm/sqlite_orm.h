#pragma once

#if defined(_MSC_VER)
__pragma(push_macro("min"))
#undef min
__pragma(push_macro("max"))
#undef max
#endif  // defined(_MSC_VER)
#pragma once

#include <type_traits>

// #include "functional/cxx_universal.h"

/*
 *  This header makes central C++ functionality on which sqlite_orm depends universally available:
 *  - alternative operator representations
 *  - ::size_t, ::ptrdiff_t, ::nullptr_t
 *  - C++ core feature macros
 *  - macros for dealing with compiler quirks
 */

#include <iso646.h>  //  alternative operator representations
#include <cstddef>  //  sqlite_orm is using size_t, ptrdiff_t, nullptr_t everywhere, pull it in early

// earlier clang versions didn't make nullptr_t available in the global namespace via stddef.h,
// though it should have according to C++ documentation (see https://en.cppreference.com/w/cpp/types/nullptr_t#Notes).
// actually it should be available when including stddef.h
using std::nullptr_t;

// #include "cxx_core_features.h"

#ifdef __has_cpp_attribute
#define SQLITE_ORM_HAS_CPP_ATTRIBUTE(attr) __has_cpp_attribute(attr)
#else
#define SQLITE_ORM_HAS_CPP_ATTRIBUTE(attr) 0L
#endif

#ifdef __has_include
#define SQLITE_ORM_HAS_INCLUDE(file) __has_include(file)
#else
#define SQLITE_ORM_HAS_INCLUDE(file) 0L
#endif

#if __cpp_aggregate_nsdmi >= 201304L
#define SQLITE_ORM_AGGREGATE_NSDMI_SUPPORTED
#endif

#if __cpp_constexpr >= 201304L
#define SQLITE_ORM_RELAXED_CONSTEXPR
#endif

#if __cpp_noexcept_function_type >= 201510L
#define SQLITE_ORM_NOTHROW_ALIASES_SUPPORTED
#endif

#if __cpp_aggregate_bases >= 201603L
#define SQLITE_ORM_AGGREGATE_BASES_SUPPORTED
#endif

#if __cpp_fold_expressions >= 201603L
#define SQLITE_ORM_FOLD_EXPRESSIONS_SUPPORTED
#endif

#if __cpp_inline_variables >= 201606L
#define SQLITE_ORM_INLINE_VARIABLES_SUPPORTED
#endif

#if __cpp_if_constexpr >= 201606L
#define SQLITE_ORM_IF_CONSTEXPR_SUPPORTED
#endif

#if __cpp_inline_variables >= 201606L
#define SQLITE_ORM_INLINE_VAR inline
#else
#define SQLITE_ORM_INLINE_VAR
#endif

#if SQLITE_ORM_HAS_CPP_ATTRIBUTE(no_unique_address) >= 201803L
#define SQLITE_ORM_NOUNIQUEADDRESS [[no_unique_address]]
#else
#define SQLITE_ORM_NOUNIQUEADDRESS
#endif

#if __cpp_consteval >= 201811L
#define SQLITE_ORM_CONSTEVAL consteval
#else
#define SQLITE_ORM_CONSTEVAL constexpr
#endif

#if __cpp_aggregate_paren_init >= 201902L
#define SQLITE_ORM_AGGREGATE_PAREN_INIT_SUPPORTED
#endif

#if __cpp_concepts >= 201907L
#define SQLITE_ORM_CONCEPTS_SUPPORTED
#endif

#if __cplusplus >= 201703L  // C++17 or later
#if __has_include(<optional>)
#define SQLITE_ORM_OPTIONAL_SUPPORTED
#endif

#if __has_include(<string_view>)
#define SQLITE_ORM_STRING_VIEW_SUPPORTED
#endif
#endif

// #include "cxx_compiler_quirks.h"

#ifdef __clang__
#define SQLITE_ORM_DO_PRAGMA(...) _Pragma(#__VA_ARGS__)
#endif

#ifdef __clang__
#define SQLITE_ORM_CLANG_SUPPRESS(warnoption, ...)                                                                     \
    SQLITE_ORM_DO_PRAGMA(clang diagnostic push)                                                                        \
    SQLITE_ORM_DO_PRAGMA(clang diagnostic ignored warnoption)                                                          \
    __VA_ARGS__                                                                                                        \
    SQLITE_ORM_DO_PRAGMA(clang diagnostic pop)

#else
#define SQLITE_ORM_CLANG_SUPPRESS(warnoption, ...) __VA_ARGS__
#endif

// clang has the bad habit of diagnosing missing brace-init-lists when constructing aggregates with base classes.
// This is a false positive, since the C++ standard is quite clear that braces for nested or base objects may be omitted,
// see https://en.cppreference.com/w/cpp/language/aggregate_initialization:
// "The braces around the nested initializer lists may be elided (omitted),
//  in which case as many initializer clauses as necessary are used to initialize every member or element of the corresponding subaggregate,
//  and the subsequent initializer clauses are used to initialize the following members of the object."
// In this sense clang should only warn about missing field initializers.
// Because we know what we are doing, we suppress the diagnostic message
#define SQLITE_ORM_CLANG_SUPPRESS_MISSING_BRACES(...) SQLITE_ORM_CLANG_SUPPRESS("-Wmissing-braces", __VA_ARGS__)

#if defined(_MSC_VER) && (_MSC_VER < 1920)
#define SQLITE_ORM_BROKEN_VARIADIC_PACK_EXPANSION
#endif

// #include "functional/cxx_polyfill.h"

#include <type_traits>

// #include "functional/cxx_universal.h"

namespace sqlite_orm {
    namespace internal {
        namespace polyfill {
#if __cpp_lib_void_t >= 201411L
            using std::void_t;
#else
            template<class...>
            using void_t = void;
#endif

#if __cpp_lib_bool_constant >= 201505L
            using std::bool_constant;
#else
            template<bool v>
            using bool_constant = std::integral_constant<bool, v>;
#endif

#if __cpp_lib_logical_traits >= 201510L && __cpp_lib_type_trait_variable_templates >= 201510L
            using std::conjunction;
            using std::conjunction_v;
            using std::disjunction;
            using std::disjunction_v;
            using std::negation;
            using std::negation_v;
#else
            template<typename...>
            struct conjunction : std::true_type {};
            template<typename B1>
            struct conjunction<B1> : B1 {};
            template<typename B1, typename... Bn>
            struct conjunction<B1, Bn...> : std::conditional_t<bool(B1::value), conjunction<Bn...>, B1> {};
            template<typename... Bs>
            SQLITE_ORM_INLINE_VAR constexpr bool conjunction_v = conjunction<Bs...>::value;

            template<typename...>
            struct disjunction : std::false_type {};
            template<typename B1>
            struct disjunction<B1> : B1 {};
            template<typename B1, typename... Bn>
            struct disjunction<B1, Bn...> : std::conditional_t<bool(B1::value), B1, disjunction<Bn...>> {};
            template<typename... Bs>
            SQLITE_ORM_INLINE_VAR constexpr bool disjunction_v = disjunction<Bs...>::value;

            template<typename B>
            struct negation : bool_constant<!bool(B::value)> {};
            template<typename B>
            SQLITE_ORM_INLINE_VAR constexpr bool negation_v = negation<B>::value;
#endif

#if __cpp_lib_remove_cvref >= 201711L
            using std::remove_cvref, std::remove_cvref_t;
#else
            template<class T>
            struct remove_cvref : std::remove_cv<std::remove_reference_t<T>> {};

            template<class T>
            using remove_cvref_t = typename remove_cvref<T>::type;
#endif

#if __cpp_lib_type_identity >= 201806L
            using std::type_identity, std::type_identity_t;
#else
            template<class T>
            struct type_identity {
                using type = T;
            };

            template<class T>
            using type_identity_t = typename type_identity<T>::type;
#endif

#if 0  // __cpp_lib_detect >= 0L  //  library fundamentals TS v2, [meta.detect]
            using std::nonesuch;
            using std::detector;
            using std::is_detected, std::is_detected_v;
            using std::detected, std::detected_t;
            using std::detected_or, std::detected_or_t;
#else
            struct nonesuch {
                ~nonesuch() = delete;
                nonesuch(const nonesuch&) = delete;
                void operator=(const nonesuch&) = delete;
            };

            template<class Default, class AlwaysVoid, template<class...> class Op, class... Args>
            struct detector {
                using value_t = std::false_type;
                using type = Default;
            };

            template<class Default, template<class...> class Op, class... Args>
            struct detector<Default, polyfill::void_t<Op<Args...>>, Op, Args...> {
                using value_t = std::true_type;
                using type = Op<Args...>;
            };

            template<template<class...> class Op, class... Args>
            using is_detected = typename detector<nonesuch, void, Op, Args...>::value_t;

            template<template<class...> class Op, class... Args>
            using detected = detector<nonesuch, void, Op, Args...>;

            template<template<class...> class Op, class... Args>
            using detected_t = typename detector<nonesuch, void, Op, Args...>::type;

            template<class Default, template<class...> class Op, class... Args>
            using detected_or = detector<Default, void, Op, Args...>;

            template<class Default, template<class...> class Op, class... Args>
            using detected_or_t = typename detected_or<Default, Op, Args...>::type;

            template<template<class...> class Op, class... Args>
            SQLITE_ORM_INLINE_VAR constexpr bool is_detected_v = is_detected<Op, Args...>::value;
#endif

#if 0  // proposed but not pursued
            using std::is_specialization_of, std::is_specialization_of_t, std::is_specialization_of_v;
#else
            // is_specialization_of: https://github.com/cplusplus/papers/issues/812

            template<typename Type, template<typename...> class Primary>
            SQLITE_ORM_INLINE_VAR constexpr bool is_specialization_of_v = false;

            template<template<typename...> class Primary, class... Types>
            SQLITE_ORM_INLINE_VAR constexpr bool is_specialization_of_v<Primary<Types...>, Primary> = true;

            template<typename Type, template<typename...> class Primary>
            struct is_specialization_of : bool_constant<is_specialization_of_v<Type, Primary>> {};
#endif

            template<typename...>
            SQLITE_ORM_INLINE_VAR constexpr bool always_false_v = false;

            template<size_t I>
            using index_constant = std::integral_constant<size_t, I>;
        }
    }

    namespace polyfill = internal::polyfill;
}

namespace sqlite_orm {
    // C++ generic traits used throughout the library
    namespace internal {
        template<class T, class... Types>
        using is_any_of = polyfill::disjunction<std::is_same<T, Types>...>;

        template<class T, class... Types>
        SQLITE_ORM_INLINE_VAR constexpr bool is_any_of_v = polyfill::disjunction_v<std::is_same<T, Types>...>;

        // enable_if for types
        template<template<typename...> class Op, class... Args>
        using match_if = std::enable_if_t<Op<Args...>::value>;

        // enable_if for types
        template<template<typename...> class Op, class... Args>
        using match_if_not = std::enable_if_t<polyfill::negation_v<Op<Args...>>>;

        // enable_if for types
        template<class T, template<typename...> class Primary>
        using match_specialization_of = std::enable_if_t<polyfill::is_specialization_of_v<T, Primary>>;

        // enable_if for functions
        template<template<typename...> class Op, class... Args>
        using satisfies = std::enable_if_t<Op<Args...>::value, bool>;

        // enable_if for functions
        template<template<typename...> class Op, class... Args>
        using satisfies_not = std::enable_if_t<polyfill::negation<Op<Args...>>::value, bool>;

        // enable_if for functions
        template<class T, template<typename...> class Primary>
        using satisfies_is_specialization_of = std::enable_if_t<polyfill::is_specialization_of_v<T, Primary>, bool>;
    }

    // type name template aliases for syntactic sugar
    namespace internal {
        template<typename T>
        using type_t = typename T::type;

        template<typename T>
        using field_type_t = typename T::field_type;

        template<typename T>
        using constraints_type_t = typename T::constraints_type;

        template<typename T>
        using object_type_t = typename T::object_type;

        template<typename T>
        using table_type_t = typename T::table_type;

        template<typename S>
        using storage_object_type_t = typename S::table_type::object_type;
    }
}
#pragma once

#include <sqlite3.h>
#include <system_error>  // std::error_code, std::system_error
#include <string>  //  std::string
#include <stdexcept>
#include <sstream>  //  std::ostringstream
#include <type_traits>

namespace sqlite_orm {

    /** @short Enables classifying sqlite error codes.

        @note We don't bother listing all possible values;
        this also allows for compatibility with
        'Construction rules for enum class values (P0138R2)'
     */
    enum class sqlite_errc {};

    enum class orm_error_code {
        not_found = 1,
        type_is_not_mapped_to_storage,
        trying_to_dereference_null_iterator,
        too_many_tables_specified,
        incorrect_set_fields_specified,
        column_not_found,
        table_has_no_primary_key_column,
        cannot_start_a_transaction_within_a_transaction,
        no_active_transaction,
        incorrect_journal_mode_string,
        invalid_collate_argument_enum,
        failed_to_init_a_backup,
        unknown_member_value,
        incorrect_order,
        cannot_use_default_value,
        arguments_count_does_not_match,
        function_not_found,
        index_is_out_of_bounds,
        value_is_null,
        no_tables_specified,
    };

}

namespace std {
    template<>
    struct is_error_code_enum<::sqlite_orm::sqlite_errc> : true_type {};

    template<>
    struct is_error_code_enum<::sqlite_orm::orm_error_code> : true_type {};
}

namespace sqlite_orm {

    class orm_error_category : public std::error_category {
      public:
        const char* name() const noexcept override final {
            return "ORM error";
        }

        std::string message(int c) const override final {
            switch(static_cast<orm_error_code>(c)) {
                case orm_error_code::not_found:
                    return "Not found";
                case orm_error_code::type_is_not_mapped_to_storage:
                    return "Type is not mapped to storage";
                case orm_error_code::trying_to_dereference_null_iterator:
                    return "Trying to dereference null iterator";
                case orm_error_code::too_many_tables_specified:
                    return "Too many tables specified";
                case orm_error_code::incorrect_set_fields_specified:
                    return "Incorrect set fields specified";
                case orm_error_code::column_not_found:
                    return "Column not found";
                case orm_error_code::table_has_no_primary_key_column:
                    return "Table has no primary key column";
                case orm_error_code::cannot_start_a_transaction_within_a_transaction:
                    return "Cannot start a transaction within a transaction";
                case orm_error_code::no_active_transaction:
                    return "No active transaction";
                case orm_error_code::invalid_collate_argument_enum:
                    return "Invalid collate_argument enum";
                case orm_error_code::failed_to_init_a_backup:
                    return "Failed to init a backup";
                case orm_error_code::unknown_member_value:
                    return "Unknown member value";
                case orm_error_code::incorrect_order:
                    return "Incorrect order";
                case orm_error_code::cannot_use_default_value:
                    return "The statement 'INSERT INTO * DEFAULT VALUES' can be used with only one row";
                case orm_error_code::arguments_count_does_not_match:
                    return "Arguments count does not match";
                case orm_error_code::function_not_found:
                    return "Function not found";
                case orm_error_code::index_is_out_of_bounds:
                    return "Index is out of bounds";
                case orm_error_code::value_is_null:
                    return "Value is null";
                case orm_error_code::no_tables_specified:
                    return "No tables specified";
                default:
                    return "unknown error";
            }
        }
    };

    class sqlite_error_category : public std::error_category {
      public:
        const char* name() const noexcept override final {
            return "SQLite error";
        }

        std::string message(int c) const override final {
            return sqlite3_errstr(c);
        }
    };

    inline const orm_error_category& get_orm_error_category() {
        static orm_error_category res;
        return res;
    }

    inline const sqlite_error_category& get_sqlite_error_category() {
        static sqlite_error_category res;
        return res;
    }

    inline std::error_code make_error_code(sqlite_errc ev) noexcept {
        return {static_cast<int>(ev), get_sqlite_error_category()};
    }

    inline std::error_code make_error_code(orm_error_code ev) noexcept {
        return {static_cast<int>(ev), get_orm_error_category()};
    }

    template<typename... T>
    std::string get_error_message(sqlite3* db, T&&... args) {
        std::ostringstream stream;
        using unpack = int[];
        static_cast<void>(unpack{0, (static_cast<void>(static_cast<void>(stream << args)), 0)...});
        stream << sqlite3_errmsg(db);
        return stream.str();
    }

    template<typename... T>
    [[noreturn]] void throw_error(sqlite3* db, T&&... args) {
        throw std::system_error{sqlite_errc(sqlite3_errcode(db)), get_error_message(db, std::forward<T>(args)...)};
    }

    inline std::system_error sqlite_to_system_error(int ev) {
        return {sqlite_errc(ev)};
    }

    inline std::system_error sqlite_to_system_error(sqlite3* db) {
        return {sqlite_errc(sqlite3_errcode(db)), sqlite3_errmsg(db)};
    }

    [[noreturn]] inline void throw_translated_sqlite_error(int ev) {
        throw sqlite_to_system_error(ev);
    }

    [[noreturn]] inline void throw_translated_sqlite_error(sqlite3* db) {
        throw sqlite_to_system_error(db);
    }

    [[noreturn]] inline void throw_translated_sqlite_error(sqlite3_stmt* stmt) {
        throw sqlite_to_system_error(sqlite3_db_handle(stmt));
    }
}
#pragma once

#include <string>  //  std::string
#include <memory>  //  std::shared_ptr, std::unique_ptr
#include <vector>  //  std::vector
#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
#include <optional>  // std::optional
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

// #include "functional/cxx_polyfill.h"

// #include "type_traits.h"

// #include "is_std_ptr.h"

#include <type_traits>
#include <memory>

namespace sqlite_orm {

    /**
     *  Specialization for optional type (std::shared_ptr / std::unique_ptr).
     */
    template<typename T>
    struct is_std_ptr : std::false_type {};

    template<typename T>
    struct is_std_ptr<std::shared_ptr<T>> : std::true_type {
        using element_type = typename std::shared_ptr<T>::element_type;

        static std::shared_ptr<T> make(std::remove_cv_t<T>&& v) {
            return std::make_shared<T>(std::move(v));
        }
    };

    template<typename T>
    struct is_std_ptr<std::unique_ptr<T>> : std::true_type {
        using element_type = typename std::unique_ptr<T>::element_type;

        static auto make(std::remove_cv_t<T>&& v) {
            return std::make_unique<T>(std::move(v));
        }
    };
}

namespace sqlite_orm {

    /**
     *  This class transforms a C++ type to a sqlite type name (int -> INTEGER, ...)
     */
    template<class T, typename Enable = void>
    struct type_printer {};

    struct integer_printer {
        const std::string& print() const {
            static const std::string res = "INTEGER";
            return res;
        }
    };

    struct text_printer {
        const std::string& print() const {
            static const std::string res = "TEXT";
            return res;
        }
    };

    struct real_printer {
        const std::string& print() const {
            static const std::string res = "REAL";
            return res;
        }
    };

    struct blob_printer {
        const std::string& print() const {
            static const std::string res = "BLOB";
            return res;
        }
    };

    // Note: char, unsigned/signed char are used for storing integer values, not char values.
    template<class T>
    struct type_printer<T,
                        std::enable_if_t<polyfill::conjunction_v<polyfill::negation<internal::is_any_of<T,
                                                                                                        wchar_t,
#ifdef __cpp_char8_t
                                                                                                        char8_t,
#endif
                                                                                                        char16_t,
                                                                                                        char32_t>>,
                                                                 std::is_integral<T>>>> : integer_printer {
    };

    template<class T>
    struct type_printer<T, std::enable_if_t<std::is_floating_point<T>::value>> : real_printer {};

    template<class T>
    struct type_printer<T,
                        std::enable_if_t<polyfill::disjunction_v<std::is_same<T, const char*>,
                                                                 std::is_base_of<std::string, T>,
                                                                 std::is_base_of<std::wstring, T>>>> : text_printer {};

    template<class T>
    struct type_printer<T, std::enable_if_t<is_std_ptr<T>::value>> : type_printer<typename T::element_type> {};

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
    template<class T>
    struct type_printer<T, std::enable_if_t<polyfill::is_specialization_of_v<T, std::optional>>>
        : type_printer<typename T::value_type> {};
#endif

    template<>
    struct type_printer<std::vector<char>, void> : blob_printer {};
}
#pragma once

namespace sqlite_orm {

    namespace internal {

        enum class collate_argument {
            binary,
            nocase,
            rtrim,
        };
    }

}
#pragma once

#include <system_error>  //  std::system_error
#include <ostream>  //  std::ostream
#include <string>  //  std::string
#include <tuple>  //  std::tuple, std::make_tuple
#include <type_traits>  //  std::is_base_of, std::false_type, std::true_type

// #include "functional/cxx_universal.h"

// #include "functional/cxx_polyfill.h"

// #include "functional/mpl.h"

/*
 *  Symbols for 'template metaprogramming' (compile-time template programming),
 *  inspired by the MPL of Aleksey Gurtovoy and David Abrahams.
 *  
 *  Currently, the focus is on facilitating advanced type filtering,
 *  such as filtering columns by constraints having various traits.
 *  Hence it contains only a very small subset of a full MPL.
 *  
 *  Two key concepts are critical to understanding:
 *  1. A 'metafunction' is a class template that represents a function invocable at compile-time.
 *  2. A 'metafunction class' is a certain form of metafunction representation that enables higher-order metaprogramming.
 *     More precisely, it's a class with a nested metafunction called "fn"
 *     Correspondingly, a metafunction class invocation is defined as invocation of its nested "fn" metafunction.
 *  
 *  Conventions:
 *  - "Fn" is the name for a metafunction template template parameter.
 *  - "FnCls" is the name for a metafunction class template parameter.
 *  - "_fn" is a suffix for a type that accepts metafunctions and turns them into metafunction classes.
 *  - "higher order" denotes a metafunction that operates on another metafunction (i.e. takes it as an argument).
 */

#include <type_traits>  //  std::false_type, std::true_type

// #include "cxx_universal.h"

// #include "cxx_polyfill.h"

namespace sqlite_orm {
    namespace internal {
        namespace mpl {
            template<template<class...> class Fn>
            struct indirectly_test_metafunction;

            /*
             *  Determines whether a class template has a nested metafunction `fn`.
             * 
             *  Implementation note: the technique of specialiazing on the inline variable must come first because
             *  of older compilers having problems with the detection of dependent templates [SQLITE_ORM_BROKEN_VARIADIC_PACK_EXPANSION].
             */
            template<class T, class SFINAE = void>
            SQLITE_ORM_INLINE_VAR constexpr bool is_metafunction_class_v = false;
            template<class FnCls>
            SQLITE_ORM_INLINE_VAR constexpr bool
                is_metafunction_class_v<FnCls, polyfill::void_t<indirectly_test_metafunction<FnCls::template fn>>> =
                    true;

            template<class T>
            struct is_metafunction_class : polyfill::bool_constant<is_metafunction_class_v<T>> {};

            /*
             *  Invoke metafunction.
             */
            template<template<class...> class Fn, class... Args>
            using invoke_fn_t = typename Fn<Args...>::type;

            /*
             *  Invoke metafunction class by invoking its nested metafunction.
             */
            template<class FnCls, class... Args>
            using invoke_t = typename FnCls::template fn<Args...>::type;

            /*
             *  Wrap given type such that `typename T::type` is valid.
             */
            template<class T, class SFINAE = void>
            struct type_wrap : polyfill::type_identity<T> {};
            template<class T>
            struct type_wrap<T, polyfill::void_t<typename T::type>> : T {};

            /*
             *  Turn metafunction into a metafunction class.
             *  
             *  Invocation of the nested metafunction `fn` is SFINAE-friendly (detection idiom).
             *  This is necessary because `fn` is a proxy to the originally quoted metafunction,
             *  and the instantiation of the metafunction might be an invalid expression.
             */
            template<template<class...> class Fn>
            struct quote_fn {
                template<class InvocableTest, template<class...> class, class...>
                struct invoke_fn;

                template<template<class...> class F, class... Args>
                struct invoke_fn<polyfill::void_t<F<Args...>>, F, Args...> {
                    using type = type_wrap<F<Args...>>;
                };

                template<class... Args>
                using fn = typename invoke_fn<void, Fn, Args...>::type;
            };

            /*
             *  Indirection wrapper for higher-order metafunctions,
             *  specialized on the argument indexes where metafunctions appear.
             */
            template<size_t...>
            struct higherorder;

            template<>
            struct higherorder<0u> {
                /*
                 *  Turn higher-order metafunction into a metafunction class.
                 */
                template<template<template<class...> class Fn, class... Args2> class HigherFn>
                struct quote_fn {
                    template<class QuotedFn, class... Args2>
                    struct fn : HigherFn<QuotedFn::template fn, Args2...> {};
                };
            };

            /*
             *  Metafunction class that extracts a the nested metafunction of its metafunction class argument,
             *  quotes the extracted metafunction and passes it on to the next metafunction class
             *  (kind of the inverse of quoting).
             */
            template<class FnCls>
            struct pass_extracted_fn_to {
                template<class... Args>
                struct fn : FnCls::template fn<Args...> {};

                // extract, quote, pass on
                template<template<class...> class Fn, class... Args>
                struct fn<Fn<Args...>> : FnCls::template fn<quote_fn<Fn>> {};
            };

            /*
             *  Bind arguments at the front of a metafunction class.
             *  Metafunction class equivalent to std::bind_front().
             */
            template<class FnCls, class... Bound>
            struct bind_front {
                template<class... Args>
                struct fn : FnCls::template fn<Bound..., Args...> {};
            };

            /*
             *  Bind arguments at the back of a metafunction class.
             *  Metafunction class equivalent to std::bind_back()
             */
            template<class FnCls, class... Bound>
            struct bind_back {
                template<class... Args>
                struct fn : FnCls::template fn<Args..., Bound...> {};
            };

            /*
             *  Metafunction class equivalent to polyfill::always_false.
             *  It ignores arguments passed to the metafunction,
             *  and always returns the given type.
             */
            template<class T>
            struct always {
                template<class...>
                struct fn : type_wrap<T> {};
            };

            /*
             *  Unary metafunction class equivalent to std::type_identity.
             */
            struct identity {
                template<class T>
                struct fn : type_wrap<T> {};
            };

            /*
             *  Metafunction class equivalent to std::negation.
             */
            template<class FnCls>
            struct not_ {
                template<class... Args>
                struct fn : polyfill::negation<invoke_t<FnCls, Args...>> {};
            };

            /*
             *  Metafunction class equivalent to std::conjunction
             */
            template<class... TraitFnCls>
            struct conjunction {
                template<class... Args>
                struct fn : polyfill::conjunction<typename TraitFnCls::template fn<Args...>...> {};
            };

            /*
             *  Metafunction class equivalent to std::disjunction.
             */
            template<class... TraitFnCls>
            struct disjunction {
                template<class... Args>
                struct fn : polyfill::disjunction<typename TraitFnCls::template fn<Args...>...> {};
            };

#ifndef SQLITE_ORM_BROKEN_VARIADIC_PACK_EXPANSION
            /*
             *  Metafunction equivalent to std::conjunction.
             */
            template<template<class...> class... TraitFn>
            using conjunction_fn = conjunction<quote_fn<TraitFn>...>;

            /*
             *  Metafunction equivalent to std::disjunction.
             */
            template<template<class...> class... TraitFn>
            using disjunction_fn = disjunction<quote_fn<TraitFn>...>;
#else
            template<template<class...> class... TraitFn>
            struct conjunction_fn : conjunction<quote_fn<TraitFn>...> {};

            template<template<class...> class... TraitFn>
            struct disjunction_fn : disjunction<quote_fn<TraitFn>...> {};
#endif

            /*
             *  Convenience template alias for binding arguments at the front of a metafunction.
             */
            template<template<class...> class Fn, class... Bound>
            using bind_front_fn = bind_front<quote_fn<Fn>, Bound...>;

            /*
             *  Convenience template alias for binding arguments at the back of a metafunction.
             */
            template<template<class...> class Fn, class... Bound>
            using bind_back_fn = bind_back<quote_fn<Fn>, Bound...>;

            /*
             *  Convenience template alias for binding a metafunction at the front of a higher-order metafunction.
             */
            template<template<template<class...> class Fn, class... Args2> class HigherFn,
                     template<class...>
                     class BoundFn,
                     class... Bound>
            using bind_front_higherorder_fn =
                bind_front<higherorder<0>::quote_fn<HigherFn>, quote_fn<BoundFn>, Bound...>;
        }
    }

    namespace mpl = internal::mpl;
}

// #include "tuple_helper/same_or_void.h"

namespace sqlite_orm {
    namespace internal {

        /**
         *  Accepts any number of arguments and evaluates `type` alias as T if all arguments are the same or void otherwise
         */
        template<class... Args>
        struct same_or_void {
            using type = void;
        };

        template<class A>
        struct same_or_void<A> {
            using type = A;
        };

        template<class A>
        struct same_or_void<A, A> {
            using type = A;
        };

        template<class A, class... Args>
        struct same_or_void<A, A, Args...> : same_or_void<A, Args...> {};

    }
}

// #include "tuple_helper/tuple_traits.h"

#include <type_traits>  //  std::is_same
#include <tuple>

// #include "../functional/cxx_polyfill.h"

// #include "../functional/mpl.h"

namespace sqlite_orm {
    namespace internal {

        template<template<class...> class Primary>
        using check_if_is_same_template =
            mpl::pass_extracted_fn_to<mpl::bind_front_fn<std::is_same, mpl::quote_fn<Primary>>>;

        /*
         *  Higher-order trait metafunction that checks whether a tuple contains a type with given trait.
         *  
         */
        template<template<class...> class TraitFn, class Tuple>
        struct tuple_has {};
        template<template<class...> class TraitFn, class... Types>
        struct tuple_has<TraitFn, std::tuple<Types...>> : polyfill::disjunction<TraitFn<Types>...> {};

        /*
         *  Metafunction class that checks whether a tuple contains a type with given trait.
         */
        template<template<class...> class TraitFn>
        using check_if_tuple_has = mpl::bind_front_higherorder_fn<tuple_has, TraitFn>;

        /*
         *  Metafunction class that checks whether a tuple doesn't contain a type with given trait.
         */
        template<template<class...> class TraitFn>
        using check_if_tuple_has_not = mpl::not_<check_if_tuple_has<TraitFn>>;

        /*
         *  Metafunction class that checks whether a tuple contains given type.
         */
        template<class T>
        using check_if_tuple_has_type =
            mpl::bind_front_higherorder_fn<tuple_has, mpl::bind_front_fn<std::is_same, T>::template fn>;

        /*
         *  Metafunction class that checks whether a tuple contains a given template.
         *
         *  Note: we are using 2 small tricks:
         *  1. A template template parameter can be treated like a metafunction, so we can just "quote" a 'primary'
         *     template into the MPL system (e.g. `std::vector`).
         *  2. This metafunction class does the opposite of the trait function `is_specialization`:
         *     `is_specialization` tries to instantiate the primary template template parameter using the
         *     template parameters of a template type, then compares both instantiated types.
         *     Here instead, `pass_extracted_fn_to` extracts the template template parameter from a template type,
         *     then compares the resulting template template parameters.
         */
        template<template<class...> class Primary>
        using check_if_tuple_has_template =
            mpl::bind_front_higherorder_fn<tuple_has, check_if_is_same_template<Primary>::template fn>;
    }
}
// #include "tuple_helper/tuple_filter.h"

#include <type_traits>  //  std::integral_constant, std::index_sequence, std::conditional, std::declval
#include <tuple>  //  std::tuple

// #include "../functional/cxx_universal.h"

// #include "../functional/mpl.h"

namespace sqlite_orm {
    namespace internal {

        template<typename... input_t>
        using tuple_cat_t = decltype(std::tuple_cat(std::declval<input_t>()...));

        template<class... Args>
        struct conc_tuple {
            using type = tuple_cat_t<Args...>;
        };

        template<class... Args>
        using conc_tuple_t = typename conc_tuple<Args...>::type;

        template<class T, template<class... C> class Fn>
        struct tuple_filter;

#ifndef SQLITE_ORM_BROKEN_VARIADIC_PACK_EXPANSION
        template<class... Types, template<class... C> class Fn>
        struct tuple_filter<std::tuple<Types...>, Fn>
            : conc_tuple<std::conditional_t<Fn<Types>::value, std::tuple<Types>, std::tuple<>>...> {};
#else
        template<class T, template<class... C> class Fn, class SFINAE = void>
        struct tuple_filter_single;

        template<class T, template<class... C> class Fn>
        struct tuple_filter_single<T, Fn, std::enable_if_t<!Fn<T>::value>> {
            using type = std::tuple<>;
        };

        template<class T, template<class... C> class Fn>
        struct tuple_filter_single<T, Fn, std::enable_if_t<Fn<T>::value>> {
            using type = std::tuple<T>;
        };

        template<class... Types, template<class... C> class Fn>
        struct tuple_filter<std::tuple<Types...>, Fn> : conc_tuple<typename tuple_filter_single<Types, Fn>::type...> {};
#endif
        template<class Tpl, template<class... C> class Fn>
        using filter_tuple_t = typename tuple_filter<Tpl, Fn>::type;

        /**
         *  Get the first value of an index_sequence.
         */
        template<size_t I, size_t... Idx>
        SQLITE_ORM_CONSTEVAL size_t first_index_sequence_value(std::index_sequence<I, Idx...>) {
            return I;
        }

        template<class... Seq>
        struct concat_idx_seq {
            using type = std::index_sequence<>;
        };

        template<size_t... Idx>
        struct concat_idx_seq<std::index_sequence<Idx...>> {
            using type = std::index_sequence<Idx...>;
        };

        template<size_t... As, size_t... Bs, class... Seq>
        struct concat_idx_seq<std::index_sequence<As...>, std::index_sequence<Bs...>, Seq...>
            : concat_idx_seq<std::index_sequence<As..., Bs...>, Seq...> {};

        template<class Tpl, template<class...> class Pred, template<class...> class Proj, class Seq>
        struct filter_tuple_sequence;

#ifndef SQLITE_ORM_BROKEN_VARIADIC_PACK_EXPANSION
        template<class Tpl, template<class...> class Pred, template<class...> class Proj, size_t... Idx>
        struct filter_tuple_sequence<Tpl, Pred, Proj, std::index_sequence<Idx...>>
            : concat_idx_seq<std::conditional_t<Pred<Proj<std::tuple_element_t<Idx, Tpl>>>::value,
                                                std::index_sequence<Idx>,
                                                std::index_sequence<>>...> {};
#else
        template<size_t Idx, class T, template<class...> class Pred, template<class...> class Proj, class SFINAE = void>
        struct tuple_seq_single;

        template<size_t Idx, class T, template<class...> class Pred, template<class...> class Proj>
        struct tuple_seq_single<Idx, T, Pred, Proj, std::enable_if_t<!Pred<Proj<T>>::value>> {
            using type = std::index_sequence<>;
        };

        template<size_t Idx, class T, template<class...> class Pred, template<class...> class Proj>
        struct tuple_seq_single<Idx, T, Pred, Proj, std::enable_if_t<Pred<Proj<T>>::value>> {
            using type = std::index_sequence<Idx>;
        };

        template<class Tpl, template<class...> class Pred, template<class...> class Proj, size_t... Idx>
        struct filter_tuple_sequence<Tpl, Pred, Proj, std::index_sequence<Idx...>>
            : concat_idx_seq<typename tuple_seq_single<Idx, std::tuple_element_t<Idx, Tpl>, Pred, Proj>::type...> {};
#endif

        template<class Tpl,
                 template<class...>
                 class Pred,
                 template<class...> class Proj = polyfill::type_identity_t,
                 class Seq = std::make_index_sequence<std::tuple_size<Tpl>::value>>
        using filter_tuple_sequence_t = typename filter_tuple_sequence<Tpl, Pred, Proj, Seq>::type;

        template<class T, template<class...> class Fn>
        struct count_tuple : std::integral_constant<int, filter_tuple_sequence_t<T, Fn>::size()> {};
    }
}

// #include "type_traits.h"

// #include "collate_argument.h"

// #include "error_code.h"

// #include "table_type.h"

namespace sqlite_orm {

    namespace internal {

        template<class T, class F>
        struct column_pointer;

        /**
         *  Trait class used to define table mapped type by setter/getter/member
         *  T - member pointer
         *  `type` is a type which is mapped.
         *  E.g.
         *  -   `table_type<decltype(&User::id)>::type` is `User`
         *  -   `table_type<decltype(&User::getName)>::type` is `User`
         *  -   `table_type<decltype(&User::setName)>::type` is `User`
         */
        template<class T, class SFINAE = void>
        struct table_type;

        template<class O, class F>
        struct table_type<F O::*, void> {
            using type = O;
        };

        template<class T, class F>
        struct table_type<column_pointer<T, F>, void> {
            using type = T;
        };
    }
}

// #include "type_printer.h"

namespace sqlite_orm {

    namespace internal {

        /**
         *  AUTOINCREMENT constraint class.
         */
        struct autoincrement_t {};

        struct primary_key_base {
            enum class order_by {
                unspecified,
                ascending,
                descending,
            };

            order_by asc_option = order_by::unspecified;

            operator std::string() const {
                std::string res = "PRIMARY KEY";
                switch(this->asc_option) {
                    case order_by::ascending:
                        res += " ASC";
                        break;
                    case order_by::descending:
                        res += " DESC";
                        break;
                    default:
                        break;
                }
                return res;
            }
        };

        /**
         *  PRIMARY KEY constraint class.
         *  Cs is parameter pack which contains columns (member pointers and/or function pointers). Can be empty when
         *  used within `make_column` function.
         */
        template<class... Cs>
        struct primary_key_t : primary_key_base {
            using order_by = primary_key_base::order_by;
            using columns_tuple = std::tuple<Cs...>;

            columns_tuple columns;

            primary_key_t(decltype(columns) c) : columns(move(c)) {}

            primary_key_t<Cs...> asc() const {
                auto res = *this;
                res.asc_option = order_by::ascending;
                return res;
            }

            primary_key_t<Cs...> desc() const {
                auto res = *this;
                res.asc_option = order_by::descending;
                return res;
            }
        };

        struct unique_base {
            operator std::string() const {
                return "UNIQUE";
            }
        };

        /**
         *  UNIQUE constraint class.
         */
        template<class... Args>
        struct unique_t : unique_base {
            using columns_tuple = std::tuple<Args...>;

            columns_tuple columns;

            unique_t(columns_tuple columns_) : columns(move(columns_)) {}
        };

        /**
         *  DEFAULT constraint class.
         *  T is a value type.
         */
        template<class T>
        struct default_t {
            using value_type = T;

            value_type value;

            operator std::string() const {
                return "DEFAULT";
            }
        };

#if SQLITE_VERSION_NUMBER >= 3006019

        /**
         *  FOREIGN KEY constraint class.
         *  Cs are columns which has foreign key
         *  Rs are column which C references to
         *  Available in SQLite 3.6.19 or higher
         */

        template<class A, class B>
        struct foreign_key_t;

        enum class foreign_key_action {
            none,  //  not specified
            no_action,
            restrict_,
            set_null,
            set_default,
            cascade,
        };

        inline std::ostream& operator<<(std::ostream& os, foreign_key_action action) {
            switch(action) {
                case foreign_key_action::no_action:
                    os << "NO ACTION";
                    break;
                case foreign_key_action::restrict_:
                    os << "RESTRICT";
                    break;
                case foreign_key_action::set_null:
                    os << "SET NULL";
                    break;
                case foreign_key_action::set_default:
                    os << "SET DEFAULT";
                    break;
                case foreign_key_action::cascade:
                    os << "CASCADE";
                    break;
                case foreign_key_action::none:
                    break;
            }
            return os;
        }

        struct on_update_delete_base {
            const bool update;  //  true if update and false if delete

            operator std::string() const {
                if(this->update) {
                    return "ON UPDATE";
                } else {
                    return "ON DELETE";
                }
            }
        };

        /**
         *  F - foreign key class
         */
        template<class F>
        struct on_update_delete_t : on_update_delete_base {
            using foreign_key_type = F;

            const foreign_key_type& fk;

            on_update_delete_t(decltype(fk) fk_, decltype(update) update_, foreign_key_action action_) :
                on_update_delete_base{update_}, fk(fk_), _action(action_) {}

            foreign_key_action _action = foreign_key_action::none;

            foreign_key_type no_action() const {
                auto res = this->fk;
                if(update) {
                    res.on_update._action = foreign_key_action::no_action;
                } else {
                    res.on_delete._action = foreign_key_action::no_action;
                }
                return res;
            }

            foreign_key_type restrict_() const {
                auto res = this->fk;
                if(update) {
                    res.on_update._action = foreign_key_action::restrict_;
                } else {
                    res.on_delete._action = foreign_key_action::restrict_;
                }
                return res;
            }

            foreign_key_type set_null() const {
                auto res = this->fk;
                if(update) {
                    res.on_update._action = foreign_key_action::set_null;
                } else {
                    res.on_delete._action = foreign_key_action::set_null;
                }
                return res;
            }

            foreign_key_type set_default() const {
                auto res = this->fk;
                if(update) {
                    res.on_update._action = foreign_key_action::set_default;
                } else {
                    res.on_delete._action = foreign_key_action::set_default;
                }
                return res;
            }

            foreign_key_type cascade() const {
                auto res = this->fk;
                if(update) {
                    res.on_update._action = foreign_key_action::cascade;
                } else {
                    res.on_delete._action = foreign_key_action::cascade;
                }
                return res;
            }

            operator bool() const {
                return this->_action != foreign_key_action::none;
            }
        };

        template<class F>
        bool operator==(const on_update_delete_t<F>& lhs, const on_update_delete_t<F>& rhs) {
            return lhs._action == rhs._action;
        }

        template<class... Cs, class... Rs>
        struct foreign_key_t<std::tuple<Cs...>, std::tuple<Rs...>> {
            using columns_type = std::tuple<Cs...>;
            using references_type = std::tuple<Rs...>;
            using self = foreign_key_t<columns_type, references_type>;

            /**
             * Holds obect type of all referenced columns.
             */
            using target_type = typename same_or_void<typename table_type<Rs>::type...>::type;

            /**
             * Holds obect type of all source columns.
             */
            using source_type = typename same_or_void<typename table_type<Cs>::type...>::type;

            columns_type columns;
            references_type references;

            on_update_delete_t<self> on_update;
            on_update_delete_t<self> on_delete;

            static_assert(std::tuple_size<columns_type>::value == std::tuple_size<references_type>::value,
                          "Columns size must be equal to references tuple");
            static_assert(!std::is_same<target_type, void>::value, "All references must have the same type");

            foreign_key_t(columns_type columns_, references_type references_) :
                columns(move(columns_)), references(move(references_)),
                on_update(*this, true, foreign_key_action::none), on_delete(*this, false, foreign_key_action::none) {}

            foreign_key_t(const self& other) :
                columns(other.columns), references(other.references), on_update(*this, true, other.on_update._action),
                on_delete(*this, false, other.on_delete._action) {}

            self& operator=(const self& other) {
                this->columns = other.columns;
                this->references = other.references;
                this->on_update = {*this, true, other.on_update._action};
                this->on_delete = {*this, false, other.on_delete._action};
                return *this;
            }
        };

        template<class A, class B>
        bool operator==(const foreign_key_t<A, B>& lhs, const foreign_key_t<A, B>& rhs) {
            return lhs.columns == rhs.columns && lhs.references == rhs.references && lhs.on_update == rhs.on_update &&
                   lhs.on_delete == rhs.on_delete;
        }

        /**
         *  Cs can be a class member pointer, a getter function member pointer or setter
         *  func member pointer
         *  Available in SQLite 3.6.19 or higher
         */
        template<class... Cs>
        struct foreign_key_intermediate_t {
            using tuple_type = std::tuple<Cs...>;

            tuple_type columns;

            template<class... Rs>
            foreign_key_t<std::tuple<Cs...>, std::tuple<Rs...>> references(Rs... refs) {
                return {std::move(this->columns), std::make_tuple(std::forward<Rs>(refs)...)};
            }
        };
#endif

        struct collate_constraint_t {
            collate_argument argument = collate_argument::binary;
#ifndef SQLITE_ORM_AGGREGATE_NSDMI_SUPPORTED
            collate_constraint_t(collate_argument argument) : argument{argument} {}
#endif

            operator std::string() const {
                return "COLLATE " + this->string_from_collate_argument(this->argument);
            }

            static std::string string_from_collate_argument(collate_argument argument) {
                switch(argument) {
                    case collate_argument::binary:
                        return "BINARY";
                    case collate_argument::nocase:
                        return "NOCASE";
                    case collate_argument::rtrim:
                        return "RTRIM";
                }
                throw std::system_error{orm_error_code::invalid_collate_argument_enum};
            }
        };

        template<class T>
        struct check_t {
            using expression_type = T;

            expression_type expression;
        };

        struct basic_generated_always {
            enum class storage_type {
                not_specified,
                virtual_,
                stored,
            };

            bool full = true;
            storage_type storage = storage_type::not_specified;

#ifndef SQLITE_ORM_AGGREGATE_NSDMI_SUPPORTED
            basic_generated_always(bool full, storage_type storage) : full{full}, storage{storage} {}
#endif
        };

        template<class T>
        struct generated_always_t : basic_generated_always {
            using expression_type = T;

            expression_type expression;

            generated_always_t(expression_type expression_, bool full, storage_type storage) :
                basic_generated_always{full, storage}, expression(std::move(expression_)) {}

            generated_always_t<T> virtual_() {
                return {std::move(this->expression), this->full, storage_type::virtual_};
            }

            generated_always_t<T> stored() {
                return {std::move(this->expression), this->full, storage_type::stored};
            }
        };

        template<class T>
        using is_generated_always = polyfill::is_specialization_of<T, generated_always_t>;

        template<class T>
        SQLITE_ORM_INLINE_VAR constexpr bool is_generated_always_v = is_generated_always<T>::value;

        template<class T>
        using is_constraint = polyfill::disjunction<std::is_same<T, autoincrement_t>,
                                                    polyfill::is_specialization_of<T, primary_key_t>,
                                                    polyfill::is_specialization_of<T, unique_t>,
                                                    polyfill::is_specialization_of<T, default_t>,
                                                    polyfill::is_specialization_of<T, foreign_key_t>,
                                                    std::is_same<T, collate_constraint_t>,
                                                    polyfill::is_specialization_of<T, check_t>,
#if SQLITE_VERSION_NUMBER >= 3031000
                                                    polyfill::is_specialization_of<T, generated_always_t>,
#endif
                                                    // dummy tail because of SQLITE_VERSION_NUMBER checks above
                                                    std::false_type>;
    }
#if SQLITE_VERSION_NUMBER >= 3031000
    template<class T>
    internal::generated_always_t<T> generated_always_as(T expression) {
        return {std::move(expression), true, internal::basic_generated_always::storage_type::not_specified};
    }

    template<class T>
    internal::generated_always_t<T> as(T expression) {
        return {std::move(expression), false, internal::basic_generated_always::storage_type::not_specified};
    }
#endif
#if SQLITE_VERSION_NUMBER >= 3006019

    /**
     *  FOREIGN KEY constraint construction function that takes member pointer as argument
     *  Available in SQLite 3.6.19 or higher
     */
    template<class... Cs>
    internal::foreign_key_intermediate_t<Cs...> foreign_key(Cs... columns) {
        return {std::make_tuple(std::forward<Cs>(columns)...)};
    }
#endif

    /**
     *  UNIQUE constraint builder function.
     */
    template<class... Args>
    internal::unique_t<Args...> unique(Args... args) {
        return {std::make_tuple(std::forward<Args>(args)...)};
    }

    inline internal::unique_t<> unique() {
        return {{}};
    }

    inline internal::autoincrement_t autoincrement() {
        return {};
    }

    template<class... Cs>
    internal::primary_key_t<Cs...> primary_key(Cs... cs) {
        return {std::make_tuple(std::forward<Cs>(cs)...)};
    }

    inline internal::primary_key_t<> primary_key() {
        return {{}};
    }

    template<class T>
    internal::default_t<T> default_value(T t) {
        return {std::move(t)};
    }

    inline internal::collate_constraint_t collate_nocase() {
        return {internal::collate_argument::nocase};
    }

    inline internal::collate_constraint_t collate_binary() {
        return {internal::collate_argument::binary};
    }

    inline internal::collate_constraint_t collate_rtrim() {
        return {internal::collate_argument::rtrim};
    }

    template<class T>
    internal::check_t<T> check(T t) {
        return {std::move(t)};
    }

    namespace internal {

        template<class T>
        SQLITE_ORM_INLINE_VAR constexpr bool is_foreign_key_v = polyfill::is_specialization_of_v<T, foreign_key_t>;

        template<class T>
        using is_foreign_key = polyfill::bool_constant<is_foreign_key_v<T>>;

        template<class T>
        using is_primary_key = polyfill::is_specialization_of<T, primary_key_t>;

        template<class T>
        SQLITE_ORM_INLINE_VAR constexpr bool is_primary_key_v = is_primary_key<T>::value;

        /**
         * PRIMARY KEY INSERTABLE traits.
         */
        template<typename T>
        struct is_primary_key_insertable
            : std::disjunction<mpl::invoke_t<mpl::disjunction<check_if_tuple_has_template<default_t>,
                                                              check_if_tuple_has_type<autoincrement_t>>,
                                             constraints_type_t<T>>,
                               std::is_base_of<integer_printer, type_printer<field_type_t<T>>>> {

            static_assert(tuple_has<is_primary_key, constraints_type_t<T>>::value, "an unexpected type was passed");
        };
    }

}
#pragma once

#include <memory>  //  std::shared_ptr, std::unique_ptr
#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
#include <optional>  // std::optional
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

// #include "functional/cxx_polyfill.h"

namespace sqlite_orm {

    /**
     *  This is class that tells `sqlite_orm` that type is nullable. Nullable types
     *  are mapped to sqlite database as `NULL` and not-nullable are mapped as `NOT NULL`.
     *  Default nullability status for all types is `NOT NULL`. So if you want to map
     *  custom type as `NULL` (for example: boost::optional) you have to create a specialiation
     *  of type_is_nullable for your type and derive from `std::true_type`.
     */
    template<class T>
    using type_is_nullable = polyfill::disjunction<
#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
        polyfill::is_specialization_of<T, std::optional>,
#endif
        polyfill::is_specialization_of<T, std::unique_ptr>,
        polyfill::is_specialization_of<T, std::shared_ptr>>;

}
#pragma once

#include <type_traits>  //  std::false_type, std::true_type
#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
#include <optional>  //  std::nullopt
#endif  //  SQLITE_ORM_OPTIONAL_SUPPORTED
// #include "tags.h"

namespace sqlite_orm {
    namespace internal {
        struct negatable_t {};

        /**
         *  Inherit from this class if target class can be chained with other conditions with '&&' and '||' operators
         */
        struct condition_t {};
    }
}

// #include "serialize_result_type.h"

#ifdef SQLITE_ORM_STRING_VIEW_SUPPORTED
#include <string_view>  //  string_view
#else
#include <string>  //  std::string
#endif

namespace sqlite_orm {
    namespace internal {
#ifdef SQLITE_ORM_STRING_VIEW_SUPPORTED
        using serialize_result_type = std::string_view;
#else
        using serialize_result_type = std::string;
#endif
    }
}

namespace sqlite_orm {

    namespace internal {

        /**
         *  Inherit this class to support arithmetic types overloading
         */
        struct arithmetic_t {};

        template<class L, class R, class... Ds>
        struct binary_operator : Ds... {
            using left_type = L;
            using right_type = R;

            left_type lhs;
            right_type rhs;

            binary_operator(left_type lhs_, right_type rhs_) : lhs(std::move(lhs_)), rhs(std::move(rhs_)) {}
        };

        struct conc_string {
            serialize_result_type serialize() const {
                return "||";
            }
        };

        /**
         *  Result of concatenation || operator
         */
        template<class L, class R>
        using conc_t = binary_operator<L, R, conc_string>;

        struct add_string {
            serialize_result_type serialize() const {
                return "+";
            }
        };

        /**
         *  Result of addition + operator
         */
        template<class L, class R>
        using add_t = binary_operator<L, R, add_string, arithmetic_t, negatable_t>;

        struct sub_string {
            serialize_result_type serialize() const {
                return "-";
            }
        };

        /**
         *  Result of substitute - operator
         */
        template<class L, class R>
        using sub_t = binary_operator<L, R, sub_string, arithmetic_t, negatable_t>;

        struct mul_string {
            serialize_result_type serialize() const {
                return "*";
            }
        };

        /**
         *  Result of multiply * operator
         */
        template<class L, class R>
        using mul_t = binary_operator<L, R, mul_string, arithmetic_t, negatable_t>;

        struct div_string {
            serialize_result_type serialize() const {
                return "/";
            }
        };

        /**
         *  Result of divide / operator
         */
        template<class L, class R>
        using div_t = binary_operator<L, R, div_string, arithmetic_t, negatable_t>;

        struct mod_operator_string {
            serialize_result_type serialize() const {
                return "%";
            }
        };

        /**
         *  Result of mod % operator
         */
        template<class L, class R>
        using mod_t = binary_operator<L, R, mod_operator_string, arithmetic_t, negatable_t>;

        struct bitwise_shift_left_string {
            serialize_result_type serialize() const {
                return "<<";
            }
        };

        /**
         * Result of bitwise shift left << operator
         */
        template<class L, class R>
        using bitwise_shift_left_t = binary_operator<L, R, bitwise_shift_left_string, arithmetic_t, negatable_t>;

        struct bitwise_shift_right_string {
            serialize_result_type serialize() const {
                return ">>";
            }
        };

        /**
         * Result of bitwise shift right >> operator
         */
        template<class L, class R>
        using bitwise_shift_right_t = binary_operator<L, R, bitwise_shift_right_string, arithmetic_t, negatable_t>;

        struct bitwise_and_string {
            serialize_result_type serialize() const {
                return "&";
            }
        };

        /**
         * Result of bitwise and & operator
         */
        template<class L, class R>
        using bitwise_and_t = binary_operator<L, R, bitwise_and_string, arithmetic_t, negatable_t>;

        struct bitwise_or_string {
            serialize_result_type serialize() const {
                return "|";
            }
        };

        /**
         * Result of bitwise or | operator
         */
        template<class L, class R>
        using bitwise_or_t = binary_operator<L, R, bitwise_or_string, arithmetic_t, negatable_t>;

        struct bitwise_not_string {
            serialize_result_type serialize() const {
                return "~";
            }
        };

        /**
         * Result of bitwise not ~ operator
         */
        template<class T>
        struct bitwise_not_t : bitwise_not_string, arithmetic_t, negatable_t {
            using argument_type = T;

            argument_type argument;

            bitwise_not_t(argument_type argument_) : argument(std::move(argument_)) {}
        };

        struct assign_string {
            serialize_result_type serialize() const {
                return "=";
            }
        };

        /**
         *  Result of assign = operator
         */
        template<class L, class R>
        using assign_t = binary_operator<L, R, assign_string>;

        /**
         *  Assign operator traits. Common case
         */
        template<class T>
        struct is_assign_t : public std::false_type {};

        /**
         *  Assign operator traits. Specialized case
         */
        template<class L, class R>
        struct is_assign_t<assign_t<L, R>> : public std::true_type {};

        template<class L, class... Args>
        struct in_t;

    }

    /**
     *  Public interface for || concatenation operator. Example: `select(conc(&User::name, "@gmail.com"));` => SELECT
     * name || '@gmail.com' FROM users
     */
    template<class L, class R>
    internal::conc_t<L, R> conc(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    /**
     *  Public interface for + operator. Example: `select(add(&User::age, 100));` => SELECT age + 100 FROM users
     */
    template<class L, class R>
    internal::add_t<L, R> add(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    /**
     *  Public interface for - operator. Example: `select(sub(&User::age, 1));` => SELECT age - 1 FROM users
     */
    template<class L, class R>
    internal::sub_t<L, R> sub(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    /**
     *  Public interface for * operator. Example: `select(mul(&User::salary, 2));` => SELECT salary * 2 FROM users
     */
    template<class L, class R>
    internal::mul_t<L, R> mul(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    /**
     *  Public interface for / operator. Example: `select(div(&User::salary, 3));` => SELECT salary / 3 FROM users
     *  @note Please notice that ::div function already exists in pure C standard library inside <cstdlib> header.
     *  If you use `using namespace sqlite_orm` directive you an specify which `div` you call explicitly using  `::div` or `sqlite_orm::div` statements.
     */
    template<class L, class R>
    internal::div_t<L, R> div(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    /**
     *  Public interface for % operator. Example: `select(mod(&User::age, 5));` => SELECT age % 5 FROM users
     */
    template<class L, class R>
    internal::mod_t<L, R> mod(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    internal::bitwise_shift_left_t<L, R> bitwise_shift_left(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    internal::bitwise_shift_right_t<L, R> bitwise_shift_right(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    internal::bitwise_and_t<L, R> bitwise_and(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    internal::bitwise_or_t<L, R> bitwise_or(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class T>
    internal::bitwise_not_t<T> bitwise_not(T t) {
        return {std::move(t)};
    }

    template<class L, class R>
    internal::assign_t<L, R> assign(L l, R r) {
        return {std::move(l), std::move(r)};
    }

}
#pragma once

#include <tuple>  //  std::tuple
#include <string>  //  std::string
#include <memory>  //  std::unique_ptr
#include <type_traits>  //  std::true_type, std::false_type, std::is_same, std::enable_if, std::decay

// #include "functional/cxx_universal.h"

// #include "functional/cxx_polyfill.h"

// #include "tuple_helper/tuple_traits.h"

// #include "tuple_helper/tuple_filter.h"

// #include "type_traits.h"

// #include "member_traits/member_traits.h"

#include <type_traits>  //  std::enable_if, std::is_function, std::true_type, std::false_type

// #include "../functional/cxx_universal.h"

// #include "../functional/cxx_polyfill.h"

namespace sqlite_orm {
    namespace internal {
        // SFINAE friendly trait to get a member object pointer's field type
        template<class T>
        struct object_field_type {};

        template<class T>
        using object_field_type_t = typename object_field_type<T>::type;

        template<class F, class O>
        struct object_field_type<F O::*> : std::enable_if<!std::is_function<F>::value, F> {};

        // SFINAE friendly trait to get a member function pointer's field type (i.e. unqualified return type)
        template<class T>
        struct getter_field_type {};

        template<class T>
        using getter_field_type_t = typename getter_field_type<T>::type;

        template<class T, class O>
        struct getter_field_type<T O::*> : getter_field_type<T> {};

        template<class F>
        struct getter_field_type<F(void) const> : polyfill::remove_cvref<F> {};

        template<class F>
        struct getter_field_type<F(void)> : polyfill::remove_cvref<F> {};

#ifdef SQLITE_ORM_NOTHROW_ALIASES_SUPPORTED
        template<class F>
        struct getter_field_type<F(void) const noexcept> : polyfill::remove_cvref<F> {};

        template<class F>
        struct getter_field_type<F(void) noexcept> : polyfill::remove_cvref<F> {};
#endif

        // SFINAE friendly trait to get a member function pointer's field type (i.e. unqualified parameter type)
        template<class T>
        struct setter_field_type {};

        template<class T>
        using setter_field_type_t = typename setter_field_type<T>::type;

        template<class T, class O>
        struct setter_field_type<T O::*> : setter_field_type<T> {};

        template<class F>
        struct setter_field_type<void(F)> : polyfill::remove_cvref<F> {};

#ifdef SQLITE_ORM_NOTHROW_ALIASES_SUPPORTED
        template<class F>
        struct setter_field_type<void(F) noexcept> : polyfill::remove_cvref<F> {};
#endif

        template<class T, class SFINAE = void>
        struct is_getter : std::false_type {};
        template<class T>
        struct is_getter<T, polyfill::void_t<getter_field_type_t<T>>> : std::true_type {};

        template<class T>
        SQLITE_ORM_INLINE_VAR constexpr bool is_getter_v = is_getter<T>::value;

        template<class T, class SFINAE = void>
        struct is_setter : std::false_type {};
        template<class T>
        struct is_setter<T, polyfill::void_t<setter_field_type_t<T>>> : std::true_type {};

        template<class T>
        SQLITE_ORM_INLINE_VAR constexpr bool is_setter_v = is_setter<T>::value;

        template<class T>
        struct member_field_type : object_field_type<T>, getter_field_type<T>, setter_field_type<T> {};

        template<class T>
        using member_field_type_t = typename member_field_type<T>::type;

        template<class T>
        struct member_object_type {};

        template<class F, class O>
        struct member_object_type<F O::*> : polyfill::type_identity<O> {};

        template<class T>
        using member_object_type_t = typename member_object_type<T>::type;
    }
}

// #include "type_is_nullable.h"

// #include "constraints.h"

namespace sqlite_orm {

    namespace internal {

        struct basic_column {

            /**
             *  Column name. Specified during construction in `make_column()`.
             */
            const std::string name;
        };

        struct empty_setter {};

        template<class G, class S>
        struct field_access_closure {
            using member_pointer_t = G;
            using setter_type = S;

            /**
             *  Member pointer used to read a field value.
             *  If it is a object member pointer it is also used to write a field value.
             */
            const member_pointer_t member_pointer;

            /**
             *  Setter member function to write a field value
             */
            SQLITE_ORM_NOUNIQUEADDRESS
            const setter_type setter;
        };

        /**
         *  This class stores information about a single column.
         *  column_t is a pair of [column_name:member_pointer] mapped to a storage.
         *  
         *  O is a mapped class, e.g. User
         *  T is a mapped class'es field type, e.g. &User::name
         *  Op... is a constraints pack, e.g. primary_key_t, autoincrement_t etc
         */
        template<class O, class T, class G, class S, class... Op>
        struct column_t : basic_column, field_access_closure<G, S> {
            using object_type = O;
            using field_type = T;
            using constraints_type = std::tuple<Op...>;

            /**
             *  Constraints tuple
             */
            constraints_type constraints;

#ifndef SQLITE_ORM_AGGREGATE_BASES_SUPPORTED
            column_t(std::string name, G memberPointer, S setter, std::tuple<Op...> op) :
                basic_column{move(name)}, field_access_closure<G, S>{memberPointer, setter}, constraints{move(op)} {}
#endif

            /**
             *  Simplified interface for `NOT NULL` constraint
             */
            constexpr bool is_not_null() const {
                return !type_is_nullable<field_type>::value;
            }

            /**
             *  Checks whether contraints are of trait `Trait`
             */
            template<template<class...> class Trait>
            constexpr bool is() const {
                return tuple_has<Trait, constraints_type>::value;
            }

            /**
             *  Simplified interface for `DEFAULT` constraint
             *  @return string representation of default value if it exists otherwise nullptr
             */
            std::unique_ptr<std::string> default_value() const;

            constexpr bool is_generated() const {
#if SQLITE_VERSION_NUMBER >= 3031000
                return is<is_generated_always>();
#else
                return false;
#endif
            }
        };

        template<class T>
        SQLITE_ORM_INLINE_VAR constexpr bool is_column_v = polyfill::is_specialization_of_v<T, column_t>;

        template<class T>
        using is_column = polyfill::bool_constant<is_column_v<T>>;

        /**
         *  Column with insertable primary key traits. Common case.
         */
        template<class T, class SFINAE = void>
        struct is_column_with_insertable_primary_key : std::false_type {};

        /**
         *  Column with insertable primary key traits. Specialized case case.
         */
        template<class C>
        struct is_column_with_insertable_primary_key<
            C,
            std::enable_if_t<tuple_has<is_primary_key, typename C::constraints_type>::value>>
            : polyfill::bool_constant<is_primary_key_insertable<C>::value> {};

        /**
         *  Column with noninsertable primary key traits. Common case.
         */
        template<class T, class SFINAE = void>
        struct is_column_with_noninsertable_primary_key : std::false_type {};

        /**
         *  Column with noninsertable primary key traits. Specialized case case.
         */
        template<class C>
        struct is_column_with_noninsertable_primary_key<
            C,
            std::enable_if_t<tuple_has<is_primary_key, typename C::constraints_type>::value>>
            : polyfill::bool_constant<!is_primary_key_insertable<C>::value> {};

        template<class T>
        struct column_field_type {
            using type = void;
        };

        template<class O, class T, class... Op>
        struct column_field_type<column_t<O, T, Op...>> {
            using type = typename column_t<O, T, Op...>::field_type;
        };

        template<class T>
        using column_field_type_t = typename column_field_type<T>::type;

        template<class T>
        struct column_constraints_type {
            using type = std::tuple<>;
        };

        template<class O, class T, class... Op>
        struct column_constraints_type<column_t<O, T, Op...>> {
            using type = typename column_t<O, T, Op...>::constraints_type;
        };

        template<class T>
        using column_constraints_type_t = typename column_constraints_type<T>::type;

    }

    /**
     *  Column builder function. You should use it to create columns instead of constructor
     */
    template<class M, internal::satisfies<std::is_member_object_pointer, M> = true, class... Op>
    internal::
        column_t<internal::member_object_type_t<M>, internal::object_field_type_t<M>, M, internal::empty_setter, Op...>
        make_column(std::string name, M m, Op... constraints) {
        static_assert(internal::count_tuple<std::tuple<Op...>, internal::is_constraint>::value ==
                          std::tuple_size<std::tuple<Op...>>::value,
                      "Incorrect constraints pack");
        SQLITE_ORM_CLANG_SUPPRESS_MISSING_BRACES(return {move(name), m, {}, std::make_tuple(constraints...)});
    }

    /**
     *  Column builder function with setter and getter. You should use it to create columns instead of constructor
     */
    template<class G,
             class S,
             internal::satisfies<internal::is_getter, G> = true,
             internal::satisfies<internal::is_setter, S> = true,
             class... Op>
    internal::column_t<internal::member_object_type_t<G>, internal::getter_field_type_t<G>, G, S, Op...>
    make_column(std::string name, S setter, G getter, Op... constraints) {
        static_assert(std::is_same<internal::setter_field_type_t<S>, internal::getter_field_type_t<G>>::value,
                      "Getter and setter must get and set same data type");
        static_assert(internal::count_tuple<std::tuple<Op...>, internal::is_constraint>::value ==
                          std::tuple_size<std::tuple<Op...>>::value,
                      "Incorrect constraints pack");
        SQLITE_ORM_CLANG_SUPPRESS_MISSING_BRACES(return {move(name), getter, setter, std::make_tuple(constraints...)});
    }

    /**
     *  Column builder function with getter and setter (reverse order). You should use it to create columns instead of
     * constructor
     */
    template<class G,
             class S,
             internal::satisfies<internal::is_getter, G> = true,
             internal::satisfies<internal::is_setter, S> = true,
             class... Op>
    internal::column_t<internal::member_object_type_t<G>, internal::getter_field_type_t<G>, G, S, Op...>
    make_column(std::string name, G getter, S setter, Op... constraints) {
        static_assert(std::is_same<internal::setter_field_type_t<S>, internal::getter_field_type_t<G>>::value,
                      "Getter and setter must get and set same data type");
        static_assert(internal::count_tuple<std::tuple<Op...>, internal::is_constraint>::value ==
                          std::tuple_size<std::tuple<Op...>>::value,
                      "Incorrect constraints pack");
        SQLITE_ORM_CLANG_SUPPRESS_MISSING_BRACES(return {move(name), getter, setter, std::make_tuple(constraints...)});
    }
}
#pragma once

#include <locale>  // std::wstring_convert
#include <string>  //  std::string
#include <sstream>  //  std::stringstream
#include <vector>  //  std::vector
#include <memory>  //  std::shared_ptr, std::unique_ptr
#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
#include <optional>  // std::optional
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED
#ifndef SQLITE_ORM_OMITS_CODECVT
#include <codecvt>  //  std::codecvt_utf8_utf16
#endif  //  SQLITE_ORM_OMITS_CODECVT

// #include "functional/cxx_universal.h"

// #include "functional/cxx_polyfill.h"

// #include "is_std_ptr.h"

namespace sqlite_orm {

    /**
     *  Is used to print members mapped to objects in storage_t::dump member function.
     *  Other developers can create own specialization to map custom types
     */
    template<class T, typename SFINAE = void>
    struct field_printer;

    namespace internal {
        template<class T, class SFINAE = void>
        SQLITE_ORM_INLINE_VAR constexpr bool is_printable_v = false;
        template<class T>
        SQLITE_ORM_INLINE_VAR constexpr bool is_printable_v<T, polyfill::void_t<decltype(field_printer<T>{})>> = true
            // Also see implementation note for `is_bindable_v`
            ;
        template<class T>
        using is_printable = polyfill::bool_constant<is_printable_v<T>>;
    }

    template<class T>
    struct field_printer<T, std::enable_if_t<std::is_arithmetic<T>::value>> {
        std::string operator()(const T& t) const {
            std::stringstream ss;
            ss << t;
            return ss.str();
        }
    };

    /**
     *  Upgrade to integer is required when using unsigned char(uint8_t)
     */
    template<>
    struct field_printer<unsigned char, void> {
        std::string operator()(const unsigned char& t) const {
            std::stringstream ss;
            ss << +t;
            return ss.str();
        }
    };

    /**
     *  Upgrade to integer is required when using signed char(int8_t)
     */
    template<>
    struct field_printer<signed char, void> {
        std::string operator()(const signed char& t) const {
            std::stringstream ss;
            ss << +t;
            return ss.str();
        }
    };

    /**
     *  char is neither signed char nor unsigned char so it has its own specialization
     */
    template<>
    struct field_printer<char, void> {
        std::string operator()(const char& t) const {
            std::stringstream ss;
            ss << +t;
            return ss.str();
        }
    };

    template<class T>
    struct field_printer<T, std::enable_if_t<std::is_base_of<std::string, T>::value>> {
        std::string operator()(std::string string) const {
            return string;
        }
    };

    template<>
    struct field_printer<std::vector<char>, void> {
        std::string operator()(const std::vector<char>& t) const {
            std::stringstream ss;
            ss << std::hex;
            for(auto c: t) {
                ss << c;
            }
            return ss.str();
        }
    };
#ifndef SQLITE_ORM_OMITS_CODECVT
    /**
     *  Specialization for std::wstring (UTF-16 assumed).
     */
    template<class T>
    struct field_printer<T, std::enable_if_t<std::is_base_of<std::wstring, T>::value>> {
        std::string operator()(const std::wstring& wideString) const {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            return converter.to_bytes(wideString);
        }
    };
#endif  //  SQLITE_ORM_OMITS_CODECVT
    template<>
    struct field_printer<nullptr_t, void> {
        std::string operator()(const nullptr_t&) const {
            return "null";
        }
    };
#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
    template<>
    struct field_printer<std::nullopt_t, void> {
        std::string operator()(const std::nullopt_t&) const {
            return "null";
        }
    };
#endif  //  SQLITE_ORM_OPTIONAL_SUPPORTED
    template<class T>
    struct field_printer<
        T,
        std::enable_if_t<polyfill::conjunction_v<is_std_ptr<T>,
                                                 internal::is_printable<std::remove_cv_t<typename T::element_type>>>>> {
        using unqualified_type = std::remove_cv_t<typename T::element_type>;

        std::string operator()(const T& t) const {
            if(t) {
                return field_printer<unqualified_type>()(*t);
            } else {
                return field_printer<nullptr_t>{}(nullptr);
            }
        }
    };

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
    template<class T>
    struct field_printer<
        T,
        std::enable_if_t<polyfill::conjunction_v<polyfill::is_specialization_of<T, std::optional>,
                                                 internal::is_printable<std::remove_cv_t<typename T::value_type>>>>> {
        using unqualified_type = std::remove_cv_t<typename T::value_type>;

        std::string operator()(const T& t) const {
            if(t.has_value()) {
                return field_printer<unqualified_type>()(*t);
            } else {
                return field_printer<std::nullopt_t>{}(std::nullopt);
            }
        }
    };
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED
}
#pragma once

#include <string>  //  std::string
#include <type_traits>  //  std::enable_if, std::is_same
#include <vector>  //  std::vector
#include <tuple>  //  std::tuple, std::tuple_size
#include <sstream>  //  std::stringstream

// #include "functional/cxx_universal.h"

// #include "functional/cxx_polyfill.h"

// #include "type_traits.h"

// #include "collate_argument.h"

// #include "constraints.h"

// #include "optional_container.h"

namespace sqlite_orm {

    namespace internal {

        /**
         *  This is a cute class which allows storing something or nothing
         *  depending on template argument. Useful for optional class members
         */
        template<class T>
        struct optional_container {
            using type = T;

            type field;

            template<class L>
            void apply(const L& l) const {
                l(this->field);
            }
        };

        template<>
        struct optional_container<void> {
            using type = void;

            template<class L>
            void apply(const L&) const {
                //..
            }
        };
    }
}

// #include "serializer_context.h"

namespace sqlite_orm {

    namespace internal {

        struct serializer_context_base {
            bool replace_bindable_with_question = false;
            bool skip_table_name = true;
            bool use_parentheses = true;
        };

        template<class I>
        struct serializer_context : serializer_context_base {
            using impl_type = I;

            const impl_type& impl;

            serializer_context(const impl_type& impl_) : impl(impl_) {}
        };

        template<class S>
        struct serializer_context_builder {
            using storage_type = S;
            using impl_type = typename storage_type::impl_type;

            serializer_context_builder(const storage_type& storage_) : storage(storage_) {}

            serializer_context<impl_type> operator()() const {
                return {obtain_const_impl(this->storage)};
            }

            const storage_type& storage;
        };

    }

}

// #include "tags.h"

// #include "expression.h"

#include <tuple>
#include <utility>  //  std::move, std::forward
#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
#include <optional>
#endif

// #include "functional/cxx_universal.h"

// #include "operators.h"

namespace sqlite_orm {

    namespace internal {

        template<class L, class R>
        struct and_condition_t;

        template<class L, class R>
        struct or_condition_t;

        /**
         *  Is not an operator but a result of c(...) function. Has operator= overloaded which returns assign_t
         */
        template<class T>
        struct expression_t : condition_t {
            T value;

            expression_t(T value_) : value(std::move(value_)) {}

            template<class R>
            assign_t<T, R> operator=(R r) const {
                return {this->value, std::move(r)};
            }

            assign_t<T, nullptr_t> operator=(nullptr_t) const {
                return {this->value, nullptr};
            }
#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
            assign_t<T, std::nullopt_t> operator=(std::nullopt_t) const {
                return {this->value, std::nullopt};
            }
#endif
            template<class... Args>
            in_t<T, Args...> in(Args... args) const {
                return {this->value, std::make_tuple(std::forward<Args>(args)...), false};
            }

            template<class... Args>
            in_t<T, Args...> not_in(Args... args) const {
                return {this->value, std::make_tuple(std::forward<Args>(args)...), true};
            }

            template<class R>
            and_condition_t<T, R> and_(R right) const {
                return {this->value, std::move(right)};
            }

            template<class R>
            or_condition_t<T, R> or_(R right) const {
                return {this->value, std::move(right)};
            }
        };

        template<class T>
        T get_from_expression(T value) {
            return std::move(value);
        }

        template<class T>
        T get_from_expression(expression_t<T> expression) {
            return std::move(expression.value);
        }
    }

    /**
     *  Public interface for syntax sugar for columns. Example: `where(c(&User::id) == 5)` or
     * `storage.update(set(c(&User::name) = "Dua Lipa"));
     */
    template<class T>
    internal::expression_t<T> c(T value) {
        return {std::move(value)};
    }
}

// #include "type_printer.h"

// #include "literal.h"

namespace sqlite_orm {
    namespace internal {

        /* 
         *  Protect an otherwise bindable element so that it is always serialized as a literal value.
         */
        template<class T>
        struct literal_holder {
            using type = T;

            type value;
        };

    }
}

namespace sqlite_orm {

    namespace internal {

        struct limit_string {
            operator std::string() const {
                return "LIMIT";
            }
        };

        /**
         *  Stores LIMIT/OFFSET info
         */
        template<class T, bool has_offset, bool offset_is_implicit, class O>
        struct limit_t : limit_string {
            T lim;
            optional_container<O> off;

            limit_t() = default;

            limit_t(decltype(lim) lim_) : lim(std::move(lim_)) {}

            limit_t(decltype(lim) lim_, decltype(off) off_) : lim(std::move(lim_)), off(std::move(off_)) {}
        };

        template<class T>
        struct is_limit : std::false_type {};

        template<class T, bool has_offset, bool offset_is_implicit, class O>
        struct is_limit<limit_t<T, has_offset, offset_is_implicit, O>> : std::true_type {};

        /**
         *  Stores OFFSET only info
         */
        template<class T>
        struct offset_t {
            T off;
        };

        template<class T>
        using is_offset = polyfill::is_specialization_of<T, offset_t>;

        /**
         *  Collated something
         */
        template<class T>
        struct collate_t : public condition_t {
            T expr;
            collate_argument argument;

            collate_t(T expr_, collate_argument argument_) : expr(std::move(expr_)), argument(argument_) {}

            operator std::string() const {
                return collate_constraint_t{this->argument};
            }
        };

        struct named_collate_base {
            std::string name;

            operator std::string() const {
                return "COLLATE " + this->name;
            }
        };

        /**
         *  Collated something with custom collate function
         */
        template<class T>
        struct named_collate : named_collate_base {
            T expr;

            named_collate(T expr_, std::string name_) : named_collate_base{std::move(name_)}, expr(std::move(expr_)) {}
        };

        struct negated_condition_string {
            operator std::string() const {
                return "NOT";
            }
        };

        /**
         *  Result of not operator
         */
        template<class C>
        struct negated_condition_t : condition_t, negated_condition_string {
            C c;

            negated_condition_t(C c_) : c(std::move(c_)) {}
        };

        /**
         *  Base class for binary conditions
         *  L is left argument type
         *  R is right argument type
         *  S is 'string' class (a class which has cast to `std::string` operator)
         *  Res is result type
         */
        template<class L, class R, class S, class Res>
        struct binary_condition : condition_t, S {
            using left_type = L;
            using right_type = R;
            using result_type = Res;

            left_type l;
            right_type r;

            binary_condition() = default;

            binary_condition(left_type l_, right_type r_) : l(std::move(l_)), r(std::move(r_)) {}
        };

        struct and_condition_string {
            operator std::string() const {
                return "AND";
            }
        };

        /**
         *  Result of and operator
         */
        template<class L, class R>
        struct and_condition_t : binary_condition<L, R, and_condition_string, bool> {
            using super = binary_condition<L, R, and_condition_string, bool>;

            using super::super;
        };

        template<class L, class R>
        and_condition_t<L, R> make_and_condition(L left, R right) {
            return {std::move(left), std::move(right)};
        }

        struct or_condition_string {
            operator std::string() const {
                return "OR";
            }
        };

        /**
         *  Result of or operator
         */
        template<class L, class R>
        struct or_condition_t : binary_condition<L, R, or_condition_string, bool> {
            using super = binary_condition<L, R, or_condition_string, bool>;

            using super::super;
        };

        template<class L, class R>
        or_condition_t<L, R> make_or_condition(L left, R right) {
            return {std::move(left), std::move(right)};
        }

        struct is_equal_string {
            operator std::string() const {
                return "=";
            }
        };

        /**
         *  = and == operators object
         */
        template<class L, class R>
        struct is_equal_t : binary_condition<L, R, is_equal_string, bool>, negatable_t {
            using self = is_equal_t<L, R>;

            using binary_condition<L, R, is_equal_string, bool>::binary_condition;

            collate_t<self> collate_binary() const {
                return {*this, collate_argument::binary};
            }

            collate_t<self> collate_nocase() const {
                return {*this, collate_argument::nocase};
            }

            collate_t<self> collate_rtrim() const {
                return {*this, collate_argument::rtrim};
            }

            named_collate<self> collate(std::string name) const {
                return {*this, std::move(name)};
            }

            template<class C>
            named_collate<self> collate() const {
                std::stringstream ss;
                ss << C::name() << std::flush;
                return {*this, ss.str()};
            }
        };

        struct is_not_equal_string {
            operator std::string() const {
                return "!=";
            }
        };

        /**
         *  != operator object
         */
        template<class L, class R>
        struct is_not_equal_t : binary_condition<L, R, is_not_equal_string, bool>, negatable_t {
            using self = is_not_equal_t<L, R>;

            using binary_condition<L, R, is_not_equal_string, bool>::binary_condition;

            collate_t<self> collate_binary() const {
                return {*this, collate_argument::binary};
            }

            collate_t<self> collate_nocase() const {
                return {*this, collate_argument::nocase};
            }

            collate_t<self> collate_rtrim() const {
                return {*this, collate_argument::rtrim};
            }
        };

        struct greater_than_string {
            operator std::string() const {
                return ">";
            }
        };

        /**
         *  > operator object.
         */
        template<class L, class R>
        struct greater_than_t : binary_condition<L, R, greater_than_string, bool>, negatable_t {
            using self = greater_than_t<L, R>;

            using binary_condition<L, R, greater_than_string, bool>::binary_condition;

            collate_t<self> collate_binary() const {
                return {*this, collate_argument::binary};
            }

            collate_t<self> collate_nocase() const {
                return {*this, collate_argument::nocase};
            }

            collate_t<self> collate_rtrim() const {
                return {*this, collate_argument::rtrim};
            }
        };

        struct greater_or_equal_string {
            operator std::string() const {
                return ">=";
            }
        };

        /**
         *  >= operator object.
         */
        template<class L, class R>
        struct greater_or_equal_t : binary_condition<L, R, greater_or_equal_string, bool>, negatable_t {
            using self = greater_or_equal_t<L, R>;

            using binary_condition<L, R, greater_or_equal_string, bool>::binary_condition;

            collate_t<self> collate_binary() const {
                return {*this, collate_argument::binary};
            }

            collate_t<self> collate_nocase() const {
                return {*this, collate_argument::nocase};
            }

            collate_t<self> collate_rtrim() const {
                return {*this, collate_argument::rtrim};
            }
        };

        struct lesser_than_string {
            operator std::string() const {
                return "<";
            }
        };

        /**
         *  < operator object.
         */
        template<class L, class R>
        struct lesser_than_t : binary_condition<L, R, lesser_than_string, bool>, negatable_t {
            using self = lesser_than_t<L, R>;

            using binary_condition<L, R, lesser_than_string, bool>::binary_condition;

            collate_t<self> collate_binary() const {
                return {*this, collate_argument::binary};
            }

            collate_t<self> collate_nocase() const {
                return {*this, collate_argument::nocase};
            }

            collate_t<self> collate_rtrim() const {
                return {*this, collate_argument::rtrim};
            }
        };

        struct lesser_or_equal_string {
            operator std::string() const {
                return "<=";
            }
        };

        /**
         *  <= operator object.
         */
        template<class L, class R>
        struct lesser_or_equal_t : binary_condition<L, R, lesser_or_equal_string, bool>, negatable_t {
            using self = lesser_or_equal_t<L, R>;

            using binary_condition<L, R, lesser_or_equal_string, bool>::binary_condition;

            collate_t<self> collate_binary() const {
                return {*this, collate_argument::binary};
            }

            collate_t<self> collate_nocase() const {
                return {*this, collate_argument::nocase};
            }

            collate_t<self> collate_rtrim() const {
                return {*this, collate_argument::rtrim};
            }
        };

        struct in_base {
            bool negative = false;  //  used in not_in

#ifndef SQLITE_ORM_AGGREGATE_NSDMI_SUPPORTED
            in_base(bool negative) : negative{negative} {}
#endif
        };

        /**
         *  IN operator object.
         */
        template<class L, class A>
        struct dynamic_in_t : condition_t, in_base, negatable_t {
            using self = dynamic_in_t<L, A>;

            L left;  //  left expression
            A argument;  //  in arg

            dynamic_in_t(L left_, A argument_, bool negative_) :
                in_base{negative_}, left(std::move(left_)), argument(std::move(argument_)) {}
        };

        template<class L, class... Args>
        struct in_t : condition_t, in_base, negatable_t {
            L left;
            std::tuple<Args...> argument;

            in_t(L left_, decltype(argument) argument_, bool negative_) :
                in_base{negative_}, left(std::move(left_)), argument(std::move(argument_)) {}
        };

        struct is_null_string {
            operator std::string() const {
                return "IS NULL";
            }
        };

        /**
         *  IS NULL operator object.
         */
        template<class T>
        struct is_null_t : is_null_string, negatable_t {
            using self = is_null_t<T>;

            T t;

            is_null_t(T t_) : t(std::move(t_)) {}
        };

        struct is_not_null_string {
            operator std::string() const {
                return "IS NOT NULL";
            }
        };

        /**
         *  IS NOT NULL operator object.
         */
        template<class T>
        struct is_not_null_t : is_not_null_string, negatable_t {
            using self = is_not_null_t<T>;

            T t;

            is_not_null_t(T t_) : t(std::move(t_)) {}
        };

        struct order_by_base {
            int asc_desc = 0;  //  1: asc, -1: desc
            std::string _collate_argument;

#ifndef SQLITE_ORM_AGGREGATE_NSDMI_SUPPORTED
            order_by_base() = default;

            order_by_base(decltype(asc_desc) asc_desc_, decltype(_collate_argument) _collate_argument_) :
                asc_desc(asc_desc_), _collate_argument(move(_collate_argument_)) {}
#endif
        };

        struct order_by_string {
            operator std::string() const {
                return "ORDER BY";
            }
        };

        /**
         *  ORDER BY argument holder.
         */
        template<class O>
        struct order_by_t : order_by_base, order_by_string {
            using expression_type = O;
            using self = order_by_t<expression_type>;

            expression_type expression;

            order_by_t(expression_type expression_) : order_by_base(), expression(std::move(expression_)) {}

            self asc() const {
                auto res = *this;
                res.asc_desc = 1;
                return res;
            }

            self desc() const {
                auto res = *this;
                res.asc_desc = -1;
                return res;
            }

            self collate_binary() const {
                auto res = *this;
                res._collate_argument = collate_constraint_t::string_from_collate_argument(collate_argument::binary);
                return res;
            }

            self collate_nocase() const {
                auto res = *this;
                res._collate_argument = collate_constraint_t::string_from_collate_argument(collate_argument::nocase);
                return res;
            }

            self collate_rtrim() const {
                auto res = *this;
                res._collate_argument = collate_constraint_t::string_from_collate_argument(collate_argument::rtrim);
                return res;
            }

            self collate(std::string name) const {
                auto res = *this;
                res._collate_argument = std::move(name);
                return res;
            }

            template<class C>
            self collate() const {
                std::stringstream ss;
                ss << C::name() << std::flush;
                return this->collate(ss.str());
            }
        };

        /**
         *  ORDER BY pack holder.
         */
        template<class... Args>
        struct multi_order_by_t : order_by_string {
            using args_type = std::tuple<Args...>;

            args_type args;

            multi_order_by_t(args_type args_) : args{std::move(args_)} {}
        };

        struct dynamic_order_by_entry_t : order_by_base {
            std::string name;

            dynamic_order_by_entry_t(decltype(name) name_, int asc_desc_, std::string collate_argument_) :
                order_by_base{asc_desc_, move(collate_argument_)}, name(move(name_)) {}
        };

        /**
         *  C - serializer context class
         */
        template<class C>
        struct dynamic_order_by_t : order_by_string {
            using context_t = C;
            using entry_t = dynamic_order_by_entry_t;
            using const_iterator = typename std::vector<entry_t>::const_iterator;

            dynamic_order_by_t(const context_t& context_) : context(context_) {}

            template<class O>
            void push_back(order_by_t<O> order_by) {
                auto newContext = this->context;
                newContext.skip_table_name = true;
                auto columnName = serialize(order_by.expression, newContext);
                entries.emplace_back(move(columnName), order_by.asc_desc, move(order_by._collate_argument));
            }

            const_iterator begin() const {
                return this->entries.begin();
            }

            const_iterator end() const {
                return this->entries.end();
            }

            void clear() {
                this->entries.clear();
            }

          protected:
            std::vector<entry_t> entries;
            context_t context;
        };

        template<class T>
        SQLITE_ORM_INLINE_VAR constexpr bool is_order_by_v =
            polyfill::disjunction_v<polyfill::is_specialization_of<T, order_by_t>,
                                    polyfill::is_specialization_of<T, multi_order_by_t>,
                                    polyfill::is_specialization_of<T, dynamic_order_by_t>>;

        template<class T>
        using is_order_by = polyfill::bool_constant<is_order_by_v<T>>;

        struct between_string {
            operator std::string() const {
                return "BETWEEN";
            }
        };

        /**
         *  BETWEEN operator object.
         */
        template<class A, class T>
        struct between_t : condition_t, between_string {
            using expression_type = A;
            using lower_type = T;
            using upper_type = T;

            expression_type expr;
            lower_type b1;
            upper_type b2;

            between_t(expression_type expr_, lower_type b1_, upper_type b2_) :
                expr(std::move(expr_)), b1(std::move(b1_)), b2(std::move(b2_)) {}
        };

        struct like_string {
            operator std::string() const {
                return "LIKE";
            }
        };

        /**
         *  LIKE operator object.
         */
        template<class A, class T, class E>
        struct like_t : condition_t, like_string, negatable_t {
            using self = like_t<A, T, E>;
            using arg_t = A;
            using pattern_t = T;
            using escape_t = E;

            arg_t arg;
            pattern_t pattern;
            optional_container<escape_t> arg3;  //  not escape cause escape exists as a function here

            like_t(arg_t arg_, pattern_t pattern_, optional_container<escape_t> escape_) :
                arg(std::move(arg_)), pattern(std::move(pattern_)), arg3(std::move(escape_)) {}

            template<class C>
            like_t<A, T, C> escape(C c) const {
                optional_container<C> newArg3{std::move(c)};
                return {std::move(this->arg), std::move(this->pattern), std::move(newArg3)};
            }
        };

        struct glob_string {
            operator std::string() const {
                return "GLOB";
            }
        };

        template<class A, class T>
        struct glob_t : condition_t, glob_string, internal::negatable_t {
            using self = glob_t<A, T>;
            using arg_t = A;
            using pattern_t = T;

            arg_t arg;
            pattern_t pattern;

            glob_t(arg_t arg_, pattern_t pattern_) : arg(std::move(arg_)), pattern(std::move(pattern_)) {}
        };

        struct cross_join_string {
            operator std::string() const {
                return "CROSS JOIN";
            }
        };

        /**
         *  CROSS JOIN holder.
         *  T is joined type which represents any mapped table.
         */
        template<class T>
        struct cross_join_t : cross_join_string {
            using type = T;
        };

        struct natural_join_string {
            operator std::string() const {
                return "NATURAL JOIN";
            }
        };

        /**
         *  NATURAL JOIN holder.
         *  T is joined type which represents any mapped table.
         */
        template<class T>
        struct natural_join_t : natural_join_string {
            using type = T;
        };

        struct left_join_string {
            operator std::string() const {
                return "LEFT JOIN";
            }
        };

        /**
         *  LEFT JOIN holder.
         *  T is joined type which represents any mapped table.
         *  O is on(...) argument type.
         */
        template<class T, class O>
        struct left_join_t : left_join_string {
            using type = T;
            using on_type = O;

            on_type constraint;

            left_join_t(on_type constraint_) : constraint(std::move(constraint_)) {}
        };

        struct join_string {
            operator std::string() const {
                return "JOIN";
            }
        };

        /**
         *  Simple JOIN holder.
         *  T is joined type which represents any mapped table.
         *  O is on(...) argument type.
         */
        template<class T, class O>
        struct join_t : join_string {
            using type = T;
            using on_type = O;

            on_type constraint;

            join_t(on_type constraint_) : constraint(std::move(constraint_)) {}
        };

        struct left_outer_join_string {
            operator std::string() const {
                return "LEFT OUTER JOIN";
            }
        };

        /**
         *  LEFT OUTER JOIN holder.
         *  T is joined type which represents any mapped table.
         *  O is on(...) argument type.
         */
        template<class T, class O>
        struct left_outer_join_t : left_outer_join_string {
            using type = T;
            using on_type = O;

            on_type constraint;

            left_outer_join_t(on_type constraint_) : constraint(std::move(constraint_)) {}
        };

        struct on_string {
            operator std::string() const {
                return "ON";
            }
        };

        /**
         *  on(...) argument holder used for JOIN, LEFT JOIN, LEFT OUTER JOIN and INNER JOIN
         *  T is on type argument.
         */
        template<class T>
        struct on_t : on_string {
            using arg_type = T;

            arg_type arg;

            on_t(arg_type arg_) : arg(std::move(arg_)) {}
        };

        /**
         *  USING argument holder.
         */
        template<class T, class M>
        struct using_t {
            column_pointer<T, M> column;

            operator std::string() const {
                return "USING";
            }
        };

        struct inner_join_string {
            operator std::string() const {
                return "INNER JOIN";
            }
        };

        /**
         *  INNER JOIN holder.
         *  T is joined type which represents any mapped table.
         *  O is on(...) argument type.
         */
        template<class T, class O>
        struct inner_join_t : inner_join_string {
            using type = T;
            using on_type = O;

            on_type constraint;

            inner_join_t(on_type constraint_) : constraint(std::move(constraint_)) {}
        };

        struct cast_string {
            operator std::string() const {
                return "CAST";
            }
        };

        /**
         *  CAST holder.
         *  T is a type to cast to
         *  E is an expression type
         *  Example: cast<std::string>(&User::id)
         */
        template<class T, class E>
        struct cast_t : cast_string {
            using to_type = T;
            using expression_type = E;

            expression_type expression;

            cast_t(expression_type expression_) : expression(std::move(expression_)) {}
        };

        template<class... Args>
        struct from_t {
            using tuple_type = std::tuple<Args...>;
        };

        template<class T>
        using is_from = polyfill::is_specialization_of<T, from_t>;
    }

    /**
     *  Explicit FROM function. Usage:
     *  `storage.select(&User::id, from<User>());`
     */
    template<class... Args>
    internal::from_t<Args...> from() {
        static_assert(std::tuple_size<std::tuple<Args...>>::value > 0, "");
        return {};
    }

    template<class T, internal::satisfies<std::is_base_of, internal::negatable_t, T> = true>
    internal::negated_condition_t<T> operator!(T arg) {
        return {std::move(arg)};
    }

    /**
     *  Cute operators for columns
     */
    template<class T, class R>
    internal::lesser_than_t<T, R> operator<(internal::expression_t<T> expr, R r) {
        return {std::move(expr.value), std::move(r)};
    }

    template<class L, class T>
    internal::lesser_than_t<L, T> operator<(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.value)};
    }

    template<class T, class R>
    internal::lesser_or_equal_t<T, R> operator<=(internal::expression_t<T> expr, R r) {
        return {std::move(expr.value), std::move(r)};
    }

    template<class L, class T>
    internal::lesser_or_equal_t<L, T> operator<=(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.value)};
    }

    template<class T, class R>
    internal::greater_than_t<T, R> operator>(internal::expression_t<T> expr, R r) {
        return {std::move(expr.value), std::move(r)};
    }

    template<class L, class T>
    internal::greater_than_t<L, T> operator>(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.value)};
    }

    template<class T, class R>
    internal::greater_or_equal_t<T, R> operator>=(internal::expression_t<T> expr, R r) {
        return {std::move(expr.value), std::move(r)};
    }

    template<class L, class T>
    internal::greater_or_equal_t<L, T> operator>=(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.value)};
    }

    template<class T, class R>
    internal::is_equal_t<T, R> operator==(internal::expression_t<T> expr, R r) {
        return {std::move(expr.value), std::move(r)};
    }

    template<class L, class T>
    internal::is_equal_t<L, T> operator==(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.value)};
    }

    template<class T, class R>
    internal::is_not_equal_t<T, R> operator!=(internal::expression_t<T> expr, R r) {
        return {std::move(expr.value), std::move(r)};
    }

    template<class L, class T>
    internal::is_not_equal_t<L, T> operator!=(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.value)};
    }

    template<class T, class R>
    internal::conc_t<T, R> operator||(internal::expression_t<T> expr, R r) {
        return {std::move(expr.value), std::move(r)};
    }

    template<class L, class T>
    internal::conc_t<L, T> operator||(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.value)};
    }

    template<class L, class R>
    internal::conc_t<L, R> operator||(internal::expression_t<L> l, internal::expression_t<R> r) {
        return {std::move(l.value), std::move(r.value)};
    }

    template<class T, class R>
    internal::add_t<T, R> operator+(internal::expression_t<T> expr, R r) {
        return {std::move(expr.value), std::move(r)};
    }

    template<class L, class T>
    internal::add_t<L, T> operator+(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.value)};
    }

    template<class L, class R>
    internal::add_t<L, R> operator+(internal::expression_t<L> l, internal::expression_t<R> r) {
        return {std::move(l.value), std::move(r.value)};
    }

    template<class T, class R>
    internal::sub_t<T, R> operator-(internal::expression_t<T> expr, R r) {
        return {std::move(expr.value), std::move(r)};
    }

    template<class L, class T>
    internal::sub_t<L, T> operator-(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.value)};
    }

    template<class L, class R>
    internal::sub_t<L, R> operator-(internal::expression_t<L> l, internal::expression_t<R> r) {
        return {std::move(l.value), std::move(r.value)};
    }

    template<class T, class R>
    internal::mul_t<T, R> operator*(internal::expression_t<T> expr, R r) {
        return {std::move(expr.value), std::move(r)};
    }

    template<class L, class T>
    internal::mul_t<L, T> operator*(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.value)};
    }

    template<class L, class R>
    internal::mul_t<L, R> operator*(internal::expression_t<L> l, internal::expression_t<R> r) {
        return {std::move(l.value), std::move(r.value)};
    }

    template<class T, class R>
    internal::div_t<T, R> operator/(internal::expression_t<T> expr, R r) {
        return {std::move(expr.value), std::move(r)};
    }

    template<class L, class T>
    internal::div_t<L, T> operator/(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.value)};
    }

    template<class L, class R>
    internal::div_t<L, R> operator/(internal::expression_t<L> l, internal::expression_t<R> r) {
        return {std::move(l.value), std::move(r.value)};
    }

    template<class T, class R>
    internal::mod_t<T, R> operator%(internal::expression_t<T> expr, R r) {
        return {std::move(expr.value), std::move(r)};
    }

    template<class L, class T>
    internal::mod_t<L, T> operator%(L l, internal::expression_t<T> expr) {
        return {std::move(l), std::move(expr.value)};
    }

    template<class L, class R>
    internal::mod_t<L, R> operator%(internal::expression_t<L> l, internal::expression_t<R> r) {
        return {std::move(l.value), std::move(r.value)};
    }

    template<class F, class O>
    internal::using_t<O, F O::*> using_(F O::*p) {
        return {p};
    }
    template<class T, class M>
    internal::using_t<T, M> using_(internal::column_pointer<T, M> cp) {
        return {std::move(cp)};
    }

    template<class T>
    internal::on_t<T> on(T t) {
        return {std::move(t)};
    }

    template<class T>
    internal::cross_join_t<T> cross_join() {
        return {};
    }

    template<class T>
    internal::natural_join_t<T> natural_join() {
        return {};
    }

    template<class T, class O>
    internal::left_join_t<T, O> left_join(O o) {
        return {std::move(o)};
    }

    template<class T, class O>
    internal::join_t<T, O> join(O o) {
        return {std::move(o)};
    }

    template<class T, class O>
    internal::left_outer_join_t<T, O> left_outer_join(O o) {
        return {std::move(o)};
    }

    template<class T, class O>
    internal::inner_join_t<T, O> inner_join(O o) {
        return {std::move(o)};
    }

    template<class T>
    internal::offset_t<T> offset(T off) {
        return {std::move(off)};
    }

    template<class T>
    internal::limit_t<T, false, false, void> limit(T lim) {
        return {std::move(lim)};
    }

    template<class T, class O, internal::satisfies_not<internal::is_offset, T> = true>
    internal::limit_t<T, true, true, O> limit(O off, T lim) {
        return {std::move(lim), {std::move(off)}};
    }

    template<class T, class O>
    internal::limit_t<T, true, false, O> limit(T lim, internal::offset_t<O> offt) {
        return {std::move(lim), {std::move(offt.off)}};
    }

    template<class L,
             class R,
             std::enable_if_t<polyfill::disjunction_v<std::is_base_of<internal::condition_t, L>,
                                                      std::is_base_of<internal::condition_t, R>>,
                              bool> = true>
    auto operator&&(L l, R r) {
        using internal::get_from_expression;
        return internal::make_and_condition(std::move(get_from_expression(l)), std::move(get_from_expression(r)));
    }

    template<class L, class R>
    auto and_(L l, R r) {
        using internal::get_from_expression;
        return internal::make_and_condition(std::move(get_from_expression(l)), std::move(get_from_expression(r)));
    }

    template<class L,
             class R,
             std::enable_if_t<polyfill::disjunction_v<std::is_base_of<internal::condition_t, L>,
                                                      std::is_base_of<internal::condition_t, R>>,
                              bool> = true>
    auto operator||(L l, R r) {
        using internal::get_from_expression;
        return internal::make_or_condition(std::move(get_from_expression(l)), std::move(get_from_expression(r)));
    }

    template<class L, class R>
    auto or_(L l, R r) {
        using internal::get_from_expression;
        return internal::make_or_condition(std::move(get_from_expression(l)), std::move(get_from_expression(r)));
    }

    template<class T>
    internal::is_not_null_t<T> is_not_null(T t) {
        return {std::move(t)};
    }

    template<class T>
    internal::is_null_t<T> is_null(T t) {
        return {std::move(t)};
    }

    template<class L, class E>
    internal::dynamic_in_t<L, std::vector<E>> in(L l, std::vector<E> values) {
        return {std::move(l), std::move(values), false};
    }

    template<class L, class E>
    internal::dynamic_in_t<L, std::vector<E>> in(L l, std::initializer_list<E> values) {
        return {std::move(l), std::move(values), false};
    }

    template<class L, class A>
    internal::dynamic_in_t<L, A> in(L l, A arg) {
        return {std::move(l), std::move(arg), false};
    }

    template<class L, class E>
    internal::dynamic_in_t<L, std::vector<E>> not_in(L l, std::vector<E> values) {
        return {std::move(l), std::move(values), true};
    }

    template<class L, class E>
    internal::dynamic_in_t<L, std::vector<E>> not_in(L l, std::initializer_list<E> values) {
        return {std::move(l), std::move(values), true};
    }

    template<class L, class A>
    internal::dynamic_in_t<L, A> not_in(L l, A arg) {
        return {std::move(l), std::move(arg), true};
    }

    template<class L, class R>
    internal::is_equal_t<L, R> is_equal(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    internal::is_equal_t<L, R> eq(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    internal::is_not_equal_t<L, R> is_not_equal(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    internal::is_not_equal_t<L, R> ne(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    internal::greater_than_t<L, R> greater_than(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    internal::greater_than_t<L, R> gt(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    internal::greater_or_equal_t<L, R> greater_or_equal(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    internal::greater_or_equal_t<L, R> ge(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    internal::lesser_than_t<L, R> lesser_than(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    internal::lesser_than_t<L, R> lt(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    internal::lesser_or_equal_t<L, R> lesser_or_equal(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L, class R>
    internal::lesser_or_equal_t<L, R> le(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    /**
     * ORDER BY column, column alias or expression
     * 
     * Examples:
     * storage.select(&User::name, order_by(&User::id))
     * storage.select(as<colalias_a>(&User::name), order_by(get<colalias_a>()))
     */
    template<class O, internal::satisfies_not<std::is_base_of, integer_printer, type_printer<O>> = true>
    internal::order_by_t<O> order_by(O o) {
        return {std::move(o)};
    }

    /**
     * ORDER BY positional ordinal
     * 
     * Examples:
     * storage.select(&User::name, order_by(1))
     */
    template<class O, internal::satisfies<std::is_base_of, integer_printer, type_printer<O>> = true>
    internal::order_by_t<internal::literal_holder<O>> order_by(O o) {
        return {{std::move(o)}};
    }

    /**
     * ORDER BY column1, column2
     * Example: storage.get_all<Singer>(multi_order_by(order_by(&Singer::name).asc(), order_by(&Singer::gender).desc())
     */
    template<class... Args>
    internal::multi_order_by_t<Args...> multi_order_by(Args&&... args) {
        return {std::make_tuple(std::forward<Args>(args)...)};
    }

    /**
     * ORDER BY column1, column2
     * Difference from `multi_order_by` is that `dynamic_order_by` can be changed at runtime using `push_back` member
     * function Example:
     *  auto orderBy = dynamic_order_by(storage);
     *  if(someCondition) {
     *      orderBy.push_back(&User::id);
     *  } else {
     *      orderBy.push_back(&User::name);
     *      orderBy.push_back(&User::birthDate);
     *  }
     */
    template<class S>
    internal::dynamic_order_by_t<internal::serializer_context<typename S::impl_type>>
    dynamic_order_by(const S& storage) {
        internal::serializer_context_builder<S> builder(storage);
        return builder();
    }

    /**
     *  X BETWEEN Y AND Z
     *  Example: storage.select(between(&User::id, 10, 20))
     */
    template<class A, class T>
    internal::between_t<A, T> between(A expr, T b1, T b2) {
        return {std::move(expr), std::move(b1), std::move(b2)};
    }

    /**
     *  X LIKE Y
     *  Example: storage.select(like(&User::name, "T%"))
     */
    template<class A, class T>
    internal::like_t<A, T, void> like(A a, T t) {
        return {std::move(a), std::move(t), {}};
    }

    /**
     *  X GLOB Y
     *  Example: storage.select(glob(&User::name, "*S"))
     */
    template<class A, class T>
    internal::glob_t<A, T> glob(A a, T t) {
        return {std::move(a), std::move(t)};
    }

    /**
     *  X LIKE Y ESCAPE Z
     *  Example: storage.select(like(&User::name, "T%", "%"))
     */
    template<class A, class T, class E>
    internal::like_t<A, T, E> like(A a, T t, E e) {
        return {std::move(a), std::move(t), {std::move(e)}};
    }

    /**
     *  CAST(X AS type).
     *  Example: cast<std::string>(&User::id)
     */
    template<class T, class E>
    internal::cast_t<T, E> cast(E e) {
        return {std::move(e)};
    }
}
#pragma once

#include <type_traits>  //  std::enable_if, std::is_base_of, std::is_member_pointer
#include <sstream>  //  std::stringstream
#include <string>  //  std::string

namespace sqlite_orm {

    /**
     *  This is base class for every class which is used as a custom table alias, column alias or expression alias.
     *  For more information please look through self_join.cpp example
     */
    struct alias_tag {};

    namespace internal {

        /**
         *  This is a common built-in class used for custom single character table aliases.
         *  For convenience there exist type aliases `alias_a`, `alias_b`, ...
         */
        template<class T, char A>
        struct table_alias : alias_tag {
            using type = T;

            static char get() {
                return A;
            }
        };

        /**
         *  Column expression with table alias attached like 'C.ID'. This is not a column alias
         */
        template<class T, class C>
        struct alias_column_t {
            using alias_type = T;
            using column_type = C;

            column_type column;
        };

        template<class T, class SFINAE = void>
        struct alias_extractor;

        template<class T>
        struct alias_extractor<T, std::enable_if_t<std::is_base_of<alias_tag, T>::value>> {
            static std::string get() {
                std::stringstream ss;
                ss << T::get();
                return ss.str();
            }
        };

        template<class T>
        struct alias_extractor<T, std::enable_if_t<!std::is_base_of<alias_tag, T>::value>> {
            static std::string get() {
                return {};
            }
        };

        /**
         * Used to store alias for expression
         */
        template<class T, class E>
        struct as_t {
            using alias_type = T;
            using expression_type = E;

            expression_type expression;
        };

        /**
         *  This is a common built-in class used for custom single-character column aliases.
         *  For convenience there exist type aliases `colalias_a`, `colalias_b`, ...
         */
        template<char A>
        struct column_alias : alias_tag {
            static std::string get() {
                return std::string(1u, A);
            }
        };

        template<class T>
        struct alias_holder {
            using type = T;
        };
    }

    /**
     *  @return column with table alias attached. Place it instead of a column statement in case you need to specify a
     *  column with table alias prefix like 'a.column'. For more information please look through self_join.cpp example
     */
    template<class T, class C>
    internal::alias_column_t<T, C> alias_column(C c) {
        static_assert(std::is_member_pointer<C>::value,
                      "alias_column argument must be a member pointer mapped to a storage");
        return {c};
    }

    template<class T, class E>
    internal::as_t<T, E> as(E expression) {
        return {std::move(expression)};
    }

    template<class T>
    internal::alias_holder<T> get() {
        return {};
    }

    template<class T>
    using alias_a = internal::table_alias<T, 'a'>;
    template<class T>
    using alias_b = internal::table_alias<T, 'b'>;
    template<class T>
    using alias_c = internal::table_alias<T, 'c'>;
    template<class T>
    using alias_d = internal::table_alias<T, 'd'>;
    template<class T>
    using alias_e = internal::table_alias<T, 'e'>;
    template<class T>
    using alias_f = internal::table_alias<T, 'f'>;
    template<class T>
    using alias_g = internal::table_alias<T, 'g'>;
    template<class T>
    using alias_h = internal::table_alias<T, 'h'>;
    template<class T>
    using alias_i = internal::table_alias<T, 'i'>;
    template<class T>
    using alias_j = internal::table_alias<T, 'j'>;
    template<class T>
    using alias_k = internal::table_alias<T, 'k'>;
    template<class T>
    using alias_l = internal::table_alias<T, 'l'>;
    template<class T>
    using alias_m = internal::table_alias<T, 'm'>;
    template<class T>
    using alias_n = internal::table_alias<T, 'n'>;
    template<class T>
    using alias_o = internal::table_alias<T, 'o'>;
    template<class T>
    using alias_p = internal::table_alias<T, 'p'>;
    template<class T>
    using alias_q = internal::table_alias<T, 'q'>;
    template<class T>
    using alias_r = internal::table_alias<T, 'r'>;
    template<class T>
    using alias_s = internal::table_alias<T, 's'>;
    template<class T>
    using alias_t = internal::table_alias<T, 't'>;
    template<class T>
    using alias_u = internal::table_alias<T, 'u'>;
    template<class T>
    using alias_v = internal::table_alias<T, 'v'>;
    template<class T>
    using alias_w = internal::table_alias<T, 'w'>;
    template<class T>
    using alias_x = internal::table_alias<T, 'x'>;
    template<class T>
    using alias_y = internal::table_alias<T, 'y'>;
    template<class T>
    using alias_z = internal::table_alias<T, 'z'>;

    using colalias_a = internal::column_alias<'a'>;
    using colalias_b = internal::column_alias<'b'>;
    using colalias_c = internal::column_alias<'c'>;
    using colalias_d = internal::column_alias<'d'>;
    using colalias_e = internal::column_alias<'e'>;
    using colalias_f = internal::column_alias<'f'>;
    using colalias_g = internal::column_alias<'g'>;
    using colalias_h = internal::column_alias<'h'>;
    using colalias_i = internal::column_alias<'i'>;
}
#pragma once

// #include "conditions.h"

namespace sqlite_orm {

    namespace internal {

        template<class... Args>
        struct join_iterator;

        template<>
        struct join_iterator<> {

            template<class L>
            void operator()(const L&) const {
                //..
            }
        };

        template<class H, class... Tail>
        struct join_iterator<H, Tail...> : public join_iterator<Tail...> {
            using super = join_iterator<Tail...>;

            template<class L>
            void operator()(const L& l) const {
                this->super::operator()(l);
            }
        };

        template<class T, class... Tail>
        struct join_iterator<cross_join_t<T>, Tail...> : public join_iterator<Tail...> {
            using super = join_iterator<Tail...>;
            using join_type = cross_join_t<T>;

            template<class L>
            void operator()(const L& l) const {
                l(*this);
                this->super::operator()(l);
            }
        };

        template<class T, class... Tail>
        struct join_iterator<natural_join_t<T>, Tail...> : public join_iterator<Tail...> {
            using super = join_iterator<Tail...>;
            using join_type = natural_join_t<T>;

            template<class L>
            void operator()(const L& l) const {
                l(*this);
                this->super::operator()(l);
            }
        };

        template<class T, class O, class... Tail>
        struct join_iterator<left_join_t<T, O>, Tail...> : public join_iterator<Tail...> {
            using super = join_iterator<Tail...>;
            using join_type = left_join_t<T, O>;

            template<class L>
            void operator()(const L& l) const {
                l(*this);
                this->super::operator()(l);
            }
        };

        template<class T, class O, class... Tail>
        struct join_iterator<join_t<T, O>, Tail...> : public join_iterator<Tail...> {
            using super = join_iterator<Tail...>;
            using join_type = join_t<T, O>;

            template<class L>
            void operator()(const L& l) const {
                l(*this);
                this->super::operator()(l);
            }
        };

        template<class T, class O, class... Tail>
        struct join_iterator<left_outer_join_t<T, O>, Tail...> : public join_iterator<Tail...> {
            using super = join_iterator<Tail...>;
            using join_type = left_outer_join_t<T, O>;

            template<class L>
            void operator()(const L& l) const {
                l(*this);
                this->super::operator()(l);
            }
        };

        template<class T, class O, class... Tail>
        struct join_iterator<inner_join_t<T, O>, Tail...> : public join_iterator<Tail...> {
            using super = join_iterator<Tail...>;
            using join_type = inner_join_t<T, O>;

            template<class L>
            void operator()(const L& l) const {
                l(*this);
                this->super::operator()(l);
            }
        };
    }
}
#pragma once

#include <string>  //  std::string
#include <tuple>  //  std::make_tuple, std::tuple_size
#include <type_traits>  //  std::forward, std::is_base_of, std::enable_if
#include <memory>  //  std::unique_ptr
#include <vector>  //  std::vector

// #include "functional/cxx_polyfill.h"

// #include "conditions.h"

// #include "is_base_of_template.h"

#include <type_traits>  //  std::true_type, std::false_type, std::declval

namespace sqlite_orm {

    namespace internal {

        /*
         * This is because of bug in MSVC, for more information, please visit
         * https://stackoverflow.com/questions/34672441/stdis-base-of-for-template-classes/34672753#34672753
         */
#ifdef SQLITE_ORM_BROKEN_VARIADIC_PACK_EXPANSION
        template<template<typename...> class Base>
        struct is_base_of_template_impl {
            template<typename... Ts>
            static constexpr std::true_type test(const Base<Ts...>&);

            static constexpr std::false_type test(...);
        };

        template<typename T, template<typename...> class C>
        using is_base_of_template = decltype(is_base_of_template_impl<C>::test(std::declval<T>()));
#else
        template<template<typename...> class C, typename... Ts>
        std::true_type is_base_of_template_impl(const C<Ts...>&);

        template<template<typename...> class C>
        std::false_type is_base_of_template_impl(...);

        template<typename T, template<typename...> class C>
        using is_base_of_template = decltype(is_base_of_template_impl<C>(std::declval<T>()));
#endif

        template<typename T, template<typename...> class C>
        SQLITE_ORM_INLINE_VAR constexpr bool is_base_of_template_v = is_base_of_template<T, C>::value;
    }
}

// #include "tuple_helper/tuple_filter.h"

// #include "serialize_result_type.h"

// #include "operators.h"

// #include "ast/into.h"

// #include "../functional/cxx_polyfill.h"

namespace sqlite_orm {
    namespace internal {

        template<class T>
        struct into_t {
            using type = T;
        };

        template<class T>
        using is_into = polyfill::is_specialization_of<T, into_t>;
    }

    template<class T>
    internal::into_t<T> into() {
        return {};
    }
}

namespace sqlite_orm {

    using int64 = sqlite_int64;
    using uint64 = sqlite_uint64;

    namespace internal {

        template<class T>
        struct unique_ptr_result_of {};

        /**
         *  Base class for operator overloading
         *  R - return type
         *  S - class with operator std::string
         *  Args - function arguments types
         */
        template<class R, class S, class... Args>
        struct built_in_function_t : S, arithmetic_t {
            using return_type = R;
            using string_type = S;
            using args_type = std::tuple<Args...>;

            static constexpr size_t args_size = std::tuple_size<args_type>::value;

            args_type args;

            built_in_function_t(args_type&& args_) : args(std::move(args_)) {}
        };

        template<class F, class W>
        struct filtered_aggregate_function {
            using function_type = F;
            using where_expression = W;

            function_type function;
            where_expression where;
        };

        template<class C>
        struct where_t;

        template<class R, class S, class... Args>
        struct built_in_aggregate_function_t : built_in_function_t<R, S, Args...> {
            using super = built_in_function_t<R, S, Args...>;

            using super::super;

            template<class W>
            filtered_aggregate_function<built_in_aggregate_function_t<R, S, Args...>, W> filter(where_t<W> wh) {
                return {*this, std::move(wh.expression)};
            }
        };

        struct typeof_string {
            serialize_result_type serialize() const {
                return "TYPEOF";
            }
        };

        struct unicode_string {
            serialize_result_type serialize() const {
                return "UNICODE";
            }
        };

        struct length_string {
            serialize_result_type serialize() const {
                return "LENGTH";
            }
        };

        struct abs_string {
            serialize_result_type serialize() const {
                return "ABS";
            }
        };

        struct lower_string {
            serialize_result_type serialize() const {
                return "LOWER";
            }
        };

        struct upper_string {
            serialize_result_type serialize() const {
                return "UPPER";
            }
        };

        struct last_insert_rowid_string {
            serialize_result_type serialize() const {
                return "LAST_INSERT_ROWID";
            }
        };

        struct total_changes_string {
            serialize_result_type serialize() const {
                return "TOTAL_CHANGES";
            }
        };

        struct changes_string {
            serialize_result_type serialize() const {
                return "CHANGES";
            }
        };

        struct trim_string {
            serialize_result_type serialize() const {
                return "TRIM";
            }
        };

        struct ltrim_string {
            serialize_result_type serialize() const {
                return "LTRIM";
            }
        };

        struct rtrim_string {
            serialize_result_type serialize() const {
                return "RTRIM";
            }
        };

        struct hex_string {
            serialize_result_type serialize() const {
                return "HEX";
            }
        };

        struct quote_string {
            serialize_result_type serialize() const {
                return "QUOTE";
            }
        };

        struct randomblob_string {
            serialize_result_type serialize() const {
                return "RANDOMBLOB";
            }
        };

        struct instr_string {
            serialize_result_type serialize() const {
                return "INSTR";
            }
        };

        struct replace_string {
            serialize_result_type serialize() const {
                return "REPLACE";
            }
        };

        struct round_string {
            serialize_result_type serialize() const {
                return "ROUND";
            }
        };

#if SQLITE_VERSION_NUMBER >= 3007016

        struct char_string {
            serialize_result_type serialize() const {
                return "CHAR";
            }
        };

        struct random_string {
            serialize_result_type serialize() const {
                return "RANDOM";
            }
        };

#endif

        struct coalesce_string {
            serialize_result_type serialize() const {
                return "COALESCE";
            }
        };

        struct ifnull_string {
            serialize_result_type serialize() const {
                return "IFNULL";
            }
        };

        struct nullif_string {
            serialize_result_type serialize() const {
                return "NULLIF";
            }
        };

        struct date_string {
            serialize_result_type serialize() const {
                return "DATE";
            }
        };

        struct time_string {
            serialize_result_type serialize() const {
                return "TIME";
            }
        };

        struct datetime_string {
            serialize_result_type serialize() const {
                return "DATETIME";
            }
        };

        struct julianday_string {
            serialize_result_type serialize() const {
                return "JULIANDAY";
            }
        };

        struct strftime_string {
            serialize_result_type serialize() const {
                return "STRFTIME";
            }
        };

        struct zeroblob_string {
            serialize_result_type serialize() const {
                return "ZEROBLOB";
            }
        };

        struct substr_string {
            serialize_result_type serialize() const {
                return "SUBSTR";
            }
        };
#ifdef SQLITE_SOUNDEX
        struct soundex_string {
            serialize_result_type serialize() const {
                return "SOUNDEX";
            }
        };
#endif
        struct total_string {
            serialize_result_type serialize() const {
                return "TOTAL";
            }
        };

        struct sum_string {
            serialize_result_type serialize() const {
                return "SUM";
            }
        };

        struct count_string {
            serialize_result_type serialize() const {
                return "COUNT";
            }
        };

        /**
         *  T is use to specify type explicitly for queries like
         *  SELECT COUNT(*) FROM table_name;
         *  T can be omitted with void.
         */
        template<class T>
        struct count_asterisk_t : count_string {
            using type = T;

            template<class W>
            filtered_aggregate_function<count_asterisk_t<T>, W> filter(where_t<W> wh) {
                return {*this, std::move(wh.expression)};
            }
        };

        /**
         *  The same thing as count<T>() but without T arg.
         *  Is used in cases like this:
         *    SELECT cust_code, cust_name, cust_city, grade
         *    FROM customer
         *    WHERE grade=2 AND EXISTS
         *        (SELECT COUNT(*)
         *        FROM customer
         *        WHERE grade=2
         *        GROUP BY grade
         *        HAVING COUNT(*)>2);
         *  `c++`
         *  auto rows =
         *      storage.select(columns(&Customer::code, &Customer::name, &Customer::city, &Customer::grade),
         *          where(is_equal(&Customer::grade, 2)
         *              and exists(select(count<Customer>(),
         *                  where(is_equal(&Customer::grade, 2)),
         *          group_by(&Customer::grade),
         *          having(greater_than(count(), 2))))));
         */
        struct count_asterisk_without_type : count_string {};

        struct avg_string {
            serialize_result_type serialize() const {
                return "AVG";
            }
        };

        struct max_string {
            serialize_result_type serialize() const {
                return "MAX";
            }
        };

        struct min_string {
            serialize_result_type serialize() const {
                return "MIN";
            }
        };

        struct group_concat_string {
            serialize_result_type serialize() const {
                return "GROUP_CONCAT";
            }
        };
#ifdef SQLITE_ENABLE_MATH_FUNCTIONS
        struct acos_string {
            serialize_result_type serialize() const {
                return "ACOS";
            }
        };

        struct acosh_string {
            serialize_result_type serialize() const {
                return "ACOSH";
            }
        };

        struct asin_string {
            serialize_result_type serialize() const {
                return "ASIN";
            }
        };

        struct asinh_string {
            serialize_result_type serialize() const {
                return "ASINH";
            }
        };

        struct atan_string {
            serialize_result_type serialize() const {
                return "ATAN";
            }
        };

        struct atan2_string {
            serialize_result_type serialize() const {
                return "ATAN2";
            }
        };

        struct atanh_string {
            serialize_result_type serialize() const {
                return "ATANH";
            }
        };

        struct ceil_string {
            serialize_result_type serialize() const {
                return "CEIL";
            }
        };

        struct ceiling_string {
            serialize_result_type serialize() const {
                return "CEILING";
            }
        };

        struct cos_string {
            serialize_result_type serialize() const {
                return "COS";
            }
        };

        struct cosh_string {
            serialize_result_type serialize() const {
                return "COSH";
            }
        };

        struct degrees_string {
            serialize_result_type serialize() const {
                return "DEGREES";
            }
        };

        struct exp_string {
            serialize_result_type serialize() const {
                return "EXP";
            }
        };

        struct floor_string {
            serialize_result_type serialize() const {
                return "FLOOR";
            }
        };

        struct ln_string {
            serialize_result_type serialize() const {
                return "LN";
            }
        };

        struct log_string {
            serialize_result_type serialize() const {
                return "LOG";
            }
        };

        struct log10_string {
            serialize_result_type serialize() const {
                return "LOG10";
            }
        };

        struct log2_string {
            serialize_result_type serialize() const {
                return "LOG2";
            }
        };

        struct mod_string {
            serialize_result_type serialize() const {
                return "MOD";
            }
        };

        struct pi_string {
            serialize_result_type serialize() const {
                return "PI";
            }
        };

        struct pow_string {
            serialize_result_type serialize() const {
                return "POW";
            }
        };

        struct power_string {
            serialize_result_type serialize() const {
                return "POWER";
            }
        };

        struct radians_string {
            serialize_result_type serialize() const {
                return "RADIANS";
            }
        };

        struct sin_string {
            serialize_result_type serialize() const {
                return "SIN";
            }
        };

        struct sinh_string {
            serialize_result_type serialize() const {
                return "SINH";
            }
        };

        struct sqrt_string {
            serialize_result_type serialize() const {
                return "SQRT";
            }
        };

        struct tan_string {
            serialize_result_type serialize() const {
                return "TAN";
            }
        };

        struct tanh_string {
            serialize_result_type serialize() const {
                return "TANH";
            }
        };

        struct trunc_string {
            serialize_result_type serialize() const {
                return "TRUNC";
            }
        };

#endif  //  SQLITE_ENABLE_MATH_FUNCTIONS
#ifdef SQLITE_ENABLE_JSON1
        struct json_string {
            serialize_result_type serialize() const {
                return "JSON";
            }
        };

        struct json_array_string {
            serialize_result_type serialize() const {
                return "JSON_ARRAY";
            }
        };

        struct json_array_length_string {
            serialize_result_type serialize() const {
                return "JSON_ARRAY_LENGTH";
            }
        };

        struct json_extract_string {
            serialize_result_type serialize() const {
                return "JSON_EXTRACT";
            }
        };

        struct json_insert_string {
            serialize_result_type serialize() const {
                return "JSON_INSERT";
            }
        };

        struct json_replace_string {
            serialize_result_type serialize() const {
                return "JSON_REPLACE";
            }
        };

        struct json_set_string {
            serialize_result_type serialize() const {
                return "JSON_SET";
            }
        };

        struct json_object_string {
            serialize_result_type serialize() const {
                return "JSON_OBJECT";
            }
        };

        struct json_patch_string {
            serialize_result_type serialize() const {
                return "JSON_PATCH";
            }
        };

        struct json_remove_string {
            serialize_result_type serialize() const {
                return "JSON_REMOVE";
            }
        };

        struct json_type_string {
            serialize_result_type serialize() const {
                return "JSON_TYPE";
            }
        };

        struct json_valid_string {
            serialize_result_type serialize() const {
                return "JSON_VALID";
            }
        };

        struct json_quote_string {
            serialize_result_type serialize() const {
                return "JSON_QUOTE";
            }
        };

        struct json_group_array_string {
            serialize_result_type serialize() const {
                return "JSON_GROUP_ARRAY";
            }
        };

        struct json_group_object_string {
            serialize_result_type serialize() const {
                return "JSON_GROUP_OBJECT";
            }
        };
#endif  //  SQLITE_ENABLE_JSON1

        template<class T>
        using field_type_or_type_t = polyfill::detected_or_t<T, type_t, member_field_type<T>>;
    }

    /**
     *  Cute operators for core functions
     */
    template<class F,
             class R,
             std::enable_if_t<internal::is_base_of_template_v<F, internal::built_in_function_t>, bool> = true>
    internal::lesser_than_t<F, R> operator<(F f, R r) {
        return {std::move(f), std::move(r)};
    }

    template<class F,
             class R,
             std::enable_if_t<internal::is_base_of_template_v<F, internal::built_in_function_t>, bool> = true>
    internal::lesser_or_equal_t<F, R> operator<=(F f, R r) {
        return {std::move(f), std::move(r)};
    }

    template<class F,
             class R,
             std::enable_if_t<internal::is_base_of_template_v<F, internal::built_in_function_t>, bool> = true>
    internal::greater_than_t<F, R> operator>(F f, R r) {
        return {std::move(f), std::move(r)};
    }

    template<class F,
             class R,
             std::enable_if_t<internal::is_base_of_template_v<F, internal::built_in_function_t>, bool> = true>
    internal::greater_or_equal_t<F, R> operator>=(F f, R r) {
        return {std::move(f), std::move(r)};
    }

    template<class F,
             class R,
             std::enable_if_t<internal::is_base_of_template_v<F, internal::built_in_function_t>, bool> = true>
    internal::is_equal_t<F, R> operator==(F f, R r) {
        return {std::move(f), std::move(r)};
    }

    template<class F,
             class R,
             std::enable_if_t<internal::is_base_of_template_v<F, internal::built_in_function_t>, bool> = true>
    internal::is_not_equal_t<F, R> operator!=(F f, R r) {
        return {std::move(f), std::move(r)};
    }
#ifdef SQLITE_ENABLE_MATH_FUNCTIONS

    /**
     *  ACOS(X) function https://www.sqlite.org/lang_mathfunc.html#acos
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::acos(&Triangle::cornerA));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::acos_string, X> acos(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  ACOS(X) function https://www.sqlite.org/lang_mathfunc.html#acos
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::acos<std::optional<double>>(&Triangle::cornerA));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::acos_string, X> acos(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  ACOSH(X) function https://www.sqlite.org/lang_mathfunc.html#acosh
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::acosh(&Triangle::cornerA));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::acosh_string, X> acosh(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  ACOSH(X) function https://www.sqlite.org/lang_mathfunc.html#acosh
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::acosh<std::optional<double>>(&Triangle::cornerA));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::acosh_string, X> acosh(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  ASIN(X) function https://www.sqlite.org/lang_mathfunc.html#asin
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::asin(&Triangle::cornerA));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::asin_string, X> asin(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  ASIN(X) function https://www.sqlite.org/lang_mathfunc.html#asin
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::asin<std::optional<double>>(&Triangle::cornerA));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::asin_string, X> asin(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  ASINH(X) function https://www.sqlite.org/lang_mathfunc.html#asinh
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::asinh(&Triangle::cornerA));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::asinh_string, X> asinh(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  ASINH(X) function https://www.sqlite.org/lang_mathfunc.html#asinh
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::asinh<std::optional<double>>(&Triangle::cornerA));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::asinh_string, X> asinh(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  ATAN(X) function https://www.sqlite.org/lang_mathfunc.html#atan
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::atan(1));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::atan_string, X> atan(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  ATAN(X) function https://www.sqlite.org/lang_mathfunc.html#atan
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::atan<std::optional<double>>(1));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::atan_string, X> atan(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  ATAN2(X, Y) function https://www.sqlite.org/lang_mathfunc.html#atan2
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::atan2(1, 3));   //  decltype(rows) is std::vector<double>
     */
    template<class X, class Y>
    internal::built_in_function_t<double, internal::atan2_string, X, Y> atan2(X x, Y y) {
        return {std::tuple<X, Y>{std::forward<X>(x), std::forward<Y>(y)}};
    }

    /**
     *  ATAN2(X, Y) function https://www.sqlite.org/lang_mathfunc.html#atan2
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::atan2<std::optional<double>>(1, 3));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X, class Y>
    internal::built_in_function_t<R, internal::atan2_string, X, Y> atan2(X x, Y y) {
        return {std::tuple<X, Y>{std::forward<X>(x), std::forward<Y>(y)}};
    }

    /**
     *  ATANH(X) function https://www.sqlite.org/lang_mathfunc.html#atanh
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::atanh(1));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::atanh_string, X> atanh(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  ATANH(X) function https://www.sqlite.org/lang_mathfunc.html#atanh
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::atanh<std::optional<double>>(1));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::atanh_string, X> atanh(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  CEIL(X) function https://www.sqlite.org/lang_mathfunc.html#ceil
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::ceil(&User::rating));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::ceil_string, X> ceil(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  CEIL(X) function https://www.sqlite.org/lang_mathfunc.html#ceil
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::ceil<std::optional<double>>(&User::rating));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::ceil_string, X> ceil(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  CEILING(X) function https://www.sqlite.org/lang_mathfunc.html#ceil
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::ceiling(&User::rating));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::ceiling_string, X> ceiling(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  CEILING(X) function https://www.sqlite.org/lang_mathfunc.html#ceil
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::ceiling<std::optional<double>>(&User::rating));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::ceiling_string, X> ceiling(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  COS(X) function https://www.sqlite.org/lang_mathfunc.html#cos
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::cos(&Triangle::cornerB));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::cos_string, X> cos(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  COS(X) function https://www.sqlite.org/lang_mathfunc.html#cos
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::cos<std::optional<double>>(&User::rating));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::cos_string, X> cos(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  COSH(X) function https://www.sqlite.org/lang_mathfunc.html#cosh
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::cosh(&Triangle::cornerB));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::cosh_string, X> cosh(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  COSH(X)  function https://www.sqlite.org/lang_mathfunc.html#cosh
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::cosh<std::optional<double>>(&User::rating));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::cosh_string, X> cosh(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  DEGREES(X) function https://www.sqlite.org/lang_mathfunc.html#degrees
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::degrees(&Triangle::cornerB));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::degrees_string, X> degrees(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  DEGREES(X) function https://www.sqlite.org/lang_mathfunc.html#degrees
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::degrees<std::optional<double>>(&User::rating));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::degrees_string, X> degrees(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  EXP(X) function https://www.sqlite.org/lang_mathfunc.html#exp
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::exp(&Triangle::cornerB));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::exp_string, X> exp(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  EXP(X) function https://www.sqlite.org/lang_mathfunc.html#exp
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::exp<std::optional<double>>(&User::rating));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::exp_string, X> exp(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  FLOOR(X) function https://www.sqlite.org/lang_mathfunc.html#floor
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::floor(&User::rating));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::floor_string, X> floor(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  FLOOR(X) function https://www.sqlite.org/lang_mathfunc.html#floor
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::floor<std::optional<double>>(&User::rating));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::floor_string, X> floor(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  LN(X) function https://www.sqlite.org/lang_mathfunc.html#ln
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::ln(200));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::ln_string, X> ln(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  LN(X) function https://www.sqlite.org/lang_mathfunc.html#ln
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::ln<std::optional<double>>(200));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::ln_string, X> ln(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  LOG(X) function https://www.sqlite.org/lang_mathfunc.html#log
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::log(100));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::log_string, X> log(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  LOG(X) function https://www.sqlite.org/lang_mathfunc.html#log
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::log<std::optional<double>>(100));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::log_string, X> log(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  LOG10(X) function https://www.sqlite.org/lang_mathfunc.html#log
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::log10(100));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::log10_string, X> log10(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  LOG10(X) function https://www.sqlite.org/lang_mathfunc.html#log
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::log10<std::optional<double>>(100));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::log10_string, X> log10(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  LOG(B, X) function https://www.sqlite.org/lang_mathfunc.html#log
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::log(10, 100));   //  decltype(rows) is std::vector<double>
     */
    template<class B, class X>
    internal::built_in_function_t<double, internal::log_string, B, X> log(B b, X x) {
        return {std::tuple<B, X>{std::forward<B>(b), std::forward<X>(x)}};
    }

    /**
     *  LOG(B, X) function https://www.sqlite.org/lang_mathfunc.html#log
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::log<std::optional<double>>(10, 100));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class B, class X>
    internal::built_in_function_t<R, internal::log_string, B, X> log(B b, X x) {
        return {std::tuple<B, X>{std::forward<B>(b), std::forward<X>(x)}};
    }

    /**
     *  LOG2(X) function https://www.sqlite.org/lang_mathfunc.html#log2
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::log2(64));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::log2_string, X> log2(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  LOG2(X) function https://www.sqlite.org/lang_mathfunc.html#log2
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::log2<std::optional<double>>(64));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::log2_string, X> log2(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  MOD(X, Y) function https://www.sqlite.org/lang_mathfunc.html#mod
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::mod_f(6, 5));   //  decltype(rows) is std::vector<double>
     */
    template<class X, class Y>
    internal::built_in_function_t<double, internal::mod_string, X, Y> mod_f(X x, Y y) {
        return {std::tuple<X, Y>{std::forward<X>(x), std::forward<Y>(y)}};
    }

    /**
     *  MOD(X, Y) function https://www.sqlite.org/lang_mathfunc.html#mod
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::mod_f<std::optional<double>>(6, 5));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X, class Y>
    internal::built_in_function_t<R, internal::mod_string, X, Y> mod_f(X x, Y y) {
        return {std::tuple<X, Y>{std::forward<X>(x), std::forward<Y>(y)}};
    }

    /**
     *  PI() function https://www.sqlite.org/lang_mathfunc.html#pi
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::pi());   //  decltype(rows) is std::vector<double>
     */
    inline internal::built_in_function_t<double, internal::pi_string> pi() {
        return {{}};
    }

    /**
     *  PI() function https://www.sqlite.org/lang_mathfunc.html#pi
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, etc.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::pi<float>());   //  decltype(rows) is std::vector<float>
     */
    template<class R>
    internal::built_in_function_t<R, internal::pi_string> pi() {
        return {{}};
    }

    /**
     *  POW(X, Y) function https://www.sqlite.org/lang_mathfunc.html#pow
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::pow(2, 5));   //  decltype(rows) is std::vector<double>
     */
    template<class X, class Y>
    internal::built_in_function_t<double, internal::pow_string, X, Y> pow(X x, Y y) {
        return {std::tuple<X, Y>{std::forward<X>(x), std::forward<Y>(y)}};
    }

    /**
     *  POW(X, Y) function https://www.sqlite.org/lang_mathfunc.html#pow
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::pow<std::optional<double>>(2, 5));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X, class Y>
    internal::built_in_function_t<R, internal::pow_string, X, Y> pow(X x, Y y) {
        return {std::tuple<X, Y>{std::forward<X>(x), std::forward<Y>(y)}};
    }

    /**
     *  POWER(X, Y) function https://www.sqlite.org/lang_mathfunc.html#pow
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::power(2, 5));   //  decltype(rows) is std::vector<double>
     */
    template<class X, class Y>
    internal::built_in_function_t<double, internal::power_string, X, Y> power(X x, Y y) {
        return {std::tuple<X, Y>{std::forward<X>(x), std::forward<Y>(y)}};
    }

    /**
     *  POWER(X, Y) function https://www.sqlite.org/lang_mathfunc.html#pow
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::power<std::optional<double>>(2, 5));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X, class Y>
    internal::built_in_function_t<R, internal::power_string, X, Y> power(X x, Y y) {
        return {std::tuple<X, Y>{std::forward<X>(x), std::forward<Y>(y)}};
    }

    /**
     *  RADIANS(X) function https://www.sqlite.org/lang_mathfunc.html#radians
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::radians(&Triangle::cornerAInDegrees));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::radians_string, X> radians(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  RADIANS(X) function https://www.sqlite.org/lang_mathfunc.html#radians
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::radians<std::optional<double>>(&Triangle::cornerAInDegrees));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::radians_string, X> radians(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  SIN(X) function https://www.sqlite.org/lang_mathfunc.html#sin
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::sin(&Triangle::cornerA));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::sin_string, X> sin(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  SIN(X) function https://www.sqlite.org/lang_mathfunc.html#sin
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::sin<std::optional<double>>(&Triangle::cornerA));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::sin_string, X> sin(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  SINH(X) function https://www.sqlite.org/lang_mathfunc.html#sinh
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::sinh(&Triangle::cornerA));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::sinh_string, X> sinh(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  SINH(X) function https://www.sqlite.org/lang_mathfunc.html#sinh
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::sinh<std::optional<double>>(&Triangle::cornerA));   //  decltype(rows) is std::vector<std::optional<double>>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::sinh_string, X> sinh(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  SQRT(X) function https://www.sqlite.org/lang_mathfunc.html#sqrt
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::sqrt(25));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::sqrt_string, X> sqrt(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  SQRT(X) function https://www.sqlite.org/lang_mathfunc.html#sqrt
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::sqrt<int>(25));   //  decltype(rows) is std::vector<int>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::sqrt_string, X> sqrt(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  TAN(X) function https://www.sqlite.org/lang_mathfunc.html#tan
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::tan(&Triangle::cornerC));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::tan_string, X> tan(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  TAN(X) function https://www.sqlite.org/lang_mathfunc.html#tan
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::tan<float>(&Triangle::cornerC));   //  decltype(rows) is std::vector<float>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::tan_string, X> tan(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  TANH(X) function https://www.sqlite.org/lang_mathfunc.html#tanh
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::tanh(&Triangle::cornerC));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::tanh_string, X> tanh(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  TANH(X) function https://www.sqlite.org/lang_mathfunc.html#tanh
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::tanh<float>(&Triangle::cornerC));   //  decltype(rows) is std::vector<float>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::tanh_string, X> tanh(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  TRUNC(X) function https://www.sqlite.org/lang_mathfunc.html#trunc
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::trunc(5.5));   //  decltype(rows) is std::vector<double>
     */
    template<class X>
    internal::built_in_function_t<double, internal::trunc_string, X> trunc(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  TRUNC(X) function https://www.sqlite.org/lang_mathfunc.html#trunc
     *
     *  Difference with the previous function is that previous override has `double` as return type but this
     *  override accepts return type from you as a template argument. You can use any bindable type:
     *  `float`, `int`, `std::optional<double>` etc. This override is handy when you expect `null` as result.
     *
     *  Example:
     *
     *  auto rows = storage.select(sqlite_orm::trunc<float>(5.5));   //  decltype(rows) is std::vector<float>
     */
    template<class R, class X>
    internal::built_in_function_t<R, internal::trunc_string, X> trunc(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }
#endif  //  SQLITE_ENABLE_MATH_FUNCTIONS
    /**
     *  TYPEOF(x) function https://sqlite.org/lang_corefunc.html#typeof
     */
    template<class T>
    internal::built_in_function_t<std::string, internal::typeof_string, T> typeof_(T t) {
        return {std::tuple<T>{std::forward<T>(t)}};
    }

    /**
     *  UNICODE(x) function https://sqlite.org/lang_corefunc.html#unicode
     */
    template<class T>
    internal::built_in_function_t<int, internal::unicode_string, T> unicode(T t) {
        return {std::tuple<T>{std::forward<T>(t)}};
    }

    /**
     *  LENGTH(x) function https://sqlite.org/lang_corefunc.html#length
     */
    template<class T>
    internal::built_in_function_t<int, internal::length_string, T> length(T t) {
        return {std::tuple<T>{std::forward<T>(t)}};
    }

    /**
     *  ABS(x) function https://sqlite.org/lang_corefunc.html#abs
     */
    template<class T>
    internal::built_in_function_t<std::unique_ptr<double>, internal::abs_string, T> abs(T t) {
        return {std::tuple<T>{std::forward<T>(t)}};
    }

    /**
     *  LOWER(x) function https://sqlite.org/lang_corefunc.html#lower
     */
    template<class T>
    internal::built_in_function_t<std::string, internal::lower_string, T> lower(T t) {
        return {std::tuple<T>{std::forward<T>(t)}};
    }

    /**
     *  UPPER(x) function https://sqlite.org/lang_corefunc.html#upper
     */
    template<class T>
    internal::built_in_function_t<std::string, internal::upper_string, T> upper(T t) {
        return {std::tuple<T>{std::forward<T>(t)}};
    }

    /**
     *  LAST_INSERT_ROWID(x) function https://www.sqlite.org/lang_corefunc.html#last_insert_rowid
     */
    inline internal::built_in_function_t<int64, internal::last_insert_rowid_string> last_insert_rowid() {
        return {{}};
    }

    /**
     *  TOTAL_CHANGES() function https://sqlite.org/lang_corefunc.html#total_changes
     */
    inline internal::built_in_function_t<int, internal::total_changes_string> total_changes() {
        return {{}};
    }

    /**
     *  CHANGES() function https://sqlite.org/lang_corefunc.html#changes
     */
    inline internal::built_in_function_t<int, internal::changes_string> changes() {
        return {{}};
    }

    /**
     *  TRIM(X) function https://sqlite.org/lang_corefunc.html#trim
     */
    template<class T>
    internal::built_in_function_t<std::string, internal::trim_string, T> trim(T t) {
        return {std::tuple<T>{std::forward<T>(t)}};
    }

    /**
     *  TRIM(X,Y) function https://sqlite.org/lang_corefunc.html#trim
     */
    template<class X, class Y>
    internal::built_in_function_t<std::string, internal::trim_string, X, Y> trim(X x, Y y) {
        return {std::tuple<X, Y>{std::forward<X>(x), std::forward<Y>(y)}};
    }

    /**
     *  LTRIM(X) function https://sqlite.org/lang_corefunc.html#ltrim
     */
    template<class X>
    internal::built_in_function_t<std::string, internal::ltrim_string, X> ltrim(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  LTRIM(X,Y) function https://sqlite.org/lang_corefunc.html#ltrim
     */
    template<class X, class Y>
    internal::built_in_function_t<std::string, internal::ltrim_string, X, Y> ltrim(X x, Y y) {
        return {std::tuple<X, Y>{std::forward<X>(x), std::forward<Y>(y)}};
    }

    /**
     *  RTRIM(X) function https://sqlite.org/lang_corefunc.html#rtrim
     */
    template<class X>
    internal::built_in_function_t<std::string, internal::rtrim_string, X> rtrim(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  RTRIM(X,Y) function https://sqlite.org/lang_corefunc.html#rtrim
     */
    template<class X, class Y>
    internal::built_in_function_t<std::string, internal::rtrim_string, X, Y> rtrim(X x, Y y) {
        return {std::tuple<X, Y>{std::forward<X>(x), std::forward<Y>(y)}};
    }

    /**
     *  HEX(X) function https://sqlite.org/lang_corefunc.html#hex
     */
    template<class X>
    internal::built_in_function_t<std::string, internal::hex_string, X> hex(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  QUOTE(X) function https://sqlite.org/lang_corefunc.html#quote
     */
    template<class X>
    internal::built_in_function_t<std::string, internal::quote_string, X> quote(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  RANDOMBLOB(X) function https://sqlite.org/lang_corefunc.html#randomblob
     */
    template<class X>
    internal::built_in_function_t<std::vector<char>, internal::randomblob_string, X> randomblob(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  INSTR(X) function https://sqlite.org/lang_corefunc.html#instr
     */
    template<class X, class Y>
    internal::built_in_function_t<int, internal::instr_string, X, Y> instr(X x, Y y) {
        return {std::tuple<X, Y>{std::forward<X>(x), std::forward<Y>(y)}};
    }

    /**
     *  REPLACE(X) function https://sqlite.org/lang_corefunc.html#replace
     */
    template<class X,
             class Y,
             class Z,
             std::enable_if_t<internal::count_tuple<std::tuple<X, Y, Z>, internal::is_into>::value == 0, bool> = true>
    internal::built_in_function_t<std::string, internal::replace_string, X, Y, Z> replace(X x, Y y, Z z) {
        return {std::tuple<X, Y, Z>{std::forward<X>(x), std::forward<Y>(y), std::forward<Z>(z)}};
    }

    /**
     *  ROUND(X) function https://sqlite.org/lang_corefunc.html#round
     */
    template<class X>
    internal::built_in_function_t<double, internal::round_string, X> round(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  ROUND(X, Y) function https://sqlite.org/lang_corefunc.html#round
     */
    template<class X, class Y>
    internal::built_in_function_t<double, internal::round_string, X, Y> round(X x, Y y) {
        return {std::tuple<X, Y>{std::forward<X>(x), std::forward<Y>(y)}};
    }

#if SQLITE_VERSION_NUMBER >= 3007016

    /**
     *  CHAR(X1,X2,...,XN) function https://sqlite.org/lang_corefunc.html#char
     */
    template<class... Args>
    internal::built_in_function_t<std::string, internal::char_string, Args...> char_(Args... args) {
        return {std::make_tuple(std::forward<Args>(args)...)};
    }

    /**
     *  RANDOM() function https://www.sqlite.org/lang_corefunc.html#random
     */
    inline internal::built_in_function_t<int, internal::random_string> random() {
        return {{}};
    }

#endif

    /**
     *  COALESCE(X,Y,...) function https://www.sqlite.org/lang_corefunc.html#coalesce
     */
    template<class R = void, class... Args>
    auto coalesce(Args... args)
        -> internal::built_in_function_t<typename std::conditional_t<  //  choose R or common type
                                             std::is_void<R>::value,
                                             std::common_type<internal::field_type_or_type_t<Args>...>,
                                             polyfill::type_identity<R>>::type,
                                         internal::coalesce_string,
                                         Args...> {
        return {std::make_tuple(std::forward<Args>(args)...)};
    }

    /**
     *  IFNULL(X,Y) function https://www.sqlite.org/lang_corefunc.html#ifnull
     */
    template<class R = void, class X, class Y>
    auto ifnull(X x, Y y) -> internal::built_in_function_t<
        typename std::conditional_t<  //  choose R or common type
            std::is_void<R>::value,
            std::common_type<internal::field_type_or_type_t<X>, internal::field_type_or_type_t<Y>>,
            polyfill::type_identity<R>>::type,
        internal::ifnull_string,
        X,
        Y> {
        return {std::make_tuple(std::move(x), std::move(y))};
    }

    /**
     *  NULLIF(X,Y) function https://www.sqlite.org/lang_corefunc.html#nullif
     */
#if defined(SQLITE_ORM_OPTIONAL_SUPPORTED) && defined(SQLITE_ORM_IF_CONSTEXPR_SUPPORTED)
    /**
     *  NULLIF(X,Y) using common return type of X and Y
     */
    template<class R = void,
             class X,
             class Y,
             std::enable_if_t<polyfill::disjunction_v<polyfill::negation<std::is_void<R>>,
                                                      polyfill::is_detected<std::common_type_t,
                                                                            internal::field_type_or_type_t<X>,
                                                                            internal::field_type_or_type_t<Y>>>,
                              bool> = true>
    auto nullif(X x, Y y) {
        if constexpr(std::is_void_v<R>) {
            using F = internal::built_in_function_t<
                std::optional<std::common_type_t<internal::field_type_or_type_t<X>, internal::field_type_or_type_t<Y>>>,
                internal::nullif_string,
                X,
                Y>;

            return F{std::make_tuple(std::move(x), std::move(y))};
        } else {
            using F = internal::built_in_function_t<R, internal::nullif_string, X, Y>;

            return F{std::make_tuple(std::move(x), std::move(y))};
        }
    }
#else
    template<class R, class X, class Y>
    internal::built_in_function_t<R, internal::nullif_string, X, Y> nullif(X x, Y y) {
        return {std::make_tuple(std::move(x), std::move(y))};
    }
#endif

    /**
     *  DATE(timestring, modifier, modifier, ...) function https://www.sqlite.org/lang_datefunc.html
     */
    template<class... Args>
    internal::built_in_function_t<std::string, internal::date_string, Args...> date(Args... args) {
        return {std::tuple<Args...>{std::forward<Args>(args)...}};
    }

    /**
     *  TIME(timestring, modifier, modifier, ...) function https://www.sqlite.org/lang_datefunc.html
     */
    template<class... Args>
    internal::built_in_function_t<std::string, internal::time_string, Args...> time(Args... args) {
        return {std::tuple<Args...>{std::forward<Args>(args)...}};
    }

    /**
     *  DATETIME(timestring, modifier, modifier, ...) function https://www.sqlite.org/lang_datefunc.html
     */
    template<class... Args>
    internal::built_in_function_t<std::string, internal::datetime_string, Args...> datetime(Args... args) {
        return {std::tuple<Args...>{std::forward<Args>(args)...}};
    }

    /**
     *  JULIANDAY(timestring, modifier, modifier, ...) function https://www.sqlite.org/lang_datefunc.html
     */
    template<class... Args>
    internal::built_in_function_t<double, internal::julianday_string, Args...> julianday(Args... args) {
        return {std::tuple<Args...>{std::forward<Args>(args)...}};
    }

    /**
     *  STRFTIME(timestring, modifier, modifier, ...) function https://www.sqlite.org/lang_datefunc.html
     */
    template<class... Args>
    internal::built_in_function_t<std::string, internal::strftime_string, Args...> strftime(Args... args) {
        return {std::tuple<Args...>{std::forward<Args>(args)...}};
    }

    /**
     *  ZEROBLOB(N) function https://www.sqlite.org/lang_corefunc.html#zeroblob
     */
    template<class N>
    internal::built_in_function_t<std::vector<char>, internal::zeroblob_string, N> zeroblob(N n) {
        return {std::tuple<N>{std::forward<N>(n)}};
    }

    /**
     *  SUBSTR(X,Y) function https://www.sqlite.org/lang_corefunc.html#substr
     */
    template<class X, class Y>
    internal::built_in_function_t<std::string, internal::substr_string, X, Y> substr(X x, Y y) {
        return {std::tuple<X, Y>{std::forward<X>(x), std::forward<Y>(y)}};
    }

    /**
     *  SUBSTR(X,Y,Z) function https://www.sqlite.org/lang_corefunc.html#substr
     */
    template<class X, class Y, class Z>
    internal::built_in_function_t<std::string, internal::substr_string, X, Y, Z> substr(X x, Y y, Z z) {
        return {std::tuple<X, Y, Z>{std::forward<X>(x), std::forward<Y>(y), std::forward<Z>(z)}};
    }

#ifdef SQLITE_SOUNDEX
    /**
     *  SOUNDEX(X) function https://www.sqlite.org/lang_corefunc.html#soundex
     */
    template<class X>
    internal::built_in_function_t<std::string, internal::soundex_string, X> soundex(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }
#endif

    /**
     *  TOTAL(X) aggregate function.
     */
    template<class X>
    internal::built_in_aggregate_function_t<double, internal::total_string, X> total(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  SUM(X) aggregate function.
     */
    template<class X>
    internal::built_in_aggregate_function_t<std::unique_ptr<double>, internal::sum_string, X> sum(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  COUNT(X) aggregate function.
     */
    template<class X>
    internal::built_in_aggregate_function_t<int, internal::count_string, X> count(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  COUNT(*) without FROM function.
     */
    inline internal::count_asterisk_without_type count() {
        return {};
    }

    /**
     *  COUNT(*) with FROM function. Specified type T will be serializeed as
     *  a from argument.
     */
    template<class T>
    internal::count_asterisk_t<T> count() {
        return {};
    }

    /**
     *  AVG(X) aggregate function.
     */
    template<class X>
    internal::built_in_aggregate_function_t<double, internal::avg_string, X> avg(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  MAX(X) aggregate function.
     */
    template<class X>
    internal::built_in_aggregate_function_t<internal::unique_ptr_result_of<X>, internal::max_string, X> max(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  MIN(X) aggregate function.
     */
    template<class X>
    internal::built_in_aggregate_function_t<internal::unique_ptr_result_of<X>, internal::min_string, X> min(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  MAX(X, Y, ...) scalar function.
     *  The return type is the type of the first argument.
     */
    template<class X, class Y, class... Rest>
    internal::built_in_function_t<internal::unique_ptr_result_of<X>, internal::max_string, X, Y, Rest...>
    max(X x, Y y, Rest... rest) {
        return {std::tuple<X, Y, Rest...>{std::forward<X>(x), std::forward<Y>(y), std::forward<Rest>(rest)...}};
    }

    /**
     *  MIN(X, Y, ...) scalar function.
     *  The return type is the type of the first argument.
     */
    template<class X, class Y, class... Rest>
    internal::built_in_function_t<internal::unique_ptr_result_of<X>, internal::min_string, X, Y, Rest...>
    min(X x, Y y, Rest... rest) {
        return {std::tuple<X, Y, Rest...>{std::forward<X>(x), std::forward<Y>(y), std::forward<Rest>(rest)...}};
    }

    /**
     *  GROUP_CONCAT(X) aggregate function.
     */
    template<class X>
    internal::built_in_aggregate_function_t<std::string, internal::group_concat_string, X> group_concat(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    /**
     *  GROUP_CONCAT(X, Y) aggregate function.
     */
    template<class X, class Y>
    internal::built_in_aggregate_function_t<std::string, internal::group_concat_string, X, Y> group_concat(X x, Y y) {
        return {std::tuple<X, Y>{std::forward<X>(x), std::forward<Y>(y)}};
    }
#ifdef SQLITE_ENABLE_JSON1
    template<class X>
    internal::built_in_function_t<std::string, internal::json_string, X> json(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    template<class... Args>
    internal::built_in_function_t<std::string, internal::json_array_string, Args...> json_array(Args... args) {
        return {std::tuple<Args...>{std::forward<Args>(args)...}};
    }

    template<class X>
    internal::built_in_function_t<int, internal::json_array_length_string, X> json_array_length(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    template<class R, class X>
    internal::built_in_function_t<R, internal::json_array_length_string, X> json_array_length(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    template<class X, class Y>
    internal::built_in_function_t<int, internal::json_array_length_string, X, Y> json_array_length(X x, Y y) {
        return {std::tuple<X, Y>{std::forward<X>(x), std::forward<Y>(y)}};
    }

    template<class R, class X, class Y>
    internal::built_in_function_t<R, internal::json_array_length_string, X, Y> json_array_length(X x, Y y) {
        return {std::tuple<X, Y>{std::forward<X>(x), std::forward<Y>(y)}};
    }

    template<class R, class X, class... Args>
    internal::built_in_function_t<R, internal::json_extract_string, X, Args...> json_extract(X x, Args... args) {
        return {std::tuple<X, Args...>{std::forward<X>(x), std::forward<Args>(args)...}};
    }

    template<class X, class... Args>
    internal::built_in_function_t<std::string, internal::json_insert_string, X, Args...> json_insert(X x,
                                                                                                     Args... args) {
        static_assert(std::tuple_size<std::tuple<Args...>>::value % 2 == 0,
                      "number of arguments in json_insert must be odd");
        return {std::tuple<X, Args...>{std::forward<X>(x), std::forward<Args>(args)...}};
    }

    template<class X, class... Args>
    internal::built_in_function_t<std::string, internal::json_replace_string, X, Args...> json_replace(X x,
                                                                                                       Args... args) {
        static_assert(std::tuple_size<std::tuple<Args...>>::value % 2 == 0,
                      "number of arguments in json_replace must be odd");
        return {std::tuple<X, Args...>{std::forward<X>(x), std::forward<Args>(args)...}};
    }

    template<class X, class... Args>
    internal::built_in_function_t<std::string, internal::json_set_string, X, Args...> json_set(X x, Args... args) {
        static_assert(std::tuple_size<std::tuple<Args...>>::value % 2 == 0,
                      "number of arguments in json_set must be odd");
        return {std::tuple<X, Args...>{std::forward<X>(x), std::forward<Args>(args)...}};
    }

    template<class... Args>
    internal::built_in_function_t<std::string, internal::json_object_string, Args...> json_object(Args... args) {
        static_assert(std::tuple_size<std::tuple<Args...>>::value % 2 == 0,
                      "number of arguments in json_object must be even");
        return {std::tuple<Args...>{std::forward<Args>(args)...}};
    }

    template<class X, class Y>
    internal::built_in_function_t<std::string, internal::json_patch_string, X, Y> json_patch(X x, Y y) {
        return {std::tuple<X, Y>{std::forward<X>(x), std::forward<Y>(y)}};
    }

    template<class X, class... Args>
    internal::built_in_function_t<std::string, internal::json_remove_string, X, Args...> json_remove(X x,
                                                                                                     Args... args) {
        return {std::tuple<X, Args...>{std::forward<X>(x), std::forward<Args>(args)...}};
    }

    template<class R, class X, class... Args>
    internal::built_in_function_t<R, internal::json_remove_string, X, Args...> json_remove(X x, Args... args) {
        return {std::tuple<X, Args...>{std::forward<X>(x), std::forward<Args>(args)...}};
    }

    template<class X>
    internal::built_in_function_t<std::string, internal::json_type_string, X> json_type(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    template<class R, class X>
    internal::built_in_function_t<R, internal::json_type_string, X> json_type(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    template<class X, class Y>
    internal::built_in_function_t<std::string, internal::json_type_string, X, Y> json_type(X x, Y y) {
        return {std::tuple<X, Y>{std::forward<X>(x), std::forward<Y>(y)}};
    }

    template<class R, class X, class Y>
    internal::built_in_function_t<R, internal::json_type_string, X, Y> json_type(X x, Y y) {
        return {std::tuple<X, Y>{std::forward<X>(x), std::forward<Y>(y)}};
    }

    template<class X>
    internal::built_in_function_t<bool, internal::json_valid_string, X> json_valid(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    template<class R, class X>
    internal::built_in_function_t<R, internal::json_quote_string, X> json_quote(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    template<class X>
    internal::built_in_function_t<std::string, internal::json_group_array_string, X> json_group_array(X x) {
        return {std::tuple<X>{std::forward<X>(x)}};
    }

    template<class X, class Y>
    internal::built_in_function_t<std::string, internal::json_group_object_string, X, Y> json_group_object(X x, Y y) {
        return {std::tuple<X, Y>{std::forward<X>(x), std::forward<Y>(y)}};
    }

#endif  //  SQLITE_ENABLE_JSON1
    template<class L,
             class R,
             std::enable_if_t<polyfill::disjunction_v<std::is_base_of<internal::arithmetic_t, L>,
                                                      std::is_base_of<internal::arithmetic_t, R>>,
                              bool> = true>
    internal::add_t<L, R> operator+(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L,
             class R,
             std::enable_if_t<polyfill::disjunction_v<std::is_base_of<internal::arithmetic_t, L>,
                                                      std::is_base_of<internal::arithmetic_t, R>>,
                              bool> = true>
    internal::sub_t<L, R> operator-(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L,
             class R,
             std::enable_if_t<polyfill::disjunction_v<std::is_base_of<internal::arithmetic_t, L>,
                                                      std::is_base_of<internal::arithmetic_t, R>>,
                              bool> = true>
    internal::mul_t<L, R> operator*(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L,
             class R,
             std::enable_if_t<polyfill::disjunction_v<std::is_base_of<internal::arithmetic_t, L>,
                                                      std::is_base_of<internal::arithmetic_t, R>>,
                              bool> = true>
    internal::div_t<L, R> operator/(L l, R r) {
        return {std::move(l), std::move(r)};
    }

    template<class L,
             class R,
             std::enable_if_t<polyfill::disjunction_v<std::is_base_of<internal::arithmetic_t, L>,
                                                      std::is_base_of<internal::arithmetic_t, R>>,
                              bool> = true>
    internal::mod_t<L, R> operator%(L l, R r) {
        return {std::move(l), std::move(r)};
    }
}
#pragma once

namespace sqlite_orm {

    namespace internal {

        template<class L, class R>
        bool compare_any(const L& /*lhs*/, const R& /*rhs*/) {
            return false;
        }
        template<class O>
        bool compare_any(const O& lhs, const O& rhs) {
            return lhs == rhs;
        }
    }
}
#pragma once

#include <string>  //  std::string
#include <utility>  //  std::declval
#include <tuple>  //  std::tuple, std::get, std::tuple_size
#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
#include <optional>  // std::optional
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

// #include "functional/cxx_universal.h"

// #include "functional/cxx_polyfill.h"

// #include "is_base_of_template.h"

// #include "tuple_helper/tuple_filter.h"

// #include "optional_container.h"

// #include "ast/where.h"

#include <type_traits>  //  std::false_type, std::true_type
#include <utility>  //  std::move

// #include "../functional/cxx_universal.h"

// #include "../functional/cxx_polyfill.h"

// #include "../serialize_result_type.h"

namespace sqlite_orm {
    namespace internal {

        struct where_string {
            serialize_result_type serialize() const {
                return "WHERE";
            }
        };

        /**
         *  WHERE argument holder.
         *  C is expression type. Can be any expression like: is_equal_t, is_null_t, exists_t etc
         *  Don't construct it manually. Call `where(...)` function instead.
         */
        template<class C>
        struct where_t : where_string {
            using expression_type = C;

            expression_type expression;

            where_t(expression_type expression_) : expression(std::move(expression_)) {}
        };

        template<class T>
        SQLITE_ORM_INLINE_VAR constexpr bool is_where_v = polyfill::is_specialization_of_v<T, where_t>;

        template<class T>
        using is_where = polyfill::bool_constant<is_where_v<T>>;
    }

    /**
     *  WHERE clause. Use it to add WHERE conditions wherever you like.
     *  C is expression type. Can be any expression like: is_equal_t, is_null_t, exists_t etc
     *  @example
     *  //  SELECT name
     *  //  FROM letters
     *  //  WHERE id > 3
     *  auto rows = storage.select(&Letter::name, where(greater_than(&Letter::id, 3)));
     */
    template<class C>
    internal::where_t<C> where(C expression) {
        return {std::move(expression)};
    }
}

// #include "ast/group_by.h"

#include <tuple>  //  std::tuple, std::make_tuple
#include <type_traits>  //  std::true_type, std::false_type
#include <utility>  //  std::forward, std::move

// #include "../functional/cxx_polyfill.h"

namespace sqlite_orm {
    namespace internal {

        template<class T, class... Args>
        struct group_by_with_having {
            using args_type = std::tuple<Args...>;
            using expression_type = T;

            args_type args;
            expression_type expression;
        };

        /**
         *  GROUP BY pack holder.
         */
        template<class... Args>
        struct group_by_t {
            using args_type = std::tuple<Args...>;

            args_type args;

            template<class T>
            group_by_with_having<T, Args...> having(T expression) {
                return {move(this->args), std::move(expression)};
            }
        };

        template<class T>
        using is_group_by = polyfill::disjunction<polyfill::is_specialization_of<T, group_by_t>,
                                                  polyfill::is_specialization_of<T, group_by_with_having>>;

        /**
         *  HAVING holder.
         *  T is having argument type.
         */
        template<class T>
        struct having_t {
            using expression_type = T;

            expression_type expression;
        };

        template<class T>
        using is_having = polyfill::is_specialization_of<T, having_t>;
    }

    /**
     *  GROUP BY column.
     *  Example: storage.get_all<Employee>(group_by(&Employee::name))
     */
    template<class... Args>
    internal::group_by_t<Args...> group_by(Args&&... args) {
        return {std::make_tuple(std::forward<Args>(args)...)};
    }

    /**
     *  [Deprecation notice]: this function is deprecated and will be removed in v1.9. Please use `group_by(...).having(...)` instead.
     *
     *  HAVING(expression).
     *  Example: storage.get_all<Employee>(group_by(&Employee::name), having(greater_than(count(&Employee::name), 2)));
     */
    template<class T>
    internal::having_t<T> having(T expression) {
        return {std::move(expression)};
    }
}

// #include "core_functions.h"

namespace sqlite_orm {

    namespace internal {
#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
        template<class T>
        struct as_optional_t {
            using value_type = T;

            value_type value;
        };
#endif  //  SQLITE_ORM_OPTIONAL_SUPPORTED

        struct distinct_string {
            operator std::string() const {
                return "DISTINCT";
            }
        };

        /**
         *  DISCTINCT generic container.
         */
        template<class T>
        struct distinct_t : distinct_string {
            using value_type = T;

            value_type value;

            distinct_t(value_type value_) : value(std::move(value_)) {}
        };

        struct all_string {
            operator std::string() const {
                return "ALL";
            }
        };

        /**
         *  ALL generic container.
         */
        template<class T>
        struct all_t : all_string {
            T value;

            all_t(T value_) : value(std::move(value_)) {}
        };

        template<class... Args>
        struct columns_t {
            using columns_type = std::tuple<Args...>;

            columns_type columns;
            bool distinct = false;

            static constexpr int count = std::tuple_size<columns_type>::value;

#ifndef SQLITE_ORM_AGGREGATE_NSDMI_SUPPORTED
            columns_t(columns_type columns) : columns{move(columns)} {}
#endif
        };

        template<class T>
        SQLITE_ORM_INLINE_VAR constexpr bool is_columns_v = polyfill::is_specialization_of_v<T, columns_t>;

        template<class T>
        using is_columns = polyfill::bool_constant<is_columns_v<T>>;

        template<class... Args>
        struct set_t {
            using assigns_type = std::tuple<Args...>;

            assigns_type assigns;
        };

        /**
         *  This class is used to store explicit mapped type T and its column descriptor (member pointer/getter/setter).
         *  Is useful when mapped type is derived from other type and base class has members mapped to a storage.
         */
        template<class T, class F>
        struct column_pointer {
            using self = column_pointer<T, F>;
            using type = T;
            using field_type = F;

            field_type field;

            template<class R>
            internal::is_equal_t<self, R> operator==(R rhs) const {
                return {*this, std::move(rhs)};
            }

            template<class R>
            internal::is_not_equal_t<self, R> operator!=(R rhs) const {
                return {*this, std::move(rhs)};
            }

            template<class R>
            internal::lesser_than_t<self, R> operator<(R rhs) const {
                return {*this, std::move(rhs)};
            }

            template<class R>
            internal::lesser_or_equal_t<self, R> operator<=(R rhs) const {
                return {*this, std::move(rhs)};
            }

            template<class R>
            internal::greater_than_t<self, R> operator>(R rhs) const {
                return {*this, std::move(rhs)};
            }

            template<class R>
            internal::greater_or_equal_t<self, R> operator>=(R rhs) const {
                return {*this, std::move(rhs)};
            }
        };

        /**
         *  Subselect object type.
         */
        template<class T, class... Args>
        struct select_t {
            using return_type = T;
            using conditions_type = std::tuple<Args...>;

            return_type col;
            conditions_type conditions;
            bool highest_level = false;

#ifndef SQLITE_ORM_AGGREGATE_NSDMI_SUPPORTED
            select_t(return_type col, conditions_type conditions) : col{std::move(col)}, conditions{move(conditions)} {}
#endif
        };

        template<class T>
        SQLITE_ORM_INLINE_VAR constexpr bool is_select_v = polyfill::is_specialization_of_v<T, select_t>;

        template<class T>
        using is_select = polyfill::bool_constant<is_select_v<T>>;

        /**
         *  Base for UNION, UNION ALL, EXCEPT and INTERSECT
         */
        template<class L, class R>
        struct compound_operator {
            using left_type = L;
            using right_type = R;

            left_type left;
            right_type right;

            compound_operator(left_type l, right_type r) : left(std::move(l)), right(std::move(r)) {
                this->left.highest_level = true;
                this->right.highest_level = true;
            }
        };

        struct union_base {
            bool all = false;

#ifndef SQLITE_ORM_AGGREGATE_NSDMI_SUPPORTED
            union_base(bool all) : all{all} {}
#endif

            operator std::string() const {
                if(!this->all) {
                    return "UNION";
                } else {
                    return "UNION ALL";
                }
            }
        };

        /**
         *  UNION object type.
         */
        template<class L, class R>
        struct union_t : public compound_operator<L, R>, union_base {
            using left_type = typename compound_operator<L, R>::left_type;
            using right_type = typename compound_operator<L, R>::right_type;

            union_t(left_type l, right_type r, bool all_) :
                compound_operator<L, R>{std::move(l), std::move(r)}, union_base{all_} {}

            union_t(left_type l, right_type r) : union_t{std::move(l), std::move(r), false} {}
        };

        struct except_string {
            operator std::string() const {
                return "EXCEPT";
            }
        };

        /**
         *  EXCEPT object type.
         */
        template<class L, class R>
        struct except_t : compound_operator<L, R>, except_string {
            using super = compound_operator<L, R>;
            using left_type = typename super::left_type;
            using right_type = typename super::right_type;

            using super::super;
        };

        struct intersect_string {
            operator std::string() const {
                return "INTERSECT";
            }
        };
        /**
         *  INTERSECT object type.
         */
        template<class L, class R>
        struct intersect_t : compound_operator<L, R>, intersect_string {
            using super = compound_operator<L, R>;
            using left_type = typename super::left_type;
            using right_type = typename super::right_type;

            using super::super;
        };

        /**
         *  Generic way to get DISTINCT value from any type.
         */
        template<class T>
        bool get_distinct(const T&) {
            return false;
        }

        template<class... Args>
        bool get_distinct(const columns_t<Args...>& cols) {
            return cols.distinct;
        }

        template<class T>
        struct asterisk_t {
            using type = T;
        };

        template<class T>
        struct object_t {
            using type = T;
        };

        template<class T>
        struct then_t {
            using expression_type = T;

            expression_type expression;
        };

        template<class R, class T, class E, class... Args>
        struct simple_case_t {
            using return_type = R;
            using case_expression_type = T;
            using args_type = std::tuple<Args...>;
            using else_expression_type = E;

            optional_container<case_expression_type> case_expression;
            args_type args;
            optional_container<else_expression_type> else_expression;
        };

        /**
         *  T is a case expression type
         *  E is else type (void is ELSE is omitted)
         *  Args... is a pack of WHEN expressions
         */
        template<class R, class T, class E, class... Args>
        struct simple_case_builder {
            using return_type = R;
            using case_expression_type = T;
            using args_type = std::tuple<Args...>;
            using else_expression_type = E;

            optional_container<case_expression_type> case_expression;
            args_type args;
            optional_container<else_expression_type> else_expression;

            template<class W, class Th>
            simple_case_builder<R, T, E, Args..., std::pair<W, Th>> when(W w, then_t<Th> t) {
                using result_args_type = std::tuple<Args..., std::pair<W, Th>>;
                std::pair<W, Th> newPair{std::move(w), std::move(t.expression)};
                result_args_type result_args =
                    std::tuple_cat(std::move(this->args), std::move(std::make_tuple(newPair)));
                std::get<std::tuple_size<result_args_type>::value - 1>(result_args) = std::move(newPair);
                return {std::move(this->case_expression), std::move(result_args), std::move(this->else_expression)};
            }

            simple_case_t<R, T, E, Args...> end() {
                return {std::move(this->case_expression), std::move(args), std::move(this->else_expression)};
            }

            template<class El>
            simple_case_builder<R, T, El, Args...> else_(El el) {
                return {{std::move(this->case_expression)}, std::move(args), {std::move(el)}};
            }
        };

        template<class T>
        void validate_conditions() {
            static_assert(count_tuple<T, is_where>::value <= 1, "a single query cannot contain > 1 WHERE blocks");
            static_assert(count_tuple<T, is_group_by>::value <= 1, "a single query cannot contain > 1 GROUP BY blocks");
            static_assert(count_tuple<T, is_order_by>::value <= 1, "a single query cannot contain > 1 ORDER BY blocks");
            static_assert(count_tuple<T, is_limit>::value <= 1, "a single query cannot contain > 1 LIMIT blocks");
            static_assert(count_tuple<T, is_from>::value <= 1, "a single query cannot contain > 1 FROM blocks");
        }
    }

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
    template<class T>
    internal::as_optional_t<T> as_optional(T value) {
        return {std::move(value)};
    }
#endif  //  SQLITE_ORM_OPTIONAL_SUPPORTED
    template<class T>
    internal::then_t<T> then(T t) {
        return {std::move(t)};
    }

    template<class R, class T>
    internal::simple_case_builder<R, T, void> case_(T t) {
        return {{std::move(t)}};
    }

    template<class R>
    internal::simple_case_builder<R, void, void> case_() {
        return {};
    }

    template<class T>
    internal::distinct_t<T> distinct(T t) {
        return {std::move(t)};
    }

    template<class T>
    internal::all_t<T> all(T t) {
        return {std::move(t)};
    }

    template<class... Args>
    internal::columns_t<Args...> distinct(internal::columns_t<Args...> cols) {
        cols.distinct = true;
        return cols;
    }

    /**
     *  SET keyword used in UPDATE ... SET queries.
     *  Args must have `assign_t` type. E.g. set(assign(&User::id, 5)) or set(c(&User::id) = 5)
     */
    template<class... Args>
    internal::set_t<Args...> set(Args... args) {
        using arg_tuple = std::tuple<Args...>;
        static_assert(std::tuple_size<arg_tuple>::value ==
                          internal::count_tuple<arg_tuple, internal::is_assign_t>::value,
                      "set function accepts assign operators only");
        return {std::make_tuple(std::forward<Args>(args)...)};
    }

    template<class... Args>
    internal::columns_t<Args...> columns(Args... args) {
        return {std::make_tuple<Args...>(std::forward<Args>(args)...)};
    }

    /**
     *  Use it like this:
     *  struct MyType : BaseType { ... };
     *  storage.select(column<MyType>(&BaseType::id));
     */
    template<class T, class F>
    internal::column_pointer<T, F> column(F f) {
        return {std::move(f)};
    }

    /**
     *  Public function for subselect query. Is useful in UNION queries.
     */
    template<class T, class... Args>
    internal::select_t<T, Args...> select(T t, Args... args) {
        using args_tuple = std::tuple<Args...>;
        internal::validate_conditions<args_tuple>();
        return {std::move(t), std::make_tuple(std::forward<Args>(args)...)};
    }

    /**
     *  Public function for UNION operator.
     *  lhs and rhs are subselect objects.
     *  Look through example in examples/union.cpp
     */
    template<class L, class R>
    internal::union_t<L, R> union_(L lhs, R rhs) {
        return {std::move(lhs), std::move(rhs)};
    }

    /**
     *  Public function for EXCEPT operator.
     *  lhs and rhs are subselect objects.
     *  Look through example in examples/except.cpp
     */
    template<class L, class R>
    internal::except_t<L, R> except(L lhs, R rhs) {
        return {std::move(lhs), std::move(rhs)};
    }

    template<class L, class R>
    internal::intersect_t<L, R> intersect(L lhs, R rhs) {
        return {std::move(lhs), std::move(rhs)};
    }

    /**
     *  Public function for UNION ALL operator.
     *  lhs and rhs are subselect objects.
     *  Look through example in examples/union.cpp
     */
    template<class L, class R>
    internal::union_t<L, R> union_all(L lhs, R rhs) {
        return {std::move(lhs), std::move(rhs), true};
    }

    /**
     * SELECT * FROM T function.
     * T is typed mapped to a storage.
     * Example: auto rows = storage.select(asterisk<User>());
     * // decltype(rows) is std::vector<std::tuple<...all column typed in declared in make_table order...>>
     * If you need to fetch result as objects not tuple please use `object<T>` instead.
     */
    template<class T>
    internal::asterisk_t<T> asterisk() {
        return {};
    }

    /**
     * SELECT * FROM T function.
     * T is typed mapped to a storage.
     * Example: auto rows = storage.select(object<User>());
     * // decltype(rows) is std::vector<User>
     * If you need to fetch result as tuples not objects please use `asterisk<T>` instead.
     */
    template<class T>
    internal::object_t<T> object() {
        return {};
    }
}
#pragma once

#include <string>  //  std::string

// #include "functional/cxx_universal.h"

namespace sqlite_orm {

    struct table_info {
        int cid = 0;
        std::string name;
        std::string type;
        bool notnull = false;
        std::string dflt_value;
        int pk = 0;

#if !defined(SQLITE_ORM_AGGREGATE_NSDMI_SUPPORTED) || !defined(SQLITE_ORM_AGGREGATE_PAREN_INIT_SUPPORTED)
        table_info(decltype(cid) cid_,
                   decltype(name) name_,
                   decltype(type) type_,
                   decltype(notnull) notnull_,
                   decltype(dflt_value) dflt_value_,
                   decltype(pk) pk_) :
            cid(cid_),
            name(move(name_)), type(move(type_)), notnull(notnull_), dflt_value(move(dflt_value_)), pk(pk_) {}
#endif
    };

    struct table_xinfo {
        int cid = 0;
        std::string name;
        std::string type;
        bool notnull = false;
        std::string dflt_value;
        int pk = 0;
        int hidden = 0;  // different than 0 => generated_always_as() - TODO verify

#if !defined(SQLITE_ORM_AGGREGATE_NSDMI_SUPPORTED) || !defined(SQLITE_ORM_AGGREGATE_PAREN_INIT_SUPPORTED)
        table_xinfo(decltype(cid) cid_,
                    decltype(name) name_,
                    decltype(type) type_,
                    decltype(notnull) notnull_,
                    decltype(dflt_value) dflt_value_,
                    decltype(pk) pk_,
                    decltype(hidden) hidden_) :
            cid(cid_),
            name(move(name_)), type(move(type_)), notnull(notnull_), dflt_value(move(dflt_value_)),
            pk(pk_), hidden{hidden_} {}
#endif
    };
}
#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <tuple>

// #include "functional/cxx_universal.h"

// #include "optional_container.h"

// NOTE Idea : Maybe also implement a custom trigger system to call a c++ callback when a trigger triggers ?
// (Could be implemented with a normal trigger that insert or update an internal table and then retreive
// the event in the C++ code, to call the C++ user callback, with update hooks: https://www.sqlite.org/c3ref/update_hook.html)
// It could be an interesting feature to bring to sqlite_orm that other libraries don't have ?

namespace sqlite_orm {
    namespace internal {
        enum class trigger_timing { trigger_before, trigger_after, trigger_instead_of };
        enum class trigger_type { trigger_delete, trigger_insert, trigger_update };

        /**
         * This class is an intermediate SQLite trigger, to be used with
         * `make_trigger` to create a full trigger.
         *  T is the base of the trigger (contains its type, timing and associated table)
         *  S is the list of trigger statements
         */
        template<class T, class... S>
        struct partial_trigger_t {
            using statements_type = std::tuple<S...>;

            /**
             * Base of the trigger (contains its type, timing and associated table)
             */
            T base;
            /**
             * Statements of the triggers (to be executed when the trigger fires)
             */
            statements_type statements;

            partial_trigger_t(T trigger_base, S... statements) :
                base{std::move(trigger_base)}, statements{std::make_tuple<S...>(std::forward<S>(statements)...)} {}

            partial_trigger_t& end() {
                return *this;
            }
        };

        struct base_trigger {
            /**
             * Name of the trigger
             */
            std::string name;
        };

        /**
         * This class represent a SQLite trigger
         *  T is the base of the trigger (contains its type, timing and associated table)
         *  S is the list of trigger statments
         */
        template<class T, class... S>
        struct trigger_t : base_trigger {
            using object_type = void;  // Not sure
            using elements_type = typename partial_trigger_t<T, S...>::statements_type;

            /**
             * Base of the trigger (contains its type, timing and associated table)
             */
            T base;

            /**
             * Statements of the triggers (to be executed when the trigger fires)
             */
            elements_type elements;

#ifndef SQLITE_ORM_AGGREGATE_BASES_SUPPORTED
            trigger_t(std::string name, T trigger_base, elements_type statements) :
                base_trigger{move(name)}, base(std::move(trigger_base)), elements(move(statements)) {}
#endif
        };

        /**
         * Base of a trigger. Contains the trigger type/timming and the table type
         * T is the table type
         * W is `when` expression type
         * Type is the trigger base type (type+timing)
         */
        template<class T, class W, class Type>
        struct trigger_base_t {
            using table_type = T;
            using when_type = W;
            using trigger_type_base = Type;

            /**
             * Contains the trigger type and timing
             */
            trigger_type_base type_base;
            /**
             * Value used to determine if we execute the trigger on each row or on each statement
             * (SQLite doesn't support the FOR EACH STATEMENT syntax yet: https://sqlite.org/lang_createtrigger.html#description
             * so this value is more of a placeholder for a later update)
             */
            bool do_for_each_row = false;
            /**
             * When expression (if any)
             * If a WHEN expression is specified, the trigger will only execute
             * if the expression evaluates to true when the trigger is fired
             */
            optional_container<when_type> container_when;

            trigger_base_t(trigger_type_base type_base_) : type_base(std::move(type_base_)) {}

            trigger_base_t& for_each_row() {
                this->do_for_each_row = true;
                return *this;
            }

            template<class WW>
            trigger_base_t<T, WW, Type> when(WW expression) {
                trigger_base_t<T, WW, Type> res(this->type_base);
                res.container_when.field = std::move(expression);
                return res;
            }

            template<class... S>
            partial_trigger_t<trigger_base_t<T, W, Type>, S...> begin(S... statements) {
                return {*this, std::forward<S>(statements)...};
            }
        };

        /**
         * Contains the trigger type and timing
         */
        struct trigger_type_base_t {
            /**
             * Value indicating if the trigger is run BEFORE, AFTER or INSTEAD OF
             * the statement that fired it.
             */
            trigger_timing timing;
            /**
             * The type of the statement that would cause the trigger to fire.
             * Can be DELETE, INSERT, or UPDATE.
             */
            trigger_type type;

            trigger_type_base_t(trigger_timing timing, trigger_type type) : timing(timing), type(type) {}

            template<class T>
            trigger_base_t<T, void, trigger_type_base_t> on() {
                return {*this};
            }
        };

        /**
         * Special case for UPDATE OF (columns)
         * Contains the trigger type and timing
         */
        template<class... Cs>
        struct trigger_update_type_t : trigger_type_base_t {
            using columns_type = std::tuple<Cs...>;

            /**
             * Contains the columns the trigger is watching. Will only
             * trigger if one of theses columns is updated.
             */
            columns_type columns;

            trigger_update_type_t(trigger_timing timing, trigger_type type, Cs... columns) :
                trigger_type_base_t(timing, type), columns(std::make_tuple<Cs...>(std::forward<Cs>(columns)...)) {}

            template<class T>
            trigger_base_t<T, void, trigger_update_type_t<Cs...>> on() {
                return {*this};
            }
        };

        struct trigger_timing_t {
            trigger_timing timing;

            trigger_type_base_t delete_() {
                return {timing, trigger_type::trigger_delete};
            }

            trigger_type_base_t insert() {
                return {timing, trigger_type::trigger_insert};
            }

            trigger_type_base_t update() {
                return {timing, trigger_type::trigger_update};
            }

            template<class... Cs>
            trigger_update_type_t<Cs...> update_of(Cs... columns) {
                return {timing, trigger_type::trigger_update, std::forward<Cs>(columns)...};
            }
        };

        struct raise_t {
            enum class type_t {
                ignore,
                rollback,
                abort,
                fail,
            };

            type_t type = type_t::ignore;
            std::string message;

#ifndef SQLITE_ORM_AGGREGATE_NSDMI_SUPPORTED
            raise_t(type_t type, std::string message) : type{type}, message{move(message)} {}
#endif
        };

        template<class T>
        struct new_t {
            using expression_type = T;

            expression_type expression;
        };

        template<class T>
        struct old_t {
            using expression_type = T;

            expression_type expression;
        };
    }  // NAMESPACE internal

    /**
     *  NEW.expression function used within TRIGGER expressions
     */
    template<class T>
    internal::new_t<T> new_(T expression) {
        return {std::move(expression)};
    }

    /**
     *  OLD.expression function used within TRIGGER expressions
     */
    template<class T>
    internal::old_t<T> old(T expression) {
        return {std::move(expression)};
    }

    /**
     *  RAISE(IGNORE) expression used within TRIGGER expressions
     */
    inline internal::raise_t raise_ignore() {
        return {internal::raise_t::type_t::ignore, {}};
    }

    /**
     *  RAISE(ROLLBACK, %message%) expression used within TRIGGER expressions
     */
    inline internal::raise_t raise_rollback(std::string message) {
        return {internal::raise_t::type_t::rollback, move(message)};
    }

    /**
     *  RAISE(ABORT, %message%) expression used within TRIGGER expressions
     */
    inline internal::raise_t raise_abort(std::string message) {
        return {internal::raise_t::type_t::abort, move(message)};
    }

    /**
     *  RAISE(FAIL, %message%) expression used within TRIGGER expressions
     */
    inline internal::raise_t raise_fail(std::string message) {
        return {internal::raise_t::type_t::fail, move(message)};
    }

    template<class T, class... S>
    internal::trigger_t<T, S...> make_trigger(std::string name, const internal::partial_trigger_t<T, S...>& part) {
        SQLITE_ORM_CLANG_SUPPRESS_MISSING_BRACES(return {move(name), std::move(part.base), std::move(part.statements)});
    }

    inline internal::trigger_timing_t before() {
        return {internal::trigger_timing::trigger_before};
    }

    inline internal::trigger_timing_t after() {
        return {internal::trigger_timing::trigger_after};
    }

    inline internal::trigger_timing_t instead_of() {
        return {internal::trigger_timing::trigger_instead_of};
    }
}
#pragma once

#include <sqlite3.h>
#include <memory>  // std::unique_ptr
#include <type_traits>  // std::integral_constant

namespace sqlite_orm {

    /**
     *  Guard class which finalizes `sqlite3_stmt` in dtor
     */
    using statement_finalizer =
        std::unique_ptr<sqlite3_stmt, std::integral_constant<decltype(&sqlite3_finalize), sqlite3_finalize>>;
}
#pragma once
#include <type_traits>

namespace sqlite_orm {

    /**
     *  Helper classes used by statement_binder and row_extractor.
     */
    struct int_or_smaller_tag {};
    struct bigint_tag {};
    struct real_tag {};

    template<class V>
    using arithmetic_tag_t =
        std::conditional_t<std::is_integral<V>::value,
                           // Integer class
                           std::conditional_t<sizeof(V) <= sizeof(int), int_or_smaller_tag, bigint_tag>,
                           // Floating-point class
                           real_tag>;
}
#pragma once

#include <type_traits>
#include <memory>
#include <utility>

// #include "functional/cxx_universal.h"

// #include "xdestroy_handling.h"

#include <type_traits>  // std::integral_constant
#if defined(SQLITE_ORM_CONCEPTS_SUPPORTED) && SQLITE_ORM_HAS_INCLUDE(<concepts>)
#include <concepts>
#endif

// #include "functional/cxx_universal.h"

// #include "functional/cxx_polyfill.h"

namespace sqlite_orm {

    using xdestroy_fn_t = void (*)(void*);
    using null_xdestroy_t = std::integral_constant<xdestroy_fn_t, nullptr>;
    SQLITE_ORM_INLINE_VAR constexpr null_xdestroy_t null_xdestroy_f{};
}

namespace sqlite_orm {
    namespace internal {
#ifdef SQLITE_ORM_CONCEPTS_SUPPORTED
        /**
         *  Constraints a deleter to be state-less.
         */
        template<typename D>
        concept stateless_deleter = std::is_empty_v<D> && std::is_default_constructible_v<D>;

        /**
         *  Constraints a deleter to be an integral function constant.
         */
        template<typename D>
        concept integral_fp_c = requires {
            typename D::value_type;
            D::value;
            requires std::is_function_v<std::remove_pointer_t<typename D::value_type>>;
        };

        /**
         *  Constraints a deleter to be or to yield a function pointer.
         */
        template<typename D>
        concept yields_fp = requires(D d) {
            // yielding function pointer by using the plus trick
            {+d};
            requires std::is_function_v<std::remove_pointer_t<decltype(+d)>>;
        };
#endif

#if __cpp_lib_concepts >= 201907L
        /**
         *  Yield a deleter's function pointer.
         */
        template<yields_fp D>
        struct yield_fp_of {
            using type = decltype(+std::declval<D>());
        };
#else

        template<typename D>
        SQLITE_ORM_INLINE_VAR constexpr bool is_stateless_deleter_v =
            std::is_empty<D>::value&& std::is_default_constructible<D>::value;

        template<typename D, typename SFINAE = void>
        struct is_integral_fp_c : std::false_type {};
        template<typename D>
        struct is_integral_fp_c<
            D,
            polyfill::void_t<typename D::value_type,
                             decltype(D::value),
                             std::enable_if_t<std::is_function<std::remove_pointer_t<typename D::value_type>>::value>>>
            : std::true_type {};
        template<typename D>
        SQLITE_ORM_INLINE_VAR constexpr bool is_integral_fp_c_v = is_integral_fp_c<D>::value;

        template<typename D, typename SFINAE = void>
        struct can_yield_fp : std::false_type {};
        template<typename D>
        struct can_yield_fp<
            D,
            polyfill::void_t<
                decltype(+std::declval<D>()),
                std::enable_if_t<std::is_function<std::remove_pointer_t<decltype(+std::declval<D>())>>::value>>>
            : std::true_type {};
        template<typename D>
        SQLITE_ORM_INLINE_VAR constexpr bool can_yield_fp_v = can_yield_fp<D>::value;

        template<typename D, bool = can_yield_fp_v<D>>
        struct yield_fp_of {
            using type = void;
        };
        template<typename D>
        struct yield_fp_of<D, true> {
            using type = decltype(+std::declval<D>());
        };
#endif
        template<typename D>
        using yielded_fn_t = typename yield_fp_of<D>::type;

#if __cpp_lib_concepts >= 201907L
        template<typename D>
        concept is_unusable_for_xdestroy = (!stateless_deleter<D> &&
                                            (yields_fp<D> && !std::convertible_to<yielded_fn_t<D>, xdestroy_fn_t>));

        /**
         *  This concept tests whether a deleter yields a function pointer, which is convertible to an xdestroy function pointer.
         *  Note: We are using 'is convertible' rather than 'is same' because of any exception specification.
         */
        template<typename D>
        concept yields_xdestroy = yields_fp<D> && std::convertible_to<yielded_fn_t<D>, xdestroy_fn_t>;

        template<typename D, typename P>
        concept needs_xdestroy_proxy = (stateless_deleter<D> &&
                                        (!yields_fp<D> || !std::convertible_to<yielded_fn_t<D>, xdestroy_fn_t>));

        /**
         *  xDestroy function that constructs and invokes the stateless deleter.
         *  
         *  Requires that the deleter can be called with the q-qualified pointer argument;
         *  it doesn't check so explicitly, but a compiler error will occur.
         */
        template<typename D, typename P>
        requires(!integral_fp_c<D>) void xdestroy_proxy(void* p) noexcept {
            // C-casting `void* -> P*` like statement_binder<pointer_binding<P, T, D>>
            auto o = (P*)p;
            // ignoring return code
            (void)D{}(o);
        }

        /**
         *  xDestroy function that invokes the integral function pointer constant.
         *  
         *  Performs a const-cast of the argument pointer in order to allow for C API functions
         *  that take a non-const parameter, but user code passes a pointer to a const object.
         */
        template<integral_fp_c D, typename P>
        void xdestroy_proxy(void* p) noexcept {
            // C-casting `void* -> P*` like statement_binder<pointer_binding<P, T, D>>,
            auto o = (std::remove_cv_t<P>*)(P*)p;
            // ignoring return code
            (void)D{}(o);
        }
#else
        template<typename D>
        SQLITE_ORM_INLINE_VAR constexpr bool is_unusable_for_xdestroy_v =
            !is_stateless_deleter_v<D> &&
            (can_yield_fp_v<D> && !std::is_convertible<yielded_fn_t<D>, xdestroy_fn_t>::value);

        template<typename D>
        SQLITE_ORM_INLINE_VAR constexpr bool can_yield_xdestroy_v =
            can_yield_fp_v<D>&& std::is_convertible<yielded_fn_t<D>, xdestroy_fn_t>::value;

        template<typename D, typename P>
        SQLITE_ORM_INLINE_VAR constexpr bool needs_xdestroy_proxy_v =
            is_stateless_deleter_v<D> &&
            (!can_yield_fp_v<D> || !std::is_convertible<yielded_fn_t<D>, xdestroy_fn_t>::value);

        template<typename D, typename P, std::enable_if_t<!is_integral_fp_c_v<D>, bool> = true>
        void xdestroy_proxy(void* p) noexcept {
            // C-casting `void* -> P*` like statement_binder<pointer_binding<P, T, D>>
            auto o = (P*)p;
            // ignoring return code
            (void)D{}(o);
        }

        template<typename D, typename P, std::enable_if_t<is_integral_fp_c_v<D>, bool> = true>
        void xdestroy_proxy(void* p) noexcept {
            // C-casting `void* -> P*` like statement_binder<pointer_binding<P, T, D>>,
            auto o = (std::remove_cv_t<P>*)(P*)p;
            // ignoring return code
            (void)D{}(o);
        }
#endif
    }
}

namespace sqlite_orm {

#if __cpp_lib_concepts >= 201907L
    /**
     *  Prohibits using a yielded function pointer, which is not of type xdestroy_fn_t.
     *  
     *  Explicitly declared for better error messages.
     */
    template<typename D, typename P>
    constexpr xdestroy_fn_t obtain_xdestroy_for(D, P*) noexcept requires(internal::is_unusable_for_xdestroy<D>) {
        static_assert(polyfill::always_false_v<D>,
                      "A function pointer, which is not of type xdestroy_fn_t, is prohibited.");
        return nullptr;
    }

    /**
     *  Obtains a proxy 'xDestroy' function pointer [of type void(*)(void*)]
     *  for a deleter in a type-safe way.
     *  
     *  The deleter can be one of:
     *         - integral function constant
     *         - state-less (empty) deleter
     *         - non-capturing lambda
     *  
     *  Type-safety is garanteed by checking whether the deleter or yielded function pointer
     *  is invocable with the non-q-qualified pointer value.
     */
    template<typename D, typename P>
    constexpr xdestroy_fn_t obtain_xdestroy_for(D, P*) noexcept requires(internal::needs_xdestroy_proxy<D, P>) {
        return internal::xdestroy_proxy<D, P>;
    }

    /**
     *  Directly obtains a 'xDestroy' function pointer [of type void(*)(void*)]
     *  from a deleter in a type-safe way.
     *  
     *  The deleter can be one of:
     *         - function pointer of type xdestroy_fn_t
     *         - structure holding a function pointer
     *         - integral function constant
     *         - non-capturing lambda
     *  ... and yield a function pointer of type xdestroy_fn_t.
     *  
     *  Type-safety is garanteed by checking whether the deleter or yielded function pointer
     *  is invocable with the non-q-qualified pointer value.
     */
    template<typename D, typename P>
    constexpr xdestroy_fn_t obtain_xdestroy_for(D d, P*) noexcept requires(internal::yields_xdestroy<D>) {
        return d;
    }
#else
    template<typename D, typename P, std::enable_if_t<internal::is_unusable_for_xdestroy_v<D>, bool> = true>
    constexpr xdestroy_fn_t obtain_xdestroy_for(D, P*) {
        static_assert(polyfill::always_false_v<D>,
                      "A function pointer, which is not of type xdestroy_fn_t, is prohibited.");
        return nullptr;
    }

    template<typename D, typename P, std::enable_if_t<internal::needs_xdestroy_proxy_v<D, P>, bool> = true>
    constexpr xdestroy_fn_t obtain_xdestroy_for(D, P*) noexcept {
        return internal::xdestroy_proxy<D, P>;
    }

    template<typename D, typename P, std::enable_if_t<internal::can_yield_xdestroy_v<D>, bool> = true>
    constexpr xdestroy_fn_t obtain_xdestroy_for(D d, P*) noexcept {
        return d;
    }
#endif
}

namespace sqlite_orm {

    /**
     *  Wraps a pointer and tags it with a pointer type,
     *  used for accepting function parameters,
     *  facilitating the 'pointer-passing interface'.
     * 
     *  Template parameters:
     *    - P: The value type, possibly const-qualified.
     *    - T: An integral constant string denoting the pointer type, e.g. `carray_pvt_name`.
     *
     */
    template<typename P, typename T>
    struct pointer_arg {

        static_assert(std::is_convertible<typename T::value_type, const char*>::value,
                      "`std::integral_constant<>` must be convertible to `const char*`");

        using tag = T;
        P* p_;

        P* ptr() const noexcept {
            return p_;
        }

        operator P*() const noexcept {
            return p_;
        }
    };

    /**
     *  Pointer value with associated deleter function,
     *  used for returning or binding pointer values
     *  as part of facilitating the 'pointer-passing interface'.
     * 
     *  Template parameters:
     *    - D: The deleter for the pointer value;
     *         can be one of:
     *         - function pointer
     *         - integral function pointer constant
     *         - state-less (empty) deleter
     *         - non-capturing lambda
     *         - structure implicitly yielding a function pointer
     *
     *  @note Use one of the factory functions to create a pointer binding,
     *  e.g. bindable_carray_pointer or statically_bindable_carray_pointer().
     *  
     *  @example
     *  ```
     *  int64 rememberedId;
     *  storage.select(func<remember_fn>(&Object::id, statically_bindable_carray_pointer(&rememberedId)));
     *  ```
     */
    template<typename P, typename T, typename D>
    class pointer_binding {

        P* p_;
        SQLITE_ORM_NOUNIQUEADDRESS
        D d_;

      protected:
        // Constructing pointer bindings must go through bindable_pointer()
        template<class T2, class P2, class D2>
        friend auto bindable_pointer(P2*, D2) noexcept -> pointer_binding<P2, T2, D2>;
        template<class B>
        friend B bindable_pointer(typename B::qualified_type*, typename B::deleter_type) noexcept;

        // Construct from pointer and deleter.
        // Transfers ownership of the passed in object.
        pointer_binding(P* p, D d = {}) noexcept : p_{p}, d_{std::move(d)} {}

      public:
        using qualified_type = P;
        using tag = T;
        using deleter_type = D;

        pointer_binding(const pointer_binding&) = delete;
        pointer_binding& operator=(const pointer_binding&) = delete;
        pointer_binding& operator=(pointer_binding&&) = delete;

        pointer_binding(pointer_binding&& other) noexcept :
            p_{std::exchange(other.p_, nullptr)}, d_{std::move(other.d_)} {}

        ~pointer_binding() {
            if(p_) {
                if(auto xDestroy = get_xdestroy()) {
                    // note: C-casting `P* -> void*` like statement_binder<pointer_binding<P, T, D>>
                    xDestroy((void*)p_);
                }
            }
        }

        P* ptr() const noexcept {
            return p_;
        }

        P* take_ptr() noexcept {
            return std::exchange(p_, nullptr);
        }

        xdestroy_fn_t get_xdestroy() const noexcept {
            return obtain_xdestroy_for(d_, p_);
        }
    };

    /**
     *  Template alias for a static pointer value binding.
     *  'Static' means that ownership won't be transferred to sqlite,
     *  sqlite doesn't delete it, and sqlite assumes the object
     *  pointed to is valid throughout the lifetime of a statement.
     */
    template<typename P, typename T>
    using static_pointer_binding = pointer_binding<P, T, null_xdestroy_t>;
}

namespace sqlite_orm {

    /**
     *  Wrap a pointer, its type and its deleter function for binding it to a statement.
     *  
     *  Unless the deleter yields a nullptr 'xDestroy' function the ownership of the pointed-to-object
     *  is transferred to the pointer binding, which will delete it through
     *  the deleter when the statement finishes.
     */
    template<class T, class P, class D>
    auto bindable_pointer(P* p, D d) noexcept -> pointer_binding<P, T, D> {
        return {p, std::move(d)};
    }

    template<class T, class P, class D>
    auto bindable_pointer(std::unique_ptr<P, D> p) noexcept -> pointer_binding<P, T, D> {
        return bindable_pointer<T>(p.release(), p.get_deleter());
    }

    template<typename B>
    B bindable_pointer(typename B::qualified_type* p, typename B::deleter_type d = {}) noexcept {
        return B{p, std::move(d)};
    }

    /**
     *  Wrap a pointer and its type for binding it to a statement.
     *  
     *  Note: 'Static' means that ownership of the pointed-to-object won't be transferred
     *  and sqlite assumes the object pointed to is valid throughout the lifetime of a statement.
     */
    template<class T, class P>
    auto statically_bindable_pointer(P* p) noexcept -> static_pointer_binding<P, T> {
        return bindable_pointer<T>(p, null_xdestroy_f);
    }

    template<typename B>
    B statically_bindable_pointer(typename B::qualified_type* p,
                                  typename B::deleter_type* /*exposition*/ = nullptr) noexcept {
        return bindable_pointer<B>(p);
    }

    /**
     *  Forward a pointer value from an argument.
     */
    template<class P, class T>
    auto rebind_statically(const pointer_arg<P, T>& pv) noexcept -> static_pointer_binding<P, T> {
        return statically_bindable_pointer<T>(pv.ptr());
    }
}
#pragma once

#include <sqlite3.h>
#include <type_traits>  //  std::enable_if_t, std::is_arithmetic, std::is_same, std::true_type, std::false_type
#include <memory>  //  std::default_delete
#include <string>  //  std::string, std::wstring
#include <vector>  //  std::vector
#include <cstring>  //  ::strncpy, ::strlen
#ifndef SQLITE_ORM_STRING_VIEW_SUPPORTED
#include <cwchar>  //  ::wcsncpy, ::wcslen
#endif

// #include "functional/cxx_universal.h"

// #include "functional/cxx_polyfill.h"

// #include "is_std_ptr.h"

// #include "tuple_helper/tuple_filter.h"

// #include "error_code.h"

// #include "arithmetic_tag.h"

// #include "xdestroy_handling.h"

// #include "pointer_value.h"

namespace sqlite_orm {

    /**
     *  Helper class used for binding fields to sqlite3 statements.
     */
    template<class V, typename Enable = void>
    struct statement_binder;

    namespace internal {

        template<class T, class SFINAE = void>
        SQLITE_ORM_INLINE_VAR constexpr bool is_bindable_v = false;
        template<class T>
        SQLITE_ORM_INLINE_VAR constexpr bool is_bindable_v<T, polyfill::void_t<decltype(statement_binder<T>())>> = true
            // note : msvc 14.0 needs the parentheses constructor, otherwise `is_bindable<const char*>` isn't recognised.
            // The strangest thing is that this is mutually exclusive with `is_printable_v`.
            ;

        template<class T>
        using is_bindable = polyfill::bool_constant<is_bindable_v<T>>;

    }

    /**
     *  Specialization for 'pointer-passing interface'.
     */
    template<class P, class T, class D>
    struct statement_binder<pointer_binding<P, T, D>, void> {
        using V = pointer_binding<P, T, D>;

        // ownership of pointed-to-object is left untouched and remains at prepared statement's AST expression
        int bind(sqlite3_stmt* stmt, int index, const V& value) const {
            // note: C-casting `P* -> void*`, internal::xdestroy_proxy() does the inverse
            return sqlite3_bind_pointer(stmt, index, (void*)value.ptr(), T::value, null_xdestroy_f);
        }

        // ownership of pointed-to-object is transferred to sqlite
        void result(sqlite3_context* context, V& value) const {
            // note: C-casting `P* -> void*`,
            // row_extractor<pointer_arg<P, T>>::extract() and internal::xdestroy_proxy() do the inverse
            sqlite3_result_pointer(context, (void*)value.take_ptr(), T::value, value.get_xdestroy());
        }
    };

    /**
     *  Specialization for arithmetic types.
     */
    template<class V>
    struct statement_binder<V, std::enable_if_t<std::is_arithmetic<V>::value>> {

        int bind(sqlite3_stmt* stmt, int index, const V& value) const {
            return this->bind(stmt, index, value, tag());
        }

        void result(sqlite3_context* context, const V& value) const {
            this->result(context, value, tag());
        }

      private:
        using tag = arithmetic_tag_t<V>;

        int bind(sqlite3_stmt* stmt, int index, const V& value, int_or_smaller_tag) const {
            return sqlite3_bind_int(stmt, index, static_cast<int>(value));
        }

        void result(sqlite3_context* context, const V& value, int_or_smaller_tag) const {
            sqlite3_result_int(context, static_cast<int>(value));
        }

        int bind(sqlite3_stmt* stmt, int index, const V& value, bigint_tag) const {
            return sqlite3_bind_int64(stmt, index, static_cast<sqlite3_int64>(value));
        }

        void result(sqlite3_context* context, const V& value, bigint_tag) const {
            sqlite3_result_int64(context, static_cast<sqlite3_int64>(value));
        }

        int bind(sqlite3_stmt* stmt, int index, const V& value, real_tag) const {
            return sqlite3_bind_double(stmt, index, static_cast<double>(value));
        }

        void result(sqlite3_context* context, const V& value, real_tag) const {
            sqlite3_result_double(context, static_cast<double>(value));
        }
    };

    /**
     *  Specialization for std::string and C-string.
     */
    template<class V>
    struct statement_binder<V,
                            std::enable_if_t<polyfill::disjunction_v<std::is_base_of<std::string, V>,
                                                                     std::is_same<V, const char*>
#ifdef SQLITE_ORM_STRING_VIEW_SUPPORTED
                                                                     ,
                                                                     std::is_same<V, std::string_view>
#endif
                                                                     >>> {

        int bind(sqlite3_stmt* stmt, int index, const V& value) const {
            auto stringData = this->string_data(value);
            return sqlite3_bind_text(stmt, index, stringData.first, stringData.second, SQLITE_TRANSIENT);
        }

        void result(sqlite3_context* context, const V& value) const {
            auto stringData = this->string_data(value);
            auto dataCopy = new char[stringData.second + 1];
            constexpr auto deleter = std::default_delete<char[]>{};
            ::strncpy(dataCopy, stringData.first, stringData.second + 1);
            sqlite3_result_text(context, dataCopy, stringData.second, obtain_xdestroy_for(deleter, dataCopy));
        }

      private:
#ifdef SQLITE_ORM_STRING_VIEW_SUPPORTED
        std::pair<const char*, int> string_data(const std::string_view& s) const {
            return {s.data(), int(s.size())};
        }
#else
        std::pair<const char*, int> string_data(const std::string& s) const {
            return {s.c_str(), int(s.size())};
        }

        std::pair<const char*, int> string_data(const char* s) const {
            return {s, int(::strlen(s))};
        }
#endif
    };

#ifndef SQLITE_ORM_OMITS_CODECVT
    template<class V>
    struct statement_binder<V,
                            std::enable_if_t<polyfill::disjunction_v<std::is_base_of<std::wstring, V>,
                                                                     std::is_same<V, const wchar_t*>
#ifdef SQLITE_ORM_STRING_VIEW_SUPPORTED
                                                                     ,
                                                                     std::is_same<V, std::wstring_view>
#endif
                                                                     >>> {

        int bind(sqlite3_stmt* stmt, int index, const V& value) const {
            auto stringData = this->string_data(value);
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::string utf8Str = converter.to_bytes(stringData.first, stringData.first + stringData.second);
            return statement_binder<decltype(utf8Str)>().bind(stmt, index, utf8Str);
        }

        void result(sqlite3_context* context, const V& value) const {
            auto stringData = this->string_data(value);
            sqlite3_result_text16(context, stringData.first, stringData.second, nullptr);
        }

      private:
#ifdef SQLITE_ORM_STRING_VIEW_SUPPORTED
        std::pair<const wchar_t*, int> string_data(const std::wstring_view& s) const {
            return {s.data(), int(s.size())};
        }
#else
        std::pair<const wchar_t*, int> string_data(const std::wstring& s) const {
            return {s.c_str(), int(s.size())};
        }

        std::pair<const wchar_t*, int> string_data(const wchar_t* s) const {
            return {s, int(::wcslen(s))};
        }
#endif
    };
#endif

    /**
     *  Specialization for nullptr_t.
     */
    template<>
    struct statement_binder<nullptr_t, void> {
        int bind(sqlite3_stmt* stmt, int index, const nullptr_t&) const {
            return sqlite3_bind_null(stmt, index);
        }

        void result(sqlite3_context* context, const nullptr_t&) const {
            sqlite3_result_null(context);
        }
    };

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
    /**
     *  Specialization for std::nullopt_t.
     */
    template<>
    struct statement_binder<std::nullopt_t, void> {
        int bind(sqlite3_stmt* stmt, int index, const std::nullopt_t&) const {
            return sqlite3_bind_null(stmt, index);
        }

        void result(sqlite3_context* context, const std::nullopt_t&) const {
            sqlite3_result_null(context);
        }
    };
#endif  //  SQLITE_ORM_OPTIONAL_SUPPORTED

    template<class V>
    struct statement_binder<
        V,
        std::enable_if_t<is_std_ptr<V>::value && internal::is_bindable_v<std::remove_cv_t<typename V::element_type>>>> {
        using unqualified_type = std::remove_cv_t<typename V::element_type>;

        int bind(sqlite3_stmt* stmt, int index, const V& value) const {
            if(value) {
                return statement_binder<unqualified_type>().bind(stmt, index, *value);
            } else {
                return statement_binder<nullptr_t>().bind(stmt, index, nullptr);
            }
        }
    };

    /**
     *  Specialization for binary data (std::vector<char>).
     */
    template<>
    struct statement_binder<std::vector<char>, void> {
        int bind(sqlite3_stmt* stmt, int index, const std::vector<char>& value) const {
            if(value.size()) {
                return sqlite3_bind_blob(stmt, index, (const void*)&value.front(), int(value.size()), SQLITE_TRANSIENT);
            } else {
                return sqlite3_bind_blob(stmt, index, "", 0, SQLITE_TRANSIENT);
            }
        }

        void result(sqlite3_context* context, const std::vector<char>& value) const {
            if(value.size()) {
                sqlite3_result_blob(context, (const void*)&value.front(), int(value.size()), nullptr);
            } else {
                sqlite3_result_blob(context, "", 0, nullptr);
            }
        }
    };

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
    template<class V>
    struct statement_binder<V,
                            std::enable_if_t<polyfill::is_specialization_of_v<V, std::optional> &&
                                             internal::is_bindable_v<std::remove_cv_t<typename V::value_type>>>> {
        using unqualified_type = std::remove_cv_t<typename V::value_type>;

        int bind(sqlite3_stmt* stmt, int index, const V& value) const {
            if(value) {
                return statement_binder<unqualified_type>().bind(stmt, index, *value);
            } else {
                return statement_binder<std::nullopt_t>().bind(stmt, index, std::nullopt);
            }
        }
    };
#endif  //  SQLITE_ORM_OPTIONAL_SUPPORTED

    namespace internal {

        struct conditional_binder {
            sqlite3_stmt* stmt = nullptr;
            int index = 1;

            explicit conditional_binder(sqlite3_stmt* stmt) : stmt{stmt} {}

            template<class T, satisfies<is_bindable, T> = true>
            void operator()(const T& t) {
                int rc = statement_binder<T>{}.bind(this->stmt, this->index++, t);
                if(SQLITE_OK != rc) {
                    throw_translated_sqlite_error(stmt);
                }
            }

            template<class T, satisfies_not<is_bindable, T> = true>
            void operator()(const T&) const {}
        };

        struct field_value_binder : conditional_binder {
            using conditional_binder::conditional_binder;
            using conditional_binder::operator();

            template<class T, satisfies_not<is_bindable, T> = true>
            void operator()(const T&) const = delete;

            template<class T>
            void operator()(const T* value) {
                if(!value) {
                    throw std::system_error{orm_error_code::value_is_null};
                }
                (*this)(*value);
            }
        };

        template<class T>
        struct bindable_filter;

        template<class... Args>
        struct bindable_filter<std::tuple<Args...>> : tuple_filter<std::tuple<Args...>, is_bindable> {};
    }
}
#pragma once

#include <sqlite3.h>
#include <type_traits>  //  std::enable_if_t, std::is_arithmetic, std::is_same, std::enable_if
#include <stdlib.h>  //  atof, atoi, atoll
#include <system_error>  //  std::system_error
#include <string>  //  std::string, std::wstring
#ifndef SQLITE_ORM_OMITS_CODECVT
#include <codecvt>  //  std::wstring_convert, std::codecvt_utf8_utf16
#endif  //  SQLITE_ORM_OMITS_CODECVT
#include <vector>  //  std::vector
#include <cstring>  //  strlen
#include <locale>
#include <algorithm>  //  std::copy
#include <iterator>  //  std::back_inserter
#include <tuple>  //  std::tuple, std::tuple_size, std::tuple_element

// #include "functional/cxx_universal.h"

// #include "arithmetic_tag.h"

// #include "pointer_value.h"

// #include "journal_mode.h"

#include <iterator>  //  std::back_inserter
#include <string>  //  std::string
#include <memory>  //  std::unique_ptr
#include <array>  //  std::array
#include <algorithm>  //  std::transform
#include <cctype>  // std::toupper

#if defined(_WINNT_)
// DELETE is a macro defined in the Windows SDK (winnt.h)
#pragma push_macro("DELETE")
#undef DELETE
#endif

namespace sqlite_orm {

    /**
     *  Caps case because of:
     *  1) delete keyword;
     *  2) https://www.sqlite.org/pragma.html#pragma_journal_mode original spelling
     */
    enum class journal_mode : signed char {
        DELETE = 0,
        // An alternate enumeration value when using the Windows SDK that defines DELETE as a macro.
        DELETE_ = DELETE,
        TRUNCATE = 1,
        PERSIST = 2,
        MEMORY = 3,
        WAL = 4,
        OFF = 5,
    };

    namespace internal {

        inline const std::string& to_string(journal_mode j) {
            static std::string res[] = {
                "DELETE",
                "TRUNCATE",
                "PERSIST",
                "MEMORY",
                "WAL",
                "OFF",
            };
            return res[static_cast<int>(j)];
        }

        inline std::unique_ptr<journal_mode> journal_mode_from_string(const std::string& str) {
            std::string upper_str;
            std::transform(str.begin(), str.end(), std::back_inserter(upper_str), [](char c) {
                return static_cast<char>(std::toupper(static_cast<int>(c)));
            });
            static std::array<journal_mode, 6> all = {{
                journal_mode::DELETE,
                journal_mode::TRUNCATE,
                journal_mode::PERSIST,
                journal_mode::MEMORY,
                journal_mode::WAL,
                journal_mode::OFF,
            }};
            for(auto j: all) {
                if(to_string(j) == upper_str) {
                    return std::make_unique<journal_mode>(j);
                }
            }
            return {};
        }
    }
}

#if defined(_WINNT_)
#pragma pop_macro("DELETE")
#endif

// #include "error_code.h"

// #include "is_std_ptr.h"

namespace sqlite_orm {

    /**
     *  Helper class used to cast values from argv to V class
     *  which depends from column type.
     *
     */
    template<class V, typename Enable = void>
    struct row_extractor {
        //  used in sqlite3_exec (select)
        V extract(const char* row_value) const = delete;

        //  used in sqlite_column (iteration, get_all)
        V extract(sqlite3_stmt* stmt, int columnIndex) const = delete;

        //  used in user defined functions
        V extract(sqlite3_value* value) const = delete;
    };

    template<class R>
    int extract_single_value(void* data, int argc, char** argv, char**) {
        auto& res = *(R*)data;
        if(argc) {
            res = row_extractor<R>{}.extract(argv[0]);
        }
        return 0;
    }

    /**
     *  Specialization for the 'pointer-passing interface'.
     * 
     *  @note The 'pointer-passing' interface doesn't support (and in fact prohibits)
     *  extracting pointers from columns.
     */
    template<class P, class T>
    struct row_extractor<pointer_arg<P, T>, void> {
        using V = pointer_arg<P, T>;

        V extract(sqlite3_value* value) const {
            return {(P*)sqlite3_value_pointer(value, T::value)};
        }
    };

    /**
     * Undefine using pointer_binding<> for querying values
     */
    template<class P, class T, class D>
    struct row_extractor<pointer_binding<P, T, D>, void>;

    /**
     *  Specialization for arithmetic types.
     */
    template<class V>
    struct row_extractor<V, std::enable_if_t<std::is_arithmetic<V>::value>> {
        V extract(const char* row_value) const {
            return this->extract(row_value, tag());
        }

        V extract(sqlite3_stmt* stmt, int columnIndex) const {
            return this->extract(stmt, columnIndex, tag());
        }

        V extract(sqlite3_value* value) const {
            return this->extract(value, tag());
        }

      private:
        using tag = arithmetic_tag_t<V>;

        V extract(const char* row_value, const int_or_smaller_tag&) const {
            return static_cast<V>(atoi(row_value));
        }

        V extract(sqlite3_stmt* stmt, int columnIndex, const int_or_smaller_tag&) const {
            return static_cast<V>(sqlite3_column_int(stmt, columnIndex));
        }

        V extract(sqlite3_value* value, const int_or_smaller_tag&) const {
            return static_cast<V>(sqlite3_value_int(value));
        }

        V extract(const char* row_value, const bigint_tag&) const {
            return static_cast<V>(atoll(row_value));
        }

        V extract(sqlite3_stmt* stmt, int columnIndex, const bigint_tag&) const {
            return static_cast<V>(sqlite3_column_int64(stmt, columnIndex));
        }

        V extract(sqlite3_value* value, const bigint_tag&) const {
            return static_cast<V>(sqlite3_value_int64(value));
        }

        V extract(const char* row_value, const real_tag&) const {
            return static_cast<V>(atof(row_value));
        }

        V extract(sqlite3_stmt* stmt, int columnIndex, const real_tag&) const {
            return static_cast<V>(sqlite3_column_double(stmt, columnIndex));
        }

        V extract(sqlite3_value* value, const real_tag&) const {
            return static_cast<V>(sqlite3_value_double(value));
        }
    };

    /**
     *  Specialization for std::string.
     */
    template<>
    struct row_extractor<std::string, void> {
        std::string extract(const char* row_value) const {
            if(row_value) {
                return row_value;
            } else {
                return {};
            }
        }

        std::string extract(sqlite3_stmt* stmt, int columnIndex) const {
            if(auto cStr = (const char*)sqlite3_column_text(stmt, columnIndex)) {
                return cStr;
            } else {
                return {};
            }
        }

        std::string extract(sqlite3_value* value) const {
            if(auto cStr = (const char*)sqlite3_value_text(value)) {
                return cStr;
            } else {
                return {};
            }
        }
    };
#ifndef SQLITE_ORM_OMITS_CODECVT
    /**
     *  Specialization for std::wstring.
     */
    template<>
    struct row_extractor<std::wstring, void> {
        std::wstring extract(const char* row_value) const {
            if(row_value) {
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                return converter.from_bytes(row_value);
            } else {
                return {};
            }
        }

        std::wstring extract(sqlite3_stmt* stmt, int columnIndex) const {
            auto cStr = (const char*)sqlite3_column_text(stmt, columnIndex);
            if(cStr) {
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                return converter.from_bytes(cStr);
            } else {
                return {};
            }
        }

        std::wstring extract(sqlite3_value* value) const {
            if(auto cStr = (const wchar_t*)sqlite3_value_text16(value)) {
                return cStr;
            } else {
                return {};
            }
        }
    };
#endif  //  SQLITE_ORM_OMITS_CODECVT

    template<class V>
    struct row_extractor<V, std::enable_if_t<is_std_ptr<V>::value>> {
        using unqualified_type = std::remove_cv_t<typename V::element_type>;

        V extract(const char* row_value) const {
            if(row_value) {
                return is_std_ptr<V>::make(row_extractor<unqualified_type>().extract(row_value));
            } else {
                return {};
            }
        }

        V extract(sqlite3_stmt* stmt, int columnIndex) const {
            auto type = sqlite3_column_type(stmt, columnIndex);
            if(type != SQLITE_NULL) {
                return is_std_ptr<V>::make(row_extractor<unqualified_type>().extract(stmt, columnIndex));
            } else {
                return {};
            }
        }

        V extract(sqlite3_value* value) const {
            auto type = sqlite3_value_type(value);
            if(type != SQLITE_NULL) {
                return is_std_ptr<V>::make(row_extractor<unqualified_type>().extract(value));
            } else {
                return {};
            }
        }
    };

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
    template<class V>
    struct row_extractor<V, std::enable_if_t<polyfill::is_specialization_of_v<V, std::optional>>> {
        using unqualified_type = std::remove_cv_t<typename V::value_type>;

        V extract(const char* row_value) const {
            if(row_value) {
                return std::make_optional(row_extractor<unqualified_type>().extract(row_value));
            } else {
                return std::nullopt;
            }
        }

        V extract(sqlite3_stmt* stmt, int columnIndex) const {
            auto type = sqlite3_column_type(stmt, columnIndex);
            if(type != SQLITE_NULL) {
                return std::make_optional(row_extractor<unqualified_type>().extract(stmt, columnIndex));
            } else {
                return std::nullopt;
            }
        }

        V extract(sqlite3_value* value) const {
            auto type = sqlite3_value_type(value);
            if(type != SQLITE_NULL) {
                return std::make_optional(row_extractor<unqualified_type>().extract(value));
            } else {
                return std::nullopt;
            }
        }
    };
#endif  //  SQLITE_ORM_OPTIONAL_SUPPORTED

    template<>
    struct row_extractor<nullptr_t> {
        nullptr_t extract(const char* /*row_value*/) const {
            return nullptr;
        }

        nullptr_t extract(sqlite3_stmt* /*stmt*/, int /*columnIndex*/) const {
            return nullptr;
        }

        nullptr_t extract(sqlite3_value* /*value*/) const {
            return nullptr;
        }
    };
    /**
     *  Specialization for std::vector<char>.
     */
    template<>
    struct row_extractor<std::vector<char>> {
        std::vector<char> extract(const char* row_value) const {
            if(row_value) {
                auto len = ::strlen(row_value);
                return this->go(row_value, len);
            } else {
                return {};
            }
        }

        std::vector<char> extract(sqlite3_stmt* stmt, int columnIndex) const {
            auto bytes = static_cast<const char*>(sqlite3_column_blob(stmt, columnIndex));
            auto len = static_cast<size_t>(sqlite3_column_bytes(stmt, columnIndex));
            return this->go(bytes, len);
        }

        std::vector<char> extract(sqlite3_value* value) const {
            auto bytes = static_cast<const char*>(sqlite3_value_blob(value));
            auto len = static_cast<size_t>(sqlite3_value_bytes(value));
            return this->go(bytes, len);
        }

      protected:
        std::vector<char> go(const char* bytes, size_t len) const {
            if(len) {
                std::vector<char> res;
                res.reserve(len);
                std::copy(bytes, bytes + len, std::back_inserter(res));
                return res;
            } else {
                return {};
            }
        }
    };

    template<class... Args>
    struct row_extractor<std::tuple<Args...>> {

        std::tuple<Args...> extract(char** argv) const {
            std::tuple<Args...> res;
            this->extract<std::tuple_size<decltype(res)>::value>(res, argv);
            return res;
        }

        std::tuple<Args...> extract(sqlite3_stmt* stmt, int /*columnIndex*/) const {
            std::tuple<Args...> res;
            this->extract<std::tuple_size<decltype(res)>::value>(res, stmt);
            return res;
        }

      protected:
        template<size_t I, std::enable_if_t<I != 0, bool> = true>
        void extract(std::tuple<Args...>& t, sqlite3_stmt* stmt) const {
            using tuple_type = std::tuple_element_t<I - 1, std::tuple<Args...>>;
            std::get<I - 1>(t) = row_extractor<tuple_type>().extract(stmt, I - 1);
            this->extract<I - 1>(t, stmt);
        }

        template<size_t I, std::enable_if_t<I == 0, bool> = true>
        void extract(std::tuple<Args...>&, sqlite3_stmt*) const {
            //..
        }

        template<size_t I, std::enable_if_t<I != 0, bool> = true>
        void extract(std::tuple<Args...>& t, char** argv) const {
            using tuple_type = std::tuple_element_t<I - 1, std::tuple<Args...>>;
            std::get<I - 1>(t) = row_extractor<tuple_type>().extract(argv[I - 1]);
            this->extract<I - 1>(t, argv);
        }

        template<size_t I, std::enable_if_t<I == 0, bool> = true>
        void extract(std::tuple<Args...>&, char**) const {
            //..
        }
    };

    /**
     *  Specialization for journal_mode.
     */
    template<>
    struct row_extractor<journal_mode, void> {
        journal_mode extract(const char* row_value) const {
            if(row_value) {
                if(auto res = internal::journal_mode_from_string(row_value)) {
                    return std::move(*res);
                } else {
                    throw std::system_error{orm_error_code::incorrect_journal_mode_string};
                }
            } else {
                throw std::system_error{orm_error_code::incorrect_journal_mode_string};
            }
        }

        journal_mode extract(sqlite3_stmt* stmt, int columnIndex) const {
            auto cStr = (const char*)sqlite3_column_text(stmt, columnIndex);
            return this->extract(cStr);
        }
    };
}
#pragma once

#include <sqlite3.h>
#include <string>  //  std::string
#include <utility>  //  std::move

// #include "error_code.h"

namespace sqlite_orm {

    /** 
     *  Escape the provided character in the given string by doubling it.
     *  @param str A copy of the original string
     *  @param char2Escape The character to escape
     */
    inline std::string sql_escape(std::string str, char char2Escape) {
        for(size_t pos = 0; (pos = str.find(char2Escape, pos)) != str.npos; pos += 2) {
            str.replace(pos, 1, 2, char2Escape);
        }

        return str;
    }

    /** 
     *  Quote the given string value using single quotes,
     *  escape containing single quotes by doubling them.
     */
    inline std::string quote_string_literal(std::string v) {
        constexpr char quoteChar = '\'';
        return quoteChar + sql_escape(move(v), quoteChar) + quoteChar;
    }

    /** 
     *  Quote the given string value using single quotes,
     *  escape containing single quotes by doubling them.
     */
    inline std::string quote_blob_literal(std::string v) {
        constexpr char quoteChar = '\'';
        return std::string{char('x'), quoteChar} + move(v) + quoteChar;
    }

    /** 
     *  Quote the given identifier using double quotes,
     *  escape containing double quotes by doubling them.
     */
    inline std::string quote_identifier(std::string identifier) {
        constexpr char quoteChar = '"';
        return quoteChar + sql_escape(move(identifier), quoteChar) + quoteChar;
    }

    namespace internal {
        // Wrapper to reduce boiler-plate code
        inline sqlite3_stmt* reset(sqlite3_stmt* stmt) {
            sqlite3_reset(stmt);
            return stmt;
        }

        inline void perform_void_exec(sqlite3* db, const std::string& query) {
            int rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);
            if(rc != SQLITE_OK) {
                throw_translated_sqlite_error(db);
            }
        }

        inline void perform_exec(sqlite3* db,
                                 const char* query,
                                 int (*callback)(void* data, int argc, char** argv, char**),
                                 void* user_data) {
            int rc = sqlite3_exec(db, query, callback, user_data, nullptr);
            if(rc != SQLITE_OK) {
                throw_translated_sqlite_error(db);
            }
        }

        inline void perform_exec(sqlite3* db,
                                 const std::string& query,
                                 int (*callback)(void* data, int argc, char** argv, char**),
                                 void* user_data) {
            return perform_exec(db, query.c_str(), callback, user_data);
        }

        inline void perform_step(sqlite3_stmt* stmt) {
            int rc = sqlite3_step(stmt);
            if(rc != SQLITE_DONE) {
                throw_translated_sqlite_error(stmt);
            }
        }

        template<class L>
        void perform_step(sqlite3_stmt* stmt, L&& lambda) {
            switch(int rc = sqlite3_step(stmt)) {
                case SQLITE_ROW: {
                    lambda(stmt);
                } break;
                case SQLITE_DONE:
                    break;
                default: {
                    throw_translated_sqlite_error(stmt);
                }
            }
        }

        template<class L>
        void perform_steps(sqlite3_stmt* stmt, L&& lambda) {
            int rc;
            do {
                switch(rc = sqlite3_step(stmt)) {
                    case SQLITE_ROW: {
                        lambda(stmt);
                    } break;
                    case SQLITE_DONE:
                        break;
                    default: {
                        throw_translated_sqlite_error(stmt);
                    }
                }
            } while(rc != SQLITE_DONE);
        }
    }
}
#pragma once

#include <ostream>

namespace sqlite_orm {

    enum class sync_schema_result {

        /**
         *  created new table, table with the same tablename did not exist
         */
        new_table_created,

        /**
         *  table schema is the same as storage, nothing to be done
         */
        already_in_sync,

        /**
         *  removed excess columns in table (than storage) without dropping a table
         */
        old_columns_removed,

        /**
         *  lacking columns in table (than storage) added without dropping a table
         */
        new_columns_added,

        /**
         *  both old_columns_removed and new_columns_added
         */
        new_columns_added_and_old_columns_removed,

        /**
         *  old table is dropped and new is recreated. Reasons :
         *      1. delete excess columns in the table than storage if preseve = false
         *      2. Lacking columns in the table cannot be added due to NULL and DEFAULT constraint
         *      3. Reasons 1 and 2 both together
         *      4. data_type mismatch between table and storage.
         */
        dropped_and_recreated,
    };

    inline std::ostream& operator<<(std::ostream& os, sync_schema_result value) {
        switch(value) {
            case sync_schema_result::new_table_created:
                return os << "new table created";
            case sync_schema_result::already_in_sync:
                return os << "table and storage is already in sync.";
            case sync_schema_result::old_columns_removed:
                return os << "old excess columns removed";
            case sync_schema_result::new_columns_added:
                return os << "new columns added";
            case sync_schema_result::new_columns_added_and_old_columns_removed:
                return os << "old excess columns removed and new columns added";
            case sync_schema_result::dropped_and_recreated:
                return os << "old table dropped and recreated";
        }
        return os;
    }
}
#pragma once

#include <tuple>  //  std::tuple, std::make_tuple
#include <string>  //  std::string
#include <utility>  //  std::forward

// #include "functional/cxx_universal.h"

// #include "tuple_helper/tuple_filter.h"

// #include "indexed_column.h"

#include <string>  //  std::string
#include <utility>  //  std::move

// #include "functional/cxx_universal.h"

// #include "ast/where.h"

namespace sqlite_orm {

    namespace internal {

        template<class C>
        struct indexed_column_t {
            using column_type = C;

#ifndef SQLITE_ORM_AGGREGATE_NSDMI_SUPPORTED
            indexed_column_t(column_type _column_or_expression) :
                column_or_expression(std::move(_column_or_expression)) {}
#endif

            column_type column_or_expression;
            std::string _collation_name;
            int _order = 0;  //  -1 = desc, 1 = asc, 0 = not specified

            indexed_column_t<column_type> collate(std::string name) {
                auto res = std::move(*this);
                res._collation_name = move(name);
                return res;
            }

            indexed_column_t<column_type> asc() {
                auto res = std::move(*this);
                res._order = 1;
                return res;
            }

            indexed_column_t<column_type> desc() {
                auto res = std::move(*this);
                res._order = -1;
                return res;
            }
        };

        template<class C>
        struct indexed_column_maker {
            using type = indexed_column_t<C>;

            indexed_column_t<C> operator()(C col) const {
                return {std::move(col)};
            }
        };

        template<class C>
        struct indexed_column_maker<where_t<C>> {
            using type = where_t<C>;

            type operator()(type wher) const {
                return {std::move(wher)};
            }
        };

        template<class C>
        struct indexed_column_maker<indexed_column_t<C>> {
            using type = indexed_column_t<C>;

            indexed_column_t<C> operator()(indexed_column_t<C> col) const {
                return std::move(col);
            }
        };

        template<class C>
        auto make_indexed_column(C col) {
            indexed_column_maker<C> maker;
            return maker(std::move(col));
        }

    }

    /**
     * Use this function to specify indexed column inside `make_index` function call.
     * Example: make_index("index_name", indexed_column(&User::id).asc())
     */
    template<class C>
    internal::indexed_column_t<C> indexed_column(C column_or_expression) {
        return {std::move(column_or_expression)};
    }

}

namespace sqlite_orm {

    namespace internal {

        struct index_base {
            std::string name;
            bool unique = false;

#ifndef SQLITE_ORM_AGGREGATE_NSDMI_SUPPORTED
            index_base(std::string name, bool unique) : name{move(name)}, unique{unique} {}
#endif
        };

        template<class... Els>
        struct index_t : index_base {
            using elements_type = std::tuple<Els...>;
            using object_type = void;

#ifndef SQLITE_ORM_AGGREGATE_BASES_SUPPORTED
            index_t(std::string name_, bool unique_, elements_type elements_) :
                index_base{move(name_), unique_}, elements(move(elements_)) {}
#endif

            elements_type elements;
        };
    }

    template<class... Cols>
    internal::index_t<typename internal::indexed_column_maker<Cols>::type...> make_index(const std::string& name,
                                                                                         Cols... cols) {
        using cols_tuple = std::tuple<Cols...>;
        static_assert(internal::count_tuple<cols_tuple, internal::is_where>::value <= 1,
                      "amount of where arguments can be 0 or 1");
        SQLITE_ORM_CLANG_SUPPRESS_MISSING_BRACES(
            return {name, false, std::make_tuple(internal::make_indexed_column(std::move(cols))...)});
    }

    template<class... Cols>
    internal::index_t<typename internal::indexed_column_maker<Cols>::type...> make_unique_index(const std::string& name,
                                                                                                Cols... cols) {
        using cols_tuple = std::tuple<Cols...>;
        static_assert(internal::count_tuple<cols_tuple, internal::is_where>::value <= 1,
                      "amount of where arguments can be 0 or 1");
        SQLITE_ORM_CLANG_SUPPRESS_MISSING_BRACES(
            return {name, true, std::make_tuple(internal::make_indexed_column(std::move(cols))...)});
    }
}
#pragma once

#include <type_traits>  //  std::enable_if, std::is_base_of, std::remove_const

// #include "alias.h"

namespace sqlite_orm {

    namespace internal {

        /**
         *  If T is alias than mapped_type_proxy<T>::type is alias::type
         *  otherwise T is unqualified T.
         */
        template<class T, class SFINAE = void>
        struct mapped_type_proxy : std::remove_const<T> {};

        template<class T>
        struct mapped_type_proxy<T, std::enable_if_t<std::is_base_of<alias_tag, T>::value>> {
            using type = typename T::type;
        };

        template<class T>
        using mapped_type_proxy_t = typename mapped_type_proxy<T>::type;
    }
}
#pragma once

#include <string>  //  std::string

namespace sqlite_orm {

    namespace internal {

        struct rowid_t {
            operator std::string() const {
                return "rowid";
            }
        };

        struct oid_t {
            operator std::string() const {
                return "oid";
            }
        };

        struct _rowid_t {
            operator std::string() const {
                return "_rowid_";
            }
        };

        template<class T>
        struct table_rowid_t : public rowid_t {
            using type = T;
        };

        template<class T>
        struct table_oid_t : public oid_t {
            using type = T;
        };
        template<class T>
        struct table__rowid_t : public _rowid_t {
            using type = T;
        };

    }

    inline internal::rowid_t rowid() {
        return {};
    }

    inline internal::oid_t oid() {
        return {};
    }

    inline internal::_rowid_t _rowid_() {
        return {};
    }

    template<class T>
    internal::table_rowid_t<T> rowid() {
        return {};
    }

    template<class T>
    internal::table_oid_t<T> oid() {
        return {};
    }

    template<class T>
    internal::table__rowid_t<T> _rowid_() {
        return {};
    }
}
#pragma once

#include <type_traits>  //  std::enable_if, std::is_same, std::decay, std::is_arithmetic, std::is_base_of
#include <tuple>  //  std::tuple
#include <functional>  //  std::reference_wrapper

// #include "functional/cxx_universal.h"

// #include "type_traits.h"

// #include "member_traits/member_traits.h"

// #include "core_functions.h"

// #include "select_constraints.h"

// #include "operators.h"

// #include "rowid.h"

// #include "alias.h"

// #include "column.h"

// #include "storage_traits.h"
#include <type_traits>  //  std::is_same, std::enable_if, std::true_type, std::false_type, std::integral_constant
#include <tuple>  //  std::tuple

// #include "type_traits.h"

// #include "storage_lookup.h"

#include <type_traits>

// #include "functional/cxx_polyfill.h"

// #include "type_traits.h"

namespace sqlite_orm {
    namespace internal {

        template<class... Ts>
        struct storage_t;

        template<class... Ts>
        struct storage_impl;

        template<class T>
        struct is_storage : std::false_type {};

        template<class... Ts>
        struct is_storage<storage_t<Ts...>> : std::true_type {};
        template<class... Ts>
        struct is_storage<const storage_t<Ts...>> : std::true_type {};

        template<class T>
        struct is_storage_impl : std::false_type {};

        template<class... Ts>
        struct is_storage_impl<storage_impl<Ts...>> : std::true_type {};
        template<class... Ts>
        struct is_storage_impl<const storage_impl<Ts...>> : std::true_type {};

        /**
         *  std::true_type if given 'table' type matches, std::false_type otherwise.
         *  
         *  A 'table' type is one of: table_t<>, index_t<>
         */
        template<typename S, typename T>
        using table_type_matches = std::is_same<T, table_type_t<S>>;

        /**
         *  std::true_type if given object is mapped, std::false_type otherwise.
         * 
         *  Note: unlike table_t<>, index_t<>::object_type is always void.
         */
        template<typename S, typename O>
        using object_type_matches =
            typename polyfill::conjunction<polyfill::negation<std::is_void<storage_object_type_t<S>>>,
                                           std::is_same<O, storage_object_type_t<S>>>::type;

        /**
         *  std::true_type if given lookup type ('table' type, object) is mapped, std::false_type otherwise.
         * 
         *  Note: we allow lookup via S::table_type because it allows us to walk the storage_impl chain (in storage_impl_cat()).
         */
        template<typename S, typename Lookup>
        using lookup_type_matches =
            typename polyfill::disjunction<table_type_matches<S, Lookup>, object_type_matches<S, Lookup>>::type;
    }

    // pick/lookup metafunctions
    namespace internal {

        /**
         *  S - storage_impl type
         *  O - mapped data type
         */
        template<class S, class O, class SFINAE = void>
        struct storage_pick_impl_type;

        template<class S, class O>
        struct storage_pick_impl_type<S, O, match_if<lookup_type_matches, S, O>> {
            using type = S;
        };

        template<class S, class O>
        struct storage_pick_impl_type<S, O, match_if_not<lookup_type_matches, S, O>>
            : storage_pick_impl_type<typename S::super, O> {};

        /**
         *  Final specialisation
         */
        template<class O>
        struct storage_pick_impl_type<storage_impl<>, O, void> {};

        /**
         *  storage_t specialization
         */
        template<class... Ts, class O>
        struct storage_pick_impl_type<storage_t<Ts...>, O, void>
            : storage_pick_impl_type<typename storage_t<Ts...>::impl_type, O> {};

        /**
         *  S - storage_impl type
         *  O - mapped data type
         */
        template<class S, class O, class SFINAE = void>
        struct storage_find_impl_type;

        template<class S, class O>
        struct storage_find_impl_type<S, O, match_if<lookup_type_matches, S, O>> {
            using type = S;
        };

        template<class S, class O>
        struct storage_find_impl_type<S, O, match_if_not<lookup_type_matches, S, O>>
            : storage_find_impl_type<typename S::super, O> {};

        /**
         *  Final specialisation
         */
        template<class O>
        struct storage_find_impl_type<storage_impl<>, O, void> {
            using type = storage_impl<>;
        };

        /**
         *  storage_t specialization
         */
        template<class... Ts, class O>
        struct storage_find_impl_type<storage_t<Ts...>, O, void>
            : storage_find_impl_type<typename storage_t<Ts...>::impl_type, O> {};
    }

    namespace internal {

        template<typename T, typename U>
        using reapply_const_of_t = std::conditional_t<std::is_const<T>::value, std::add_const_t<U>, U>;

        /**
         *  S - storage_t or storage_impl type, possibly const-qualified
         *  Lookup - 'table' type, mapped data type
         */
        template<class S, class Lookup>
        using storage_pick_impl_t =
            reapply_const_of_t<S, type_t<storage_pick_impl_type<std::remove_const_t<S>, Lookup>>>;

        /**
         *  S - storage_t or storage_impl type, possibly const-qualified
         *  Lookup - 'table' type, mapped data type
         */
        template<class S, class Lookup>
        using storage_find_impl_t =
            reapply_const_of_t<S, type_t<storage_find_impl_type<std::remove_const_t<S>, Lookup>>>;
    }
}

// runtime lookup functions
namespace sqlite_orm {
    namespace internal {
        /**
         *  Given a storage implementation pack, pick the specific storage implementation for the given lookup type.
         * 
         *  Note: This function requires Lookup to be mapped, otherwise it is removed from the overload resolution set.
         */
        template<class Lookup, class S, satisfies<is_storage_impl, S> = true>
        storage_pick_impl_t<S, Lookup>& pick_impl(S& impl) {
            return impl;
        }

        /**
         *  Given a storage implementation pack, find the specific storage implementation for the given lookup type.
         * 
         *  Note: This function returns the empty `storage_impl<>` if Lookup isn't mapped.
         */
        template<class Lookup, class S, satisfies<is_storage_impl, S> = true>
        storage_find_impl_t<S, Lookup>& find_impl(S& impl) {
            return impl;
        }

        /**
         *  Given a storage, pick the specific const-qualified storage implementation for the lookup type.
         * 
         *  Note: This function requires Lookup to be mapped, otherwise it is removed from the overload resolution set.
         */
        template<class Lookup, class S, satisfies<is_storage, S> = true>
        storage_pick_impl_t<const S, Lookup>& pick_const_impl(S& storage) {
            return obtain_const_impl(storage);
        }
    }
}

// #include "tuple_helper/tuple_filter.h"

// #include "tuple_helper/tuple_transformer.h"

#include <tuple>  //  std::tuple

namespace sqlite_orm {
    namespace internal {

        template<class T, template<class...> class Fn>
        struct tuple_transformer;

        template<class... Args, template<class...> class Fn>
        struct tuple_transformer<std::tuple<Args...>, Fn> {
            using type = std::tuple<typename Fn<Args>::type...>;
        };

        template<class T, template<class...> class Fn>
        using transform_tuple_t = typename tuple_transformer<T, Fn>::type;

    }
}

namespace sqlite_orm {

    namespace internal {

        template<class... Ts>
        struct storage_impl;

        template<class T, bool WithoutRowId, class... Cs>
        struct table_t;

        template<class A, class B>
        struct foreign_key_t;

        namespace storage_traits {

            template<class S, class O, class SFINAE = void>
            SQLITE_ORM_INLINE_VAR constexpr bool is_mapped_v = false;
            template<class S, class O>
            SQLITE_ORM_INLINE_VAR constexpr bool is_mapped_v<S, O, polyfill::void_t<storage_pick_impl_t<S, O>>> = true;

            template<class S, class O>
            using is_mapped = polyfill::bool_constant<is_mapped_v<S, O>>;

            template<class S>
            struct storage_columns_count_impl : std::integral_constant<int, S::table_type::elements_count> {};

            template<>
            struct storage_columns_count_impl<storage_impl<>> : std::integral_constant<int, 0> {};

            /**
             *  S - storage
             *  O - mapped or not mapped data type
             */
            template<class S, class O>
            struct storage_columns_count : storage_columns_count_impl<storage_find_impl_t<S, O>> {};

            /**
             *  T - table type.
             */
            template<class T>
            struct table_types;

            /**
             *  type is std::tuple of field types of mapped colums.
             */
            template<class T, bool WithoutRowId, class... Args>
            struct table_types<table_t<T, WithoutRowId, Args...>> {
                using args_tuple = std::tuple<Args...>;
                using columns_tuple = typename tuple_filter<args_tuple, is_column>::type;

                using type = transform_tuple_t<columns_tuple, column_field_type>;
            };

            /**
             *  S - storage_impl type
             *  T - mapped or not mapped data type
             */
            template<class S, class T, class SFINAE = void>
            struct storage_mapped_columns_impl;

            /**
             *  S - storage
             *  T - mapped or not mapped data type
             */
            template<class S, class T>
            struct storage_mapped_columns : storage_mapped_columns_impl<typename S::impl_type, T> {};

            /**
             *  Final specialisation
             */
            template<class T>
            struct storage_mapped_columns_impl<storage_impl<>, T, void> {
                using type = std::tuple<>;
            };

            template<class S, class T>
            struct storage_mapped_columns_impl<S, T, match_if<std::is_same, T, storage_object_type_t<S>>>
                : table_types<table_type_t<S>> {};

            template<class S, class T>
            struct storage_mapped_columns_impl<S, T, match_if_not<std::is_same, T, storage_object_type_t<S>>>
                : storage_mapped_columns_impl<typename S::super, T> {};

            /**
             *  C is any column type: column_t or constraint type
             *  O - object type references in FOREIGN KEY
             */
            template<class C, class O>
            struct column_foreign_keys_count : std::integral_constant<int, 0> {};

            template<class A, class B, class O>
            struct column_foreign_keys_count<foreign_key_t<A, B>, O> {
                using target_type = typename foreign_key_t<A, B>::target_type;

                static constexpr int value = std::is_same<O, target_type>::value ? 1 : 0;
            };

            /**
             * O - object type references in FOREIGN KEY
             * Cs - column types which are stored in table_t::columns_type
             */
            template<class O, class... Cs>
            struct table_foreign_keys_count_impl;

            template<class O>
            struct table_foreign_keys_count_impl<O> {
                static constexpr int value = 0;
            };

            template<class O, class H, class... Tail>
            struct table_foreign_keys_count_impl<O, H, Tail...> {
                static constexpr int value =
                    column_foreign_keys_count<H, O>::value + table_foreign_keys_count_impl<O, Tail...>::value;
            };

            /**
             *  T is table_t type
             *  O is object type which is the reference target (e.g. foreign_key(&Visit::userId).references(&User::id) has O = User)
             */
            template<class T, class O>
            struct table_foreign_keys_count;

            template<class T, class... Cs, class O>
            struct table_foreign_keys_count<table_t<T, false, Cs...>, O> {
                using table_type = table_t<T, false, Cs...>;

                static constexpr int value = table_foreign_keys_count_impl<O, Cs...>::value;
            };

            /**
             *  S - storage class
             *  O - type mapped to S
             */
            template<class S, class O>
            struct storage_foreign_keys_count_impl;

            template<class O>
            struct storage_foreign_keys_count_impl<storage_impl<>, O> : std::integral_constant<int, 0> {};

            template<class H, class... Ts, class O>
            struct storage_foreign_keys_count_impl<storage_impl<H, Ts...>, O> {
                static constexpr int value = table_foreign_keys_count<H, O>::value +
                                             storage_foreign_keys_count_impl<storage_impl<Ts...>, O>::value;
            };

            /**
             * S - storage class
             * O - type mapped to S
             * This class tells how many types mapped to S have foreign keys to O
             */
            template<class S, class O>
            struct storage_foreign_keys_count {
                using impl_type = typename S::impl_type;

                static constexpr int value = storage_foreign_keys_count_impl<impl_type, O>::value;
            };

            /**
             * C is any table element type: column_t or constraint type
             * O - object type references in FOREIGN KEY
             */
            template<class C, class O, class SFINAE = void>
            struct column_fk_references {
                using type = std::tuple<>;
            };

            template<class C, class O, class SFINAE = void>
            struct column_foreign_keys {
                using type = std::tuple<>;
            };

            template<class A, class B, class O>
            struct column_foreign_keys<foreign_key_t<A, B>,
                                       O,
                                       match_if<std::is_same, O, typename foreign_key_t<A, B>::target_type>> {
                using type = std::tuple<foreign_key_t<A, B>>;
            };

            template<class A, class B, class O>
            struct column_foreign_keys<foreign_key_t<A, B>,
                                       O,
                                       match_if_not<std::is_same, O, typename foreign_key_t<A, B>::target_type>> {
                using type = std::tuple<>;
            };

            template<class A, class B, class O>
            struct column_fk_references<foreign_key_t<A, B>,
                                        O,
                                        match_if<std::is_same, O, typename foreign_key_t<A, B>::target_type>> {
                using target_type = typename foreign_key_t<A, B>::source_type;

                using type = std::tuple<target_type>;
            };

            template<class A, class B, class O>
            struct column_fk_references<foreign_key_t<A, B>,
                                        O,
                                        match_if_not<std::is_same, O, typename foreign_key_t<A, B>::target_type>> {
                using type = std::tuple<>;
            };

            /**
             * O - object type references in FOREIGN KEY
             * Cs - column types which are stored in table_t::columns_type
             */
            template<class O, class... Cs>
            struct table_fk_references_impl;

            template<class O, class... Cs>
            struct table_foreign_keys_impl;

            template<class O>
            struct table_fk_references_impl<O> {
                using type = std::tuple<>;
            };

            template<class O>
            struct table_foreign_keys_impl<O> {
                using type = std::tuple<>;
            };

            template<class O, class H, class... Tail>
            struct table_fk_references_impl<O, H, Tail...> {
                using head_tuple = typename column_fk_references<H, O>::type;
                using tail_tuple = typename table_fk_references_impl<O, Tail...>::type;
                using type = conc_tuple_t<head_tuple, tail_tuple>;
            };

            template<class O, class H, class... Tail>
            struct table_foreign_keys_impl<O, H, Tail...> {
                using head_tuple = typename column_foreign_keys<H, O>::type;
                using tail_tuple = typename table_foreign_keys_impl<O, Tail...>::type;
                using type = conc_tuple_t<head_tuple, tail_tuple>;
            };

            /**
             *  T is table_t type
             *  O is object type which is the reference target (e.g. foreign_key(&Visit::userId).references(&User::id) has O = User)
             */
            template<class T, class O>
            struct table_fk_references;

            template<class T, class O>
            struct table_foreign_keys;

            template<class T, bool WithoutRowId, class... Cs, class O>
            struct table_fk_references<table_t<T, WithoutRowId, Cs...>, O> {
                using table_type = table_t<T, WithoutRowId, Cs...>;

                using type = typename table_fk_references_impl<O, Cs...>::type;
            };

            template<class T, bool WithoutRowId, class... Cs, class O>
            struct table_foreign_keys<table_t<T, WithoutRowId, Cs...>, O> {
                using table_type = table_t<T, WithoutRowId, Cs...>;

                using type = typename table_foreign_keys_impl<O, Cs...>::type;
            };

            /**
             *  S - storage class
             *  O - type mapped to S
             */
            template<class S, class O>
            struct storage_fk_references_impl;

            template<class S, class O>
            struct storage_foreign_keys_impl;

            template<class O>
            struct storage_fk_references_impl<storage_impl<>, O> {
                using type = std::tuple<>;
            };

            template<class O>
            struct storage_foreign_keys_impl<storage_impl<>, O> {
                using type = std::tuple<>;
            };

            template<class H, class... Ts, class O>
            struct storage_fk_references_impl<storage_impl<H, Ts...>, O> {
                using head_tuple = typename table_fk_references<H, O>::type;
                using tail_tuple = typename storage_fk_references_impl<storage_impl<Ts...>, O>::type;
                using type = conc_tuple_t<head_tuple, tail_tuple>;
            };

            template<class H, class... Ts, class O>
            struct storage_foreign_keys_impl<storage_impl<H, Ts...>, O> {
                using head_tuple = typename table_foreign_keys<H, O>::type;
                using tail_tuple = typename storage_foreign_keys_impl<storage_impl<Ts...>, O>::type;
                using type = conc_tuple_t<head_tuple, tail_tuple>;
            };

            /**
             *  S - storage class
             *  O - type mapped to S
             *  type holds `std::tuple` with types that has references to O as  foreign keys
             */
            template<class S, class O>
            struct storage_fk_references {
                using impl_type = typename S::impl_type;

                using type = typename storage_fk_references_impl<impl_type, O>::type;
            };

            template<class S, class O>
            struct storage_foreign_keys {
                using impl_type = typename S::impl_type;

                using type = typename storage_foreign_keys_impl<impl_type, O>::type;
            };

        }
    }
}

// #include "function.h"

#include <sqlite3.h>
#include <type_traits>
#include <string>  //  std::string
#include <tuple>  //  std::tuple
#include <functional>  //  std::function
#include <algorithm>  //  std::min
#include <utility>  //  std::move, std::forward

// #include "functional/cxx_universal.h"

// #include "functional/cxx_polyfill.h"

namespace sqlite_orm {

    struct arg_values;

    template<class T, class P>
    struct pointer_arg;
    template<class T, class P, class D>
    class pointer_binding;

    namespace internal {

        struct user_defined_function_base {
            using func_call = std::function<
                void(sqlite3_context* context, void* functionPointer, int argsCount, sqlite3_value** values)>;
            using final_call = std::function<void(sqlite3_context* context, void* functionPointer)>;

            std::string name;
            int argumentsCount = 0;
            std::function<int*()> create;
            void (*destroy)(int*) = nullptr;

#ifndef SQLITE_ORM_AGGREGATE_NSDMI_SUPPORTED
            user_defined_function_base(decltype(name) name_,
                                       decltype(argumentsCount) argumentsCount_,
                                       decltype(create) create_,
                                       decltype(destroy) destroy_) :
                name(move(name_)),
                argumentsCount(argumentsCount_), create(move(create_)), destroy(destroy_) {}
#endif
        };

        struct user_defined_scalar_function_t : user_defined_function_base {
            func_call run;

            user_defined_scalar_function_t(decltype(name) name_,
                                           int argumentsCount_,
                                           decltype(create) create_,
                                           decltype(run) run_,
                                           decltype(destroy) destroy_) :
                user_defined_function_base{move(name_), argumentsCount_, move(create_), destroy_},
                run(move(run_)) {}
        };

        struct user_defined_aggregate_function_t : user_defined_function_base {
            func_call step;
            final_call finalCall;

            user_defined_aggregate_function_t(decltype(name) name_,
                                              int argumentsCount_,
                                              decltype(create) create_,
                                              decltype(step) step_,
                                              decltype(finalCall) finalCall_,
                                              decltype(destroy) destroy_) :
                user_defined_function_base{move(name_), argumentsCount_, move(create_), destroy_},
                step(move(step_)), finalCall(move(finalCall_)) {}
        };

        template<class F>
        using scalar_call_function_t = decltype(&F::operator());

        template<class F>
        using aggregate_step_function_t = decltype(&F::step);

        template<class F>
        using aggregate_fin_function_t = decltype(&F::fin);

        template<class F, class = void>
        SQLITE_ORM_INLINE_VAR constexpr bool is_scalar_function_v = false;
        template<class F>
        SQLITE_ORM_INLINE_VAR constexpr bool is_scalar_function_v<F, polyfill::void_t<scalar_call_function_t<F>>> =
            true;

        template<class F, class = void>
        SQLITE_ORM_INLINE_VAR constexpr bool is_aggregate_function_v = false;
        template<class F>
        SQLITE_ORM_INLINE_VAR constexpr bool is_aggregate_function_v<
            F,
            polyfill::void_t<aggregate_step_function_t<F>,
                             aggregate_fin_function_t<F>,
                             std::enable_if_t<std::is_member_function_pointer<aggregate_step_function_t<F>>::value>,
                             std::enable_if_t<std::is_member_function_pointer<aggregate_fin_function_t<F>>::value>>> =
            true;

        template<class T>
        struct member_function_arguments;

        template<class O, class R, class... Args>
        struct member_function_arguments<R (O::*)(Args...) const> {
            using member_function_type = R (O::*)(Args...) const;
            using tuple_type = std::tuple<std::decay_t<Args>...>;
            using return_type = R;
        };

        template<class O, class R, class... Args>
        struct member_function_arguments<R (O::*)(Args...)> {
            using member_function_type = R (O::*)(Args...);
            using tuple_type = std::tuple<std::decay_t<Args>...>;
            using return_type = R;
        };

        template<class F, class SFINAE = void>
        struct callable_arguments_impl;

        template<class F>
        struct callable_arguments_impl<F, std::enable_if_t<is_scalar_function_v<F>>> {
            using args_tuple = typename member_function_arguments<scalar_call_function_t<F>>::tuple_type;
            using return_type = typename member_function_arguments<scalar_call_function_t<F>>::return_type;
        };

        template<class F>
        struct callable_arguments_impl<F, std::enable_if_t<is_aggregate_function_v<F>>> {
            using args_tuple = typename member_function_arguments<aggregate_step_function_t<F>>::tuple_type;
            using return_type = typename member_function_arguments<aggregate_fin_function_t<F>>::return_type;
        };

        template<class F>
        struct callable_arguments : callable_arguments_impl<F> {};

        template<class F, class... Args>
        struct function_call {
            using function_type = F;
            using args_tuple = std::tuple<Args...>;

            args_tuple args;
        };

        template<class T>
        struct unpacked_arg {
            using type = T;
        };
        template<class F, class... Args>
        struct unpacked_arg<function_call<F, Args...>> {
            using type = typename callable_arguments<F>::return_type;
        };
        template<class T>
        using unpacked_arg_t = typename unpacked_arg<T>::type;

        template<size_t I, class FnArg, class CallArg>
        SQLITE_ORM_CONSTEVAL bool expected_pointer_value() {
            static_assert(polyfill::always_false_v<FnArg, CallArg>, "Expected a pointer value for I-th argument");
            return false;
        }

        template<size_t I, class FnArg, class CallArg, class EnableIfTag = void>
        constexpr bool is_same_pvt_v = expected_pointer_value<I, FnArg, CallArg>();

        // Always allow binding nullptr to a pointer argument
        template<size_t I, class PointerArg>
        constexpr bool is_same_pvt_v<I, PointerArg, nullptr_t, polyfill::void_t<typename PointerArg::tag>> = true;

#if __cplusplus >= 201703L  // using C++17 or higher
        template<size_t I, const char* PointerArg, const char* Binding>
        SQLITE_ORM_CONSTEVAL bool assert_same_pointer_type() {
            constexpr bool valid = Binding == PointerArg;
            static_assert(valid, "Pointer value types of I-th argument do not match");
            return valid;
        }

        template<size_t I, class PointerArg, class Binding>
        constexpr bool
            is_same_pvt_v<I, PointerArg, Binding, polyfill::void_t<typename PointerArg::tag, typename Binding::tag>> =
                assert_same_pointer_type<I, PointerArg::tag::value, Binding::tag::value>();
#else
        template<size_t I, class PointerArg, class Binding>
        SQLITE_ORM_CONSTEVAL bool assert_same_pointer_type() {
            constexpr bool valid = Binding::value == PointerArg::value;
            static_assert(valid, "Pointer value types of I-th argument do not match");
            return valid;
        }

        template<size_t I, class PointerArg, class Binding>
        constexpr bool
            is_same_pvt_v<I, PointerArg, Binding, polyfill::void_t<typename PointerArg::tag, typename Binding::tag>> =
                assert_same_pointer_type<I, typename PointerArg::tag, typename Binding::tag>();
#endif

        template<size_t I, class FnArg, class CallArg>
        SQLITE_ORM_CONSTEVAL bool validate_pointer_value_type(std::false_type) {
            return true;
        }

        template<size_t I, class FnArg, class CallArg>
        SQLITE_ORM_CONSTEVAL bool validate_pointer_value_type(std::true_type) {
            return is_same_pvt_v<I, FnArg, CallArg>;
        }

        template<class FnArgs, class CallArgs>
        SQLITE_ORM_CONSTEVAL bool validate_pointer_value_types(polyfill::index_constant<size_t(-1)>) {
            return true;
        }
        template<class FnArgs, class CallArgs, size_t I>
        SQLITE_ORM_CONSTEVAL bool validate_pointer_value_types(polyfill::index_constant<I>) {
            using func_arg_t = std::tuple_element_t<I, FnArgs>;
            using passed_arg_t = unpacked_arg_t<std::tuple_element_t<I, CallArgs>>;

#ifdef SQLITE_ORM_RELAXED_CONSTEXPR
            constexpr bool valid = validate_pointer_value_type<I,
                                                               std::tuple_element_t<I, FnArgs>,
                                                               unpacked_arg_t<std::tuple_element_t<I, CallArgs>>>(
                polyfill::bool_constant < (polyfill::is_specialization_of_v<func_arg_t, pointer_arg>) ||
                (polyfill::is_specialization_of_v<passed_arg_t, pointer_binding>) > {});

            return validate_pointer_value_types<FnArgs, CallArgs>(polyfill::index_constant<I - 1>{}) && valid;
#else
            return validate_pointer_value_types<FnArgs, CallArgs>(polyfill::index_constant<I - 1>{}) &&
                   validate_pointer_value_type<I,
                                               std::tuple_element_t<I, FnArgs>,
                                               unpacked_arg_t<std::tuple_element_t<I, CallArgs>>>(
                       polyfill::bool_constant < (polyfill::is_specialization_of_v<func_arg_t, pointer_arg>) ||
                       (polyfill::is_specialization_of_v<passed_arg_t, pointer_binding>) > {});
#endif
        }
    }

    /**
     *  Used to call user defined function: `func<MyFunc>(...);`
     */
    template<class F, class... Args>
    internal::function_call<F, Args...> func(Args... args) {
        using args_tuple = std::tuple<Args...>;
        using function_args_tuple = typename internal::callable_arguments<F>::args_tuple;
        constexpr auto argsCount = std::tuple_size<args_tuple>::value;
        constexpr auto functionArgsCount = std::tuple_size<function_args_tuple>::value;
        static_assert((argsCount == functionArgsCount &&
                       !std::is_same<function_args_tuple, std::tuple<arg_values>>::value &&
                       internal::validate_pointer_value_types<function_args_tuple, args_tuple>(
                           polyfill::index_constant<std::min<>(functionArgsCount, argsCount) - 1>{})) ||
                          std::is_same<function_args_tuple, std::tuple<arg_values>>::value,
                      "Number of arguments does not match");
        return {std::make_tuple(std::forward<Args>(args)...)};
    }

}

namespace sqlite_orm {

    namespace internal {

        /**
         *  This is a proxy class used to define what type must have result type depending on select
         *  arguments (member pointer, aggregate functions, etc). Below you can see specializations
         *  for different types. E.g. specialization for internal::length_t has `type` int cause
         *  LENGTH returns INTEGER in sqlite. Every column_result_t must have `type` type that equals
         *  c++ SELECT return type for T
         *  T - C++ type
         *  SFINAE - sfinae argument
         */
        template<class St, class T, class SFINAE = void>
        struct column_result_t;

        template<class St, class T>
        using column_result_of_t = typename column_result_t<St, T>::type;

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
        template<class St, class T>
        struct column_result_t<St, as_optional_t<T>, void> {
            using type = std::optional<column_result_of_t<St, T>>;
        };

        template<class St, class T>
        struct column_result_t<St, std::optional<T>, void> {
            using type = std::optional<T>;
        };
#endif  //  SQLITE_ORM_OPTIONAL_SUPPORTED

        template<class St, class L, class A>
        struct column_result_t<St, dynamic_in_t<L, A>, void> {
            using type = bool;
        };

        template<class St, class L, class... Args>
        struct column_result_t<St, in_t<L, Args...>, void> {
            using type = bool;
        };

        template<class St, class T>
        struct column_result_t<St, T, match_if<std::is_member_pointer, T>> : member_field_type<T> {};

        template<class St, class R, class S, class... Args>
        struct column_result_t<St, built_in_function_t<R, S, Args...>, void> {
            using type = R;
        };

        template<class St, class R, class S, class... Args>
        struct column_result_t<St, built_in_aggregate_function_t<R, S, Args...>, void> {
            using type = R;
        };

        template<class St, class F, class... Args>
        struct column_result_t<St, function_call<F, Args...>, void> {
            using type = typename callable_arguments<F>::return_type;
        };

        template<class St, class X, class... Rest, class S>
        struct column_result_t<St, built_in_function_t<internal::unique_ptr_result_of<X>, S, X, Rest...>, void> {
            using type = std::unique_ptr<column_result_of_t<St, X>>;
        };

        template<class St, class X, class S>
        struct column_result_t<St, built_in_aggregate_function_t<internal::unique_ptr_result_of<X>, S, X>, void> {
            using type = std::unique_ptr<column_result_of_t<St, X>>;
        };

        template<class St, class T>
        struct column_result_t<St, count_asterisk_t<T>, void> {
            using type = int;
        };

        template<class St>
        struct column_result_t<St, nullptr_t, void> {
            using type = nullptr_t;
        };

        template<class St>
        struct column_result_t<St, count_asterisk_without_type, void> {
            using type = int;
        };

        template<class St, class T>
        struct column_result_t<St, distinct_t<T>, void> : column_result_t<St, T> {};

        template<class St, class T>
        struct column_result_t<St, all_t<T>, void> : column_result_t<St, T> {};

        template<class St, class L, class R>
        struct column_result_t<St, conc_t<L, R>, void> {
            using type = std::string;
        };

        template<class St, class L, class R>
        struct column_result_t<St, add_t<L, R>, void> {
            using type = double;
        };

        template<class St, class L, class R>
        struct column_result_t<St, sub_t<L, R>, void> {
            using type = double;
        };

        template<class St, class L, class R>
        struct column_result_t<St, mul_t<L, R>, void> {
            using type = double;
        };

        template<class St, class L, class R>
        struct column_result_t<St, internal::div_t<L, R>, void> {
            using type = double;
        };

        template<class St, class L, class R>
        struct column_result_t<St, mod_t<L, R>, void> {
            using type = double;
        };

        template<class St, class L, class R>
        struct column_result_t<St, bitwise_shift_left_t<L, R>, void> {
            using type = int;
        };

        template<class St, class L, class R>
        struct column_result_t<St, bitwise_shift_right_t<L, R>, void> {
            using type = int;
        };

        template<class St, class L, class R>
        struct column_result_t<St, bitwise_and_t<L, R>, void> {
            using type = int;
        };

        template<class St, class L, class R>
        struct column_result_t<St, bitwise_or_t<L, R>, void> {
            using type = int;
        };

        template<class St, class T>
        struct column_result_t<St, bitwise_not_t<T>, void> {
            using type = int;
        };

        template<class St>
        struct column_result_t<St, rowid_t, void> {
            using type = int64;
        };

        template<class St>
        struct column_result_t<St, oid_t, void> {
            using type = int64;
        };

        template<class St>
        struct column_result_t<St, _rowid_t, void> {
            using type = int64;
        };

        template<class St, class T>
        struct column_result_t<St, table_rowid_t<T>, void> {
            using type = int64;
        };

        template<class St, class T>
        struct column_result_t<St, table_oid_t<T>, void> {
            using type = int64;
        };

        template<class St, class T>
        struct column_result_t<St, table__rowid_t<T>, void> {
            using type = int64;
        };

        template<class St, class T, class C>
        struct column_result_t<St, alias_column_t<T, C>, void> : column_result_t<St, C> {};

        template<class St, class T, class F>
        struct column_result_t<St, column_pointer<T, F>, void> : column_result_t<St, F> {};

        template<class St, class... Args>
        struct column_result_t<St, columns_t<Args...>, void> {
            using type = std::tuple<column_result_of_t<St, std::decay_t<Args>>...>;
        };

        template<class St, class T, class... Args>
        struct column_result_t<St, select_t<T, Args...>> : column_result_t<St, T> {};

        template<class St, class T>
        struct column_result_t<St, T, std::enable_if_t<is_base_of_template_v<T, compound_operator>>> {
            using left_result = column_result_of_t<St, typename T::left_type>;
            using right_result = column_result_of_t<St, typename T::right_type>;
            static_assert(std::is_same<left_result, right_result>::value,
                          "Compound subselect queries must return same types");
            using type = left_result;
        };

        template<class St, class T>
        struct column_result_t<St, T, std::enable_if_t<is_base_of_template_v<T, binary_condition>>> {
            using type = typename T::result_type;
        };

        /**
         *  Result for the most simple queries like `SELECT 1`
         */
        template<class St, class T>
        struct column_result_t<St, T, match_if<std::is_arithmetic, T>> {
            using type = T;
        };

        /**
         *  Result for the most simple queries like `SELECT 'ototo'`
         */
        template<class St>
        struct column_result_t<St, const char*, void> {
            using type = std::string;
        };

        template<class St>
        struct column_result_t<St, std::string, void> {
            using type = std::string;
        };

        template<class St, class T, class E>
        struct column_result_t<St, as_t<T, E>, void> : column_result_t<St, std::decay_t<E>> {};

        template<class St, class T>
        struct column_result_t<St, asterisk_t<T>, match_if_not<std::is_base_of, alias_tag, T>>
            : storage_traits::storage_mapped_columns<St, T> {};

        template<class St, class A>
        struct column_result_t<St, asterisk_t<A>, match_if<std::is_base_of, alias_tag, A>>
            : storage_traits::storage_mapped_columns<St, type_t<A>> {};

        template<class St, class T>
        struct column_result_t<St, object_t<T>, void> {
            using type = T;
        };

        template<class St, class T, class E>
        struct column_result_t<St, cast_t<T, E>, void> {
            using type = T;
        };

        template<class St, class R, class T, class E, class... Args>
        struct column_result_t<St, simple_case_t<R, T, E, Args...>, void> {
            using type = R;
        };

        template<class St, class A, class T, class E>
        struct column_result_t<St, like_t<A, T, E>, void> {
            using type = bool;
        };

        template<class St, class A, class T>
        struct column_result_t<St, glob_t<A, T>, void> {
            using type = bool;
        };

        template<class St, class C>
        struct column_result_t<St, negated_condition_t<C>, void> {
            using type = bool;
        };

        template<class St, class T>
        struct column_result_t<St, std::reference_wrapper<T>, void> : column_result_t<St, T> {};
    }
}
#pragma once

#include <string>  //  std::string
#include <type_traits>  //  std::remove_reference, std::is_same, std::decay
#include <vector>  //  std::vector
#include <tuple>  //  std::tuple_size, std::tuple_element
#include <utility>  //  std::forward, std::move

// #include "functional/cxx_universal.h"

// #include "functional/cxx_polyfill.h"

// #include "functional/cxx_functional_polyfill.h"

#if __cpp_lib_invoke < 201411L
#include <type_traits>  //  std::enable_if, std::is_member_object_pointer, std::is_member_function_pointer
#endif
#include <functional>
#include <utility>  //  std::forward

namespace sqlite_orm {
    namespace internal {
        namespace polyfill {
#if __cpp_lib_invoke >= 201411L
            using std::invoke;
#else
            // pointer-to-data-member+object
            template<class Callable,
                     class Arg1,
                     class... Args,
                     class Unqualified = remove_cvref_t<Callable>,
                     std::enable_if_t<std::is_member_object_pointer<Unqualified>::value, bool> = true>
            decltype(auto) invoke(Callable&& obj, Arg1&& arg1, Args&&... args) {
                return std::forward<Arg1>(arg1).*obj;
            }

            // pointer-to-member-function+object
            template<class Callable,
                     class Arg1,
                     class... Args,
                     class Unqualified = remove_cvref_t<Callable>,
                     std::enable_if_t<std::is_member_function_pointer<Unqualified>::value, bool> = true>
            decltype(auto) invoke(Callable&& obj, Arg1&& arg1, Args&&... args) {
                return (std::forward<Arg1>(arg1).*obj)(std::forward<Args>(args)...);
            }

            // pointer-to-member+reference-wrapped object
            template<class Callable, class Arg1, class... Args>
            decltype(auto) invoke(Callable&& obj, std::reference_wrapper<Arg1> arg1, Args&&... args) {
                return invoke(std::forward<Callable>(obj), arg1.get(), std::forward<Args>(args)...);
            }

            // functor
            template<class Callable, class... Args>
            decltype(auto) invoke(Callable&& obj, Args&&... args) {
                return std::forward<Callable>(obj)(std::forward<Args>(args)...);
            }
#endif
        }
    }

    namespace polyfill = internal::polyfill;
}

// #include "functional/static_magic.h"

#include <type_traits>  //  std::false_type, std::true_type, std::integral_constant
#include <utility>  //  std::forward

namespace sqlite_orm {

    //  got from here
    //  https://stackoverflow.com/questions/37617677/implementing-a-compile-time-static-if-logic-for-different-string-types-in-a-co
    namespace internal {

        template<class R = void>
        decltype(auto) empty_callable() {
            static auto res = [](auto&&...) -> R {
                return R();
            };
            return (res);
        }

#ifdef SQLITE_ORM_IF_CONSTEXPR_SUPPORTED
        template<bool B, typename T, typename F>
        decltype(auto) static_if(T&& trueFn, F&& falseFn) {
            if constexpr(B) {
                return std::forward<T>(trueFn);
            } else {
                return std::forward<F>(falseFn);
            }
        }

        template<bool B, typename T>
        decltype(auto) static_if(T&& trueFn) {
            if constexpr(B) {
                return std::forward<T>(trueFn);
            } else {
                return empty_callable();
            }
        }

        template<bool B, typename L, typename... Args>
        void call_if_constexpr(L&& lambda, Args&&... args) {
            if constexpr(B) {
                lambda(std::forward<Args>(args)...);
            }
        }
#else
        template<typename T, typename F>
        decltype(auto) static_if(std::true_type, T&& trueFn, const F&) {
            return std::forward<T>(trueFn);
        }

        template<typename T, typename F>
        decltype(auto) static_if(std::false_type, const T&, F&& falseFn) {
            return std::forward<F>(falseFn);
        }

        template<bool B, typename T, typename F>
        decltype(auto) static_if(T&& trueFn, F&& falseFn) {
            return static_if(std::integral_constant<bool, B>{}, std::forward<T>(trueFn), std::forward<F>(falseFn));
        }

        template<bool B, typename T>
        decltype(auto) static_if(T&& trueFn) {
            return static_if(std::integral_constant<bool, B>{}, std::forward<T>(trueFn), empty_callable());
        }

        template<bool B, typename L, typename... Args>
        void call_if_constexpr(L&& lambda, Args&&... args) {
            static_if<B>(std::forward<L>(lambda))(std::forward<Args>(args)...);
        }
#endif
    }

}

// #include "functional/mpl.h"

// #include "typed_comparator.h"

// #include "tuple_helper/tuple_iteration.h"

#include <tuple>  //  std::tuple, std::get, std::tuple_element, std::tuple_size
#include <type_traits>  //  std::index_sequence, std::make_index_sequence
#include <utility>  //  std::forward

// #include "../functional/cxx_universal.h"

// #include "../functional/cxx_polyfill.h"

// #include "../functional/cxx_functional_polyfill.h"

namespace sqlite_orm {
    namespace internal {

        //  got it form here https://stackoverflow.com/questions/7858817/unpacking-a-tuple-to-call-a-matching-function-pointer
        template<class Function, class FunctionPointer, class Tuple, size_t... I>
        auto call_impl(Function& f, FunctionPointer functionPointer, Tuple t, std::index_sequence<I...>) {
            return (f.*functionPointer)(std::get<I>(move(t))...);
        }

        template<class Function, class FunctionPointer, class Tuple>
        auto call(Function& f, FunctionPointer functionPointer, Tuple t) {
            constexpr size_t size = std::tuple_size<Tuple>::value;
            return call_impl(f, functionPointer, move(t), std::make_index_sequence<size>{});
        }

        template<class Function, class Tuple>
        auto call(Function& f, Tuple t) {
            return call(f, &Function::operator(), move(t));
        }

#if defined(SQLITE_ORM_FOLD_EXPRESSIONS_SUPPORTED) && defined(SQLITE_ORM_IF_CONSTEXPR_SUPPORTED)
        template<bool reversed = false, class Tpl, size_t... Idx, class L>
        void iterate_tuple(const Tpl& tpl, std::index_sequence<Idx...>, L&& lambda) {
            if constexpr(reversed) {
                (lambda(std::get<sizeof...(Idx) - 1u - Idx>(tpl)), ...);
            } else {
                (lambda(std::get<Idx>(tpl)), ...);
            }
        }
#else
        template<bool reversed = false, class Tpl, class L>
        void iterate_tuple(const Tpl& /*tpl*/, std::index_sequence<>, L&& /*lambda*/) {}

        template<bool reversed = false, class Tpl, size_t I, size_t... Idx, class L>
        void iterate_tuple(const Tpl& tpl, std::index_sequence<I, Idx...>, L&& lambda) {
#ifdef SQLITE_ORM_IF_CONSTEXPR_SUPPORTED
            if constexpr(reversed) {
#else
            if(reversed) {
#endif
                iterate_tuple<reversed>(tpl, std::index_sequence<Idx...>{}, std::forward<L>(lambda));
                lambda(std::get<I>(tpl));
            } else {
                lambda(std::get<I>(tpl));
                iterate_tuple<reversed>(tpl, std::index_sequence<Idx...>{}, std::forward<L>(lambda));
            }
        }
#endif
        template<bool reversed = false, class Tpl, class L>
        void iterate_tuple(const Tpl& tpl, L&& lambda) {
            iterate_tuple<reversed>(tpl,
                                    std::make_index_sequence<std::tuple_size<Tpl>::value>{},
                                    std::forward<L>(lambda));
        }

#ifdef SQLITE_ORM_FOLD_EXPRESSIONS_SUPPORTED
        template<class Tpl, size_t... Idx, class L>
        void iterate_tuple(std::index_sequence<Idx...>, L&& lambda) {
            (lambda((std::tuple_element_t<Idx, Tpl>*)nullptr), ...);
        }
#else
        template<class Tpl, class L>
        void iterate_tuple(std::index_sequence<>, L&& /*lambda*/) {}

        template<class Tpl, size_t I, size_t... Idx, class L>
        void iterate_tuple(std::index_sequence<I, Idx...>, L&& lambda) {
            lambda((std::tuple_element_t<I, Tpl>*)nullptr);
            iterate_tuple<Tpl>(std::index_sequence<Idx...>{}, std::forward<L>(lambda));
        }
#endif
        template<class Tpl, class L>
        void iterate_tuple(L&& lambda) {
            iterate_tuple<Tpl>(std::make_index_sequence<std::tuple_size<Tpl>::value>{}, std::forward<L>(lambda));
        }

        template<class R, class Tpl, size_t... Idx, class Proj>
        R create_from_tuple(Tpl&& tpl, std::index_sequence<Idx...>, Proj&& projection) {
            return R{polyfill::invoke(std::forward<Proj>(projection), std::get<Idx>(std::forward<Tpl>(tpl)))...};
        }

        template<class R, class Tpl, class Proj>
        R create_from_tuple(Tpl&& tpl, Proj&& projection) {
            return create_from_tuple(std::make_index_sequence<std::tuple_size<Tpl>::value>{},
                                     std::forward<Tpl>(tpl),
                                     std::forward<Proj>(projection)...);
        }
    }
}

// #include "tuple_helper/tuple_traits.h"

// #include "tuple_helper/tuple_filter.h"

// #include "member_traits/member_traits.h"

// #include "type_traits.h"

// #include "constraints.h"

// #include "table_info.h"

// #include "column.h"

namespace sqlite_orm {

    namespace internal {

        struct basic_table {

            /**
             *  Table name.
             */
            std::string name;
        };

        /**
         *  Table definition.
         */
        template<class T, bool WithoutRowId, class... Cs>
        struct table_t : basic_table {
            using object_type = T;
            using elements_type = std::tuple<Cs...>;

            static constexpr bool is_without_rowid_v = WithoutRowId;
            using is_without_rowid = polyfill::bool_constant<is_without_rowid_v>;
            static constexpr int elements_count = static_cast<int>(std::tuple_size<elements_type>::value);

            elements_type elements;

#ifndef SQLITE_ORM_AGGREGATE_BASES_SUPPORTED
            table_t(std::string name_, elements_type elements_) : basic_table{move(name_)}, elements{move(elements_)} {}
#endif

            table_t<T, true, Cs...> without_rowid() const {
                return {this->name, this->elements};
            }

            /**
             *  Returns foreign keys count in table definition
             */
            constexpr int foreign_keys_count() const {
#if SQLITE_VERSION_NUMBER >= 3006019
                using fk_index_sequence = filter_tuple_sequence_t<elements_type, is_foreign_key>;
                return int(fk_index_sequence::size());
#else
                return 0;
#endif
            }

            /**
             *  Function used to get field value from object by mapped member pointer/setter/getter.
             *  
             *  For a setter the corresponding getter has to be searched,
             *  so the method returns a pointer to the field as returned by the found getter.
             *  Otherwise the method invokes the member pointer and returns its result.
             */
            template<class M, satisfies_not<is_setter, M> = true>
            decltype(auto) object_field_value(const object_type& object, M memberPointer) const {
                return polyfill::invoke(memberPointer, object);
            }

            template<class M, satisfies<is_setter, M> = true>
            const member_field_type_t<M>* object_field_value(const object_type& object, M memberPointer) const {
                using F = member_field_type_t<M>;
                const F* res = nullptr;
                this->for_each_column_with_field_type<F>([&res, &memberPointer, &object](auto& column) {
                    if(compare_any(column.setter, memberPointer)) {
                        res = &polyfill::invoke(column.member_pointer, object);
                    }
                });
                return res;
            }

            const basic_generated_always::storage_type*
            find_column_generated_storage_type(const std::string& name) const {
                const basic_generated_always::storage_type* result = nullptr;
#if SQLITE_VERSION_NUMBER >= 3031000
                this->for_each_column<column_constraints_type_t, check_if_tuple_has<is_generated_always>>(
                    [&result, &name](auto& column) {
                        if(column.name != name) {
                            return;
                        }
                        using generated_op_index_sequence =
                            filter_tuple_sequence_t<decltype(column.constraints), is_generated_always>;
                        constexpr size_t opIndex = first_index_sequence_value(generated_op_index_sequence{});
                        result = &get<opIndex>(column.constraints).storage;
                    });
#endif
                return result;
            }

            template<class C>
            bool exists_in_composite_primary_key(const C& column) const {
                bool res = false;
                this->for_each_primary_key([&column, &res](auto& primaryKey) {
                    using colrefs_tuple = decltype(primaryKey.columns);
                    using check_if_same_field_type = mpl::bind_front_fn<std::is_same, typename C::field_type>;
                    using same_type_index_sequence = filter_tuple_sequence_t<colrefs_tuple,
                                                                             check_if_same_field_type::template fn,
                                                                             member_field_type_t>;
                    iterate_tuple(primaryKey.columns, same_type_index_sequence{}, [&res, &column](auto& memberPointer) {
                        if(compare_any(memberPointer, column.member_pointer) ||
                           compare_any(memberPointer, column.setter)) {
                            res = true;
                        }
                    });
                });
                return res;
            }

            /**
             *  Call passed lambda with all defined primary keys.
             */
            template<class L>
            void for_each_primary_key(L&& lambda) const {
                using pk_index_sequence = filter_tuple_sequence_t<elements_type, is_primary_key>;
                iterate_tuple(this->elements, pk_index_sequence{}, lambda);
            }

            std::vector<std::string> composite_key_columns_names() const {
                std::vector<std::string> res;
                this->for_each_primary_key([this, &res](auto& primaryKey) {
                    res = this->composite_key_columns_names(primaryKey);
                });
                return res;
            }

            std::vector<std::string> primary_key_column_names() const {
                using col_index_sequence = filter_tuple_sequence_t<elements_type, is_column>;
                using pkcol_index_sequence = filter_tuple_sequence_t<elements_type,
                                                                     check_if_tuple_has<is_primary_key>::template fn,
                                                                     column_constraints_type_t,
                                                                     col_index_sequence>;
                if(pkcol_index_sequence::size() > 0) {
                    return create_from_tuple<std::vector<std::string>>(this->elements,
                                                                       pkcol_index_sequence{},
                                                                       &basic_column::name);
                } else {
                    return this->composite_key_columns_names();
                }
            }

            template<class L>
            void for_each_primary_key_column(L&& lambda) const {
                this->for_each_column_with<is_primary_key>([&lambda](auto& column) {
                    lambda(column.member_pointer);
                });
                this->for_each_primary_key([this, &lambda](auto& primaryKey) {
                    this->for_each_column_in_primary_key(primaryKey, lambda);
                });
            }

            template<class L, class... Args>
            void for_each_column_in_primary_key(const primary_key_t<Args...>& primaryKey, L&& lambda) const {
                iterate_tuple(primaryKey.columns, lambda);
            }

            template<class... Args>
            std::vector<std::string> composite_key_columns_names(const primary_key_t<Args...>& primaryKey) const {
                using colrefs_tuple = decltype(primaryKey.columns);

                std::vector<std::string> res;
                res.reserve(std::tuple_size<colrefs_tuple>::value);
                iterate_tuple(primaryKey.columns, [this, &res](auto& memberPointer) {
                    if(auto* columnName = this->find_column_name(memberPointer)) {
                        res.push_back(*columnName);
                    } else {
                        res.emplace_back();
                    }
                });
                return res;
            }

            /**
             *  Searches column name by class member pointer passed as the first argument.
             *  @return column name or empty string if nothing found.
             */
            template<class M, satisfies<std::is_member_pointer, M> = true>
            const std::string* find_column_name(M m) const {
                const std::string* res = nullptr;
                using field_type = member_field_type_t<M>;
                this->for_each_column_with_field_type<field_type>([&res, m](auto& c) {
                    if(compare_any(c.member_pointer, m) || compare_any(c.setter, m)) {
                        res = &c.name;
                    }
                });
                return res;
            }

            /**
             *  Counts and returns amount of columns without GENERATED ALWAYS constraints. Skips table constraints.
             */
            constexpr int non_generated_columns_count() const {
#if SQLITE_VERSION_NUMBER >= 3031000
                using col_index_sequence = filter_tuple_sequence_t<elements_type, is_column>;
                using non_generated_col_index_sequence =
                    filter_tuple_sequence_t<elements_type,
                                            check_if_tuple_has_not<is_generated_always>::template fn,
                                            column_constraints_type_t,
                                            col_index_sequence>;
                return int(non_generated_col_index_sequence::size());
#else
                return 0;
#endif
            }

            /**
             *  Counts and returns amount of columns. Skips constraints.
             */
            constexpr int count_columns_amount() const {
                using col_index_sequence = filter_tuple_sequence_t<elements_type, is_column>;
                return int(col_index_sequence::size());
            }

            /**
             *  Call passed lambda with all defined foreign keys.
             *  @param lambda Lambda to be called for each column. Must have signature like this [] (auto col) -> void {}
             */
            template<class L>
            void for_each_foreign_key(L&& lambda) const {
                using fk_index_sequence = filter_tuple_sequence_t<elements_type, is_foreign_key>;
                iterate_tuple(this->elements, fk_index_sequence{}, lambda);
            }

            /**
             *  Call passed lambda with all defined columns.
             *  @param lambda Lambda to be called for each column. Must have signature like this [] (auto col) -> void {}
             */
            template<class L>
            void for_each_column(L&& lambda) const {
                using col_index_sequence = filter_tuple_sequence_t<elements_type, is_column>;
                iterate_tuple(this->elements, col_index_sequence{}, lambda);
            }

            /**
             *  Call passed lambda with columns filtered on `Predicate(Transform(column_t))`.
             *  @param lambda Lambda to be called for each column. Must have signature like this [] (auto col) -> void {}
             */
            template<template<class...> class Transform, template<class...> class PredicateFn, class L>
            void for_each_column(L&& lambda) const {
                using col_index_sequence = filter_tuple_sequence_t<elements_type, is_column>;
                using filtered_index_sequence =
                    filter_tuple_sequence_t<elements_type, PredicateFn, Transform, col_index_sequence>;
                iterate_tuple(this->elements, filtered_index_sequence{}, lambda);
            }

            /**
             *  Call passed lambda with columns filtered on `Predicate(Transform(column_t))`.
             *  @param lambda Lambda to be called for each column. Must have signature like this [] (auto col) -> void {}
             */
            template<template<class...> class Transform,
                     class PredicateFnCls,
                     class L,
                     satisfies<mpl::is_metafunction_class, PredicateFnCls> = true>
            void for_each_column(L&& lambda) const {
                return this->for_each_column<Transform, PredicateFnCls::template fn>(lambda);
            }

            /**
             *  Call passed lambda with columns having the specified field type `F`.
             *  @param lambda Lambda to be called for each column. Must have signature like this [] (auto col) -> void {}
             */
            template<class F, class L>
            void for_each_column_with_field_type(L&& lambda) const {
                using check_if_same_field_type = mpl::bind_front_fn<std::is_same, F>;
                this->for_each_column<column_field_type_t, check_if_same_field_type>(lambda);
            }

            /**
             *  Call passed lambda with columns having the specified constraint trait `OpTrait`.
             *  @param lambda Lambda to be called for each column. Must have signature like this [] (auto col) -> void {}
             */
            template<template<class...> class OpTraitFn, class L>
            void for_each_column_with(L&& lambda) const {
                this->for_each_column<column_constraints_type_t, check_if_tuple_has<OpTraitFn>>(lambda);
            }

            /**
             *  Call passed lambda with columns not having the specified constraint trait `OpTrait`.
             *  @param lambda Lambda to be called for each column.
             */
            template<template<class...> class OpTraitFn, class L>
            void for_each_column_excluding(L&& lambda) const {
                this->for_each_column<column_constraints_type_t, check_if_tuple_has_not<OpTraitFn>>(lambda);
            }

            /**
             *  Call passed lambda with columns not having the specified constraint trait `OpTrait`.
             *  @param lambda Lambda to be called for each column.
             */
            template<class OpTraitFnCls, class L, satisfies<mpl::is_metafunction_class, OpTraitFnCls> = true>
            void for_each_column_excluding(L&& lambda) const {
                this->for_each_column_excluding<OpTraitFnCls::template fn>(lambda);
            }

            std::vector<table_xinfo> get_table_info() const;
        };
    }

    /**
     *  Function used for table creation. Do not use table constructor - use this function
     *  cause table class is templated and its constructing too (just like std::make_unique or std::make_pair).
     */
    template<class... Cs, class T = typename std::tuple_element_t<0, std::tuple<Cs...>>::object_type>
    internal::table_t<T, false, Cs...> make_table(const std::string& name, Cs... args) {
        SQLITE_ORM_CLANG_SUPPRESS_MISSING_BRACES(return {name, std::make_tuple<Cs...>(std::forward<Cs>(args)...)});
    }

    template<class T, class... Cs>
    internal::table_t<T, false, Cs...> make_table(const std::string& name, Cs... args) {
        SQLITE_ORM_CLANG_SUPPRESS_MISSING_BRACES(return {name, std::make_tuple<Cs...>(std::forward<Cs>(args)...)});
    }
}
#pragma once

#include <utility>  //  std::forward, std::move

namespace sqlite_orm {

    namespace internal {

        /**
         *  This is a generic implementation. Used as a tail in storage_impl inheritance chain
         */
        template<class... Ts>
        struct storage_impl;

        template<class H, class... Ts>
        struct storage_impl<H, Ts...> : storage_impl<Ts...> {
            using super = storage_impl<Ts...>;
            using table_type = H;

            storage_impl(H h, Ts... ts) : super{std::forward<Ts>(ts)...}, table{std::move(h)} {}

            table_type table;

            template<class L>
            void for_each(const L& l) const {
                this->super::for_each(l);
                l(*this);
            }
        };

        template<>
        struct storage_impl<> {

            template<class L>
            void for_each(const L&) const {}
        };
    }
}

#include <typeindex>  //  std::type_index
#include <string>  //  std::string
#include <type_traits>  //  std::is_same, std::decay

// #include "functional/cxx_universal.h"

// #include "functional/static_magic.h"

// #include "type_traits.h"

// #include "select_constraints.h"

// #include "storage_lookup.h"

// interface functions
namespace sqlite_orm {
    namespace internal {

        template<class S>
        using table_type_or_none_t = polyfill::detected_t<table_type_t, S>;

        template<class Lookup, class S, satisfies<is_storage_impl, S> = true>
        auto lookup_table(const S& strg) {
            const auto& tImpl = find_impl<Lookup>(strg);
            return static_if<std::is_same<decltype(tImpl), const storage_impl<>&>::value>(empty_callable<nullptr_t>(),
                                                                                          [](const auto& tImpl) {
                                                                                              return &tImpl.table;
                                                                                          })(tImpl);
        }

        template<class S, satisfies<is_storage_impl, S> = true>
        std::string find_table_name(const S& strg, const std::type_index& ti) {
            return static_if<std::is_same<S, storage_impl<>>::value>(
                empty_callable<std::string>(),
                [&ti](const auto& tImpl) {
                    using qualified_type = std::decay_t<decltype(tImpl)>;
                    return ti == typeid(storage_object_type_t<qualified_type>)
                               ? tImpl.table.name
                               : find_table_name<typename qualified_type::super>(tImpl, ti);
                })(strg);
        }

        template<class Lookup, class S, satisfies<is_storage_impl, S> = true>
        std::string lookup_table_name(const S& strg) {
            const auto& tImpl = find_impl<Lookup>(strg);
            return static_if<std::is_same<decltype(tImpl), const storage_impl<>&>::value>(empty_callable<std::string>(),
                                                                                          [](const auto& tImpl) {
                                                                                              return tImpl.table.name;
                                                                                          })(tImpl);
        }

        template<class Lookup, class S, satisfies<is_storage_impl, S> = true>
        const std::string& get_table_name(const S& strg) {
            return pick_impl<Lookup>(strg).table.name;
        }

        /**
         *  Find column name by its type and member pointer.
         */
        template<class O, class F, class S, satisfies<is_storage_impl, S> = true>
        const std::string* find_column_name(const S& strg, F O::*field) {
            return pick_impl<O>(strg).table.find_column_name(field);
        }

        /**
         *  Materialize column pointer:
         *  1. by explicit object type and member pointer.
         */
        template<class O, class F, class S, satisfies<is_storage_impl, S> = true>
        constexpr decltype(auto) materialize_column_pointer(const S&, const column_pointer<O, F>& cp) {
            return cp.field;
        }

        /**
         *  Find column name by:
         *  1. by explicit object type and member pointer.
         */
        template<class O, class F, class S, satisfies<is_storage_impl, S> = true>
        const std::string* find_column_name(const S& strg, const column_pointer<O, F>& cp) {
            auto field = materialize_column_pointer(strg, cp);
            return pick_impl<O>(strg).table.find_column_name(field);
        }
    }
}
#pragma once

#include <string>  //  std::string

// #include "constraints.h"

// #include "serializer_context.h"

// #include "storage_lookup.h"

namespace sqlite_orm {

    namespace internal {

        template<class T, class I>
        std::string serialize(const T& t, const serializer_context<I>& context);

        /**
         *  Serialize default value of a column's default valu
         */
        template<class T>
        std::string serialize_default_value(const default_t<T>& dft) {
            storage_impl<> storage;
            serializer_context<storage_impl<>> context{storage};
            return serialize(dft.value, context);
        }

    }

}
#pragma once

#include <sqlite3.h>
#include <memory>  //  std::unique/shared_ptr, std::make_unique/shared
#include <system_error>  //  std::system_error
#include <string>  //  std::string
#include <type_traits>  //  std::remove_reference, std::is_base_of, std::decay, std::false_type, std::true_type
#include <iterator>  //  std::input_iterator_tag, std::iterator_traits, std::distance
#include <functional>  //  std::function
#include <sstream>  //  std::stringstream
#include <map>  //  std::map
#include <vector>  //  std::vector
#include <tuple>  //  std::tuple_size, std::tuple, std::make_tuple
#include <utility>  //  std::forward, std::pair
#include <algorithm>  //  std::find

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
#include <optional>  // std::optional
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

// #include "functional/cxx_universal.h"

// #include "functional/cxx_functional_polyfill.h"

// #include "functional/static_magic.h"

// #include "functional/mpl.h"

// #include "tuple_helper/tuple_traits.h"

// #include "tuple_helper/tuple_filter.h"

// #include "tuple_helper/tuple_iteration.h"

// #include "type_traits.h"

// #include "alias.h"

// #include "row_extractor_builder.h"

// #include "functional/cxx_universal.h"

// #include "row_extractor.h"

// #include "mapped_row_extractor.h"

#include <sqlite3.h>

// #include "object_from_column_builder.h"

#include <sqlite3.h>
#include <type_traits>  //  std::is_member_object_pointer

// #include "functional/static_magic.h"

// #include "row_extractor.h"

namespace sqlite_orm {

    namespace internal {

        struct object_from_column_builder_base {
            sqlite3_stmt* stmt = nullptr;
            int index = 0;

#ifndef SQLITE_ORM_AGGREGATE_NSDMI_SUPPORTED
            object_from_column_builder_base(sqlite3_stmt* stmt) : stmt{stmt} {}
#endif
        };

        /**
         * This is a cute lambda replacement which is used in several places.
         */
        template<class O>
        struct object_from_column_builder : object_from_column_builder_base {
            using object_type = O;

            object_type& object;

            object_from_column_builder(object_type& object_, sqlite3_stmt* stmt_) :
                object_from_column_builder_base{stmt_}, object(object_) {}

            template<class C>
            void operator()(const C& c) {
                using field_type = typename C::field_type;
                auto value = row_extractor<field_type>().extract(this->stmt, this->index++);
                static_if<std::is_member_object_pointer<typename C::member_pointer_t>::value>(
                    [&value, &object = this->object](const auto& c) {
                        object.*c.member_pointer = std::move(value);
                    },
                    [&value, &object = this->object](const auto& c) {
                        (object.*c.setter)(std::move(value));
                    })(c);
            }
        };
    }
}

namespace sqlite_orm {

    namespace internal {

        /**
         * This is a private row extractor class. It is used for extracting rows as objects instead of tuple.
         * Main difference from regular `row_extractor` is that this class takes table info which is required
         * for constructing objects by member pointers. To construct please use `make_row_extractor()`.
         * Type arguments:
         * V is value type just like regular `row_extractor` has
         * T is table info class `table_t`
         */
        template<class V, class T>
        struct mapped_row_extractor {
            using table_info_t = T;

            V extract(sqlite3_stmt* stmt, int /*columnIndex*/) const {
                V res;
                object_from_column_builder<V> builder{res, stmt};
                this->tableInfo.for_each_column(builder);
                return res;
            }

            const table_info_t& tableInfo;
        };

    }

}

namespace sqlite_orm {

    namespace internal {

        template<class T>
        row_extractor<T> make_row_extractor(nullptr_t) {
            return {};
        }

        template<class T, class I>
        mapped_row_extractor<T, I> make_row_extractor(const I* tableInfo) {
            return {*tableInfo};
        }
    }

}

// #include "error_code.h"

// #include "type_printer.h"

// #include "constraints.h"

// #include "field_printer.h"

// #include "rowid.h"

// #include "operators.h"

// #include "select_constraints.h"

// #include "core_functions.h"

// #include "conditions.h"

// #include "statement_binder.h"

// #include "column_result.h"

// #include "mapped_type_proxy.h"

// #include "sync_schema_result.h"

// #include "table_info.h"

// #include "storage_impl.h"

// #include "journal_mode.h"

// #include "view.h"

#include <sqlite3.h>
#include <memory>  //  std::shared_ptr
#include <string>  //  std::string
#include <utility>  //  std::forward, std::move
#include <tuple>  //  std::tuple, std::make_tuple

// #include "row_extractor.h"

// #include "error_code.h"

// #include "iterator.h"

#include <sqlite3.h>
#include <memory>  //  std::shared_ptr, std::unique_ptr, std::make_shared
#include <type_traits>  //  std::decay
#include <utility>  //  std::move
#include <iterator>  //  std::input_iterator_tag
#include <system_error>  //  std::system_error

// #include "functional/cxx_universal.h"

// #include "row_extractor.h"

// #include "statement_finalizer.h"

// #include "error_code.h"

// #include "object_from_column_builder.h"

namespace sqlite_orm {

    namespace internal {

        template<class V>
        struct iterator_t {
            using view_type = V;
            using value_type = typename view_type::mapped_type;

          protected:
            /**
             *  The double-indirection is so that copies of the iterator
             *  share the same sqlite3_stmt from a sqlite3_prepare_v2()
             *  call. When one finishes iterating it the pointer
             *  inside the shared_ptr is nulled out in all copies.
             */
            std::shared_ptr<statement_finalizer> stmt;

            // only null for the default constructed iterator
            view_type* view;

            /**
             *  shared_ptr is used over unique_ptr here
             *  so that the iterator can be copyable.
             */
            std::shared_ptr<value_type> current;

            void extract_value() {
                auto& storage = this->view->storage;
                auto& impl = pick_const_impl<value_type>(storage);
                this->current = std::make_shared<value_type>();
                object_from_column_builder<value_type> builder{*this->current, this->stmt->get()};
                impl.table.for_each_column(builder);
            }

          public:
            using difference_type = ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type&;
            using iterator_category = std::input_iterator_tag;

            iterator_t() : view(nullptr){};

            iterator_t(sqlite3_stmt* stmt_, view_type& view_) :
                stmt(std::make_shared<statement_finalizer>(stmt_)), view(&view_) {
                next();
            }

            const value_type& operator*() const {
                if(!this->stmt || !this->current) {
                    throw std::system_error{orm_error_code::trying_to_dereference_null_iterator};
                }
                return *this->current;
            }

            const value_type* operator->() const {
                return &(this->operator*());
            }

          private:
            void next() {
                this->current.reset();
                if(this->stmt) {
                    int rc = sqlite3_step(this->stmt->get());
                    switch(rc) {
                        case SQLITE_ROW:
                            this->extract_value();
                            break;
                        case SQLITE_DONE:
                            this->stmt.reset();
                            break;
                        default: {
                            sqlite3* db = this->view->connection.get();
                            throw_translated_sqlite_error(db);
                        }
                    }
                }
            }

          public:
            iterator_t<V>& operator++() {
                next();
                return *this;
            }

            void operator++(int) {
                this->operator++();
            }

            bool operator==(const iterator_t& other) const {
                return this->current == other.current;
            }

            bool operator!=(const iterator_t& other) const {
                return !(*this == other);
            }
        };
    }
}

// #include "ast_iterator.h"

#include <vector>  //  std::vector
#include <functional>  //  std::reference_wrapper

// #include "tuple_helper/tuple_iteration.h"

// #include "conditions.h"

// #include "select_constraints.h"

// #include "operators.h"

// #include "core_functions.h"

// #include "prepared_statement.h"

#include <sqlite3.h>
#include <iterator>  //  std::iterator_traits
#include <string>  //  std::string
#include <type_traits>  //  std::true_type, std::false_type
#include <utility>  //  std::pair

// #include "functional/cxx_universal.h"

// #include "functional/cxx_polyfill.h"

// #include "tuple_helper/tuple_filter.h"

// #include "connection_holder.h"

#include <sqlite3.h>
#include <string>  //  std::string

// #include "error_code.h"

namespace sqlite_orm {

    namespace internal {

        struct connection_holder {

            connection_holder(std::string filename_) : filename(move(filename_)) {}

            void retain() {
                ++this->_retain_count;
                if(1 == this->_retain_count) {
                    auto rc = sqlite3_open(this->filename.c_str(), &this->db);
                    if(rc != SQLITE_OK) {
                        throw_translated_sqlite_error(db);
                    }
                }
            }

            void release() {
                --this->_retain_count;
                if(0 == this->_retain_count) {
                    auto rc = sqlite3_close(this->db);
                    if(rc != SQLITE_OK) {
                        throw_translated_sqlite_error(db);
                    }
                }
            }

            sqlite3* get() const {
                return this->db;
            }

            int retain_count() const {
                return this->_retain_count;
            }

            const std::string filename;

          protected:
            sqlite3* db = nullptr;
            int _retain_count = 0;
        };

        struct connection_ref {
            connection_ref(connection_holder& holder_) : holder(holder_) {
                this->holder.retain();
            }

            connection_ref(const connection_ref& other) : holder(other.holder) {
                this->holder.retain();
            }

            connection_ref(connection_ref&& other) : holder(other.holder) {
                this->holder.retain();
            }

            ~connection_ref() {
                this->holder.release();
            }

            sqlite3* get() const {
                return this->holder.get();
            }

          protected:
            connection_holder& holder;
        };
    }
}

// #include "select_constraints.h"

// #include "values.h"

#include <vector>  //  std::vector
#include <tuple>  //  std::tuple
#include <utility>  //  std::forward

// #include "functional/cxx_universal.h"

// #include "functional/cxx_polyfill.h"

namespace sqlite_orm {

    namespace internal {

        template<class... Args>
        struct values_t {
            using args_tuple = std::tuple<Args...>;

            args_tuple tuple;
        };

        template<class T>
        SQLITE_ORM_INLINE_VAR constexpr bool is_values_v = polyfill::is_specialization_of_v<T, values_t>;

        template<class T>
        using is_values = polyfill::bool_constant<is_values_v<T>>;

        template<class T>
        struct dynamic_values_t {
            std::vector<T> vector;
        };

    }

    template<class... Args>
    internal::values_t<Args...> values(Args... args) {
        return {{std::forward<Args>(args)...}};
    }

    template<class T>
    internal::dynamic_values_t<T> values(std::vector<T> vector) {
        return {{move(vector)}};
    }

}

// #include "ast/upsert_clause.h"

#include <tuple>  //  std::tuple, std::make_tuple
#include <type_traits>  //  std::false_type, std::true_type
#include <utility>  //  std::forward, std::move

// #include "../functional/cxx_polyfill.h"

namespace sqlite_orm {
    namespace internal {
#if SQLITE_VERSION_NUMBER >= 3024000
        template<class T, class A>
        struct upsert_clause;

        template<class... Args>
        struct conflict_target {
            using args_tuple = std::tuple<Args...>;

            args_tuple args;

            upsert_clause<args_tuple, std::tuple<>> do_nothing() {
                return {move(this->args), {}};
            }

            template<class... ActionsArgs>
            upsert_clause<args_tuple, std::tuple<ActionsArgs...>> do_update(ActionsArgs... actions) {
                return {move(this->args), {std::make_tuple(std::forward<ActionsArgs>(actions)...)}};
            }
        };

        template<class... TargetArgs, class... ActionsArgs>
        struct upsert_clause<std::tuple<TargetArgs...>, std::tuple<ActionsArgs...>> {
            using target_args_tuple = std::tuple<TargetArgs...>;
            using actions_tuple = std::tuple<ActionsArgs...>;

            target_args_tuple target_args;

            actions_tuple actions;
        };

        template<class T>
        using is_upsert_clause = polyfill::is_specialization_of<T, upsert_clause>;
    }

    /**
     *  ON CONFLICT upsert clause builder function.
     *  @example
     *  storage.insert(into<Employee>(),
     *            columns(&Employee::id, &Employee::name, &Employee::age, &Employee::address, &Employee::salary),
     *            values(std::make_tuple(3, "Sofia", 26, "Madrid", 15000.0),
     *                 std::make_tuple(4, "Doja", 26, "LA", 25000.0)),
     *            on_conflict(&Employee::id).do_update(set(c(&Employee::name) = excluded(&Employee::name),
     *                                           c(&Employee::age) = excluded(&Employee::age),
     *                                           c(&Employee::address) = excluded(&Employee::address),
     *                                           c(&Employee::salary) = excluded(&Employee::salary))));
     */
    template<class... Args>
    internal::conflict_target<Args...> on_conflict(Args... args) {
        return {std::tuple<Args...>(std::forward<Args>(args)...)};
    }
#endif
}

namespace sqlite_orm {

    namespace internal {

        struct prepared_statement_base {
            sqlite3_stmt* stmt = nullptr;
            connection_ref con;

#ifndef SQLITE_ORM_AGGREGATE_NSDMI_SUPPORTED
            prepared_statement_base(sqlite3_stmt* stmt, connection_ref con) : stmt{stmt}, con{std::move(con)} {}
#endif

            ~prepared_statement_base() {
                if(this->stmt) {
                    sqlite3_finalize(this->stmt);
                }
            }

            std::string sql() const {
                if(this->stmt) {
                    if(auto res = sqlite3_sql(this->stmt)) {
                        return res;
                    } else {
                        return {};
                    }
                } else {
                    return {};
                }
            }

#if SQLITE_VERSION_NUMBER >= 3014000
            std::string expanded_sql() const {
                if(this->stmt) {
                    if(auto res = sqlite3_expanded_sql(this->stmt)) {
                        std::string result = res;
                        sqlite3_free(res);
                        return result;
                    } else {
                        return {};
                    }
                } else {
                    return {};
                }
            }
#endif
#if SQLITE_VERSION_NUMBER >= 3026000 and defined(SQLITE_ENABLE_NORMALIZE)
            std::string normalized_sql() const {
                if(this->stmt) {
                    if(auto res = sqlite3_normalized_sql(this->stmt)) {
                        return res;
                    } else {
                        return {};
                    }
                } else {
                    return {};
                }
            }
#endif
        };

        template<class T>
        struct prepared_statement_t : prepared_statement_base {
            using expression_type = T;

            expression_type expression;

            prepared_statement_t(T expression_, sqlite3_stmt* stmt_, connection_ref con_) :
                prepared_statement_base{stmt_, std::move(con_)}, expression(std::move(expression_)) {}

            prepared_statement_t(prepared_statement_t&& prepared_stmt) :
                prepared_statement_base{prepared_stmt.stmt, std::move(prepared_stmt.con)},
                expression(std::move(prepared_stmt.expression)) {
                prepared_stmt.stmt = nullptr;
            }
        };

        template<class T>
        SQLITE_ORM_INLINE_VAR constexpr bool is_prepared_statement_v =
            polyfill::is_specialization_of_v<T, prepared_statement_t>;

        template<class T>
        using is_prepared_statement = polyfill::bool_constant<is_prepared_statement_v<T>>;

        /**
         *  T - type of object to obtain from a database
         */
        template<class T, class R, class... Args>
        struct get_all_t {
            using type = T;
            using return_type = R;

            using conditions_type = std::tuple<Args...>;

            conditions_type conditions;
        };

        template<class T, class R, class... Args>
        struct get_all_pointer_t {
            using type = T;
            using return_type = R;

            using conditions_type = std::tuple<Args...>;

            conditions_type conditions;
        };

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
        template<class T, class R, class... Args>
        struct get_all_optional_t {
            using type = T;
            using return_type = R;

            using conditions_type = std::tuple<Args...>;

            conditions_type conditions;
        };
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

        template<class T, class... Wargs>
        struct update_all_t;

        template<class... Args, class... Wargs>
        struct update_all_t<set_t<Args...>, Wargs...> {
            using set_type = set_t<Args...>;
            using conditions_type = std::tuple<Wargs...>;

            set_type set;
            conditions_type conditions;
        };

        template<class T, class... Args>
        struct remove_all_t {
            using type = T;
            using conditions_type = std::tuple<Args...>;

            conditions_type conditions;
        };

        template<class T, class... Ids>
        struct get_t {
            using type = T;
            using ids_type = std::tuple<Ids...>;

            ids_type ids;
        };

        template<class T, class... Ids>
        struct get_pointer_t {
            using type = T;
            using ids_type = std::tuple<Ids...>;

            ids_type ids;
        };

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
        template<class T, class... Ids>
        struct get_optional_t {
            using type = T;
            using ids_type = std::tuple<Ids...>;

            ids_type ids;
        };
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

        template<class T>
        struct update_t {
            using type = T;

            type object;
        };

        template<class T, class... Ids>
        struct remove_t {
            using type = T;
            using ids_type = std::tuple<Ids...>;

            ids_type ids;
        };

        template<class T>
        struct insert_t {
            using type = T;

            type object;
        };

        template<class T>
        SQLITE_ORM_INLINE_VAR constexpr bool is_insert_v = polyfill::is_specialization_of_v<T, insert_t>;

        template<class T>
        using is_insert = polyfill::bool_constant<is_insert_v<T>>;

        template<class T, class... Cols>
        struct insert_explicit {
            using type = T;
            using columns_type = columns_t<Cols...>;

            type obj;
            columns_type columns;
        };

        template<class T>
        struct replace_t {
            using type = T;

            type object;
        };

        template<class T>
        SQLITE_ORM_INLINE_VAR constexpr bool is_replace_v = polyfill::is_specialization_of_v<T, replace_t>;

        template<class T>
        using is_replace = polyfill::bool_constant<is_replace_v<T>>;

        template<class It, class L, class O>
        struct insert_range_t {
            using iterator_type = It;
            using container_object_type = typename std::iterator_traits<iterator_type>::value_type;
            using transformer_type = L;
            using object_type = O;

            std::pair<iterator_type, iterator_type> range;
            transformer_type transformer;
        };

        template<class T>
        SQLITE_ORM_INLINE_VAR constexpr bool is_insert_range_v = polyfill::is_specialization_of_v<T, insert_range_t>;

        template<class T>
        using is_insert_range = polyfill::bool_constant<is_insert_range_v<T>>;

        template<class It, class L, class O>
        struct replace_range_t {
            using iterator_type = It;
            using container_object_type = typename std::iterator_traits<iterator_type>::value_type;
            using transformer_type = L;
            using object_type = O;

            std::pair<iterator_type, iterator_type> range;
            transformer_type transformer;
        };

        template<class T>
        SQLITE_ORM_INLINE_VAR constexpr bool is_replace_range_v = polyfill::is_specialization_of_v<T, replace_range_t>;

        template<class T>
        using is_replace_range = polyfill::bool_constant<is_replace_range_v<T>>;

        template<class... Args>
        struct insert_raw_t {
            using args_tuple = std::tuple<Args...>;

            args_tuple args;
        };

        template<class T>
        SQLITE_ORM_INLINE_VAR constexpr bool is_insert_raw_v = polyfill::is_specialization_of_v<T, insert_raw_t>;

        template<class T>
        using is_insert_raw = polyfill::bool_constant<is_insert_raw_v<T>>;

        template<class... Args>
        struct replace_raw_t {
            using args_tuple = std::tuple<Args...>;

            args_tuple args;
        };

        template<class T>
        SQLITE_ORM_INLINE_VAR constexpr bool is_replace_raw_v = polyfill::is_specialization_of_v<T, replace_raw_t>;

        template<class T>
        using is_replace_raw = polyfill::bool_constant<is_replace_raw_v<T>>;

        struct default_transformer {

            template<class T>
            const T& operator()(const T& object) const {
                return object;
            }
        };

        struct default_values_t {};

        template<class T>
        using is_default_values = std::is_same<T, default_values_t>;

        enum class conflict_action {
            abort,
            fail,
            ignore,
            replace,
            rollback,
        };

        struct insert_constraint {
            conflict_action action = conflict_action::abort;

#ifndef SQLITE_ORM_AGGREGATE_NSDMI_SUPPORTED
            insert_constraint(conflict_action action) : action{action} {}
#endif
        };

        template<class T>
        using is_insert_constraint = std::is_same<T, insert_constraint>;
    }

    inline internal::insert_constraint or_rollback() {
        return {internal::conflict_action::rollback};
    }

    inline internal::insert_constraint or_replace() {
        return {internal::conflict_action::replace};
    }

    inline internal::insert_constraint or_ignore() {
        return {internal::conflict_action::ignore};
    }

    inline internal::insert_constraint or_fail() {
        return {internal::conflict_action::fail};
    }

    inline internal::insert_constraint or_abort() {
        return {internal::conflict_action::abort};
    }

    /**
     *  Use this function to add `DEFAULT VALUES` modifier to raw `INSERT`.
     *
     *  @example
     *  ```
     *  storage.insert(into<Singer>(), default_values());
     *  ```
     */
    inline internal::default_values_t default_values() {
        return {};
    }

    /**
     *  Raw insert statement creation routine. Use this if `insert` with object does not fit you. This insert is designed to be able
     *  to call any type of `INSERT` query with no limitations.
     *  @example
     *  ```sql
     *  INSERT INTO users (id, name) VALUES(5, 'Little Mix')
     *  ```
     *  will be
     *  ```c++
     *  auto statement = storage.prepare(insert(into<User>, columns(&User::id, &User::name), values(std::make_tuple(5, "Little Mix"))));
     *  storage.execute(statement));
     *  ```
     *  One more example:
     *  ```sql
     *  INSERT INTO singers (name) VALUES ('Sofia Reyes')('Kungs')
     *  ```
     *  will be
     *  ```c++
     *  auto statement = storage.prepare(insert(into<Singer>(), columns(&Singer::name), values(std::make_tuple("Sofia Reyes"), std::make_tuple("Kungs"))));
     *  storage.execute(statement));
     *  ```
     *  One can use `default_values` to add `DEFAULT VALUES` modifier:
     *  ```sql
     *  INSERT INTO users DEFAULT VALUES
     *  ```
     *  will be
     *  ```c++
     *  auto statement = storage.prepare(insert(into<Singer>(), default_values()));
     *  storage.execute(statement));
     *  ```
     *  Also one can use `INSERT OR ABORT`/`INSERT OR FAIL`/`INSERT OR IGNORE`/`INSERT OR REPLACE`/`INSERT ROLLBACK`:
     *  ```c++
     *  auto statement = storage.prepare(insert(or_ignore(), into<Singer>(), columns(&Singer::name), values(std::make_tuple("Sofia Reyes"), std::make_tuple("Kungs"))));
     *  auto statement2 = storage.prepare(insert(or_rollback(), into<Singer>(), default_values()));
     *  auto statement3 = storage.prepare(insert(or_abort(), into<User>, columns(&User::id, &User::name), values(std::make_tuple(5, "Little Mix"))));
     *  ```
     */
    template<class... Args>
    internal::insert_raw_t<Args...> insert(Args... args) {
        using args_tuple = std::tuple<Args...>;
        using internal::count_tuple;
        using internal::is_columns;
        using internal::is_insert_constraint;
        using internal::is_into;
        using internal::is_select;
        using internal::is_upsert_clause;
        using internal::is_values;

        constexpr int orArgsCount = count_tuple<args_tuple, is_insert_constraint>::value;
        static_assert(orArgsCount < 2, "Raw insert must have only one OR... argument");

        constexpr int intoArgsCount = count_tuple<args_tuple, is_into>::value;
        static_assert(intoArgsCount != 0, "Raw insert must have into<T> argument");
        static_assert(intoArgsCount < 2, "Raw insert must have only one into<T> argument");

        constexpr int columnsArgsCount = count_tuple<args_tuple, is_columns>::value;
        static_assert(columnsArgsCount < 2, "Raw insert must have only one columns(...) argument");

        constexpr int valuesArgsCount = count_tuple<args_tuple, is_values>::value;
        static_assert(valuesArgsCount < 2, "Raw insert must have only one values(...) argument");

        constexpr int defaultValuesCount = count_tuple<args_tuple, internal::is_default_values>::value;
        static_assert(defaultValuesCount < 2, "Raw insert must have only one default_values() argument");

        constexpr int selectsArgsCount = count_tuple<args_tuple, is_select>::value;
        static_assert(selectsArgsCount < 2, "Raw insert must have only one select(...) argument");

        constexpr int upsertClausesCount = count_tuple<args_tuple, is_upsert_clause>::value;
        static_assert(upsertClausesCount <= 2, "Raw insert can contain 2 instances of upsert clause maximum");

        constexpr int argsCount = int(std::tuple_size<args_tuple>::value);
        static_assert(argsCount == intoArgsCount + columnsArgsCount + valuesArgsCount + defaultValuesCount +
                                       selectsArgsCount + orArgsCount + upsertClausesCount,
                      "Raw insert has invalid arguments");

        return {{std::forward<Args>(args)...}};
    }

    /**
     *  Raw replace statement creation routine. Use this if `replace` with object does not fit you. This replace is designed to be able
     *  to call any type of `REPLACE` query with no limitations. Actually this is the same query as raw insert except `OR...` option existance.
     *  @example
     *  ```sql
     *  REPLACE INTO users (id, name) VALUES(5, 'Little Mix')
     *  ```
     *  will be
     *  ```c++
     *  auto statement = storage.prepare(replace(into<User>, columns(&User::id, &User::name), values(std::make_tuple(5, "Little Mix"))));
     *  storage.execute(statement));
     *  ```
     *  One more example:
     *  ```sql
     *  REPLACE INTO singers (name) VALUES ('Sofia Reyes')('Kungs')
     *  ```
     *  will be
     *  ```c++
     *  auto statement = storage.prepare(replace(into<Singer>(), columns(&Singer::name), values(std::make_tuple("Sofia Reyes"), std::make_tuple("Kungs"))));
     *  storage.execute(statement));
     *  ```
     *  One can use `default_values` to add `DEFAULT VALUES` modifier:
     *  ```sql
     *  REPLACE INTO users DEFAULT VALUES
     *  ```
     *  will be
     *  ```c++
     *  auto statement = storage.prepare(replace(into<Singer>(), default_values()));
     *  storage.execute(statement));
     *  ```
     */
    template<class... Args>
    internal::replace_raw_t<Args...> replace(Args... args) {
        using args_tuple = std::tuple<Args...>;
        using internal::count_tuple;
        using internal::is_columns;
        using internal::is_into;
        using internal::is_values;

        constexpr int intoArgsCount = count_tuple<args_tuple, is_into>::value;
        static_assert(intoArgsCount != 0, "Raw replace must have into<T> argument");
        static_assert(intoArgsCount < 2, "Raw replace must have only one into<T> argument");

        constexpr int columnsArgsCount = count_tuple<args_tuple, is_columns>::value;
        static_assert(columnsArgsCount < 2, "Raw replace must have only one columns(...) argument");

        constexpr int valuesArgsCount = count_tuple<args_tuple, is_values>::value;
        static_assert(valuesArgsCount < 2, "Raw replace must have only one values(...) argument");

        constexpr int defaultValuesCount = count_tuple<args_tuple, internal::is_default_values>::value;
        static_assert(defaultValuesCount < 2, "Raw replace must have only one default_values() argument");

        constexpr int selectsArgsCount = count_tuple<args_tuple, internal::is_select>::value;
        static_assert(selectsArgsCount < 2, "Raw replace must have only one select(...) argument");

        constexpr int argsCount = int(std::tuple_size<args_tuple>::value);
        static_assert(argsCount ==
                          intoArgsCount + columnsArgsCount + valuesArgsCount + defaultValuesCount + selectsArgsCount,
                      "Raw replace has invalid arguments");

        return {{std::forward<Args>(args)...}};
    }

    /**
     *  Create a replace range statement
     *
     *  @example
     *  ```
     *  std::vector<User> users;
     *  users.push_back(User{1, "Leony"});
     *  auto statement = storage.prepare(replace_range(users.begin(), users.end()));
     *  storage.execute(statement);
     *  ```
     */
    template<class It>
    internal::replace_range_t<It, internal::default_transformer, typename std::iterator_traits<It>::value_type>
    replace_range(It from, It to) {
        return {{std::move(from), std::move(to)}};
    }

    /**
     *  Create an replace range statement with explicit transformer. Transformer is used to apply containers with no strict objects with other kind of objects like pointers,
     *  optionals or whatever.
     *  @example
     *  ```
     *  std::vector<std::unique_ptr<User>> userPointers;
     *  userPointers.push_back(std::make_unique<User>(1, "Eneli"));
     *  auto statement = storage.prepare(replace_range<User>(userPointers.begin(), userPointers.end(), [](const std::unique_ptr<User> &userPointer) -> const User & {
     *      return *userPointer;
     *  }));
     *  storage.execute(statement);
     *  ```
     */
    template<class T, class It, class L>
    internal::replace_range_t<It, L, T> replace_range(It from, It to, L transformer) {
        return {{std::move(from), std::move(to)}, std::move(transformer)};
    }

    /**
     *  Create an insert range statement
     *  @example
     *  ```
     *  std::vector<User> users;
     *  users.push_back(User{1, "Leony"});
     *  auto statement = storage.prepare(insert_range(users.begin(), users.end()));
     *  storage.execute(statement);
     *  ```
     */
    template<class It>
    internal::insert_range_t<It, internal::default_transformer, typename std::iterator_traits<It>::value_type>
    insert_range(It from, It to) {
        return {{std::move(from), std::move(to)}, internal::default_transformer{}};
    }

    /**
     *  Create an insert range statement with explicit transformer. Transformer is used to apply containers with no strict objects with other kind of objects like pointers,
     *  optionals or whatever.
     *  @example
     *  ```
     *  std::vector<std::unique_ptr<User>> userPointers;
     *  userPointers.push_back(std::make_unique<User>(1, "Eneli"));
     *  auto statement = storage.prepare(insert_range<User>(userPointers.begin(), userPointers.end(), [](const std::unique_ptr<User> &userPointer) -> const User & {
     *      return *userPointer;
     *  }));
     *  storage.execute(statement);
     *  ```
     */
    template<class T, class It, class L>
    internal::insert_range_t<It, L, T> insert_range(It from, It to, L transformer) {
        return {{std::move(from), std::move(to)}, std::move(transformer)};
    }
    /**
     *  Create a replace statement.
     *  T is an object type mapped to a storage.
     *  Usage: storage.replace(myUserInstance);
     *  Parameter obj is accepted by value. If you want to accept it by ref
     *  please use std::ref function: storage.replace(std::ref(myUserInstance));
     */
    template<class T>
    internal::replace_t<T> replace(T obj) {
        return {std::move(obj)};
    }

    /**
     *  Create an insert statement.
     *  T is an object type mapped to a storage.
     *  Usage: storage.insert(myUserInstance);
     *  Parameter obj is accepted by value. If you want to accept it by ref
     *  please use std::ref function: storage.insert(std::ref(myUserInstance));
     */
    template<class T>
    internal::insert_t<T> insert(T obj) {
        return {std::move(obj)};
    }

    /**
     *  Create an explicit insert statement.
     *  T is an object type mapped to a storage.
     *  Cols is columns types aparameter pack. Must contain member pointers
     *  Usage: storage.insert(myUserInstance, columns(&User::id, &User::name));
     *  Parameter obj is accepted by value. If you want to accept it by ref
     *  please use std::ref function: storage.insert(std::ref(myUserInstance), columns(&User::id, &User::name));
     */
    template<class T, class... Cols>
    internal::insert_explicit<T, Cols...> insert(T obj, internal::columns_t<Cols...> cols) {
        return {std::move(obj), std::move(cols)};
    }

    /**
     *  Create a remove statement
     *  T is an object type mapped to a storage.
     *  Usage: remove<User>(5);
     */
    template<class T, class... Ids>
    internal::remove_t<T, Ids...> remove(Ids... ids) {
        std::tuple<Ids...> idsTuple{std::forward<Ids>(ids)...};
        return {move(idsTuple)};
    }

    /**
     *  Create an update statement.
     *  T is an object type mapped to a storage.
     *  Usage: storage.update(myUserInstance);
     *  Parameter obj is accepted by value. If you want to accept it by ref
     *  please use std::ref function: storage.update(std::ref(myUserInstance));
     */
    template<class T>
    internal::update_t<T> update(T obj) {
        return {std::move(obj)};
    }

    /**
     *  Create a get statement.
     *  T is an object type mapped to a storage.
     *  Usage: get<User>(5);
     */
    template<class T, class... Ids>
    internal::get_t<T, Ids...> get(Ids... ids) {
        std::tuple<Ids...> idsTuple{std::forward<Ids>(ids)...};
        return {move(idsTuple)};
    }

    /**
     *  Create a get pointer statement.
     *  T is an object type mapped to a storage.
     *  Usage: get_pointer<User>(5);
     */
    template<class T, class... Ids>
    internal::get_pointer_t<T, Ids...> get_pointer(Ids... ids) {
        std::tuple<Ids...> idsTuple{std::forward<Ids>(ids)...};
        return {move(idsTuple)};
    }

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
    /**
     *  Create a get optional statement.
     *  T is an object type mapped to a storage.
     *  Usage: get_optional<User>(5);
     */
    template<class T, class... Ids>
    internal::get_optional_t<T, Ids...> get_optional(Ids... ids) {
        std::tuple<Ids...> idsTuple{std::forward<Ids>(ids)...};
        return {move(idsTuple)};
    }
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

    /**
     *  Create a remove all statement.
     *  T is an object type mapped to a storage.
     *  Usage: storage.remove_all<User>(...);
     */
    template<class T, class... Args>
    internal::remove_all_t<T, Args...> remove_all(Args... args) {
        using args_tuple = std::tuple<Args...>;
        internal::validate_conditions<args_tuple>();
        args_tuple conditions{std::forward<Args>(args)...};
        return {move(conditions)};
    }

    /**
     *  Create a get all statement.
     *  T is an object type mapped to a storage.
     *  Usage: storage.get_all<User>(...);
     */
    template<class T, class... Args>
    internal::get_all_t<T, std::vector<T>, Args...> get_all(Args... args) {
        using args_tuple = std::tuple<Args...>;
        internal::validate_conditions<args_tuple>();
        args_tuple conditions{std::forward<Args>(args)...};
        return {move(conditions)};
    }

    /**
     *  Create a get all statement.
     *  T is an object type mapped to a storage.
     *  R is a container type. std::vector<T> is default
     *  Usage: storage.get_all<User>(...);
    */
    template<class T, class R, class... Args>
    internal::get_all_t<T, R, Args...> get_all(Args... args) {
        using args_tuple = std::tuple<Args...>;
        internal::validate_conditions<args_tuple>();
        args_tuple conditions{std::forward<Args>(args)...};
        return {move(conditions)};
    }

    /**
     *  Create an update all statement.
     *  Usage: storage.update_all(set(...), ...);
     */
    template<class... Args, class... Wargs>
    internal::update_all_t<internal::set_t<Args...>, Wargs...> update_all(internal::set_t<Args...> set, Wargs... wh) {
        using args_tuple = std::tuple<Wargs...>;
        internal::validate_conditions<args_tuple>();
        args_tuple conditions{std::forward<Wargs>(wh)...};
        return {std::move(set), move(conditions)};
    }

    /**
     *  Create a get all pointer statement.
     *  T is an object type mapped to a storage.
     *  Usage: storage.get_all_pointer<User>(...);
     */
    template<class T, class... Args>
    internal::get_all_pointer_t<T, std::vector<std::unique_ptr<T>>, Args...> get_all_pointer(Args... args) {
        using args_tuple = std::tuple<Args...>;
        internal::validate_conditions<args_tuple>();
        args_tuple conditions{std::forward<Args>(args)...};
        return {move(conditions)};
    }
    /**
     *  Create a get all pointer statement.
     *  T is an object type mapped to a storage.
     *  R is a container return type. std::vector<std::unique_ptr<T>> is default
     *  Usage: storage.get_all_pointer<User>(...);
    */
    template<class T, class R, class... Args>
    internal::get_all_pointer_t<T, R, Args...> get_all_pointer(Args... args) {
        using args_tuple = std::tuple<Args...>;
        internal::validate_conditions<args_tuple>();
        args_tuple conditions{std::forward<Args>(args)...};
        return {move(conditions)};
    }

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
    /**
     *  Create a get all optional statement.
     *  T is an object type mapped to a storage.
     *  Usage: storage.get_all_optional<User>(...);
     */
    template<class T, class... Args>
    internal::get_all_optional_t<T, std::vector<std::optional<T>>, Args...> get_all_optional(Args... args) {
        using args_tuple = std::tuple<Args...>;
        internal::validate_conditions<args_tuple>();
        args_tuple conditions{std::forward<Args>(args)...};
        return {move(conditions)};
    }

    /**
     *  Create a get all optional statement.
     *  T is an object type mapped to a storage.
     *  R is a container return type. std::vector<std::optional<T>> is default
     *  Usage: storage.get_all_optional<User>(...);
     */
    template<class T, class R, class... Args>
    internal::get_all_optional_t<T, R, Args...> get_all_optional(Args... args) {
        using args_tuple = std::tuple<Args...>;
        internal::validate_conditions<args_tuple>();
        args_tuple conditions{std::forward<Args>(args)...};
        return {move(conditions)};
    }
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED
}

// #include "values.h"

// #include "function.h"

// #include "ast/excluded.h"

#include <utility>  //  std::move

namespace sqlite_orm {
    namespace internal {

        template<class T>
        struct excluded_t {
            using expression_type = T;

            expression_type expression;
        };
    }

    template<class T>
    internal::excluded_t<T> excluded(T expression) {
        return {std::move(expression)};
    }
}

// #include "ast/upsert_clause.h"

// #include "ast/where.h"

// #include "ast/into.h"

// #include "ast/group_by.h"

// #include "ast/exists.h"

#include <utility>  //  std::move

// #include "../tags.h"

namespace sqlite_orm {
    namespace internal {

        template<class T>
        struct exists_t : condition_t, negatable_t {
            using expression_type = T;
            using self = exists_t<expression_type>;

            expression_type expression;

            exists_t(expression_type expression_) : expression(std::move(expression_)) {}
        };
    }

    /**
     *  EXISTS(condition).
     *  Example: storage.select(columns(&Agent::code, &Agent::name, &Agent::workingArea, &Agent::comission),
         where(exists(select(asterisk<Customer>(),
         where(is_equal(&Customer::grade, 3) and
         is_equal(&Agent::code, &Customer::agentCode))))),
         order_by(&Agent::comission));
     */
    template<class T>
    internal::exists_t<T> exists(T expression) {
        return {std::move(expression)};
    }
}

namespace sqlite_orm {

    namespace internal {

        /**
         *  ast_iterator accepts any expression and a callable object
         *  which will be called for any node of provided expression.
         *  E.g. if we pass `where(is_equal(5, max(&User::id, 10))` then
         *  callable object will be called with 5, &User::id and 10.
         *  ast_iterator is used mostly in finding literals to be bound to
         *  a statement. To use it just call `iterate_ast(object, callable);`
         *  T is an ast element. E.g. where_t
         */
        template<class T, class SFINAE = void>
        struct ast_iterator {
            using node_type = T;

            /**
             *  L is a callable type. Mostly is a templated lambda
             */
            template<class L>
            void operator()(const T& t, L& lambda) const {
                lambda(t);
            }
        };

        /**
         *  Simplified API
         */
        template<class T, class L>
        void iterate_ast(const T& t, L&& lambda) {
            ast_iterator<T> iterator;
            iterator(t, lambda);
        }

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
        template<class T>
        struct ast_iterator<as_optional_t<T>, void> {
            using node_type = as_optional_t<T>;

            template<class L>
            void operator()(const node_type& node, L& lambda) const {
                iterate_ast(node.value, lambda);
            }
        };
#endif  //  SQLITE_ORM_OPTIONAL_SUPPORTED

        template<class T>
        struct ast_iterator<std::reference_wrapper<T>, void> {
            using node_type = std::reference_wrapper<T>;

            template<class L>
            void operator()(const node_type& expression, L& lambda) const {
                iterate_ast(expression.get(), lambda);
            }
        };

        template<class... Args>
        struct ast_iterator<group_by_t<Args...>, void> {
            using node_type = group_by_t<Args...>;

            template<class L>
            void operator()(const node_type& expression, L& lambda) const {
                iterate_ast(expression.args, lambda);
            }
        };

        template<class T>
        struct ast_iterator<excluded_t<T>, void> {
            using node_type = excluded_t<T>;

            template<class L>
            void operator()(const node_type& expression, L& lambda) const {
                iterate_ast(expression.expression, lambda);
            }
        };

        template<class... TargetArgs, class... ActionsArgs>
        struct ast_iterator<upsert_clause<std::tuple<TargetArgs...>, std::tuple<ActionsArgs...>>, void> {
            using node_type = upsert_clause<std::tuple<TargetArgs...>, std::tuple<ActionsArgs...>>;

            template<class L>
            void operator()(const node_type& expression, L& lambda) const {
                iterate_ast(expression.actions, lambda);
            }
        };

        template<class C>
        struct ast_iterator<where_t<C>, void> {
            using node_type = where_t<C>;

            template<class L>
            void operator()(const node_type& expression, L& lambda) const {
                iterate_ast(expression.expression, lambda);
            }
        };

        template<class T>
        struct ast_iterator<T, std::enable_if_t<is_base_of_template_v<T, binary_condition>>> {
            using node_type = T;

            template<class L>
            void operator()(const node_type& binaryCondition, L& lambda) const {
                iterate_ast(binaryCondition.l, lambda);
                iterate_ast(binaryCondition.r, lambda);
            }
        };

        template<class L, class R, class... Ds>
        struct ast_iterator<binary_operator<L, R, Ds...>, void> {
            using node_type = binary_operator<L, R, Ds...>;

            template<class C>
            void operator()(const node_type& binaryOperator, C& lambda) const {
                iterate_ast(binaryOperator.lhs, lambda);
                iterate_ast(binaryOperator.rhs, lambda);
            }
        };

        template<class... Args>
        struct ast_iterator<columns_t<Args...>, void> {
            using node_type = columns_t<Args...>;

            template<class L>
            void operator()(const node_type& cols, L& lambda) const {
                iterate_ast(cols.columns, lambda);
            }
        };

        template<class L, class A>
        struct ast_iterator<dynamic_in_t<L, A>, void> {
            using node_type = dynamic_in_t<L, A>;

            template<class C>
            void operator()(const node_type& in, C& lambda) const {
                iterate_ast(in.left, lambda);
                iterate_ast(in.argument, lambda);
            }
        };

        template<class L, class... Args>
        struct ast_iterator<in_t<L, Args...>, void> {
            using node_type = in_t<L, Args...>;

            template<class C>
            void operator()(const node_type& in, C& lambda) const {
                iterate_ast(in.left, lambda);
                iterate_ast(in.argument, lambda);
            }
        };

        template<class T>
        struct ast_iterator<std::vector<T>, void> {
            using node_type = std::vector<T>;

            template<class L>
            void operator()(const node_type& vec, L& lambda) const {
                for(auto& i: vec) {
                    iterate_ast(i, lambda);
                }
            }
        };

        template<>
        struct ast_iterator<std::vector<char>, void> {
            using node_type = std::vector<char>;

            template<class L>
            void operator()(const node_type& vec, L& lambda) const {
                lambda(vec);
            }
        };

        template<class T>
        struct ast_iterator<T, std::enable_if_t<is_base_of_template_v<T, compound_operator>>> {
            using node_type = T;

            template<class L>
            void operator()(const node_type& c, L& lambda) const {
                iterate_ast(c.left, lambda);
                iterate_ast(c.right, lambda);
            }
        };

        template<class T>
        struct ast_iterator<into_t<T>, void> {
            using node_type = into_t<T>;

            template<class L>
            void operator()(const node_type& /*node*/, L& /*lambda*/) const {
                //..
            }
        };

        template<class... Args>
        struct ast_iterator<insert_raw_t<Args...>, void> {
            using node_type = insert_raw_t<Args...>;

            template<class L>
            void operator()(const node_type& node, L& lambda) const {
                iterate_ast(node.args, lambda);
            }
        };

        template<class... Args>
        struct ast_iterator<replace_raw_t<Args...>, void> {
            using node_type = replace_raw_t<Args...>;

            template<class L>
            void operator()(const node_type& node, L& lambda) const {
                iterate_ast(node.args, lambda);
            }
        };

        template<class T, class... Args>
        struct ast_iterator<select_t<T, Args...>, void> {
            using node_type = select_t<T, Args...>;

            template<class L>
            void operator()(const node_type& sel, L& lambda) const {
                iterate_ast(sel.col, lambda);
                iterate_ast(sel.conditions, lambda);
            }
        };

        template<class T, class R, class... Args>
        struct ast_iterator<get_all_t<T, R, Args...>, void> {
            using node_type = get_all_t<T, R, Args...>;

            template<class L>
            void operator()(const node_type& get, L& lambda) const {
                iterate_ast(get.conditions, lambda);
            }
        };

        template<class T, class... Args>
        struct ast_iterator<get_all_pointer_t<T, Args...>, void> {
            using node_type = get_all_pointer_t<T, Args...>;

            template<class L>
            void operator()(const node_type& get, L& lambda) const {
                iterate_ast(get.conditions, lambda);
            }
        };

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
        template<class T, class... Args>
        struct ast_iterator<get_all_optional_t<T, Args...>, void> {
            using node_type = get_all_optional_t<T, Args...>;

            template<class L>
            void operator()(const node_type& get, L& lambda) const {
                iterate_ast(get.conditions, lambda);
            }
        };
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

        template<class... Args, class... Wargs>
        struct ast_iterator<update_all_t<set_t<Args...>, Wargs...>, void> {
            using node_type = update_all_t<set_t<Args...>, Wargs...>;

            template<class L>
            void operator()(const node_type& u, L& lambda) const {
                iterate_ast(u.set, lambda);
                iterate_ast(u.conditions, lambda);
            }
        };

        template<class T, class... Args>
        struct ast_iterator<remove_all_t<T, Args...>, void> {
            using node_type = remove_all_t<T, Args...>;

            template<class L>
            void operator()(const node_type& r, L& lambda) const {
                iterate_ast(r.conditions, lambda);
            }
        };

        template<class... Args>
        struct ast_iterator<set_t<Args...>, void> {
            using node_type = set_t<Args...>;

            template<class L>
            void operator()(const node_type& s, L& lambda) const {
                iterate_ast(s.assigns, lambda);
            }
        };

        template<class... Args>
        struct ast_iterator<std::tuple<Args...>, void> {
            using node_type = std::tuple<Args...>;

            template<class L>
            void operator()(const node_type& node, L& lambda) const {
                iterate_tuple(node, [&lambda](auto& v) {
                    iterate_ast(v, lambda);
                });
            }
        };

        template<class T, class... Args>
        struct ast_iterator<group_by_with_having<T, Args...>, void> {
            using node_type = group_by_with_having<T, Args...>;

            template<class L>
            void operator()(const node_type& node, L& lambda) const {
                iterate_ast(node.args, lambda);
                iterate_ast(node.expression, lambda);
            }
        };

        template<class T>
        struct ast_iterator<having_t<T>, void> {
            using node_type = having_t<T>;

            template<class L>
            void operator()(const node_type& node, L& lambda) const {
                iterate_ast(node.expression, lambda);
            }
        };

        template<class T, class E>
        struct ast_iterator<cast_t<T, E>, void> {
            using node_type = cast_t<T, E>;

            template<class L>
            void operator()(const node_type& c, L& lambda) const {
                iterate_ast(c.expression, lambda);
            }
        };

        template<class T>
        struct ast_iterator<exists_t<T>, void> {
            using node_type = exists_t<T>;

            template<class L>
            void operator()(const node_type& node, L& lambda) const {
                iterate_ast(node.expression, lambda);
            }
        };

        template<class A, class T, class E>
        struct ast_iterator<like_t<A, T, E>, void> {
            using node_type = like_t<A, T, E>;

            template<class L>
            void operator()(const node_type& lk, L& lambda) const {
                iterate_ast(lk.arg, lambda);
                iterate_ast(lk.pattern, lambda);
                lk.arg3.apply([&lambda](auto& value) {
                    iterate_ast(value, lambda);
                });
            }
        };

        template<class A, class T>
        struct ast_iterator<glob_t<A, T>, void> {
            using node_type = glob_t<A, T>;

            template<class L>
            void operator()(const node_type& lk, L& lambda) const {
                iterate_ast(lk.arg, lambda);
                iterate_ast(lk.pattern, lambda);
            }
        };

        template<class A, class T>
        struct ast_iterator<between_t<A, T>, void> {
            using node_type = between_t<A, T>;

            template<class L>
            void operator()(const node_type& b, L& lambda) const {
                iterate_ast(b.expr, lambda);
                iterate_ast(b.b1, lambda);
                iterate_ast(b.b2, lambda);
            }
        };

        template<class T>
        struct ast_iterator<named_collate<T>, void> {
            using node_type = named_collate<T>;

            template<class L>
            void operator()(const node_type& col, L& lambda) const {
                iterate_ast(col.expr, lambda);
            }
        };

        template<class C>
        struct ast_iterator<negated_condition_t<C>, void> {
            using node_type = negated_condition_t<C>;

            template<class L>
            void operator()(const node_type& neg, L& lambda) const {
                iterate_ast(neg.c, lambda);
            }
        };

        template<class T>
        struct ast_iterator<is_null_t<T>, void> {
            using node_type = is_null_t<T>;

            template<class L>
            void operator()(const node_type& i, L& lambda) const {
                iterate_ast(i.t, lambda);
            }
        };

        template<class T>
        struct ast_iterator<is_not_null_t<T>, void> {
            using node_type = is_not_null_t<T>;

            template<class L>
            void operator()(const node_type& i, L& lambda) const {
                iterate_ast(i.t, lambda);
            }
        };

        template<class F, class... Args>
        struct ast_iterator<function_call<F, Args...>, void> {
            using node_type = function_call<F, Args...>;

            template<class L>
            void operator()(const node_type& f, L& lambda) const {
                iterate_ast(f.args, lambda);
            }
        };

        template<class R, class S, class... Args>
        struct ast_iterator<built_in_function_t<R, S, Args...>, void> {
            using node_type = built_in_function_t<R, S, Args...>;

            template<class L>
            void operator()(const node_type& node, L& lambda) const {
                iterate_ast(node.args, lambda);
            }
        };

        template<class R, class S, class... Args>
        struct ast_iterator<built_in_aggregate_function_t<R, S, Args...>, void> {
            using node_type = built_in_aggregate_function_t<R, S, Args...>;

            template<class L>
            void operator()(const node_type& node, L& lambda) const {
                iterate_ast(node.args, lambda);
            }
        };

        template<class F, class W>
        struct ast_iterator<filtered_aggregate_function<F, W>, void> {
            using node_type = filtered_aggregate_function<F, W>;

            template<class L>
            void operator()(const node_type& node, L& lambda) const {
                iterate_ast(node.function, lambda);
                iterate_ast(node.where, lambda);
            }
        };

        template<class T, class O>
        struct ast_iterator<left_join_t<T, O>, void> {
            using node_type = left_join_t<T, O>;

            template<class L>
            void operator()(const node_type& j, L& lambda) const {
                iterate_ast(j.constraint, lambda);
            }
        };

        template<class T>
        struct ast_iterator<on_t<T>, void> {
            using node_type = on_t<T>;

            template<class L>
            void operator()(const node_type& o, L& lambda) const {
                iterate_ast(o.arg, lambda);
            }
        };

        // note: not strictly necessary as there's no binding support for USING;
        // we provide it nevertheless, in line with on_t.
        template<class T>
        struct ast_iterator<T, std::enable_if_t<polyfill::is_specialization_of_v<T, using_t>>> {
            using node_type = T;

            template<class L>
            void operator()(const node_type& o, L& lambda) const {
                iterate_ast(o.column, lambda);
            }
        };

        template<class T, class O>
        struct ast_iterator<join_t<T, O>, void> {
            using node_type = join_t<T, O>;

            template<class L>
            void operator()(const node_type& j, L& lambda) const {
                iterate_ast(j.constraint, lambda);
            }
        };

        template<class T, class O>
        struct ast_iterator<left_outer_join_t<T, O>, void> {
            using node_type = left_outer_join_t<T, O>;

            template<class L>
            void operator()(const node_type& j, L& lambda) const {
                iterate_ast(j.constraint, lambda);
            }
        };

        template<class T, class O>
        struct ast_iterator<inner_join_t<T, O>, void> {
            using node_type = inner_join_t<T, O>;

            template<class L>
            void operator()(const node_type& j, L& lambda) const {
                iterate_ast(j.constraint, lambda);
            }
        };

        template<class R, class T, class E, class... Args>
        struct ast_iterator<simple_case_t<R, T, E, Args...>, void> {
            using node_type = simple_case_t<R, T, E, Args...>;

            template<class L>
            void operator()(const node_type& c, L& lambda) const {
                c.case_expression.apply([&lambda](auto& c_) {
                    iterate_ast(c_, lambda);
                });
                iterate_tuple(c.args, [&lambda](auto& pair) {
                    iterate_ast(pair.first, lambda);
                    iterate_ast(pair.second, lambda);
                });
                c.else_expression.apply([&lambda](auto& el) {
                    iterate_ast(el, lambda);
                });
            }
        };

        template<class T, class E>
        struct ast_iterator<as_t<T, E>, void> {
            using node_type = as_t<T, E>;

            template<class L>
            void operator()(const node_type& a, L& lambda) const {
                iterate_ast(a.expression, lambda);
            }
        };

        template<class T, bool OI>
        struct ast_iterator<limit_t<T, false, OI, void>, void> {
            using node_type = limit_t<T, false, OI, void>;

            template<class L>
            void operator()(const node_type& a, L& lambda) const {
                iterate_ast(a.lim, lambda);
            }
        };

        template<class T, class O>
        struct ast_iterator<limit_t<T, true, false, O>, void> {
            using node_type = limit_t<T, true, false, O>;

            template<class L>
            void operator()(const node_type& a, L& lambda) const {
                iterate_ast(a.lim, lambda);
                a.off.apply([&lambda](auto& value) {
                    iterate_ast(value, lambda);
                });
            }
        };

        template<class T, class O>
        struct ast_iterator<limit_t<T, true, true, O>, void> {
            using node_type = limit_t<T, true, true, O>;

            template<class L>
            void operator()(const node_type& a, L& lambda) const {
                a.off.apply([&lambda](auto& value) {
                    iterate_ast(value, lambda);
                });
                iterate_ast(a.lim, lambda);
            }
        };

        template<class T>
        struct ast_iterator<distinct_t<T>, void> {
            using node_type = distinct_t<T>;

            template<class L>
            void operator()(const node_type& a, L& lambda) const {
                iterate_ast(a.value, lambda);
            }
        };

        template<class T>
        struct ast_iterator<all_t<T>, void> {
            using node_type = all_t<T>;

            template<class L>
            void operator()(const node_type& a, L& lambda) const {
                iterate_ast(a.value, lambda);
            }
        };

        template<class T>
        struct ast_iterator<bitwise_not_t<T>, void> {
            using node_type = bitwise_not_t<T>;

            template<class L>
            void operator()(const node_type& a, L& lambda) const {
                iterate_ast(a.argument, lambda);
            }
        };

        template<class... Args>
        struct ast_iterator<values_t<Args...>, void> {
            using node_type = values_t<Args...>;

            template<class L>
            void operator()(const node_type& node, L& lambda) const {
                iterate_ast(node.tuple, lambda);
            }
        };

        template<class T>
        struct ast_iterator<dynamic_values_t<T>, void> {
            using node_type = dynamic_values_t<T>;

            template<class L>
            void operator()(const node_type& node, L& lambda) const {
                iterate_ast(node.vector, lambda);
            }
        };

        /**
         *  Column alias or literal
         */
        template<class T>
        struct ast_iterator<
            T,
            std::enable_if_t<polyfill::disjunction_v<polyfill::is_specialization_of<T, alias_holder>,
                                                     polyfill::is_specialization_of<T, literal_holder>>>> {
            using node_type = T;

            template<class L>
            void operator()(const node_type& /*node*/, L& /*lambda*/) const {}
        };

        template<class E>
        struct ast_iterator<order_by_t<E>, void> {
            using node_type = order_by_t<E>;

            template<class L>
            void operator()(const node_type& node, L& lambda) const {
                iterate_ast(node.expression, lambda);
            }
        };

        template<class T>
        struct ast_iterator<collate_t<T>, void> {
            using node_type = collate_t<T>;

            template<class L>
            void operator()(const node_type& node, L& lambda) const {
                iterate_ast(node.expr, lambda);
            }
        };

    }
}

// #include "prepared_statement.h"

// #include "connection_holder.h"

namespace sqlite_orm {

    namespace internal {

        /**
         * This class does not related to SQL view. This is a container like class which is returned by
         * by storage_t::iterate function. This class contains STL functions:
         *  -   size_t size()
         *  -   bool empty()
         *  -   iterator end()
         *  -   iterator begin()
         *  All these functions are not right const cause all of them may open SQLite connections.
         */
        template<class T, class S, class... Args>
        struct view_t {
            using mapped_type = T;
            using storage_type = S;
            using self = view_t<T, S, Args...>;

            storage_type& storage;
            connection_ref connection;
            get_all_t<T, std::vector<T>, Args...> args;

            view_t(storage_type& stor, decltype(connection) conn, Args&&... args_) :
                storage(stor), connection(std::move(conn)), args{std::make_tuple(std::forward<Args>(args_)...)} {}

            size_t size() {
                return this->storage.template count<T>();
            }

            bool empty() {
                return !this->size();
            }

            iterator_t<self> begin() {
                using context_t = serializer_context<typename storage_type::impl_type>;
                context_t context{obtain_const_impl(this->storage)};
                context.skip_table_name = false;
                context.replace_bindable_with_question = true;
                auto query = serialize(this->args, context);

                sqlite3* db = this->connection.get();
                sqlite3_stmt* stmt = nullptr;
                int ret = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
                if(ret != SQLITE_OK) {
                    throw_translated_sqlite_error(db);
                }
                iterate_ast(this->args.conditions, conditional_binder{stmt});
                return {stmt, *this};
            }

            iterator_t<self> end() {
                return {};
            }
        };
    }
}

// #include "ast_iterator.h"

// #include "storage_base.h"

#include <sqlite3.h>
#include <functional>  //  std::function, std::bind
#include <string>  //  std::string
#include <sstream>  //  std::stringstream
#include <utility>  //  std::move
#include <system_error>  //  std::system_error
#include <vector>  //  std::vector
#include <memory>  //  std::make_shared, std::shared_ptr
#include <map>  //  std::map
#include <type_traits>  //  std::decay, std::is_same

// #include "functional/cxx_universal.h"

// #include "functional/static_magic.h"

// #include "tuple_helper/tuple_iteration.h"

// #include "pragma.h"

#include <sqlite3.h>
#include <string>  //  std::string
#include <functional>  //  std::function
#include <memory>  // std::shared_ptr
#include <vector>  //  std::vector
#include <sstream>

// #include "error_code.h"

// #include "row_extractor.h"

// #include "journal_mode.h"

// #include "connection_holder.h"

// #include "util.h"

// #include "serializing_util.h"

#include <type_traits>  //  std::index_sequence
#include <tuple>
#include <array>
#include <string>
#include <ostream>
#include <utility>  //  std::exchange, std::tuple_size
#if __cplusplus >= 202002L && __cpp_lib_concepts
#include <string_view>
#include <algorithm>  //  std::find
#endif

// #include "functional/cxx_universal.h"

// #include "functional/cxx_polyfill.h"

// #include "error_code.h"

// #include "serializer_context.h"

// #include "util.h"

namespace sqlite_orm {
    namespace internal {
        template<class O>
        struct order_by_t;

        template<class T, class I>
        std::string serialize(const T& t, const serializer_context<I>& context);

        template<class T, class Ctx>
        std::string serialize_order_by(const T& t, const Ctx& context);

#if __cplusplus >= 202002L &&                                                                                          \
    __cpp_lib_concepts  //  contiguous iterator ranges depend on contiguous_iterator, sized_sentinel_for in all major implementations
        inline void stream_sql_escaped(std::ostream& os, const std::string& str, char char2Escape) {
            for(std::string::const_iterator it = str.cbegin(), next; true; it = next + 1) {
                next = std::find(it, str.cend(), char2Escape);
                os << std::string_view{it, next};

                if(next == str.cend()) [[likely]] {
                    break;
                }
                os << std::string(2, char2Escape);
            }
        }
#else
            inline void stream_sql_escaped(std::ostream& os, const std::string& str, char char2Escape) {
                if(str.find(char2Escape) == str.npos) {
                    os << str;
                } else {
                    for(char c: str) {
                        if(c == char2Escape) {
                            os << char2Escape;
                        }
                        os << c;
                    }
                }
            }
#endif

        inline void stream_identifier(std::ostream& ss,
                                      const std::string& qualifier,
                                      const std::string& identifier,
                                      const std::string& alias) {
            constexpr char quoteChar = '"';
            constexpr char qualified[] = {quoteChar, '.', '\0'};
            constexpr char aliased[] = {' ', quoteChar, '\0'};

            // note: In practice, escaping double quotes in identifiers is arguably overkill,
            // but since the SQLite grammar allows it, it's better to be safe than sorry.

            if(!qualifier.empty()) {
                ss << quoteChar;
                stream_sql_escaped(ss, qualifier, quoteChar);
                ss << qualified;
            }
            {
                ss << quoteChar;
                stream_sql_escaped(ss, identifier, quoteChar);
                ss << quoteChar;
            }
            if(!alias.empty()) {
                ss << aliased;
                stream_sql_escaped(ss, alias, quoteChar);
                ss << quoteChar;
            }
        }

        inline void stream_identifier(std::ostream& ss, const std::string& identifier, const std::string& alias) {
            return stream_identifier(ss, std::string{}, identifier, alias);
        }

        inline void stream_identifier(std::ostream& ss, const std::string& identifier) {
            return stream_identifier(ss, std::string{}, identifier, std::string{});
        }

        template<typename Tpl, size_t... Is>
        void stream_identifier(std::ostream& ss, const Tpl& tpl, std::index_sequence<Is...>) {
            static_assert(sizeof...(Is) > 0 && sizeof...(Is) <= 3, "");
            return stream_identifier(ss, std::get<Is>(tpl)...);
        }

        template<typename Tpl, std::enable_if_t<polyfill::is_detected_v<type_t, std::tuple_size<Tpl>>, bool> = true>
        void stream_identifier(std::ostream& ss, const Tpl& tpl) {
            return stream_identifier(ss, tpl, std::make_index_sequence<std::tuple_size<Tpl>::value>{});
        }

        enum class stream_as {
            conditions_tuple,
            actions_tuple,
            expressions_tuple,
            dynamic_expressions,
            serialized,
            identifier,
            identifiers,
            values_placeholders,
            table_columns,
            non_generated_columns,
            column_field_values_excluding,
            mapped_columns_expressions,
        };

        template<stream_as mode>
        struct streaming {
            template<class... Ts>
            auto operator()(const Ts&... ts) const {
                return std::forward_as_tuple(*this, ts...);
            }

            template<size_t... Idx>
            constexpr std::index_sequence<1u + Idx...> offset_index(std::index_sequence<Idx...>) const {
                return {};
            }
        };
        constexpr streaming<stream_as::conditions_tuple> streaming_conditions_tuple{};
        constexpr streaming<stream_as::actions_tuple> streaming_actions_tuple{};
        constexpr streaming<stream_as::expressions_tuple> streaming_expressions_tuple{};
        constexpr streaming<stream_as::dynamic_expressions> streaming_dynamic_expressions{};
        constexpr streaming<stream_as::serialized> streaming_serialized{};
        constexpr streaming<stream_as::identifier> streaming_identifier{};
        constexpr streaming<stream_as::identifiers> streaming_identifiers{};
        constexpr streaming<stream_as::values_placeholders> streaming_values_placeholders{};
        constexpr streaming<stream_as::table_columns> streaming_table_column_names{};
        constexpr streaming<stream_as::non_generated_columns> streaming_non_generated_column_names{};
        constexpr streaming<stream_as::column_field_values_excluding> streaming_column_field_values_excluding{};
        constexpr streaming<stream_as::mapped_columns_expressions> streaming_mapped_columns_expressions{};

        // serialize and stream a tuple of condition expressions;
        // space + space-separated
        template<class T, class Ctx>
        std::ostream& operator<<(std::ostream& ss,
                                 std::tuple<const streaming<stream_as::conditions_tuple>&, T, Ctx> tpl) {
            const auto& conditions = get<1>(tpl);
            auto& context = get<2>(tpl);

            iterate_tuple(conditions, [&ss, &context](auto& c) {
                ss << " " << serialize(c, context);
            });
            return ss;
        }

        // serialize and stream a tuple of action expressions;
        // space-separated
        template<class T, class Ctx>
        std::ostream& operator<<(std::ostream& ss, std::tuple<const streaming<stream_as::actions_tuple>&, T, Ctx> tpl) {
            const auto& actions = get<1>(tpl);
            auto& context = get<2>(tpl);

            iterate_tuple(actions, [&ss, &context, first = true](auto& action) mutable {
                constexpr std::array<const char*, 2> sep = {" ", ""};
                ss << sep[std::exchange(first, false)]  ///
                   << serialize(action, context);
            });
            return ss;
        }

        // serialize and stream a tuple of expressions;
        // comma-separated
        template<class T, class Ctx>
        std::ostream& operator<<(std::ostream& ss,
                                 std::tuple<const streaming<stream_as::expressions_tuple>&, T, Ctx> tpl) {
            const auto& args = get<1>(tpl);
            auto& context = get<2>(tpl);

            iterate_tuple(args, [&ss, &context, first = true](auto& arg) mutable {
                constexpr std::array<const char*, 2> sep = {", ", ""};
                ss << sep[std::exchange(first, false)]  ///
                   << serialize(arg, context);
            });
            return ss;
        }

        // serialize and stream multi_order_by arguments;
        // comma-separated
        template<class... Os, class Ctx>
        std::ostream& operator<<(
            std::ostream& ss,
            std::tuple<const streaming<stream_as::expressions_tuple>&, const std::tuple<order_by_t<Os>...>&, Ctx> tpl) {
            const auto& args = get<1>(tpl);
            auto& context = get<2>(tpl);

            iterate_tuple(args, [&ss, &context, first = true](auto& arg) mutable {
                constexpr std::array<const char*, 2> sep = {", ", ""};
                ss << sep[std::exchange(first, false)] << serialize_order_by(arg, context);
            });
            return ss;
        }

        // serialize and stream a vector of expressions;
        // comma-separated
        template<class C, class Ctx>
        std::ostream& operator<<(std::ostream& ss,
                                 std::tuple<const streaming<stream_as::dynamic_expressions>&, C, Ctx> tpl) {
            const auto& args = get<1>(tpl);
            auto& context = get<2>(tpl);

            constexpr std::array<const char*, 2> sep = {", ", ""};
            for(size_t i = 0, first = true; i < args.size(); ++i) {
                ss << sep[std::exchange(first, false)]  ///
                   << serialize(args[i], context);
            }
            return ss;
        }

        // stream a vector of already serialized strings;
        // comma-separated
        template<class C>
        std::ostream& operator<<(std::ostream& ss, std::tuple<const streaming<stream_as::serialized>&, C> tpl) {
            const auto& strings = get<1>(tpl);

            constexpr std::array<const char*, 2> sep = {", ", ""};
            for(size_t i = 0, first = true; i < strings.size(); ++i) {
                ss << sep[std::exchange(first, false)]  ///
                   << strings[i];
            }
            return ss;
        }

        // stream an identifier described by a variadic string pack, which is one of:
        // 1. identifier
        // 2. identifier, alias
        // 3. qualifier, identifier, alias
        template<class... Strings>
        std::ostream& operator<<(std::ostream& ss,
                                 std::tuple<const streaming<stream_as::identifier>&, Strings...> tpl) {
            stream_identifier(ss, tpl, streaming_identifier.offset_index(std::index_sequence_for<Strings...>{}));
            return ss;
        }

        // stream a container of identifiers described by a string or a tuple, which is one of:
        // 1. identifier
        // 1. tuple(identifier)
        // 2. tuple(identifier, alias), pair(identifier, alias)
        // 3. tuple(qualifier, identifier, alias)
        //
        // comma-separated
        template<class C>
        std::ostream& operator<<(std::ostream& ss, std::tuple<const streaming<stream_as::identifiers>&, C> tpl) {
            const auto& identifiers = get<1>(tpl);

            constexpr std::array<const char*, 2> sep = {", ", ""};
            bool first = true;
            for(auto& identifier: identifiers) {
                ss << sep[std::exchange(first, false)];
                stream_identifier(ss, identifier);
            }
            return ss;
        }

        // stream placeholders as part of a values clause
        template<class... Ts>
        std::ostream& operator<<(std::ostream& ss,
                                 std::tuple<const streaming<stream_as::values_placeholders>&, Ts...> tpl) {
            const size_t& columnsCount = get<1>(tpl);
            const ptrdiff_t& valuesCount = get<2>(tpl);

            if(!valuesCount || !columnsCount) {
                return ss;
            }

            std::string result;
            result.reserve((1 + (columnsCount * 1) + (columnsCount * 2 - 2) + 1) * valuesCount + (valuesCount * 2 - 2));

            constexpr std::array<const char*, 2> sep = {", ", ""};
            for(ptrdiff_t i = 0, first = true; i < valuesCount; ++i) {
                result += sep[std::exchange(first, false)];
                result += "(";
                for(size_t i = 0, first = true; i < columnsCount; ++i) {
                    result += sep[std::exchange(first, false)];
                    result += "?";
                }
                result += ")";
            }
            ss << result;
            return ss;
        }

        // stream a table's column identifiers, possibly qualified;
        // comma-separated
        template<class Table>
        std::ostream& operator<<(std::ostream& ss,
                                 std::tuple<const streaming<stream_as::table_columns>&, Table, const bool&> tpl) {
            const auto& table = get<1>(tpl);
            const bool& qualified = get<2>(tpl);

            table.for_each_column(
                [&ss, &tableName = qualified ? table.name : std::string{}, first = true](auto& column) mutable {
                    constexpr std::array<const char*, 2> sep = {", ", ""};
                    ss << sep[std::exchange(first, false)];
                    stream_identifier(ss, tableName, column.name, std::string{});
                });
            return ss;
        }

        // stream a table's non-generated column identifiers, unqualified;
        // comma-separated
        template<class Table>
        std::ostream& operator<<(std::ostream& ss,
                                 std::tuple<const streaming<stream_as::non_generated_columns>&, Table> tpl) {
            const auto& table = get<1>(tpl);

            table.template for_each_column_excluding<is_generated_always>([&ss, first = true](auto& column) mutable {
                constexpr std::array<const char*, 2> sep = {", ", ""};
                ss << sep[std::exchange(first, false)];
                stream_identifier(ss, column.name);
            });
            return ss;
        }

        // stream a table's non-generated column identifiers, unqualified;
        // comma-separated
        template<class PredFnCls, class L, class Ctx, class Obj>
        std::ostream&
        operator<<(std::ostream& ss,
                   std::tuple<const streaming<stream_as::column_field_values_excluding>&, PredFnCls, L, Ctx, Obj> tpl) {
            using check_if_excluded = polyfill::remove_cvref_t<std::tuple_element_t<1, decltype(tpl)>>;
            auto& excluded = get<2>(tpl);
            auto& context = get<3>(tpl);
            auto& object = get<4>(tpl);
            using object_type = polyfill::remove_cvref_t<decltype(object)>;
            auto& tImpl = pick_impl<object_type>(context.impl);

            tImpl.table.template for_each_column_excluding<check_if_excluded>(
                [&ss, &excluded, &context, &object, first = true](auto& column) mutable {
                    if(excluded(column)) {
                        return;
                    }

                    constexpr std::array<const char*, 2> sep = {", ", ""};
                    ss << sep[std::exchange(first, false)]  ///
                       << serialize(polyfill::invoke(column.member_pointer, object), context);
                });
            return ss;
        }

        // stream a tuple of mapped columns (which are member pointers or column pointers);
        // comma-separated
        template<class T, class Ctx>
        std::ostream& operator<<(std::ostream& ss,
                                 std::tuple<const streaming<stream_as::mapped_columns_expressions>&, T, Ctx> tpl) {
            const auto& columns = get<1>(tpl);
            auto& context = get<2>(tpl);

            iterate_tuple(columns, [&ss, &context, first = true](auto& column) mutable {
                const std::string* columnName = find_column_name(context.impl, column);
                if(!columnName) {
                    throw std::system_error{orm_error_code::column_not_found};
                }

                constexpr std::array<const char*, 2> sep = {", ", ""};
                ss << sep[std::exchange(first, false)];
                stream_identifier(ss, *columnName);
            });
            return ss;
        }
    }
}

namespace sqlite_orm {

    namespace internal {
        struct storage_base;

        template<class T>
        int getPragmaCallback(void* data, int argc, char** argv, char** x) {
            return extract_single_value<T>(data, argc, argv, x);
        }

        template<>
        inline int getPragmaCallback<std::vector<std::string>>(void* data, int argc, char** argv, char**) {
            auto& res = *(std::vector<std::string>*)data;
            res.reserve(argc);
            for(decltype(argc) i = 0; i < argc; ++i) {
                auto rowString = row_extractor<std::string>().extract(argv[i]);
                res.push_back(move(rowString));
            }
            return 0;
        }

        struct pragma_t {
            using get_connection_t = std::function<internal::connection_ref()>;

            pragma_t(get_connection_t get_connection_) : get_connection(move(get_connection_)) {}

            void busy_timeout(int value) {
                this->set_pragma("busy_timeout", value);
            }

            int busy_timeout() {
                return this->get_pragma<int>("busy_timeout");
            }

            sqlite_orm::journal_mode journal_mode() {
                return this->get_pragma<sqlite_orm::journal_mode>("journal_mode");
            }

            void journal_mode(sqlite_orm::journal_mode value) {
                this->_journal_mode = -1;
                this->set_pragma("journal_mode", value);
                this->_journal_mode = static_cast<decltype(this->_journal_mode)>(value);
            }

            /**
             *  https://www.sqlite.org/pragma.html#pragma_application_id
             */
            int application_id() {
                return this->get_pragma<int>("application_id");
            }

            /**
             *  https://www.sqlite.org/pragma.html#pragma_application_id
             */
            void application_id(int value) {
                this->set_pragma("application_id", value);
            }

            int synchronous() {
                return this->get_pragma<int>("synchronous");
            }

            void synchronous(int value) {
                this->_synchronous = -1;
                this->set_pragma("synchronous", value);
                this->_synchronous = value;
            }

            int user_version() {
                return this->get_pragma<int>("user_version");
            }

            void user_version(int value) {
                this->set_pragma("user_version", value);
            }

            int auto_vacuum() {
                return this->get_pragma<int>("auto_vacuum");
            }

            void auto_vacuum(int value) {
                this->set_pragma("auto_vacuum", value);
            }

            std::vector<std::string> integrity_check() {
                return this->get_pragma<std::vector<std::string>>("integrity_check");
            }

            template<class T>
            std::vector<std::string> integrity_check(T table_name) {
                std::ostringstream ss;
                ss << "integrity_check(" << table_name << ")" << std::flush;
                return this->get_pragma<std::vector<std::string>>(ss.str());
            }

            std::vector<std::string> integrity_check(int n) {
                std::ostringstream ss;
                ss << "integrity_check(" << n << ")" << std::flush;
                return this->get_pragma<std::vector<std::string>>(ss.str());
            }

            // will include generated columns in response as opposed to table_info
            std::vector<sqlite_orm::table_xinfo> table_xinfo(const std::string& tableName) const {
                auto connection = this->get_connection();

                std::vector<sqlite_orm::table_xinfo> result;
                std::ostringstream ss;
                ss << "PRAGMA "
                      "table_xinfo("
                   << streaming_identifier(tableName) << ")" << std::flush;
                perform_exec(
                    connection.get(),
                    ss.str(),
                    [](void* data, int argc, char** argv, char**) -> int {
                        auto& res = *(std::vector<sqlite_orm::table_xinfo>*)data;
                        if(argc) {
                            auto index = 0;
                            auto cid = std::atoi(argv[index++]);
                            std::string name = argv[index++];
                            std::string type = argv[index++];
                            bool notnull = !!std::atoi(argv[index++]);
                            std::string dflt_value = argv[index] ? argv[index] : "";
                            ++index;
                            auto pk = std::atoi(argv[index++]);
                            auto hidden = std::atoi(argv[index++]);
                            res.emplace_back(cid, name, type, notnull, dflt_value, pk, hidden);
                        }
                        return 0;
                    },
                    &result);
                return result;
            }

            std::vector<sqlite_orm::table_info> table_info(const std::string& tableName) const {
                auto connection = this->get_connection();

                std::ostringstream ss;
                ss << "PRAGMA "
                      "table_info("
                   << streaming_identifier(tableName) << ")" << std::flush;
                std::vector<sqlite_orm::table_info> result;
                perform_exec(
                    connection.get(),
                    ss.str(),
                    [](void* data, int argc, char** argv, char**) -> int {
                        auto& res = *(std::vector<sqlite_orm::table_info>*)data;
                        if(argc) {
                            auto index = 0;
                            auto cid = std::atoi(argv[index++]);
                            std::string name = argv[index++];
                            std::string type = argv[index++];
                            bool notnull = !!std::atoi(argv[index++]);
                            std::string dflt_value = argv[index] ? argv[index] : "";
                            ++index;
                            auto pk = std::atoi(argv[index++]);
                            res.emplace_back(cid, move(name), move(type), notnull, move(dflt_value), pk);
                        }
                        return 0;
                    },
                    &result);
                return result;
            }

          private:
            friend struct storage_base;

            int _synchronous = -1;
            signed char _journal_mode = -1;  //  if != -1 stores static_cast<sqlite_orm::journal_mode>(journal_mode)
            get_connection_t get_connection;

            template<class T>
            T get_pragma(const std::string& name) {
                auto connection = this->get_connection();
                T result;
                perform_exec(connection.get(), "PRAGMA " + name, getPragmaCallback<T>, &result);
                return result;
            }

            /**
             *  Yevgeniy Zakharov: I wanted to refactor this function with statements and value bindings
             *  but it turns out that bindings in pragma statements are not supported.
             */
            template<class T>
            void set_pragma(const std::string& name, const T& value, sqlite3* db = nullptr) {
                auto con = this->get_connection();
                if(!db) {
                    db = con.get();
                }
                std::stringstream ss;
                ss << "PRAGMA " << name << " = " << value << std::flush;
                perform_void_exec(db, ss.str());
            }

            void set_pragma(const std::string& name, const sqlite_orm::journal_mode& value, sqlite3* db = nullptr) {
                auto con = this->get_connection();
                if(!db) {
                    db = con.get();
                }
                std::stringstream ss;
                ss << "PRAGMA " << name << " = " << to_string(value) << std::flush;
                perform_void_exec(db, ss.str());
            }
        };
    }
}

// #include "limit_accessor.h"

#include <sqlite3.h>
#include <map>  //  std::map
#include <functional>  //  std::function
#include <memory>  //  std::shared_ptr

// #include "connection_holder.h"

namespace sqlite_orm {

    namespace internal {

        struct limit_accessor {
            using get_connection_t = std::function<connection_ref()>;

            limit_accessor(get_connection_t get_connection_) : get_connection(std::move(get_connection_)) {}

            int length() {
                return this->get(SQLITE_LIMIT_LENGTH);
            }

            void length(int newValue) {
                this->set(SQLITE_LIMIT_LENGTH, newValue);
            }

            int sql_length() {
                return this->get(SQLITE_LIMIT_SQL_LENGTH);
            }

            void sql_length(int newValue) {
                this->set(SQLITE_LIMIT_SQL_LENGTH, newValue);
            }

            int column() {
                return this->get(SQLITE_LIMIT_COLUMN);
            }

            void column(int newValue) {
                this->set(SQLITE_LIMIT_COLUMN, newValue);
            }

            int expr_depth() {
                return this->get(SQLITE_LIMIT_EXPR_DEPTH);
            }

            void expr_depth(int newValue) {
                this->set(SQLITE_LIMIT_EXPR_DEPTH, newValue);
            }

            int compound_select() {
                return this->get(SQLITE_LIMIT_COMPOUND_SELECT);
            }

            void compound_select(int newValue) {
                this->set(SQLITE_LIMIT_COMPOUND_SELECT, newValue);
            }

            int vdbe_op() {
                return this->get(SQLITE_LIMIT_VDBE_OP);
            }

            void vdbe_op(int newValue) {
                this->set(SQLITE_LIMIT_VDBE_OP, newValue);
            }

            int function_arg() {
                return this->get(SQLITE_LIMIT_FUNCTION_ARG);
            }

            void function_arg(int newValue) {
                this->set(SQLITE_LIMIT_FUNCTION_ARG, newValue);
            }

            int attached() {
                return this->get(SQLITE_LIMIT_ATTACHED);
            }

            void attached(int newValue) {
                this->set(SQLITE_LIMIT_ATTACHED, newValue);
            }

            int like_pattern_length() {
                return this->get(SQLITE_LIMIT_LIKE_PATTERN_LENGTH);
            }

            void like_pattern_length(int newValue) {
                this->set(SQLITE_LIMIT_LIKE_PATTERN_LENGTH, newValue);
            }

            int variable_number() {
                return this->get(SQLITE_LIMIT_VARIABLE_NUMBER);
            }

            void variable_number(int newValue) {
                this->set(SQLITE_LIMIT_VARIABLE_NUMBER, newValue);
            }

            int trigger_depth() {
                return this->get(SQLITE_LIMIT_TRIGGER_DEPTH);
            }

            void trigger_depth(int newValue) {
                this->set(SQLITE_LIMIT_TRIGGER_DEPTH, newValue);
            }

#if SQLITE_VERSION_NUMBER >= 3008007
            int worker_threads() {
                return this->get(SQLITE_LIMIT_WORKER_THREADS);
            }

            void worker_threads(int newValue) {
                this->set(SQLITE_LIMIT_WORKER_THREADS, newValue);
            }
#endif

          protected:
            get_connection_t get_connection;

            friend struct storage_base;

            /**
             *  Stores limit set between connections.
             */
            std::map<int, int> limits;

            int get(int id) {
                auto connection = this->get_connection();
                return sqlite3_limit(connection.get(), id, -1);
            }

            void set(int id, int newValue) {
                this->limits[id] = newValue;
                auto connection = this->get_connection();
                sqlite3_limit(connection.get(), id, newValue);
            }
        };
    }
}

// #include "transaction_guard.h"

#include <functional>  //  std::function
#include <utility>  //  std::move

// #include "connection_holder.h"

namespace sqlite_orm {

    namespace internal {

        /**
         *  Class used as a guard for a transaction. Calls `ROLLBACK` in destructor.
         *  Has explicit `commit()` and `rollback()` functions. After explicit function is fired
         *  guard won't do anything in d-tor. Also you can set `commit_on_destroy` to true to
         *  make it call `COMMIT` on destroy.
         * 
         *  Note: The guard's destructor is explicitly marked as potentially throwing,
         *  so exceptions that occur during commit or rollback are propagated to the caller.
         */
        struct transaction_guard_t {
            /**
             *  This is a public lever to tell a guard what it must do in its destructor
             *  if `gotta_fire` is true
             */
            bool commit_on_destroy = false;

            transaction_guard_t(connection_ref connection_,
                                std::function<void()> commit_func_,
                                std::function<void()> rollback_func_) :
                connection(std::move(connection_)),
                commit_func(move(commit_func_)), rollback_func(move(rollback_func_)) {}

            transaction_guard_t(transaction_guard_t&& other) :
                commit_on_destroy(other.commit_on_destroy), connection(std::move(other.connection)),
                commit_func(move(other.commit_func)), rollback_func(move(other.rollback_func)),
                gotta_fire(other.gotta_fire) {
                other.gotta_fire = false;
            }

            ~transaction_guard_t() noexcept(false) {
                if(this->gotta_fire) {
                    if(this->commit_on_destroy) {
                        this->commit_func();
                    } else {
                        this->rollback_func();
                    }
                }
            }

            transaction_guard_t& operator=(transaction_guard_t&&) = delete;

            /**
             *  Call `COMMIT` explicitly. After this call
             *  guard will not call `COMMIT` or `ROLLBACK`
             *  in its destructor.
             */
            void commit() {
                this->gotta_fire = false;
                this->commit_func();
            }

            /**
             *  Call `ROLLBACK` explicitly. After this call
             *  guard will not call `COMMIT` or `ROLLBACK`
             *  in its destructor.
             */
            void rollback() {
                this->gotta_fire = false;
                this->rollback_func();
            }

          protected:
            connection_ref connection;
            std::function<void()> commit_func;
            std::function<void()> rollback_func;
            bool gotta_fire = true;
        };
    }
}

// #include "statement_finalizer.h"

// #include "row_extractor.h"

// #include "connection_holder.h"

// #include "backup.h"

#include <sqlite3.h>
#include <system_error>  //  std::system_error
#include <string>  //  std::string
#include <memory>
#include <utility>  //  std::move, std::exchange

// #include "error_code.h"

// #include "connection_holder.h"

namespace sqlite_orm {

    namespace internal {

        /**
         *  A backup class. Don't construct it as is, call storage.make_backup_from or storage.make_backup_to instead.
         *  An instance of this class represents a wrapper around sqlite3_backup pointer. Use this class
         *  to have maximum control on a backup operation. In case you need a single backup in one line you
         *  can skip creating a backup_t instance and just call storage.backup_from or storage.backup_to function.
         */
        struct backup_t {
            backup_t(connection_ref to_,
                     const std::string& zDestName,
                     connection_ref from_,
                     const std::string& zSourceName,
                     std::unique_ptr<connection_holder> holder_) :
                handle(sqlite3_backup_init(to_.get(), zDestName.c_str(), from_.get(), zSourceName.c_str())),
                holder(move(holder_)), to(to_), from(from_) {
                if(!this->handle) {
                    throw std::system_error{orm_error_code::failed_to_init_a_backup};
                }
            }

            backup_t(backup_t&& other) :
                handle(std::exchange(other.handle, nullptr)), holder(move(other.holder)), to(other.to),
                from(other.from) {}

            ~backup_t() {
                if(this->handle) {
                    (void)sqlite3_backup_finish(this->handle);
                }
            }

            /**
             *  Calls sqlite3_backup_step with pages argument
             */
            int step(int pages) {
                return sqlite3_backup_step(this->handle, pages);
            }

            /**
             *  Returns sqlite3_backup_remaining result
             */
            int remaining() const {
                return sqlite3_backup_remaining(this->handle);
            }

            /**
             *  Returns sqlite3_backup_pagecount result
             */
            int pagecount() const {
                return sqlite3_backup_pagecount(this->handle);
            }

          protected:
            sqlite3_backup* handle = nullptr;
            std::unique_ptr<connection_holder> holder;
            connection_ref to;
            connection_ref from;
        };
    }
}

// #include "function.h"

// #include "values_to_tuple.h"

#include <sqlite3.h>
#include <tuple>  //  std::get, std::tuple_element

// #include "row_extractor.h"

// #include "arg_values.h"

#include <sqlite3.h>

// #include "row_extractor.h"

namespace sqlite_orm {

    struct arg_value {

        arg_value() : arg_value(nullptr) {}

        arg_value(sqlite3_value* value_) : value(value_) {}

        template<class T>
        T get() const {
            return row_extractor<T>().extract(this->value);
        }

        bool is_null() const {
            auto type = sqlite3_value_type(this->value);
            return type == SQLITE_NULL;
        }

        bool is_text() const {
            auto type = sqlite3_value_type(this->value);
            return type == SQLITE_TEXT;
        }

        bool is_integer() const {
            auto type = sqlite3_value_type(this->value);
            return type == SQLITE_INTEGER;
        }

        bool is_float() const {
            auto type = sqlite3_value_type(this->value);
            return type == SQLITE_FLOAT;
        }

        bool is_blob() const {
            auto type = sqlite3_value_type(this->value);
            return type == SQLITE_BLOB;
        }

        bool empty() const {
            return this->value == nullptr;
        }

      private:
        sqlite3_value* value = nullptr;
    };

    struct arg_values {

        struct iterator {

            iterator(const arg_values& container_, int index_) :
                container(container_), index(index_),
                currentValue(index_ < int(container_.size()) ? container_[index_] : arg_value()) {}

            iterator& operator++() {
                ++this->index;
                if(this->index < int(this->container.size())) {
                    this->currentValue = this->container[this->index];
                } else {
                    this->currentValue = {};
                }
                return *this;
            }

            iterator operator++(int) {
                auto res = *this;
                ++this->index;
                if(this->index < int(this->container.size())) {
                    this->currentValue = this->container[this->index];
                } else {
                    this->currentValue = {};
                }
                return res;
            }

            arg_value operator*() const {
                if(this->index < int(this->container.size()) && this->index >= 0) {
                    return this->currentValue;
                } else {
                    throw std::system_error{orm_error_code::index_is_out_of_bounds};
                }
            }

            arg_value* operator->() const {
                return &this->currentValue;
            }

            bool operator==(const iterator& other) const {
                return &other.container == &this->container && other.index == this->index;
            }

            bool operator!=(const iterator& other) const {
                return !(*this == other);
            }

          private:
            const arg_values& container;
            int index = 0;
            mutable arg_value currentValue;
        };

        arg_values() : arg_values(0, nullptr) {}

        arg_values(int argsCount_, sqlite3_value** values_) : argsCount(argsCount_), values(values_) {}

        size_t size() const {
            return this->argsCount;
        }

        bool empty() const {
            return 0 == this->argsCount;
        }

        arg_value operator[](int index) const {
            if(index < this->argsCount && index >= 0) {
                sqlite3_value* value = this->values[index];
                return {value};
            } else {
                throw std::system_error{orm_error_code::index_is_out_of_bounds};
            }
        }

        arg_value at(int index) const {
            return this->operator[](index);
        }

        iterator begin() const {
            return {*this, 0};
        }

        iterator end() const {
            return {*this, this->argsCount};
        }

      private:
        int argsCount = 0;
        sqlite3_value** values = nullptr;
    };
}

namespace sqlite_orm {

    namespace internal {

        /**
         *  T is a std::tuple type
         *  I is index to extract value from values C array to tuple. I must me < std::tuple_size<T>::value
         */
        template<class T, size_t I>
        struct values_to_tuple {

            void extract(sqlite3_value** values, T& tuple, int argsCount) const {
                using element_type = std::tuple_element_t<I, T>;
                std::get<I>(tuple) = row_extractor<element_type>().extract(values[I]);

                values_to_tuple<T, I - 1>().extract(values, tuple, argsCount);
            }
        };

        template<class T>
        struct values_to_tuple<T, size_t(-1)> {
            void extract(sqlite3_value** /*values*/, T& /*tuple*/, int /*argsCount*/) const {
                //..
            }
        };

        template<>
        struct values_to_tuple<std::tuple<arg_values>, 0> {
            void extract(sqlite3_value** values, std::tuple<arg_values>& tuple, int argsCount) const {
                std::get<0>(tuple) = arg_values(argsCount, values);
            }
        };
    }
}

// #include "arg_values.h"

// #include "util.h"

// #include "serializing_util.h"

namespace sqlite_orm {

    namespace internal {

        struct storage_base {
            using collating_function = std::function<int(int, const void*, int, const void*)>;

            std::function<void(sqlite3*)> on_open;
            pragma_t pragma;
            limit_accessor limit;

            transaction_guard_t transaction_guard() {
                this->begin_transaction();
                return {this->get_connection(),
                        std::bind(&storage_base::commit, this),
                        std::bind(&storage_base::rollback, this)};
            }

            void drop_index(const std::string& indexName) {
                std::stringstream ss;
                ss << "DROP INDEX " << quote_identifier(indexName) << std::flush;
                perform_void_exec(this->get_connection().get(), ss.str());
            }

            void drop_trigger(const std::string& triggerName) {
                std::stringstream ss;
                ss << "DROP TRIGGER " << quote_identifier(triggerName) << std::flush;
                perform_void_exec(this->get_connection().get(), ss.str());
            }

            void vacuum() {
                perform_void_exec(this->get_connection().get(), "VACUUM");
            }

            /**
             *  Drops table with given name.
             */
            void drop_table(const std::string& tableName) {
                this->drop_table_internal(this->get_connection().get(), tableName);
            }

            /**
             * Rename table named `from` to `to`.
             */
            void rename_table(const std::string& from, const std::string& to) {
                this->rename_table(this->get_connection().get(), from, to);
            }

          protected:
            void rename_table(sqlite3* db, const std::string& oldName, const std::string& newName) const {
                std::stringstream ss;
                ss << "ALTER TABLE " << quote_identifier(oldName) << " RENAME TO " << quote_identifier(newName)
                   << std::flush;
                perform_void_exec(db, ss.str());
            }

            bool table_exists(sqlite3* db, const std::string& tableName) const {
                bool result = false;
                std::stringstream ss;
                ss << "SELECT COUNT(*) FROM sqlite_master WHERE type = " << quote_string_literal("table")
                   << " AND name = " << quote_string_literal(tableName) << std::flush;
                perform_exec(
                    db,
                    ss.str(),
                    [](void* data, int argc, char** argv, char** /*azColName*/) -> int {
                        auto& res = *(bool*)data;
                        if(argc) {
                            res = !!std::atoi(argv[0]);
                        }
                        return 0;
                    },
                    &result);
                return result;
            }

          public:
            /**
             *  sqlite3_changes function.
             */
            int changes() {
                auto con = this->get_connection();
                return sqlite3_changes(con.get());
            }

            /**
             *  sqlite3_total_changes function.
             */
            int total_changes() {
                auto con = this->get_connection();
                return sqlite3_total_changes(con.get());
            }

            int64 last_insert_rowid() {
                auto con = this->get_connection();
                return sqlite3_last_insert_rowid(con.get());
            }

            int busy_timeout(int ms) {
                auto con = this->get_connection();
                return sqlite3_busy_timeout(con.get(), ms);
            }

            /**
             *  Returns libsqltie3 lib version, not sqlite_orm
             */
            std::string libversion() {
                return sqlite3_libversion();
            }

            bool transaction(const std::function<bool()>& f) {
                auto guard = this->transaction_guard();
                return guard.commit_on_destroy = f();
            }

            std::string current_timestamp() {
                auto con = this->get_connection();
                return this->current_timestamp(con.get());
            }

#if SQLITE_VERSION_NUMBER >= 3007010
            /**
             * \fn db_release_memory
             * \brief Releases freeable memory of database. It is function can/should be called periodically by
             * application, if application has less memory usage constraint. \note sqlite3_db_release_memory added
             * in 3.7.10 https://sqlite.org/changes.html
             */
            int db_release_memory() {
                auto con = this->get_connection();
                return sqlite3_db_release_memory(con.get());
            }
#endif

            /**
             *  Returns existing permanent table names in database. Doesn't check storage itself - works only with
             * actual database.
             *  @return Returns list of tables in database.
             */
            std::vector<std::string> table_names() {
                auto con = this->get_connection();
                std::vector<std::string> tableNames;
                using data_t = std::vector<std::string>;
                perform_exec(
                    con.get(),
                    "SELECT name FROM sqlite_master WHERE type='table'",
                    [](void* data, int argc, char** argv, char** /*columnName*/) -> int {
                        auto& tableNames_ = *(data_t*)data;
                        for(int i = 0; i < argc; ++i) {
                            if(argv[i]) {
                                tableNames_.emplace_back(argv[i]);
                            }
                        }
                        return 0;
                    },
                    &tableNames);
                return tableNames;
            }

            void open_forever() {
                this->isOpenedForever = true;
                this->connection->retain();
                if(1 == this->connection->retain_count()) {
                    this->on_open_internal(this->connection->get());
                }
            }

            /**
             * Call this to create user defined scalar function. Can be called at any time no matter connection is opened or no.
             * T - function class. T must have operator() overload and static name function like this:
             * ```
             *  struct SqrtFunction {
             *
             *      double operator()(double arg) const {
             *          return std::sqrt(arg);
             *      }
             *
             *      static const char *name() {
             *          return "SQRT";
             *      }
             *  };
             * ```
             * 
             * Note: Currently, a function's name must not contain white-space characters, because it doesn't get quoted.
             */
            template<class F>
            void create_scalar_function() {
                static_assert(is_scalar_function_v<F>, "F can't be an aggregate function");

                std::stringstream ss;
                ss << F::name() << std::flush;
                auto name = ss.str();
                using args_tuple = typename callable_arguments<F>::args_tuple;
                using return_type = typename callable_arguments<F>::return_type;
                auto argsCount = int(std::tuple_size<args_tuple>::value);
                if(std::is_same<args_tuple, std::tuple<arg_values>>::value) {
                    argsCount = -1;
                }
                this->scalarFunctions.emplace_back(new user_defined_scalar_function_t{
                    move(name),
                    argsCount,
                    []() -> int* {
                        return (int*)(new F());
                    },
                    /* call = */
                    [](sqlite3_context* context, void* functionVoidPointer, int argsCount, sqlite3_value** values) {
                        auto& function = *static_cast<F*>(functionVoidPointer);
                        args_tuple argsTuple;
                        using tuple_size = std::tuple_size<args_tuple>;
                        values_to_tuple<args_tuple, tuple_size::value - 1>().extract(values, argsTuple, argsCount);
                        auto result = call(function, std::move(argsTuple));
                        statement_binder<return_type>().result(context, result);
                    },
                    delete_function_callback<F>,
                });

                if(this->connection->retain_count() > 0) {
                    sqlite3* db = this->connection->get();
                    try_to_create_function(db,
                                           static_cast<user_defined_scalar_function_t&>(*this->scalarFunctions.back()));
                }
            }

            /**
             * Call this to create user defined aggregate function. Can be called at any time no matter connection is opened or no.
             * T - function class. T must have step member function, fin member function and static name function like this:
             * ```
             *   struct MeanFunction {
             *       double total = 0;
             *       int count = 0;
             *
             *       void step(double value) {
             *           total += value;
             *           ++count;
             *       }
             *
             *       int fin() const {
             *           return total / count;
             *       }
             *
             *       static std::string name() {
             *           return "MEAN";
             *       }
             *   };
             * ```
             * 
             * Note: Currently, a function's name must not contain white-space characters, because it doesn't get quoted.
             */
            template<class F>
            void create_aggregate_function() {
                static_assert(is_aggregate_function_v<F>, "F can't be a scalar function");

                std::stringstream ss;
                ss << F::name() << std::flush;
                auto name = ss.str();
                using args_tuple = typename callable_arguments<F>::args_tuple;
                using return_type = typename callable_arguments<F>::return_type;
                auto argsCount = int(std::tuple_size<args_tuple>::value);
                if(std::is_same<args_tuple, std::tuple<arg_values>>::value) {
                    argsCount = -1;
                }
                this->aggregateFunctions.emplace_back(new user_defined_aggregate_function_t{
                    move(name),
                    argsCount,
                    /* create = */
                    []() -> int* {
                        return (int*)(new F());
                    },
                    /* step = */
                    [](sqlite3_context*, void* functionVoidPointer, int argsCount, sqlite3_value** values) {
                        auto& function = *static_cast<F*>(functionVoidPointer);
                        args_tuple argsTuple;
                        using tuple_size = std::tuple_size<args_tuple>;
                        values_to_tuple<args_tuple, tuple_size::value - 1>().extract(values, argsTuple, argsCount);
                        call(function, &F::step, move(argsTuple));
                    },
                    /* finalCall = */
                    [](sqlite3_context* context, void* functionVoidPointer) {
                        auto& function = *static_cast<F*>(functionVoidPointer);
                        auto result = function.fin();
                        statement_binder<return_type>().result(context, result);
                    },
                    delete_function_callback<F>,
                });

                if(this->connection->retain_count() > 0) {
                    sqlite3* db = this->connection->get();
                    try_to_create_function(
                        db,
                        static_cast<user_defined_aggregate_function_t&>(*this->aggregateFunctions.back()));
                }
            }

            /**
             *  Use it to delete scalar function you created before. Can be called at any time no matter connection is open or no.
             */
            template<class F>
            void delete_scalar_function() {
                static_assert(is_scalar_function_v<F>, "F cannot be an aggregate function");
                std::stringstream ss;
                ss << F::name() << std::flush;
                this->delete_function_impl(ss.str(), this->scalarFunctions);
            }

            /**
             *  Use it to delete aggregate function you created before. Can be called at any time no matter connection is open or no.
             */
            template<class F>
            void delete_aggregate_function() {
                static_assert(is_aggregate_function_v<F>, "F cannot be a scalar function");
                std::stringstream ss;
                ss << F::name() << std::flush;
                this->delete_function_impl(ss.str(), this->aggregateFunctions);
            }

            template<class C>
            void create_collation() {
                collating_function func = [](int leftLength, const void* lhs, int rightLength, const void* rhs) {
                    C collatingObject;
                    return collatingObject(leftLength, lhs, rightLength, rhs);
                };
                std::stringstream ss;
                ss << C::name() << std::flush;
                this->create_collation(ss.str(), move(func));
            }

            void create_collation(const std::string& name, collating_function f) {
                collating_function* function = nullptr;
                const auto functionExists = bool(f);
                if(functionExists) {
                    function = &(collatingFunctions[name] = std::move(f));
                } else {
                    collatingFunctions.erase(name);
                }

                //  create collations if db is open
                if(this->connection->retain_count() > 0) {
                    sqlite3* db = this->connection->get();
                    auto resultCode = sqlite3_create_collation(db,
                                                               name.c_str(),
                                                               SQLITE_UTF8,
                                                               function,
                                                               functionExists ? collate_callback : nullptr);
                    if(resultCode != SQLITE_OK) {
                        throw_translated_sqlite_error(db);
                    }
                }
            }

            template<class C>
            void delete_collation() {
                std::stringstream ss;
                ss << C::name() << std::flush;
                this->create_collation(ss.str(), {});
            }

            void begin_transaction() {
                this->begin_transaction_internal("BEGIN TRANSACTION");
            }

            void begin_deferred_transaction() {
                this->begin_transaction_internal("BEGIN DEFERRED TRANSACTION");
            }

            void begin_immediate_transaction() {
                this->begin_transaction_internal("BEGIN IMMEDIATE TRANSACTION");
            }

            void begin_exclusive_transaction() {
                this->begin_transaction_internal("BEGIN EXCLUSIVE TRANSACTION");
            }

            void commit() {
                sqlite3* db = this->connection->get();
                perform_void_exec(db, "COMMIT");
                this->connection->release();
                if(this->connection->retain_count() < 0) {
                    throw std::system_error{orm_error_code::no_active_transaction};
                }
            }

            void rollback() {
                sqlite3* db = this->connection->get();
                perform_void_exec(db, "ROLLBACK");
                this->connection->release();
                if(this->connection->retain_count() < 0) {
                    throw std::system_error{orm_error_code::no_active_transaction};
                }
            }

            void backup_to(const std::string& filename) {
                auto backup = this->make_backup_to(filename);
                backup.step(-1);
            }

            void backup_to(storage_base& other) {
                auto backup = this->make_backup_to(other);
                backup.step(-1);
            }

            void backup_from(const std::string& filename) {
                auto backup = this->make_backup_from(filename);
                backup.step(-1);
            }

            void backup_from(storage_base& other) {
                auto backup = this->make_backup_from(other);
                backup.step(-1);
            }

            backup_t make_backup_to(const std::string& filename) {
                auto holder = std::make_unique<connection_holder>(filename);
                connection_ref conRef{*holder};
                return {conRef, "main", this->get_connection(), "main", move(holder)};
            }

            backup_t make_backup_to(storage_base& other) {
                return {other.get_connection(), "main", this->get_connection(), "main", {}};
            }

            backup_t make_backup_from(const std::string& filename) {
                auto holder = std::make_unique<connection_holder>(filename);
                connection_ref conRef{*holder};
                return {this->get_connection(), "main", conRef, "main", move(holder)};
            }

            backup_t make_backup_from(storage_base& other) {
                return {this->get_connection(), "main", other.get_connection(), "main", {}};
            }

            const std::string& filename() const {
                return this->connection->filename;
            }

            /**
             * Checks whether connection to database is opened right now.
             * Returns always `true` for in memory databases.
             */
            bool is_opened() const {
                return this->connection->retain_count() > 0;
            }

            /*
             * returning false when there is a transaction in place
             * otherwise true; function is not const because it has to call get_connection()
             */
            bool get_autocommit() {
                auto con = this->get_connection();
                return sqlite3_get_autocommit(con.get());
            }

            int busy_handler(std::function<int(int)> handler) {
                _busy_handler = move(handler);
                if(this->is_opened()) {
                    if(_busy_handler) {
                        return sqlite3_busy_handler(this->connection->get(), busy_handler_callback, this);
                    } else {
                        return sqlite3_busy_handler(this->connection->get(), nullptr, nullptr);
                    }
                } else {
                    return SQLITE_OK;
                }
            }

          protected:
            storage_base(const std::string& filename_, int foreignKeysCount) :
                pragma(std::bind(&storage_base::get_connection, this)),
                limit(std::bind(&storage_base::get_connection, this)),
                inMemory(filename_.empty() || filename_ == ":memory:"),
                connection(std::make_unique<connection_holder>(filename_)), cachedForeignKeysCount(foreignKeysCount) {
                if(this->inMemory) {
                    this->connection->retain();
                    this->on_open_internal(this->connection->get());
                }
            }

            storage_base(const storage_base& other) :
                on_open(other.on_open), pragma(std::bind(&storage_base::get_connection, this)),
                limit(std::bind(&storage_base::get_connection, this)), inMemory(other.inMemory),
                connection(std::make_unique<connection_holder>(other.connection->filename)),
                cachedForeignKeysCount(other.cachedForeignKeysCount) {
                if(this->inMemory) {
                    this->connection->retain();
                    this->on_open_internal(this->connection->get());
                }
            }

            ~storage_base() {
                if(this->isOpenedForever) {
                    this->connection->release();
                }
                if(this->inMemory) {
                    this->connection->release();
                }
            }

            void begin_transaction_internal(const std::string& query) {
                this->connection->retain();
                if(1 == this->connection->retain_count()) {
                    this->on_open_internal(this->connection->get());
                }
                sqlite3* db = this->connection->get();
                perform_void_exec(db, query);
            }

            connection_ref get_connection() {
                connection_ref res{*this->connection};
                if(1 == this->connection->retain_count()) {
                    this->on_open_internal(this->connection->get());
                }
                return res;
            }

#if SQLITE_VERSION_NUMBER >= 3006019

            void foreign_keys(sqlite3* db, bool value) {
                std::stringstream ss;
                ss << "PRAGMA foreign_keys = " << value << std::flush;
                perform_void_exec(db, ss.str());
            }

            bool foreign_keys(sqlite3* db) {
                bool result = false;
                perform_exec(db, "PRAGMA foreign_keys", extract_single_value<bool>, &result);
                return result;
            }

#endif
            void on_open_internal(sqlite3* db) {

#if SQLITE_VERSION_NUMBER >= 3006019
                if(this->cachedForeignKeysCount) {
                    this->foreign_keys(db, true);
                }
#endif
                if(this->pragma._synchronous != -1) {
                    this->pragma.synchronous(this->pragma._synchronous);
                }

                if(this->pragma._journal_mode != -1) {
                    this->pragma.set_pragma("journal_mode", static_cast<journal_mode>(this->pragma._journal_mode), db);
                }

                for(auto& p: this->collatingFunctions) {
                    auto resultCode =
                        sqlite3_create_collation(db, p.first.c_str(), SQLITE_UTF8, &p.second, collate_callback);
                    if(resultCode != SQLITE_OK) {
                        throw_translated_sqlite_error(db);
                    }
                }

                for(auto& p: this->limit.limits) {
                    sqlite3_limit(db, p.first, p.second);
                }

                if(_busy_handler) {
                    sqlite3_busy_handler(this->connection->get(), busy_handler_callback, this);
                }

                for(auto& functionPointer: this->scalarFunctions) {
                    try_to_create_function(db, static_cast<user_defined_scalar_function_t&>(*functionPointer));
                }

                for(auto& functionPointer: this->aggregateFunctions) {
                    try_to_create_function(db, static_cast<user_defined_aggregate_function_t&>(*functionPointer));
                }

                if(this->on_open) {
                    this->on_open(db);
                }
            }

            void delete_function_impl(const std::string& name,
                                      std::vector<std::unique_ptr<user_defined_function_base>>& functionsVector) const {
                auto it = find_if(functionsVector.begin(), functionsVector.end(), [&name](auto& functionPointer) {
                    return functionPointer->name == name;
                });
                if(it != functionsVector.end()) {
                    functionsVector.erase(it);
                    it = functionsVector.end();

                    if(this->connection->retain_count() > 0) {
                        sqlite3* db = this->connection->get();
                        auto resultCode = sqlite3_create_function_v2(db,
                                                                     name.c_str(),
                                                                     0,
                                                                     SQLITE_UTF8,
                                                                     nullptr,
                                                                     nullptr,
                                                                     nullptr,
                                                                     nullptr,
                                                                     nullptr);
                        if(resultCode != SQLITE_OK) {
                            throw_translated_sqlite_error(db);
                        }
                    }
                } else {
                    throw std::system_error{orm_error_code::function_not_found};
                }
            }

            void try_to_create_function(sqlite3* db, user_defined_scalar_function_t& function) {
                auto resultCode = sqlite3_create_function_v2(db,
                                                             function.name.c_str(),
                                                             function.argumentsCount,
                                                             SQLITE_UTF8,
                                                             &function,
                                                             scalar_function_callback,
                                                             nullptr,
                                                             nullptr,
                                                             nullptr);
                if(resultCode != SQLITE_OK) {
                    throw_translated_sqlite_error(db);
                }
            }

            void try_to_create_function(sqlite3* db, user_defined_aggregate_function_t& function) {
                auto resultCode = sqlite3_create_function(db,
                                                          function.name.c_str(),
                                                          function.argumentsCount,
                                                          SQLITE_UTF8,
                                                          &function,
                                                          nullptr,
                                                          aggregate_function_step_callback,
                                                          aggregate_function_final_callback);
                if(resultCode != SQLITE_OK) {
                    throw_translated_sqlite_error(resultCode);
                }
            }

            static void
            aggregate_function_step_callback(sqlite3_context* context, int argsCount, sqlite3_value** values) {
                auto functionVoidPointer = sqlite3_user_data(context);
                auto functionPointer = static_cast<user_defined_aggregate_function_t*>(functionVoidPointer);
                auto aggregateContextVoidPointer = sqlite3_aggregate_context(context, sizeof(int**));
                auto aggregateContextIntPointer = static_cast<int**>(aggregateContextVoidPointer);
                if(*aggregateContextIntPointer == nullptr) {
                    *aggregateContextIntPointer = functionPointer->create();
                }
                functionPointer->step(context, *aggregateContextIntPointer, argsCount, values);
            }

            static void aggregate_function_final_callback(sqlite3_context* context) {
                auto functionVoidPointer = sqlite3_user_data(context);
                auto functionPointer = static_cast<user_defined_aggregate_function_t*>(functionVoidPointer);
                auto aggregateContextVoidPointer = sqlite3_aggregate_context(context, sizeof(int**));
                auto aggregateContextIntPointer = static_cast<int**>(aggregateContextVoidPointer);
                functionPointer->finalCall(context, *aggregateContextIntPointer);
                functionPointer->destroy(*aggregateContextIntPointer);
            }

            static void scalar_function_callback(sqlite3_context* context, int argsCount, sqlite3_value** values) {
                auto functionVoidPointer = sqlite3_user_data(context);
                auto functionPointer = static_cast<user_defined_scalar_function_t*>(functionVoidPointer);
                std::unique_ptr<int, void (*)(int*)> callablePointer(functionPointer->create(),
                                                                     functionPointer->destroy);
                if(functionPointer->argumentsCount != -1 && functionPointer->argumentsCount != argsCount) {
                    throw std::system_error{orm_error_code::arguments_count_does_not_match};
                }
                functionPointer->run(context, functionPointer, argsCount, values);
            }

            template<class F>
            static void delete_function_callback(int* pointer) {
                auto voidPointer = static_cast<void*>(pointer);
                auto fPointer = static_cast<F*>(voidPointer);
                delete fPointer;
            }

            std::string current_timestamp(sqlite3* db) {
                std::string result;
                perform_exec(db, "SELECT CURRENT_TIMESTAMP", extract_single_value<std::string>, &result);
                return result;
            }

            void drop_table_internal(sqlite3* db, const std::string& tableName) {
                std::stringstream ss;
                ss << "DROP TABLE " << streaming_identifier(tableName) << std::flush;
                perform_void_exec(db, ss.str());
            }

            static int collate_callback(void* arg, int leftLen, const void* lhs, int rightLen, const void* rhs) {
                auto& f = *(collating_function*)arg;
                return f(leftLen, lhs, rightLen, rhs);
            }

            static int busy_handler_callback(void* selfPointer, int triesCount) {
                auto& storage = *static_cast<storage_base*>(selfPointer);
                if(storage._busy_handler) {
                    return storage._busy_handler(triesCount);
                } else {
                    return 0;
                }
            }

            //  returns foreign keys count in storage definition
            template<class S>
            static int foreign_keys_count(const S& storageImpl) {
                auto res = 0;

                storageImpl.for_each([&res](const auto& tImpl) {
                    using qualified_type = std::decay_t<decltype(tImpl)>;
                    constexpr bool c = std::is_base_of<basic_table, table_type_or_none_t<qualified_type>>::value;

                    call_if_constexpr<c>(
                        [&res](const auto& tImpl) {
                            res += tImpl.table.foreign_keys_count();
                        },
                        tImpl);
                });
                return res;
            }

            bool calculate_remove_add_columns(std::vector<const table_xinfo*>& columnsToAdd,
                                              std::vector<table_xinfo>& storageTableInfo,
                                              std::vector<table_xinfo>& dbTableInfo) const {
                bool notEqual = false;

                //  iterate through storage columns
                for(size_t storageColumnInfoIndex = 0; storageColumnInfoIndex < storageTableInfo.size();
                    ++storageColumnInfoIndex) {

                    //  get storage's column info
                    auto& storageColumnInfo = storageTableInfo[storageColumnInfoIndex];
                    auto& columnName = storageColumnInfo.name;

                    //  search for a column in db eith the same name
                    auto dbColumnInfoIt = std::find_if(dbTableInfo.begin(), dbTableInfo.end(), [&columnName](auto& ti) {
                        return ti.name == columnName;
                    });
                    if(dbColumnInfoIt != dbTableInfo.end()) {
                        auto& dbColumnInfo = *dbColumnInfoIt;
                        auto columnsAreEqual =
                            dbColumnInfo.name == storageColumnInfo.name &&
                            dbColumnInfo.notnull == storageColumnInfo.notnull &&
                            (!dbColumnInfo.dflt_value.empty()) == (!storageColumnInfo.dflt_value.empty()) &&
                            dbColumnInfo.pk == storageColumnInfo.pk &&
                            (dbColumnInfo.hidden == 0) == (storageColumnInfo.hidden == 0);
                        if(!columnsAreEqual) {
                            notEqual = true;
                            break;
                        }
                        dbTableInfo.erase(dbColumnInfoIt);
                        storageTableInfo.erase(storageTableInfo.begin() +
                                               static_cast<ptrdiff_t>(storageColumnInfoIndex));
                        --storageColumnInfoIndex;
                    } else {
                        columnsToAdd.push_back(&storageColumnInfo);
                    }
                }
                return notEqual;
            }

            const bool inMemory;
            bool isOpenedForever = false;
            std::unique_ptr<connection_holder> connection;
            std::map<std::string, collating_function> collatingFunctions;
            const int cachedForeignKeysCount;
            std::function<int(int)> _busy_handler;
            std::vector<std::unique_ptr<user_defined_function_base>> scalarFunctions;
            std::vector<std::unique_ptr<user_defined_function_base>> aggregateFunctions;
        };
    }
}

// #include "prepared_statement.h"

// #include "expression_object_type.h"

#include <type_traits>  //  std::decay
#include <functional>  //  std::reference_wrapper

// #include "prepared_statement.h"

namespace sqlite_orm {

    namespace internal {

        template<class T, class SFINAE = void>
        struct expression_object_type;

        template<class T>
        struct expression_object_type<update_t<T>> : std::decay<T> {};

        template<class T>
        struct expression_object_type<update_t<std::reference_wrapper<T>>> : std::decay<T> {};

        template<class T>
        struct expression_object_type<replace_t<T>> : std::decay<T> {};

        template<class T>
        struct expression_object_type<replace_t<std::reference_wrapper<T>>> : std::decay<T> {};

        template<class It, class L, class O>
        struct expression_object_type<replace_range_t<It, L, O>> {
            using type = typename replace_range_t<It, L, O>::object_type;
        };

        template<class It, class L, class O>
        struct expression_object_type<replace_range_t<std::reference_wrapper<It>, L, O>> {
            using type = typename replace_range_t<std::reference_wrapper<It>, L, O>::object_type;
        };

        template<class T, class... Ids>
        struct expression_object_type<remove_t<T, Ids...>> {
            using type = T;
        };

        template<class T, class... Ids>
        struct expression_object_type<remove_t<std::reference_wrapper<T>, Ids...>> {
            using type = T;
        };

        template<class T>
        struct expression_object_type<insert_t<T>> : std::decay<T> {};

        template<class T>
        struct expression_object_type<insert_t<std::reference_wrapper<T>>> : std::decay<T> {};

        template<class It, class L, class O>
        struct expression_object_type<insert_range_t<It, L, O>> {
            using type = typename insert_range_t<It, L, O>::object_type;
        };

        template<class It, class L, class O>
        struct expression_object_type<insert_range_t<std::reference_wrapper<It>, L, O>> {
            using type = typename insert_range_t<std::reference_wrapper<It>, L, O>::object_type;
        };

        template<class T, class... Cols>
        struct expression_object_type<insert_explicit<T, Cols...>> : std::decay<T> {};

        template<class T, class... Cols>
        struct expression_object_type<insert_explicit<std::reference_wrapper<T>, Cols...>> : std::decay<T> {};

        template<class T>
        struct get_ref_t {

            template<class O>
            auto& operator()(O& t) const {
                return t;
            }
        };

        template<class T>
        struct get_ref_t<std::reference_wrapper<T>> {

            template<class O>
            auto& operator()(O& t) const {
                return t.get();
            }
        };

        template<class T>
        auto& get_ref(T& t) {
            using arg_type = std::decay_t<T>;
            get_ref_t<arg_type> g;
            return g(t);
        }

        template<class T>
        struct get_object_t;

        template<class T>
        struct get_object_t<const T> : get_object_t<T> {};

        template<class T>
        auto& get_object(T& t) {
            using expression_type = std::decay_t<T>;
            get_object_t<expression_type> obj;
            return obj(t);
        }

        template<class T>
        struct get_object_t<replace_t<T>> {
            using expression_type = replace_t<T>;

            template<class O>
            auto& operator()(O& e) const {
                return get_ref(e.object);
            }
        };

        template<class T>
        struct get_object_t<insert_t<T>> {
            using expression_type = insert_t<T>;

            template<class O>
            auto& operator()(O& e) const {
                return get_ref(e.object);
            }
        };

        template<class T>
        struct get_object_t<update_t<T>> {
            using expression_type = update_t<T>;

            template<class O>
            auto& operator()(O& e) const {
                return get_ref(e.object);
            }
        };
    }
}

// #include "statement_serializer.h"

#include <sstream>  //  std::stringstream
#include <string>  //  std::string
#include <type_traits>  //  std::enable_if, std::remove_pointer
#include <vector>  //  std::vector
#include <algorithm>  //  std::iter_swap
#ifndef SQLITE_ORM_OMITS_CODECVT
#include <codecvt>  //  std::codecvt_utf8_utf16
#endif  //  SQLITE_ORM_OMITS_CODECVT
#include <memory>
#include <array>
#ifdef SQLITE_ORM_STRING_VIEW_SUPPORTED
#include <string_view>
#endif

// #include "functional/cxx_universal.h"

// #include "functional/cxx_functional_polyfill.h"

// #include "functional/mpl.h"

// #include "tuple_helper/tuple_filter.h"

// #include "ast/upsert_clause.h"

// #include "ast/excluded.h"

// #include "ast/group_by.h"

// #include "ast/into.h"

// #include "core_functions.h"

// #include "constraints.h"

// #include "conditions.h"

// #include "column.h"

// #include "indexed_column.h"

// #include "function.h"

// #include "prepared_statement.h"

// #include "rowid.h"

// #include "pointer_value.h"

// #include "type_printer.h"

// #include "field_printer.h"

// #include "literal.h"

// #include "table_name_collector.h"

#include <set>  //  std::set
#include <string>  //  std::string
#include <functional>  //  std::function
#include <typeindex>  //  std::type_index

// #include "functional/cxx_polyfill.h"

// #include "type_traits.h"

// #include "select_constraints.h"

// #include "alias.h"

// #include "core_functions.h"

namespace sqlite_orm {

    namespace internal {

        struct table_name_collector {
            using table_name_set = std::set<std::pair<std::string, std::string>>;
            using find_table_name_t = std::function<std::string(std::type_index)>;

            find_table_name_t find_table_name;
            mutable table_name_set table_names;

            table_name_collector() = default;

            table_name_collector(find_table_name_t find_table_name) : find_table_name{move(find_table_name)} {}

            template<class T>
            table_name_set operator()(const T&) const {
                return {};
            }

            template<class F, class O>
            void operator()(F O::*, std::string alias = {}) const {
                table_names.emplace(this->find_table_name(typeid(O)), move(alias));
            }

            template<class T, class F>
            void operator()(const column_pointer<T, F>&) const {
                table_names.emplace(this->find_table_name(typeid(T)), "");
            }

            template<class T, class C>
            void operator()(const alias_column_t<T, C>& a) const {
                (*this)(a.column, alias_extractor<T>::get());
            }

            template<class T>
            void operator()(const count_asterisk_t<T>&) const {
                auto tableName = this->find_table_name(typeid(T));
                if(!tableName.empty()) {
                    table_names.emplace(move(tableName), "");
                }
            }

            template<class T, satisfies_not<std::is_base_of, alias_tag, T> = true>
            void operator()(const asterisk_t<T>&) const {
                table_names.emplace(this->find_table_name(typeid(T)), "");
            }

            template<class T, satisfies<std::is_base_of, alias_tag, T> = true>
            void operator()(const asterisk_t<T>&) const {
                // note: not all alias classes have a nested A::type
                static_assert(polyfill::is_detected_v<type_t, T>,
                              "alias<O> must have a nested alias<O>::type typename");
                auto tableName = this->find_table_name(typeid(type_t<T>));
                table_names.emplace(move(tableName), alias_extractor<T>::get());
            }

            template<class T>
            void operator()(const object_t<T>&) const {
                table_names.emplace(this->find_table_name(typeid(T)), "");
            }

            template<class T>
            void operator()(const table_rowid_t<T>&) const {
                table_names.emplace(this->find_table_name(typeid(T)), "");
            }

            template<class T>
            void operator()(const table_oid_t<T>&) const {
                table_names.emplace(this->find_table_name(typeid(T)), "");
            }

            template<class T>
            void operator()(const table__rowid_t<T>&) const {
                table_names.emplace(this->find_table_name(typeid(T)), "");
            }
        };

    }

}

// #include "column_names_getter.h"

#include <system_error>  //  std::system_error
#include <string>  //  std::string
#include <vector>  //  std::vector
#include <functional>  //  std::reference_wrapper

// #include "error_code.h"

// #include "serializer_context.h"

// #include "select_constraints.h"

// #include "util.h"

namespace sqlite_orm {

    namespace internal {

        template<class T, class I>
        std::string serialize(const T& t, const serializer_context<I>& context);

        template<class T, class SFINAE = void>
        struct column_names_getter {
            using expression_type = T;

            template<class Ctx>
            std::vector<std::string> operator()(const expression_type& t, const Ctx& context) const {
                auto newContext = context;
                newContext.skip_table_name = false;
                auto columnName = serialize(t, newContext);
                if(!columnName.empty()) {
                    return {move(columnName)};
                } else {
                    throw std::system_error{orm_error_code::column_not_found};
                }
            }
        };

        template<class T, class Ctx>
        std::vector<std::string> get_column_names(const T& t, const Ctx& context) {
            column_names_getter<T> serializer;
            return serializer(t, context);
        }

        template<class T>
        struct column_names_getter<std::reference_wrapper<T>, void> {
            using expression_type = std::reference_wrapper<T>;

            template<class Ctx>
            std::vector<std::string> operator()(const expression_type& expression, const Ctx& context) const {
                return get_column_names(expression.get(), context);
            }
        };

        template<class T>
        struct column_names_getter<asterisk_t<T>, match_if_not<std::is_base_of, alias_tag, T>> {
            using expression_type = asterisk_t<T>;

            template<class Ctx>
            std::vector<std::string> operator()(const expression_type&, const Ctx&) const {
                return {"*"};
            }
        };

        template<class A>
        struct column_names_getter<asterisk_t<A>, match_if<std::is_base_of, alias_tag, A>> {
            using expression_type = asterisk_t<A>;

            template<class Ctx>
            std::vector<std::string> operator()(const expression_type&, const Ctx&) const {
                return {quote_identifier(alias_extractor<A>::get()) + ".*"};
            }
        };

        template<class T>
        struct column_names_getter<object_t<T>, void> {
            using expression_type = object_t<T>;

            template<class Ctx>
            std::vector<std::string> operator()(const expression_type&, const Ctx&) const {
                return {"*"};
            }
        };

        template<class... Args>
        struct column_names_getter<columns_t<Args...>, void> {
            using expression_type = columns_t<Args...>;

            template<class Ctx>
            std::vector<std::string> operator()(const expression_type& cols, const Ctx& context) const {
                std::vector<std::string> columnNames;
                columnNames.reserve(static_cast<size_t>(cols.count));
                auto newContext = context;
                newContext.skip_table_name = false;
                iterate_tuple(cols.columns, [&columnNames, &newContext](auto& m) {
                    auto columnName = serialize(m, newContext);
                    if(!columnName.empty()) {
                        columnNames.push_back(move(columnName));
                    } else {
                        throw std::system_error{orm_error_code::column_not_found};
                    }
                });
                return columnNames;
            }
        };

    }
}

// #include "order_by_serializer.h"

#include <string>  //  std::string
#include <sstream>  //  std::stringstream

namespace sqlite_orm {

    namespace internal {

        template<class T, class SFINAE = void>
        struct order_by_serializer;

        template<class T, class Ctx>
        std::string serialize_order_by(const T& t, const Ctx& context) {
            order_by_serializer<T> serializer;
            return serializer(t, context);
        }

        template<class O>
        struct order_by_serializer<order_by_t<O>, void> {
            using statement_type = order_by_t<O>;

            template<class Ctx>
            std::string operator()(const statement_type& orderBy, const Ctx& context) const {
                std::stringstream ss;
                auto newContext = context;
                newContext.skip_table_name = false;

                ss << serialize(orderBy.expression, newContext);
                if(!orderBy._collate_argument.empty()) {
                    ss << " COLLATE " << orderBy._collate_argument;
                }
                switch(orderBy.asc_desc) {
                    case 1:
                        ss << " ASC";
                        break;
                    case -1:
                        ss << " DESC";
                        break;
                }
                return ss.str();
            }
        };

        template<class S>
        struct order_by_serializer<dynamic_order_by_t<S>, void> {
            using statement_type = dynamic_order_by_t<S>;

            template<class Ctx>
            std::string operator()(const statement_type& orderBy, const Ctx&) const {
                std::stringstream ss;
                ss << static_cast<std::string>(orderBy) << " ";
                int index = 0;
                for(const dynamic_order_by_entry_t& entry: orderBy) {
                    if(index > 0) {
                        ss << ", ";
                    }

                    ss << entry.name;
                    if(!entry._collate_argument.empty()) {
                        ss << " COLLATE " << entry._collate_argument;
                    }
                    switch(entry.asc_desc) {
                        case 1:
                            ss << " ASC";
                            break;
                        case -1:
                            ss << " DESC";
                            break;
                    }
                    ++index;
                };
                return ss.str();
            }
        };

    }
}

// #include "serializing_util.h"

// #include "statement_binder.h"

// #include "values.h"

// #include "triggers.h"

// #include "table_type.h"

// #include "index.h"

// #include "util.h"

namespace sqlite_orm {

    namespace internal {

        template<class T, class SFINAE = void>
        struct statement_serializer;

        template<class T, class I>
        std::string serialize(const T& t, const serializer_context<I>& context) {
            statement_serializer<T> serializer;
            return serializer(t, context);
        }

        /**
         *  Serializer for bindable types.
         */
        template<class T>
        struct statement_serializer<T, match_if<is_bindable, T>> {
            using statement_type = T;

            template<class Ctx>
            std::string operator()(const T& statement, const Ctx& context) const {
                if(context.replace_bindable_with_question) {
                    return "?";
                } else {
                    return this->do_serialize(statement);
                }
            }

          private:
            template<class X,
                     std::enable_if_t<is_printable_v<X> && !std::is_base_of<std::string, X>::value
#ifndef SQLITE_ORM_OMITS_CODECVT
                                          && !std::is_base_of<std::wstring, X>::value
#endif
                                      ,
                                      bool> = true>
            std::string do_serialize(const X& c) const {
                static_assert(std::is_same<X, T>::value, "");

                // implementation detail: utilizing field_printer
                return field_printer<X>{}(c);
            }

            std::string do_serialize(const std::string& c) const {
                // implementation detail: utilizing field_printer
                return quote_string_literal(field_printer<std::string>{}(c));
            }

            std::string do_serialize(const char* c) const {
                return quote_string_literal(c);
            }
#ifndef SQLITE_ORM_OMITS_CODECVT
            std::string do_serialize(const std::wstring& c) const {
                // implementation detail: utilizing field_printer
                return quote_string_literal(field_printer<std::wstring>{}(c));
            }

            std::string do_serialize(const wchar_t* c) const {
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                return quote_string_literal(converter.to_bytes(c));
            }
#endif
#ifdef SQLITE_ORM_STRING_VIEW_SUPPORTED
            std::string do_serialize(const std::string_view& c) const {
                return quote_string_literal(std::string(c));
            }
#ifndef SQLITE_ORM_OMITS_CODECVT
            std::string do_serialize(const std::wstring_view& c) const {
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                return quote_string_literal(converter.to_bytes(c.data(), c.data() + c.size()));
            }
#endif
#endif
            /**
             *  Specialization for binary data (std::vector<char>).
             */
            std::string do_serialize(const std::vector<char>& t) const {
                return quote_blob_literal(field_printer<std::vector<char>>{}(t));
            }

            template<class P, class PT, class D>
            std::string do_serialize(const pointer_binding<P, PT, D>&) const {
                // always serialize null (security reasons)
                return field_printer<nullptr_t>{}(nullptr);
            }
        };

        /**
         *  Serializer for literal values.
         */
        template<class T>
        struct statement_serializer<T, internal::match_specialization_of<T, literal_holder>> {
            using statement_type = T;

            template<class Ctx>
            std::string operator()(const T& literal, const Ctx& context) const {
                static_assert(is_bindable_v<type_t<T>>, "A literal value must be also bindable");

                Ctx literalCtx = context;
                literalCtx.replace_bindable_with_question = false;
                statement_serializer<type_t<T>> serializer{};
                return serializer(literal.value, literalCtx);
            }
        };

        template<class F, class W>
        struct statement_serializer<filtered_aggregate_function<F, W>, void> {
            using statement_type = filtered_aggregate_function<F, W>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) {
                std::stringstream ss;
                ss << serialize(statement.function, context);
                ss << " FILTER (WHERE " << serialize(statement.where, context) << ")";
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<excluded_t<T>, void> {
            using statement_type = excluded_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                ss << "excluded.";
                if(auto* columnName = find_column_name(context.impl, statement.expression)) {
                    ss << streaming_identifier(*columnName);
                } else {
                    throw std::system_error{orm_error_code::column_not_found};
                }
                return ss.str();
            }
        };
#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
        template<class T>
        struct statement_serializer<as_optional_t<T>, void> {
            using statement_type = as_optional_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                return serialize(statement.value, context);
            }
        };
#endif  //  SQLITE_ORM_OPTIONAL_SUPPORTED
        template<class T>
        struct statement_serializer<std::reference_wrapper<T>, void> {
            using statement_type = std::reference_wrapper<T>;

            template<class Ctx>
            std::string operator()(const statement_type& s, const Ctx& context) const {
                return serialize(s.get(), context);
            }
        };

        template<class T>
        struct statement_serializer<alias_holder<T>, void> {
            using statement_type = alias_holder<T>;

            template<class Ctx>
            std::string operator()(const statement_type&, const Ctx&) {
                std::stringstream ss;
                ss << streaming_identifier(T::get());
                return ss.str();
            }
        };

        template<class... TargetArgs, class... ActionsArgs>
        struct statement_serializer<upsert_clause<std::tuple<TargetArgs...>, std::tuple<ActionsArgs...>>, void> {
            using statement_type = upsert_clause<std::tuple<TargetArgs...>, std::tuple<ActionsArgs...>>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                ss << "ON CONFLICT";
                iterate_tuple(statement.target_args, [&ss, &context](auto& value) {
                    using value_type = std::decay_t<decltype(value)>;
                    auto needParenthesis = std::is_member_pointer<value_type>::value;
                    ss << ' ';
                    if(needParenthesis) {
                        ss << '(';
                    }
                    ss << serialize(value, context);
                    if(needParenthesis) {
                        ss << ')';
                    }
                });
                ss << ' ' << "DO";
                if(std::tuple_size<typename statement_type::actions_tuple>::value == 0) {
                    ss << " NOTHING";
                } else {
                    auto updateContext = context;
                    updateContext.use_parentheses = false;
                    ss << " UPDATE " << streaming_actions_tuple(statement.actions, updateContext);
                }
                return ss.str();
            }
        };

        template<class R, class S, class... Args>
        struct statement_serializer<built_in_function_t<R, S, Args...>, void> {
            using statement_type = built_in_function_t<R, S, Args...>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                if(context.use_parentheses) {
                    ss << '(';
                }
                ss << statement.serialize() << "(" << streaming_expressions_tuple(statement.args, context) << ")";
                if(context.use_parentheses) {
                    ss << ')';
                }
                return ss.str();
            }
        };

        template<class R, class S, class... Args>
        struct statement_serializer<built_in_aggregate_function_t<R, S, Args...>, void>
            : statement_serializer<built_in_function_t<R, S, Args...>, void> {};

        template<class F, class... Args>
        struct statement_serializer<function_call<F, Args...>, void> {
            using statement_type = function_call<F, Args...>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                ss << F::name() << "(" << streaming_expressions_tuple(statement.args, context) << ")";
                return ss.str();
            }
        };

        template<class T, class E>
        struct statement_serializer<as_t<T, E>, void> {
            using statement_type = as_t<T, E>;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                std::stringstream ss;
                ss << serialize(c.expression, context) + " AS " << streaming_identifier(alias_extractor<T>::get());
                return ss.str();
            }
        };

        template<class T, class P>
        struct statement_serializer<alias_column_t<T, P>, void> {
            using statement_type = alias_column_t<T, P>;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                std::stringstream ss;
                if(!context.skip_table_name) {
                    ss << streaming_identifier(alias_extractor<T>::get()) << ".";
                }
                auto newContext = context;
                newContext.skip_table_name = true;
                ss << serialize(c.column, newContext);
                return ss.str();
            }
        };

        template<class O, class F>
        struct statement_serializer<F O::*, void> {
            using statement_type = F O::*;

            template<class Ctx>
            std::string operator()(const statement_type& m, const Ctx& context) const {
                std::stringstream ss;
                if(!context.skip_table_name) {
                    ss << streaming_identifier(lookup_table_name<O>(context.impl)) << ".";
                }
                if(auto* columnName = find_column_name(context.impl, m)) {
                    ss << streaming_identifier(*columnName);
                } else {
                    throw std::system_error{orm_error_code::column_not_found};
                }
                return ss.str();
            }
        };

        template<>
        struct statement_serializer<rowid_t, void> {
            using statement_type = rowid_t;

            template<class Ctx>
            std::string operator()(const statement_type& s, const Ctx&) const {
                return static_cast<std::string>(s);
            }
        };

        template<>
        struct statement_serializer<oid_t, void> {
            using statement_type = oid_t;

            template<class Ctx>
            std::string operator()(const statement_type& s, const Ctx&) const {
                return static_cast<std::string>(s);
            }
        };

        template<>
        struct statement_serializer<_rowid_t, void> {
            using statement_type = _rowid_t;

            template<class Ctx>
            std::string operator()(const statement_type& s, const Ctx&) const {
                return static_cast<std::string>(s);
            }
        };

        template<class O>
        struct statement_serializer<table_rowid_t<O>, void> {
            using statement_type = table_rowid_t<O>;

            template<class Ctx>
            std::string operator()(const statement_type& s, const Ctx& context) const {
                std::stringstream ss;
                if(!context.skip_table_name) {
                    ss << streaming_identifier(lookup_table_name<O>(context.impl)) << ".";
                }
                ss << static_cast<std::string>(s);
                return ss.str();
            }
        };

        template<class O>
        struct statement_serializer<table_oid_t<O>, void> {
            using statement_type = table_oid_t<O>;

            template<class Ctx>
            std::string operator()(const statement_type& s, const Ctx& context) const {
                std::stringstream ss;
                if(!context.skip_table_name) {
                    ss << streaming_identifier(lookup_table_name<O>(context.impl)) << ".";
                }
                ss << static_cast<std::string>(s);
                return ss.str();
            }
        };

        template<class O>
        struct statement_serializer<table__rowid_t<O>, void> {
            using statement_type = table__rowid_t<O>;

            template<class Ctx>
            std::string operator()(const statement_type& s, const Ctx& context) const {
                std::stringstream ss;
                if(!context.skip_table_name) {
                    ss << streaming_identifier(lookup_table_name<O>(context.impl)) << ".";
                }
                ss << static_cast<std::string>(s);
                return ss.str();
            }
        };

        template<class L, class R, class... Ds>
        struct statement_serializer<binary_operator<L, R, Ds...>, void> {
            using statement_type = binary_operator<L, R, Ds...>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                auto lhs = serialize(statement.lhs, context);
                auto rhs = serialize(statement.rhs, context);
                std::stringstream ss;
                if(context.use_parentheses) {
                    ss << '(';
                }
                ss << lhs << " " << statement.serialize() << " " << rhs;
                if(context.use_parentheses) {
                    ss << ')';
                }
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<count_asterisk_t<T>, void> {
            using statement_type = count_asterisk_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type&, const Ctx& context) const {
                return serialize(count_asterisk_without_type{}, context);
            }
        };

        template<>
        struct statement_serializer<count_asterisk_without_type, void> {
            using statement_type = count_asterisk_without_type;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx&) const {
                std::stringstream ss;
                auto functionName = c.serialize();
                ss << functionName << "(*)";
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<distinct_t<T>, void> {
            using statement_type = distinct_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                std::stringstream ss;
                auto expr = serialize(c.value, context);
                ss << static_cast<std::string>(c) << "(" << expr << ")";
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<all_t<T>, void> {
            using statement_type = all_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                std::stringstream ss;
                auto expr = serialize(c.value, context);
                ss << static_cast<std::string>(c) << "(" << expr << ")";
                return ss.str();
            }
        };

        template<class T, class F>
        struct statement_serializer<column_pointer<T, F>, void> {
            using statement_type = column_pointer<T, F>;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                std::stringstream ss;
                if(!context.skip_table_name) {
                    ss << streaming_identifier(lookup_table_name<T>(context.impl)) << ".";
                }
                if(auto* columnName = find_column_name(context.impl, c)) {
                    ss << streaming_identifier(*columnName);
                } else {
                    throw std::system_error{orm_error_code::column_not_found};
                }
                return ss.str();
            }
        };

        template<class T, class E>
        struct statement_serializer<cast_t<T, E>, void> {
            using statement_type = cast_t<T, E>;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                std::stringstream ss;
                ss << static_cast<std::string>(c) << " (";
                ss << serialize(c.expression, context) << " AS " << type_printer<T>().print() << ")";
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<T, std::enable_if_t<is_base_of_template_v<T, compound_operator>>> {
            using statement_type = T;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                std::stringstream ss;
                ss << serialize(c.left, context) << " ";
                ss << static_cast<std::string>(c) << " ";
                ss << serialize(c.right, context);
                return ss.str();
            }
        };

        template<class R, class T, class E, class... Args>
        struct statement_serializer<simple_case_t<R, T, E, Args...>, void> {
            using statement_type = simple_case_t<R, T, E, Args...>;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                std::stringstream ss;
                ss << "CASE ";
                c.case_expression.apply([&ss, context](auto& c_) {
                    ss << serialize(c_, context) << " ";
                });
                iterate_tuple(c.args, [&ss, context](auto& pair) {
                    ss << "WHEN " << serialize(pair.first, context) << " ";
                    ss << "THEN " << serialize(pair.second, context) << " ";
                });
                c.else_expression.apply([&ss, context](auto& el) {
                    ss << "ELSE " << serialize(el, context) << " ";
                });
                ss << "END";
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<is_null_t<T>, void> {
            using statement_type = is_null_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                std::stringstream ss;
                ss << serialize(c.t, context) << " " << static_cast<std::string>(c);
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<is_not_null_t<T>, void> {
            using statement_type = is_not_null_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                std::stringstream ss;
                ss << serialize(c.t, context) << " " << static_cast<std::string>(c);
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<bitwise_not_t<T>, void> {
            using statement_type = bitwise_not_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                ss << statement.serialize() << " ";
                auto cString = serialize(statement.argument, context);
                ss << " (" << cString << " )";
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<negated_condition_t<T>, void> {
            using statement_type = negated_condition_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                std::stringstream ss;
                ss << static_cast<std::string>(c) << " ";
                auto cString = serialize(c.c, context);
                ss << " (" << cString << " )";
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<T, std::enable_if_t<is_base_of_template_v<T, binary_condition>>> {
            using statement_type = T;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                auto leftString = serialize(c.l, context);
                auto rightString = serialize(c.r, context);
                std::stringstream ss;
                if(context.use_parentheses) {
                    ss << "(";
                }
                ss << leftString << " " << static_cast<std::string>(c) << " " << rightString;
                if(context.use_parentheses) {
                    ss << ")";
                }
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<named_collate<T>, void> {
            using statement_type = named_collate<T>;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                auto newContext = context;
                newContext.use_parentheses = false;
                auto res = serialize(c.expr, newContext);
                return res + " " + static_cast<std::string>(c);
            }
        };

        template<class T>
        struct statement_serializer<collate_t<T>, void> {
            using statement_type = collate_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                auto newContext = context;
                newContext.use_parentheses = false;
                auto res = serialize(c.expr, newContext);
                return res + " " + static_cast<std::string>(c);
            }
        };

        template<class L, class A>
        struct statement_serializer<dynamic_in_t<L, A>, void> {
            using statement_type = dynamic_in_t<L, A>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                auto leftString = serialize(statement.left, context);
                ss << leftString << " ";
                if(!statement.negative) {
                    ss << "IN";
                } else {
                    ss << "NOT IN";
                }
                ss << " ";
                constexpr bool isCompoundOperator = is_base_of_template_v<A, compound_operator>;
                if(isCompoundOperator) {
                    ss << '(';
                }
                auto newContext = context;
                newContext.use_parentheses = true;
                ss << serialize(statement.argument, newContext);
                if(isCompoundOperator) {
                    ss << ')';
                }
                return ss.str();
            }
        };

        template<class L, class E>
        struct statement_serializer<dynamic_in_t<L, std::vector<E>>, void> {
            using statement_type = dynamic_in_t<L, std::vector<E>>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                auto leftString = serialize(statement.left, context);
                ss << leftString << " ";
                if(!statement.negative) {
                    ss << "IN";
                } else {
                    ss << "NOT IN";
                }
                ss << " (" << streaming_dynamic_expressions(statement.argument, context) << ")";
                return ss.str();
            }
        };

        template<class L, class... Args>
        struct statement_serializer<in_t<L, Args...>, void> {
            using statement_type = in_t<L, Args...>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                auto leftString = serialize(statement.left, context);
                ss << leftString << " ";
                if(!statement.negative) {
                    ss << "IN";
                } else {
                    ss << "NOT IN";
                }
                ss << " ";
                using args_type = std::tuple<Args...>;
                const bool theOnlySelect =
                    std::tuple_size<args_type>::value == 1 && is_select_v<std::tuple_element_t<0, args_type>>;
                if(!theOnlySelect) {
                    ss << "(";
                }
                ss << streaming_expressions_tuple(statement.argument, context);
                if(!theOnlySelect) {
                    ss << ")";
                }
                return ss.str();
            }
        };

        template<class A, class T, class E>
        struct statement_serializer<like_t<A, T, E>, void> {
            using statement_type = like_t<A, T, E>;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                std::stringstream ss;
                ss << serialize(c.arg, context) << " ";
                ss << static_cast<std::string>(c) << " ";
                ss << serialize(c.pattern, context);
                c.arg3.apply([&ss, &context](auto& value) {
                    ss << " ESCAPE " << serialize(value, context);
                });
                return ss.str();
            }
        };

        template<class A, class T>
        struct statement_serializer<glob_t<A, T>, void> {
            using statement_type = glob_t<A, T>;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                std::stringstream ss;
                ss << serialize(c.arg, context) << " ";
                ss << static_cast<std::string>(c) << " ";
                ss << serialize(c.pattern, context);
                return ss.str();
            }
        };

        template<class A, class T>
        struct statement_serializer<between_t<A, T>, void> {
            using statement_type = between_t<A, T>;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                std::stringstream ss;
                auto expr = serialize(c.expr, context);
                ss << expr << " " << static_cast<std::string>(c) << " ";
                ss << serialize(c.b1, context);
                ss << " AND ";
                ss << serialize(c.b2, context);
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<exists_t<T>, void> {
            using statement_type = exists_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                ss << "EXISTS ";
                ss << serialize(statement.expression, context);
                return ss.str();
            }
        };

        template<>
        struct statement_serializer<autoincrement_t, void> {
            using statement_type = autoincrement_t;

            template<class Ctx>
            std::string operator()(const statement_type&, const Ctx&) const {
                return "AUTOINCREMENT";
            }
        };

        template<class... Cs>
        struct statement_serializer<primary_key_t<Cs...>, void> {
            using statement_type = primary_key_t<Cs...>;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                std::stringstream ss;
                ss << static_cast<std::string>(c);
                using columns_tuple = typename statement_type::columns_tuple;
                const size_t columnsCount = std::tuple_size<columns_tuple>::value;
                if(columnsCount) {
                    ss << "(" << streaming_mapped_columns_expressions(c.columns, context) << ")";
                }
                return ss.str();
            }
        };

        template<class... Args>
        struct statement_serializer<unique_t<Args...>, void> {
            using statement_type = unique_t<Args...>;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                std::stringstream ss;
                ss << static_cast<std::string>(c);
                using columns_tuple = typename statement_type::columns_tuple;
                const size_t columnsCount = std::tuple_size<columns_tuple>::value;
                if(columnsCount) {
                    ss << "(" << streaming_mapped_columns_expressions(c.columns, context) << ")";
                }
                return ss.str();
            }
        };

        template<>
        struct statement_serializer<collate_constraint_t, void> {
            using statement_type = collate_constraint_t;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx&) const {
                return static_cast<std::string>(c);
            }
        };

        template<class T>
        struct statement_serializer<default_t<T>, void> {
            using statement_type = default_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                return static_cast<std::string>(c) + " (" + serialize(c.value, context) + ")";
            }
        };

        template<class... Cs, class... Rs>
        struct statement_serializer<foreign_key_t<std::tuple<Cs...>, std::tuple<Rs...>>, void> {
            using statement_type = foreign_key_t<std::tuple<Cs...>, std::tuple<Rs...>>;

            template<class Ctx>
            std::string operator()(const statement_type& fk, const Ctx& context) const {
                std::stringstream ss;
                ss << "FOREIGN KEY(" << streaming_mapped_columns_expressions(fk.columns, context) << ") REFERENCES ";
                {
                    using references_type_t = typename std::decay_t<decltype(fk)>::references_type;
                    using first_reference_t = std::tuple_element_t<0, references_type_t>;
                    using first_reference_mapped_type = typename internal::table_type<first_reference_t>::type;
                    auto refTableName = lookup_table_name<first_reference_mapped_type>(context.impl);
                    ss << streaming_identifier(refTableName);
                }
                ss << "(" << streaming_mapped_columns_expressions(fk.references, context) << ")";
                if(fk.on_update) {
                    ss << ' ' << static_cast<std::string>(fk.on_update) << " " << fk.on_update._action;
                }
                if(fk.on_delete) {
                    ss << ' ' << static_cast<std::string>(fk.on_delete) << " " << fk.on_delete._action;
                }
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<check_t<T>, void> {
            using statement_type = check_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                return "CHECK (" + serialize(statement.expression, context) + ")";
            }
        };
#if SQLITE_VERSION_NUMBER >= 3031000
        template<class T>
        struct statement_serializer<generated_always_t<T>, void> {
            using statement_type = generated_always_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                if(statement.full) {
                    ss << "GENERATED ALWAYS ";
                }
                ss << "AS (";
                ss << serialize(statement.expression, context) << ")";
                switch(statement.storage) {
                    case basic_generated_always::storage_type::not_specified:
                        //..
                        break;
                    case basic_generated_always::storage_type::virtual_:
                        ss << " VIRTUAL";
                        break;
                    case basic_generated_always::storage_type::stored:
                        ss << " STORED";
                        break;
                }
                return ss.str();
            }
        };
#endif
        template<class O, class T, class G, class S, class... Op>
        struct statement_serializer<column_t<O, T, G, S, Op...>, void> {
            using statement_type = column_t<O, T, G, S, Op...>;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                using column_type = std::decay_t<decltype(c)>;
                using field_type = typename column_type::field_type;
                using constraints_type = typename column_type::constraints_type;

                std::stringstream ss;
                ss << streaming_identifier(c.name) << " " << type_printer<field_type>().print() << " ";
                {
                    std::vector<std::string> constraintsStrings;
                    constexpr size_t constraintsCount = std::tuple_size<constraints_type>::value;
                    constraintsStrings.reserve(constraintsCount);
                    int primaryKeyIndex = -1;
                    int autoincrementIndex = -1;
                    int tupleIndex = 0;
                    iterate_tuple(
                        c.constraints,
                        [&constraintsStrings, &primaryKeyIndex, &autoincrementIndex, &tupleIndex, &context](auto& v) {
                            using constraint_type = std::decay_t<decltype(v)>;
                            constraintsStrings.push_back(serialize(v, context));
                            if(is_primary_key_v<constraint_type>) {
                                primaryKeyIndex = tupleIndex;
                            } else if(std::is_same<autoincrement_t, constraint_type>::value) {
                                autoincrementIndex = tupleIndex;
                            }
                            ++tupleIndex;
                        });
                    if(primaryKeyIndex != -1 && autoincrementIndex != -1 && autoincrementIndex < primaryKeyIndex) {
                        iter_swap(constraintsStrings.begin() + primaryKeyIndex,
                                  constraintsStrings.begin() + autoincrementIndex);
                    }
                    for(auto& str: constraintsStrings) {
                        ss << str << ' ';
                    }
                }
                if(c.is_not_null()) {
                    ss << "NOT NULL ";
                }
                return ss.str();
            }
        };

        template<class T, class... Args>
        struct statement_serializer<remove_all_t<T, Args...>, void> {
            using statement_type = remove_all_t<T, Args...>;

            template<class Ctx>
            std::string operator()(const statement_type& rem, const Ctx& context) const {
                auto& tImpl = pick_impl<T>(context.impl);

                std::stringstream ss;
                ss << "DELETE FROM " << streaming_identifier(tImpl.table.name)
                   << streaming_conditions_tuple(rem.conditions, context);
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<replace_t<T>, void> {
            using statement_type = replace_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                using expression_type = std::decay_t<decltype(statement)>;
                using object_type = typename expression_object_type<expression_type>::type;
                auto& tImpl = pick_impl<object_type>(context.impl);
                std::stringstream ss;
                ss << "REPLACE INTO " << streaming_identifier(tImpl.table.name)  ///
                   << " (" << streaming_non_generated_column_names(tImpl.table) << ")"  ///
                   << " VALUES ("
                   << streaming_column_field_values_excluding(
                          mpl::quote_fn<is_generated_always>{},
                          [](auto& /*column*/) {
                              return false;  //  don't exclude
                          },
                          context,
                          get_ref(statement.object))
                   << ")";
                return ss.str();
            }
        };

        template<class T, class... Cols>
        struct statement_serializer<insert_explicit<T, Cols...>, void> {
            using statement_type = insert_explicit<T, Cols...>;

            template<class Ctx>
            std::string operator()(const statement_type& ins, const Ctx& context) const {
                constexpr size_t colsCount = std::tuple_size<std::tuple<Cols...>>::value;
                static_assert(colsCount > 0, "Use insert or replace with 1 argument instead");
                using expression_type = std::decay_t<decltype(ins)>;
                using object_type = typename expression_object_type<expression_type>::type;
                auto& tImpl = pick_impl<object_type>(context.impl);
                std::stringstream ss;
                ss << "INSERT INTO " << streaming_identifier(tImpl.table.name) << " ";
                ss << "(" << streaming_mapped_columns_expressions(ins.columns.columns, context) << ") "
                   << "VALUES (";
                iterate_tuple(ins.columns.columns,
                              [&ss, &context, &object = get_ref(ins.obj), first = true](auto& memberPointer) mutable {
                                  using member_pointer_type = std::decay_t<decltype(memberPointer)>;
                                  static_assert(!is_setter_v<member_pointer_type>,
                                                "Unable to use setter within insert explicit");

                                  constexpr std::array<const char*, 2> sep = {", ", ""};
                                  ss << sep[std::exchange(first, false)]  ///
                                     << serialize(polyfill::invoke(memberPointer, object), context);
                              });
                ss << ")";
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<update_t<T>, void> {
            using statement_type = update_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                using expression_type = std::decay_t<decltype(statement)>;
                using object_type = typename expression_object_type<expression_type>::type;
                auto& tImpl = pick_impl<object_type>(context.impl);

                std::stringstream ss;
                ss << "UPDATE " << streaming_identifier(tImpl.table.name) << " SET ";
                tImpl.table
                    .template for_each_column_excluding<mpl::disjunction_fn<is_primary_key, is_generated_always>>(
                        [&tImpl, &ss, &context, &object = get_ref(statement.object), first = true](
                            auto& column) mutable {
                            if(tImpl.table.exists_in_composite_primary_key(column)) {
                                return;
                            }

                            constexpr std::array<const char*, 2> sep = {", ", ""};
                            ss << sep[std::exchange(first, false)]  ///
                               << streaming_identifier(column.name) << " = "
                               << serialize(polyfill::invoke(column.member_pointer, object), context);
                        });
                ss << " WHERE ";
                tImpl.table.for_each_column([&tImpl, &context, &ss, &object = get_ref(statement.object), first = true](
                                                auto& column) mutable {
                    if(!column.template is<is_primary_key>() && !tImpl.table.exists_in_composite_primary_key(column)) {
                        return;
                    }

                    constexpr std::array<const char*, 2> sep = {" AND ", ""};
                    ss << sep[std::exchange(first, false)]  ///
                       << streaming_identifier(column.name) << " = "
                       << serialize(polyfill::invoke(column.member_pointer, object), context);
                });
                return ss.str();
            }
        };

        template<class... Args>
        struct statement_serializer<set_t<Args...>, void> {
            using statement_type = set_t<Args...>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                ss << "SET ";
                size_t assignIndex = 0;
                auto leftContext = context;
                leftContext.skip_table_name = true;
                iterate_tuple(statement.assigns, [&ss, &context, &leftContext, first = true](auto& value) mutable {
                    constexpr std::array<const char*, 2> sep = {", ", ""};
                    ss << sep[std::exchange(first, false)]  ///
                       << serialize(value.lhs, leftContext)  ///
                       << ' ' << value.serialize() << ' ' << serialize(value.rhs, context);
                });
                return ss.str();
            }
        };

        template<class... Args, class... Wargs>
        struct statement_serializer<update_all_t<set_t<Args...>, Wargs...>, void> {
            using statement_type = update_all_t<set_t<Args...>, Wargs...>;

            template<class Ctx>
            std::string operator()(const statement_type& upd, const Ctx& context) const {
                table_name_collector collector([&context](const std::type_index& ti) {
                    return find_table_name(context.impl, ti);
                });
                iterate_ast(upd.set.assigns, collector);

                if(collector.table_names.empty()) {
                    throw std::system_error{orm_error_code::no_tables_specified};
                }

                std::stringstream ss;
                ss << "UPDATE " << streaming_identifier(collector.table_names.begin()->first) << " SET ";
                {
                    std::vector<std::string> setPairs;
                    setPairs.reserve(std::tuple_size<typename set_t<Args...>::assigns_type>::value);
                    auto leftContext = context;
                    leftContext.skip_table_name = true;
                    iterate_tuple(upd.set.assigns, [&context, &leftContext, &setPairs](auto& asgn) {
                        std::stringstream sss;
                        sss << serialize(asgn.lhs, leftContext);
                        sss << ' ' << asgn.serialize() << ' ';
                        sss << serialize(asgn.rhs, context);
                        setPairs.push_back(sss.str());
                    });
                    ss << streaming_serialized(setPairs) << streaming_conditions_tuple(upd.conditions, context);
                    return ss.str();
                }
            }
        };

        template<class T>
        struct statement_serializer<insert_t<T>, void> {
            using statement_type = insert_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                using object_type = typename expression_object_type<statement_type>::type;
                auto& tImpl = pick_impl<object_type>(context.impl);
                using is_without_rowid = typename std::decay_t<decltype(tImpl.table)>::is_without_rowid;

                std::vector<std::reference_wrapper<const std::string>> columnNames;
                tImpl.table.template for_each_column_excluding<
                    mpl::conjunction<mpl::not_<mpl::always<is_without_rowid>>,
                                     mpl::disjunction_fn<is_primary_key, is_generated_always>>>(
                    [&tImpl, &columnNames](auto& column) {
                        if(tImpl.table.exists_in_composite_primary_key(column)) {
                            return;
                        }

                        columnNames.push_back(cref(column.name));
                    });
                const size_t columnNamesCount = columnNames.size();

                std::stringstream ss;
                ss << "INSERT INTO " << streaming_identifier(tImpl.table.name) << " ";
                if(columnNamesCount) {
                    ss << "(" << streaming_identifiers(columnNames) << ")";
                } else {
                    ss << "DEFAULT";
                }
                ss << " VALUES";
                if(columnNamesCount) {
                    ss << " ("
                       << streaming_column_field_values_excluding(
                              mpl::conjunction<mpl::not_<mpl::always<is_without_rowid>>,
                                               mpl::disjunction_fn<is_primary_key, is_generated_always>>{},
                              [&tImpl](auto& column) {
                                  return tImpl.table.exists_in_composite_primary_key(column);
                              },
                              context,
                              get_ref(statement.object))
                       << ")";
                }

                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<into_t<T>, void> {
            using statement_type = into_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type&, const Ctx& context) const {
                auto& tImpl = pick_impl<T>(context.impl);

                std::stringstream ss;
                ss << "INTO " << streaming_identifier(tImpl.table.name);
                return ss.str();
            }
        };

        template<class... Args>
        struct statement_serializer<columns_t<Args...>, void> {
            using statement_type = columns_t<Args...>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                if(context.use_parentheses) {
                    ss << '(';
                }
                ss << streaming_expressions_tuple(statement.columns, context);
                if(context.use_parentheses) {
                    ss << ')';
                }
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<T, std::enable_if_t<polyfill::disjunction_v<is_insert_raw<T>, is_replace_raw<T>>>> {
            using statement_type = T;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                if(is_insert_raw_v<T>) {
                    ss << "INSERT";
                } else {
                    ss << "REPLACE";
                }
                iterate_tuple(statement.args, [&context, &ss](auto& value) {
                    using value_type = std::decay_t<decltype(value)>;
                    ss << ' ';
                    if(is_columns_v<value_type>) {
                        auto newContext = context;
                        newContext.skip_table_name = true;
                        newContext.use_parentheses = true;
                        ss << serialize(value, newContext);
                    } else if(is_values_v<value_type> || is_select_v<value_type>) {
                        auto newContext = context;
                        newContext.use_parentheses = false;
                        ss << serialize(value, newContext);
                    } else {
                        ss << serialize(value, context);
                    }
                });
                return ss.str();
            }
        };

        template<class T, class... Ids>
        struct statement_serializer<remove_t<T, Ids...>, void> {
            using statement_type = remove_t<T, Ids...>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                auto& tImpl = pick_impl<T>(context.impl);
                std::stringstream ss;
                ss << "DELETE FROM " << streaming_identifier(tImpl.table.name) << " "
                   << "WHERE ";
                std::vector<std::string> idsStrings;
                idsStrings.reserve(std::tuple_size<typename statement_type::ids_type>::value);
                iterate_tuple(statement.ids, [&idsStrings, &context](auto& idValue) {
                    idsStrings.push_back(serialize(idValue, context));
                });
                tImpl.table.for_each_primary_key_column(
                    [&ss, &tImpl, &idsStrings, index = 0](auto& memberPointer) mutable {
                        auto* columnName = tImpl.table.find_column_name(memberPointer);
                        if(!columnName) {
                            throw std::system_error{orm_error_code::column_not_found};
                        }

                        if(index > 0) {
                            ss << " AND ";
                        }
                        ss << streaming_identifier(*columnName) << " = " << idsStrings[index];
                        ++index;
                    });
                return ss.str();
            }
        };

        template<class It, class L, class O>
        struct statement_serializer<replace_range_t<It, L, O>, void> {
            using statement_type = replace_range_t<It, L, O>;

            template<class Ctx>
            std::string operator()(const statement_type& rep, const Ctx& context) const {
                using expression_type = std::decay_t<decltype(rep)>;
                using object_type = typename expression_object_type<expression_type>::type;
                auto& tImpl = pick_impl<object_type>(context.impl);

                std::stringstream ss;
                ss << "REPLACE INTO " << streaming_identifier(tImpl.table.name) << " ("
                   << streaming_non_generated_column_names(tImpl.table) << ")";
                const auto valuesCount = std::distance(rep.range.first, rep.range.second);
                const auto columnsCount = tImpl.table.non_generated_columns_count();
                ss << " VALUES " << streaming_values_placeholders(columnsCount, valuesCount);
                return ss.str();
            }
        };

        template<class It, class L, class O>
        struct statement_serializer<insert_range_t<It, L, O>, void> {
            using statement_type = insert_range_t<It, L, O>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                using object_type = typename expression_object_type<statement_type>::type;
                auto& tImpl = pick_impl<object_type>(context.impl);
                using is_without_rowid = typename std::decay_t<decltype(tImpl.table)>::is_without_rowid;

                std::vector<std::reference_wrapper<const std::string>> columnNames;
                tImpl.table.template for_each_column_excluding<
                    mpl::conjunction<mpl::not_<mpl::always<is_without_rowid>>,
                                     mpl::disjunction_fn<is_primary_key, is_generated_always>>>(
                    [&tImpl, &columnNames](auto& column) {
                        if(tImpl.table.exists_in_composite_primary_key(column)) {
                            return;
                        }

                        columnNames.push_back(cref(column.name));
                    });
                const size_t valuesCount = std::distance(statement.range.first, statement.range.second);
                const size_t columnNamesCount = columnNames.size();

                std::stringstream ss;
                ss << "INSERT INTO " << streaming_identifier(tImpl.table.name) << " ";
                if(columnNamesCount) {
                    ss << "(" << streaming_identifiers(columnNames) << ")";
                } else {
                    ss << "DEFAULT";
                }
                ss << " VALUES ";
                if(columnNamesCount) {
                    ss << streaming_values_placeholders(columnNamesCount, valuesCount);
                } else if(valuesCount != 1) {
                    throw std::system_error{orm_error_code::cannot_use_default_value};
                }
                return ss.str();
            }
        };

        template<class T, class Ctx>
        std::string serialize_get_all_impl(const T& get, const Ctx& context) {
            using primary_type = type_t<T>;

            table_name_collector collector;
            collector.table_names.emplace(lookup_table_name<primary_type>(context.impl), "");
            // note: not collecting table names from get.conditions;

            auto& tImpl = pick_impl<primary_type>(context.impl);
            std::stringstream ss;
            ss << "SELECT " << streaming_table_column_names(tImpl.table, true);
            if(!collector.table_names.empty()) {
                ss << " FROM " << streaming_identifiers(collector.table_names);
            }
            ss << streaming_conditions_tuple(get.conditions, context);
            return ss.str();
        }

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
        template<class T, class R, class... Args>
        struct statement_serializer<get_all_optional_t<T, R, Args...>, void> {
            using statement_type = get_all_optional_t<T, R, Args...>;

            template<class Ctx>
            std::string operator()(const statement_type& get, const Ctx& context) const {
                return serialize_get_all_impl(get, context);
            }
        };
#endif  //  SQLITE_ORM_OPTIONAL_SUPPORTED

        template<class T, class R, class... Args>
        struct statement_serializer<get_all_pointer_t<T, R, Args...>, void> {
            using statement_type = get_all_pointer_t<T, R, Args...>;

            template<class Ctx>
            std::string operator()(const statement_type& get, const Ctx& context) const {
                return serialize_get_all_impl(get, context);
            }
        };

        template<class T, class R, class... Args>
        struct statement_serializer<get_all_t<T, R, Args...>, void> {
            using statement_type = get_all_t<T, R, Args...>;

            template<class Ctx>
            std::string operator()(const statement_type& get, const Ctx& context) const {
                return serialize_get_all_impl(get, context);
            }
        };

        template<class T, class Ctx>
        std::string serialize_get_impl(const T&, const Ctx& context) {
            using primary_type = type_t<T>;
            auto& tImpl = pick_impl<primary_type>(context.impl);
            std::stringstream ss;
            ss << "SELECT " << streaming_table_column_names(tImpl.table, false) << " FROM "
               << streaming_identifier(tImpl.table.name) << " WHERE ";

            auto primaryKeyColumnNames = tImpl.table.primary_key_column_names();
            if(primaryKeyColumnNames.empty()) {
                throw std::system_error{orm_error_code::table_has_no_primary_key_column};
            }

            for(size_t i = 0; i < primaryKeyColumnNames.size(); ++i) {
                if(i > 0) {
                    ss << " AND ";
                }
                ss << streaming_identifier(primaryKeyColumnNames[i]) << " = ?";
            }
            return ss.str();
        }

        template<class T, class... Ids>
        struct statement_serializer<get_t<T, Ids...>, void> {
            using statement_type = get_t<T, Ids...>;

            template<class Ctx>
            std::string operator()(const statement_type& get, const Ctx& context) const {
                return serialize_get_impl(get, context);
            }
        };

        template<class T, class... Ids>
        struct statement_serializer<get_pointer_t<T, Ids...>, void> {
            using statement_type = get_pointer_t<T, Ids...>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                return serialize_get_impl(statement, context);
            }
        };

        template<>
        struct statement_serializer<conflict_action, void> {
            using statement_type = conflict_action;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx&) const {
                switch(statement) {
                    case conflict_action::replace:
                        return "REPLACE";
                    case conflict_action::abort:
                        return "ABORT";
                    case conflict_action::fail:
                        return "FAIL";
                    case conflict_action::ignore:
                        return "IGNORE";
                    case conflict_action::rollback:
                        return "ROLLBACK";
                }
                return {};
            }
        };

        template<>
        struct statement_serializer<insert_constraint, void> {
            using statement_type = insert_constraint;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                return "OR " + serialize(statement.action, context);
            }
        };

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
        template<class T, class... Ids>
        struct statement_serializer<get_optional_t<T, Ids...>, void> {
            using statement_type = get_optional_t<T, Ids...>;

            template<class Ctx>
            std::string operator()(const statement_type& get, const Ctx& context) const {
                return serialize_get_impl(get, context);
            }
        };
#endif  //  SQLITE_ORM_OPTIONAL_SUPPORTED
        template<class T, class... Args>
        struct statement_serializer<select_t<T, Args...>, void> {
            using statement_type = select_t<T, Args...>;

            template<class Ctx>
            std::string operator()(const statement_type& sel, const Ctx& context) const {
                std::stringstream ss;
                constexpr bool isCompoundOperator = is_base_of_template_v<T, compound_operator>;
                if(!isCompoundOperator) {
                    if(!sel.highest_level && context.use_parentheses) {
                        ss << "(";
                    }
                    ss << "SELECT ";
                }
                if(get_distinct(sel.col)) {
                    ss << static_cast<std::string>(distinct(0)) << " ";
                }
                ss << streaming_serialized(get_column_names(sel.col, context));
                table_name_collector collector([&context](const std::type_index& ti) {
                    return find_table_name(context.impl, ti);
                });
                constexpr bool explicitFromItemsCount = count_tuple<std::tuple<Args...>, is_from>::value;
                if(!explicitFromItemsCount) {
                    iterate_ast(sel.col, collector);
                    iterate_ast(sel.conditions, collector);
                    join_iterator<Args...>()([&collector, &context](const auto& c) {
                        using original_join_type = typename std::decay_t<decltype(c)>::join_type::type;
                        using cross_join_type = mapped_type_proxy_t<original_join_type>;
                        auto crossJoinedTableName = lookup_table_name<cross_join_type>(context.impl);
                        auto tableAliasString = alias_extractor<original_join_type>::get();
                        std::pair<std::string, std::string> tableNameWithAlias{std::move(crossJoinedTableName),
                                                                               std::move(tableAliasString)};
                        collector.table_names.erase(tableNameWithAlias);
                    });
                    if(!collector.table_names.empty() && !isCompoundOperator) {
                        ss << " FROM " << streaming_identifiers(collector.table_names);
                    }
                }
                ss << streaming_conditions_tuple(sel.conditions, context);
                if(!is_base_of_template_v<T, compound_operator>) {
                    if(!sel.highest_level && context.use_parentheses) {
                        ss << ")";
                    }
                }
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<indexed_column_t<T>, void> {
            using statement_type = indexed_column_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                ss << serialize(statement.column_or_expression, context);
                if(!statement._collation_name.empty()) {
                    ss << " COLLATE " << statement._collation_name;
                }
                if(statement._order) {
                    switch(statement._order) {
                        case -1:
                            ss << " DESC";
                            break;
                        case 1:
                            ss << " ASC";
                            break;
                        default:
                            throw std::system_error{orm_error_code::incorrect_order};
                    }
                }
                return ss.str();
            }
        };

        template<class... Cols>
        struct statement_serializer<index_t<Cols...>, void> {
            using statement_type = index_t<Cols...>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                ss << "CREATE ";
                if(statement.unique) {
                    ss << "UNIQUE ";
                }
                using elements_type = typename std::decay_t<decltype(statement)>::elements_type;
                using head_t = typename std::tuple_element_t<0, elements_type>::column_type;
                using indexed_type = typename table_type<head_t>::type;
                ss << "INDEX IF NOT EXISTS " << streaming_identifier(statement.name) << " ON "
                   << streaming_identifier(lookup_table_name<indexed_type>(context.impl));
                std::vector<std::string> columnNames;
                std::string whereString;
                iterate_tuple(statement.elements, [&columnNames, &context, &whereString](auto& value) {
                    using value_type = std::decay_t<decltype(value)>;
                    if(!is_where_v<value_type>) {
                        auto newContext = context;
                        newContext.use_parentheses = false;
                        auto whereString = serialize(value, newContext);
                        columnNames.push_back(move(whereString));
                    } else {
                        auto columnName = serialize(value, context);
                        whereString = move(columnName);
                    }
                });
                ss << " (" << streaming_serialized(columnNames) << ")";
                if(!whereString.empty()) {
                    ss << ' ' << whereString;
                }
                return ss.str();
            }
        };

        template<class... Args>
        struct statement_serializer<from_t<Args...>, void> {
            using statement_type = from_t<Args...>;

            template<class Ctx>
            std::string operator()(const statement_type&, const Ctx& context) const {
                using tuple = std::tuple<Args...>;

                std::stringstream ss;
                ss << "FROM ";
                iterate_tuple<tuple>([&context, &ss, first = true](auto* item) mutable {
                    using from_type = std::remove_pointer_t<decltype(item)>;

                    constexpr std::array<const char*, 2> sep = {", ", ""};
                    ss << sep[std::exchange(first, false)]
                       << streaming_identifier(lookup_table_name<mapped_type_proxy_t<from_type>>(context.impl),
                                               alias_extractor<from_type>::get());
                });
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<old_t<T>, void> {
            using statement_type = old_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                ss << "OLD.";
                auto newContext = context;
                newContext.skip_table_name = true;
                ss << serialize(statement.expression, newContext);
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<new_t<T>, void> {
            using statement_type = new_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                ss << "NEW.";
                auto newContext = context;
                newContext.skip_table_name = true;
                ss << serialize(statement.expression, newContext);
                return ss.str();
            }
        };

        template<>
        struct statement_serializer<raise_t, void> {
            using statement_type = raise_t;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                switch(statement.type) {
                    case raise_t::type_t::ignore:
                        return "RAISE(IGNORE)";

                    case raise_t::type_t::rollback:
                        return "RAISE(ROLLBACK, " + serialize(statement.message, context) + ")";

                    case raise_t::type_t::abort:
                        return "RAISE(ABORT, " + serialize(statement.message, context) + ")";

                    case raise_t::type_t::fail:
                        return "RAISE(FAIL, " + serialize(statement.message, context) + ")";
                }
                return {};
            }
        };

        template<>
        struct statement_serializer<trigger_timing, void> {
            using statement_type = trigger_timing;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx&) const {
                switch(statement) {
                    case trigger_timing::trigger_before:
                        return "BEFORE";
                    case trigger_timing::trigger_after:
                        return "AFTER";
                    case trigger_timing::trigger_instead_of:
                        return "INSTEAD OF";
                }
                return {};
            }
        };

        template<>
        struct statement_serializer<trigger_type, void> {
            using statement_type = trigger_type;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx&) const {
                switch(statement) {
                    case trigger_type::trigger_delete:
                        return "DELETE";
                    case trigger_type::trigger_insert:
                        return "INSERT";
                    case trigger_type::trigger_update:
                        return "UPDATE";
                }
                return {};
            }
        };

        template<>
        struct statement_serializer<trigger_type_base_t, void> {
            using statement_type = trigger_type_base_t;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;

                ss << serialize(statement.timing, context) << " " << serialize(statement.type, context);
                return ss.str();
            }
        };

        template<class... Cs>
        struct statement_serializer<trigger_update_type_t<Cs...>, void> {
            using statement_type = trigger_update_type_t<Cs...>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;

                ss << serialize(statement.timing, context) << " UPDATE OF "
                   << streaming_mapped_columns_expressions(statement.columns, context);
                return ss.str();
            }
        };

        template<class T, class W, class Trigger>
        struct statement_serializer<trigger_base_t<T, W, Trigger>, void> {
            using statement_type = trigger_base_t<T, W, Trigger>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;

                ss << serialize(statement.type_base, context);
                ss << " ON " << streaming_identifier(lookup_table_name<T>(context.impl));
                if(statement.do_for_each_row) {
                    ss << " FOR EACH ROW";
                }
                statement.container_when.apply([&ss, &context](auto& value) {
                    ss << " WHEN " << serialize(value, context);
                });
                return ss.str();
            }
        };

        template<class... S>
        struct statement_serializer<trigger_t<S...>, void> {
            using statement_type = trigger_t<S...>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                ss << "CREATE ";

                ss << "TRIGGER IF NOT EXISTS " << streaming_identifier(statement.name) << " "
                   << serialize(statement.base, context);
                ss << " BEGIN ";
                iterate_tuple(statement.elements, [&ss, &context](auto& element) {
                    using element_type = std::decay_t<decltype(element)>;
                    if(is_select_v<element_type>) {
                        auto newContext = context;
                        newContext.use_parentheses = false;
                        ss << serialize(element, newContext);
                    } else {
                        ss << serialize(element, context);
                    }
                    ss << ";";
                });
                ss << " END";

                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<where_t<T>, void> {
            using statement_type = where_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                ss << statement.serialize() << " ";
                auto whereString = serialize(statement.expression, context);
                ss << '(' << whereString << ')';
                return ss.str();
            }
        };

        template<class O>
        struct statement_serializer<order_by_t<O>, void> {
            using statement_type = order_by_t<O>;

            template<class Ctx>
            std::string operator()(const statement_type& orderBy, const Ctx& context) const {
                std::stringstream ss;
                ss << static_cast<std::string>(orderBy) << " ";
                ss << serialize_order_by(orderBy, context);
                return ss.str();
            }
        };

        template<class C>
        struct statement_serializer<dynamic_order_by_t<C>, void> {
            using statement_type = dynamic_order_by_t<C>;

            template<class Ctx>
            std::string operator()(const statement_type& orderBy, const Ctx& context) const {
                return serialize_order_by(orderBy, context);
            }
        };

        template<class... Args>
        struct statement_serializer<multi_order_by_t<Args...>, void> {
            using statement_type = multi_order_by_t<Args...>;

            template<class Ctx>
            std::string operator()(const statement_type& orderBy, const Ctx& context) const {
                std::stringstream ss;
                ss << static_cast<std::string>(orderBy) << " " << streaming_expressions_tuple(orderBy.args, context);
                return ss.str();
            }
        };

        template<class O>
        struct statement_serializer<cross_join_t<O>, void> {
            using statement_type = cross_join_t<O>;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                std::stringstream ss;
                ss << static_cast<std::string>(c) << " " << streaming_identifier(lookup_table_name<O>(context.impl));
                return ss.str();
            }
        };

        template<class T, class O>
        struct statement_serializer<inner_join_t<T, O>, void> {
            using statement_type = inner_join_t<T, O>;

            template<class Ctx>
            std::string operator()(const statement_type& l, const Ctx& context) const {
                std::stringstream ss;
                ss << static_cast<std::string>(l) << " "
                   << streaming_identifier(lookup_table_name<mapped_type_proxy_t<T>>(context.impl),
                                           alias_extractor<T>::get())
                   << serialize(l.constraint, context);
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<on_t<T>, void> {
            using statement_type = on_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& t, const Ctx& context) const {
                std::stringstream ss;
                auto newContext = context;
                newContext.skip_table_name = false;
                ss << static_cast<std::string>(t) << " " << serialize(t.arg, newContext) << " ";
                return ss.str();
            }
        };

        template<class T, class O>
        struct statement_serializer<join_t<T, O>, void> {
            using statement_type = join_t<T, O>;

            template<class Ctx>
            std::string operator()(const statement_type& l, const Ctx& context) const {
                std::stringstream ss;
                ss << static_cast<std::string>(l) << " "
                   << streaming_identifier(lookup_table_name<mapped_type_proxy_t<T>>(context.impl),
                                           alias_extractor<T>::get())
                   << " " << serialize(l.constraint, context);
                return ss.str();
            }
        };

        template<class T, class O>
        struct statement_serializer<left_join_t<T, O>, void> {
            using statement_type = left_join_t<T, O>;

            template<class Ctx>
            std::string operator()(const statement_type& l, const Ctx& context) const {
                std::stringstream ss;
                ss << static_cast<std::string>(l) << " "
                   << streaming_identifier(lookup_table_name<mapped_type_proxy_t<T>>(context.impl),
                                           alias_extractor<T>::get())
                   << " " << serialize(l.constraint, context);
                return ss.str();
            }
        };

        template<class T, class O>
        struct statement_serializer<left_outer_join_t<T, O>, void> {
            using statement_type = left_outer_join_t<T, O>;

            template<class Ctx>
            std::string operator()(const statement_type& l, const Ctx& context) const {
                std::stringstream ss;
                ss << static_cast<std::string>(l) << " "
                   << streaming_identifier(lookup_table_name<mapped_type_proxy_t<T>>(context.impl),
                                           alias_extractor<T>::get())
                   << " " << serialize(l.constraint, context);
                return ss.str();
            }
        };

        template<class O>
        struct statement_serializer<natural_join_t<O>, void> {
            using statement_type = natural_join_t<O>;

            template<class Ctx>
            std::string operator()(const statement_type& c, const Ctx& context) const {
                std::stringstream ss;
                ss << static_cast<std::string>(c) << " " << streaming_identifier(lookup_table_name<O>(context.impl));
                return ss.str();
            }
        };

        template<class T, class... Args>
        struct statement_serializer<group_by_with_having<T, Args...>, void> {
            using statement_type = group_by_with_having<T, Args...>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                auto newContext = context;
                newContext.skip_table_name = false;
                ss << "GROUP BY " << streaming_expressions_tuple(statement.args, newContext) << " HAVING "
                   << serialize(statement.expression, context);
                return ss.str();
            }
        };

        template<class... Args>
        struct statement_serializer<group_by_t<Args...>, void> {
            using statement_type = group_by_t<Args...>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                auto newContext = context;
                newContext.skip_table_name = false;
                ss << "GROUP BY " << streaming_expressions_tuple(statement.args, newContext);
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<having_t<T>, void> {
            using statement_type = having_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                auto newContext = context;
                newContext.skip_table_name = false;
                ss << "HAVING " << serialize(statement.expression, newContext);
                return ss.str();
            }
        };

        /**
         *  HO - has offset
         *  OI - offset is implicit
         */
        template<class T, bool HO, bool OI, class O>
        struct statement_serializer<limit_t<T, HO, OI, O>, void> {
            using statement_type = limit_t<T, HO, OI, O>;

            template<class Ctx>
            std::string operator()(const statement_type& limt, const Ctx& context) const {
                auto newContext = context;
                newContext.skip_table_name = false;
                std::stringstream ss;
                ss << static_cast<std::string>(limt) << " ";
                if(HO) {
                    if(OI) {
                        limt.off.apply([&newContext, &ss](auto& value) {
                            ss << serialize(value, newContext);
                        });
                        ss << ", ";
                        ss << serialize(limt.lim, newContext);
                    } else {
                        ss << serialize(limt.lim, newContext) << " OFFSET ";
                        limt.off.apply([&newContext, &ss](auto& value) {
                            ss << serialize(value, newContext);
                        });
                    }
                } else {
                    ss << serialize(limt.lim, newContext);
                }
                return ss.str();
            }
        };

        template<>
        struct statement_serializer<default_values_t, void> {
            using statement_type = default_values_t;

            template<class Ctx>
            std::string operator()(const statement_type&, const Ctx&) const {
                return "DEFAULT VALUES";
            }
        };

        template<class T, class M>
        struct statement_serializer<using_t<T, M>, void> {
            using statement_type = using_t<T, M>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                auto newContext = context;
                newContext.skip_table_name = true;
                return static_cast<std::string>(statement) + " (" + serialize(statement.column, newContext) + ")";
            }
        };

        template<class... Args>
        struct statement_serializer<std::tuple<Args...>, void> {
            using statement_type = std::tuple<Args...>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                ss << '(' << streaming_expressions_tuple(statement, context) << ')';
                return ss.str();
            }
        };

        template<class... Args>
        struct statement_serializer<values_t<Args...>, void> {
            using statement_type = values_t<Args...>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                if(context.use_parentheses) {
                    ss << '(';
                }
                ss << "VALUES " << streaming_expressions_tuple(statement.tuple, context);
                if(context.use_parentheses) {
                    ss << ')';
                }
                return ss.str();
            }
        };

        template<class T>
        struct statement_serializer<dynamic_values_t<T>, void> {
            using statement_type = dynamic_values_t<T>;

            template<class Ctx>
            std::string operator()(const statement_type& statement, const Ctx& context) const {
                std::stringstream ss;
                if(context.use_parentheses) {
                    ss << '(';
                }
                ss << "VALUES " << streaming_dynamic_expressions(statement.vector, context);
                if(context.use_parentheses) {
                    ss << ')';
                }
                return ss.str();
            }
        };
    }
}

// #include "triggers.h"

// #include "object_from_column_builder.h"

// #include "table.h"

// #include "column.h"

// #include "index.h"

// #include "util.h"

// #include "serializing_util.h"

namespace sqlite_orm {

    namespace internal {

        template<class S, class E, class SFINAE = void>
        SQLITE_ORM_INLINE_VAR constexpr bool is_preparable_v = false;

        template<class S, class E>
        SQLITE_ORM_INLINE_VAR constexpr bool
            is_preparable_v<S, E, polyfill::void_t<decltype(std::declval<S>().prepare(std::declval<E>()))>> = true;

        /**
         *  Storage class itself. Create an instanse to use it as an interfacto to sqlite db by calling `make_storage`
         *  function.
         */
        template<class... Ts>
        struct storage_t : storage_base {
            using self = storage_t<Ts...>;
            using impl_type = storage_impl<Ts...>;

            /**
             *  @param filename database filename.
             *  @param impl_ storage_impl head
             */
            storage_t(const std::string& filename, impl_type impl_) :
                storage_base{filename, foreign_keys_count(impl_)}, impl(std::move(impl_)) {}

            storage_t(const storage_t& other) : storage_base(other), impl(other.impl) {}

          protected:
            impl_type impl;

            /**
             *  Obtain a storage_t's const storage_impl.
             *  
             *  @note Historically, `serializer_context_builder` was declared friend, along with
             *  a few other library stock objects, in order limit access to the storage_impl.
             *  However, one could gain access to a storage_t's storage_impl through
             *  `serializer_context_builder`, hence leading the whole friend declaration mambo-jumbo
             *  ad absurdum.
             *  Providing a free function is way better and cleaner.
             *
             *  Hence, friend was replaced by `obtain_const_impl()` and `pick_const_impl()`.
             */
            friend const impl_type& obtain_const_impl(const self& storage) noexcept {
                return storage.impl;
            }

            template<class I>
            void create_table(sqlite3* db, const std::string& tableName, const I& tImpl) {
                using table_type = std::decay_t<decltype(tImpl.table)>;
                using context_t = serializer_context<impl_type>;

                std::stringstream ss;
                context_t context{this->impl};
                ss << "CREATE TABLE " << streaming_identifier(tableName) << " ( "
                   << streaming_expressions_tuple(tImpl.table.elements, context) << ")";
                if(table_type::is_without_rowid_v) {
                    ss << " WITHOUT ROWID";
                }
                ss.flush();
                perform_void_exec(db, ss.str());
            }
            /**
			*  Copies sourceTableName to another table with name: destinationTableName
			*  Performs INSERT INTO %destinationTableName% () SELECT %tImpl.table.column_names% FROM %sourceTableName%
			*/

            template<class I>
            void copy_table(sqlite3* db,
                            const std::string& sourceTableName,
                            const std::string& destinationTableName,
                            const I& tImpl,
                            const std::vector<const table_xinfo*>& columnsToIgnore) const;

#if SQLITE_VERSION_NUMBER >= 3035000  //  DROP COLUMN feature exists (v3.35.0)
            void drop_column(sqlite3* db, const std::string& tableName, const std::string& columnName) {
                std::stringstream ss;
                ss << "ALTER TABLE " << streaming_identifier(tableName) << " DROP COLUMN "
                   << streaming_identifier(columnName) << std::flush;
                perform_void_exec(db, ss.str());
            }
#endif
            void add_generated_cols(std::vector<const table_xinfo*>& columnsToAdd,
                                    const std::vector<table_xinfo>& storageTableInfo) {
                //  iterate through storage columns
                for(const table_xinfo& storageColumnInfo: storageTableInfo) {
                    if(storageColumnInfo.hidden) {
                        columnsToAdd.push_back(&storageColumnInfo);
                    }
                }
            }
            template<class I>
            void drop_create_with_loss(sqlite3* db, const I& tImpl) {
                // eliminated all transaction handling
                this->drop_table_internal(db, tImpl.table.name);
                this->create_table(db, tImpl.table.name, tImpl);
            }

            template<class I>
            void backup_table(sqlite3* db, const I& tImpl, const std::vector<const table_xinfo*>& columnsToIgnore) {

                //  here we copy source table to another with a name with '_backup' suffix, but in case table with such
                //  a name already exists we append suffix 1, then 2, etc until we find a free name..
                auto backupTableName = tImpl.table.name + "_backup";
                if(this->table_exists(db, backupTableName)) {
                    int suffix = 1;
                    do {
                        std::stringstream ss;
                        ss << suffix << std::flush;
                        auto anotherBackupTableName = backupTableName + ss.str();
                        if(!this->table_exists(db, anotherBackupTableName)) {
                            backupTableName = move(anotherBackupTableName);
                            break;
                        }
                        ++suffix;
                    } while(true);
                }
                this->create_table(db, backupTableName, tImpl);

                this->copy_table(db, tImpl.table.name, backupTableName, tImpl, columnsToIgnore);

                this->drop_table_internal(db, tImpl.table.name);

                this->rename_table(db, backupTableName, tImpl.table.name);
            }

            template<class O>
            void assert_mapped_type() const {
                using mapped_types_tuple = std::tuple<typename Ts::object_type...>;
                static_assert(mpl::invoke_t<check_if_tuple_has_type<O>, mapped_types_tuple>::value,
                              "type is not mapped to a storage");
            }

            template<class O>
            void assert_insertable_type() const {
                auto& tImpl = this->get_impl<O>();
                using table_type = std::decay_t<decltype(tImpl.table)>;
                using elements_type = std::decay_t<decltype(tImpl.table.elements)>;

#ifdef SQLITE_ORM_IF_CONSTEXPR_SUPPORTED
                if constexpr(!table_type::is_without_rowid_v) {
#else
                    static_if<!table_type::is_without_rowid_v>(
                        [](auto& tImpl) {  // unfortunately, this static_assert's can't see any composite keys((
                            std::ignore = tImpl;
#endif
                    static_assert(count_tuple<elements_type, is_column_with_insertable_primary_key>::value <= 1,
                                  "Attempting to execute 'insert' request into an noninsertable table was detected. "
                                  "Insertable table cannot contain > 1 primary keys. Please use 'replace' instead of "
                                  "'insert', or you can use 'insert' with explicit column listing.");
                    static_assert(
                        count_tuple<elements_type, is_column_with_noninsertable_primary_key>::value == 0,
                        "Attempting to execute 'insert' request into an noninsertable table was detected. "
                        "Insertable table cannot contain non-standard primary keys. Please use 'replace' instead "
                        "of 'insert', or you can use 'insert' with explicit column listing.");
#ifdef SQLITE_ORM_IF_CONSTEXPR_SUPPORTED
                }
#else
                        })(tImpl);
#endif
            }

            template<class O>
            auto& get_impl() const {
                return pick_impl<O>(this->impl);
            }

            template<class O>
            auto& get_impl() {
                return pick_impl<O>(this->impl);
            }

          public:
            template<class T, class... Args>
            view_t<T, self, Args...> iterate(Args&&... args) {
                this->assert_mapped_type<T>();

                auto con = this->get_connection();
                return {*this, std::move(con), std::forward<Args>(args)...};
            }

            /**
             * Delete from routine.
             * O is an object's type. Must be specified explicitly.
             * @param args optional conditions: `where`, `join` etc
             * @example: storage.remove_all<User>(); - DELETE FROM users
             * @example: storage.remove_all<User>(where(in(&User::id, {5, 6, 7}))); - DELETE FROM users WHERE id IN (5, 6, 7)
             */
            template<class O, class... Args>
            void remove_all(Args&&... args) {
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::remove_all<O>(std::forward<Args>(args)...));
                this->execute(statement);
            }

            /**
             *  Delete routine.
             *  O is an object's type. Must be specified explicitly.
             *  @param ids ids of object to be removed.
             */
            template<class O, class... Ids>
            void remove(Ids... ids) {
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::remove<O>(std::forward<Ids>(ids)...));
                this->execute(statement);
            }

            /**
             *  Update routine. Sets all non primary key fields where primary key is equal.
             *  O is an object type. May be not specified explicitly cause it can be deduced by
             *      compiler from first parameter.
             *  @param o object to be updated.
             */
            template<class O>
            void update(const O& o) {
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::update(std::ref(o)));
                this->execute(statement);
            }

            template<class... Args, class... Wargs>
            void update_all(internal::set_t<Args...> set, Wargs... wh) {
                auto statement = this->prepare(sqlite_orm::update_all(std::move(set), std::forward<Wargs>(wh)...));
                this->execute(statement);
            }

          protected:
            template<class F, class O, class... Args>
            std::string group_concat_internal(F O::*m, std::unique_ptr<std::string> y, Args&&... args) {
                this->assert_mapped_type<O>();
                std::vector<std::string> rows;
                if(y) {
                    rows = this->select(sqlite_orm::group_concat(m, move(*y)), std::forward<Args>(args)...);
                } else {
                    rows = this->select(sqlite_orm::group_concat(m), std::forward<Args>(args)...);
                }
                if(!rows.empty()) {
                    return move(rows.front());
                } else {
                    return {};
                }
            }

          public:
            /**
             *  SELECT * routine.
             *  O is an object type to be extracted. Must be specified explicitly.
             *  @return All objects of type O stored in database at the moment in `std::vector`.
             *  @note If you need to return the result in a different container type then use a different `get_all` function overload `get_all<User, std::list<User>>`
             *  @example: storage.get_all<User>() - SELECT * FROM users
             *  @example: storage.get_all<User>(where(like(&User::name, "N%")), order_by(&User::id)); - SELECT * FROM users WHERE name LIKE 'N%' ORDER BY id
             */
            template<class O, class... Args>
            auto get_all(Args&&... args) {
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::get_all<O>(std::forward<Args>(args)...));
                return this->execute(statement);
            }

            /**
             *  SELECT * routine.
             *  O is an object type to be extracted. Must be specified explicitly.
             *  R is an explicit return type. This type must have `push_back(O &&)` function.
             *  @return All objects of type O stored in database at the moment in `R`.
             *  @example: storage.get_all<User, std::list<User>>(); - SELECT * FROM users
             *  @example: storage.get_all<User, std::list<User>>(where(like(&User::name, "N%")), order_by(&User::id)); - SELECT * FROM users WHERE name LIKE 'N%' ORDER BY id
            */
            template<class O, class R, class... Args>
            auto get_all(Args&&... args) {
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::get_all<O, R>(std::forward<Args>(args)...));
                return this->execute(statement);
            }

            /**
             *  SELECT * routine.
             *  O is an object type to be extracted. Must be specified explicitly.
             *  @return All objects of type O as `std::unique_ptr<O>` inside a `std::vector` stored in database at the moment.
             *  @note If you need to return the result in a different container type then use a different `get_all_pointer` function overload `get_all_pointer<User, std::list<User>>`
             *  @example: storage.get_all_pointer<User>(); - SELECT * FROM users
             *  @example: storage.get_all_pointer<User>(where(length(&User::name) > 6)); - SELECT * FROM users WHERE LENGTH(name)  > 6
             */
            template<class O, class... Args>
            auto get_all_pointer(Args&&... args) {
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::get_all_pointer<O>(std::forward<Args>(args)...));
                return this->execute(statement);
            }

            /**
             *  SELECT * routine.
             *  O is an object type to be extracted. Must be specified explicitly.
             *  R is a container type. std::vector<std::unique_ptr<O>> is default
             *  @return All objects of type O as std::unique_ptr<O> stored in database at the moment.
             *  @example: storage.get_all_pointer<User, std::list<User>>(); - SELECT * FROM users
             *  @example: storage.get_all_pointer<User, std::list<User>>(where(length(&User::name) > 6)); - SELECT * FROM users WHERE LENGTH(name)  > 6
            */
            template<class O, class R, class... Args>
            auto get_all_pointer(Args&&... args) {
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::get_all_pointer<O, R>(std::forward<Args>(args)...));
                return this->execute(statement);
            }

            /**
             *  Select * by id routine.
             *  throws std::system_error{orm_error_code::not_found} if object not found with given
             * id. throws std::system_error with orm_error_category in case of db error. O is an object type to be
             * extracted. Must be specified explicitly.
             *  @return Object of type O where id is equal parameter passed or throws
             * `std::system_error{orm_error_code::not_found}` if there is no object with such id.
             */
            template<class O, class... Ids>
            O get(Ids... ids) {
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::get<O>(std::forward<Ids>(ids)...));
                return this->execute(statement);
            }

            /**
             *  The same as `get` function but doesn't throw an exception if noting found but returns std::unique_ptr
             * with null value. throws std::system_error in case of db error.
             */
            template<class O, class... Ids>
            std::unique_ptr<O> get_pointer(Ids... ids) {
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::get_pointer<O>(std::forward<Ids>(ids)...));
                return this->execute(statement);
            }

            /**
             * A previous version of get_pointer() that returns a shared_ptr
             * instead of a unique_ptr. New code should prefer get_pointer()
             * unless the data needs to be shared.
             *
             * @note
             * Most scenarios don't need shared ownership of data, so we should prefer
             * unique_ptr when possible. It's more efficient, doesn't require atomic
             * ops for a reference count (which can cause major slowdowns on
             * weakly-ordered platforms like ARM), and can be easily promoted to a
             * shared_ptr, exactly like we're doing here.
             * (Conversely, you _can't_ go from shared back to unique.)
             */
            template<class O, class... Ids>
            std::shared_ptr<O> get_no_throw(Ids... ids) {
                return std::shared_ptr<O>(get_pointer<O>(std::forward<Ids>(ids)...));
            }

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
            /**
             *  The same as `get` function but doesn't throw an exception if noting found but
             * returns an empty std::optional. throws std::system_error in case of db error.
             */
            template<class O, class... Ids>
            std::optional<O> get_optional(Ids... ids) {
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::get_optional<O>(std::forward<Ids>(ids)...));
                return this->execute(statement);
            }
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

            /**
             *  SELECT COUNT(*) https://www.sqlite.org/lang_aggfunc.html#count
             *  @return Number of O object in table.
             */
            template<class O, class... Args, class R = mapped_type_proxy_t<O>>
            int count(Args&&... args) {
                this->assert_mapped_type<R>();
                auto rows = this->select(sqlite_orm::count<R>(), std::forward<Args>(args)...);
                if(!rows.empty()) {
                    return rows.front();
                } else {
                    return 0;
                }
            }

            /**
             *  SELECT COUNT(X) https://www.sqlite.org/lang_aggfunc.html#count
             *  @param m member pointer to class mapped to the storage.
             *  @return count of `m` values from database.
             */
            template<class F, class O, class... Args>
            int count(F O::*m, Args&&... args) {
                this->assert_mapped_type<O>();
                auto rows = this->select(sqlite_orm::count(m), std::forward<Args>(args)...);
                if(!rows.empty()) {
                    return rows.front();
                } else {
                    return 0;
                }
            }

            /**
             *  AVG(X) query.   https://www.sqlite.org/lang_aggfunc.html#avg
             *  @param m is a class member pointer (the same you passed into make_column).
             *  @return average value from database.
             */
            template<class F, class O, class... Args>
            double avg(F O::*m, Args&&... args) {
                this->assert_mapped_type<O>();
                auto rows = this->select(sqlite_orm::avg(m), std::forward<Args>(args)...);
                if(!rows.empty()) {
                    return rows.front();
                } else {
                    return 0;
                }
            }

            template<class F, class O>
            std::string group_concat(F O::*m) {
                return this->group_concat_internal(m, {});
            }

            /**
             *  GROUP_CONCAT(X) query.  https://www.sqlite.org/lang_aggfunc.html#groupconcat
             *  @param m is a class member pointer (the same you passed into make_column).
             *  @return group_concat query result.
             */
            template<class F,
                     class O,
                     class... Args,
                     class Tuple = std::tuple<Args...>,
                     std::enable_if_t<std::tuple_size<Tuple>::value >= 1, bool> = true>
            std::string group_concat(F O::*m, Args&&... args) {
                return this->group_concat_internal(m, {}, std::forward<Args>(args)...);
            }

            /**
             *  GROUP_CONCAT(X, Y) query.   https://www.sqlite.org/lang_aggfunc.html#groupconcat
             *  @param m is a class member pointer (the same you passed into make_column).
             *  @return group_concat query result.
             */
            template<class F, class O, class... Args>
            std::string group_concat(F O::*m, std::string y, Args&&... args) {
                return this->group_concat_internal(m,
                                                   std::make_unique<std::string>(move(y)),
                                                   std::forward<Args>(args)...);
            }

            template<class F, class O, class... Args>
            std::string group_concat(F O::*m, const char* y, Args&&... args) {
                std::unique_ptr<std::string> str;
                if(y) {
                    str = std::make_unique<std::string>(y);
                } else {
                    str = std::make_unique<std::string>();
                }
                return this->group_concat_internal(m, move(str), std::forward<Args>(args)...);
            }

            /**
             *  MAX(x) query.
             *  @param m is a class member pointer (the same you passed into make_column).
             *  @return std::unique_ptr with max value or null if sqlite engine returned null.
             */
            template<class F, class O, class... Args, class Ret = column_result_of_t<self, F O::*>>
            std::unique_ptr<Ret> max(F O::*m, Args&&... args) {
                this->assert_mapped_type<O>();
                auto rows = this->select(sqlite_orm::max(m), std::forward<Args>(args)...);
                if(!rows.empty()) {
                    return std::move(rows.front());
                } else {
                    return {};
                }
            }

            /**
             *  MIN(x) query.
             *  @param m is a class member pointer (the same you passed into make_column).
             *  @return std::unique_ptr with min value or null if sqlite engine returned null.
             */
            template<class F, class O, class... Args, class Ret = column_result_of_t<self, F O::*>>
            std::unique_ptr<Ret> min(F O::*m, Args&&... args) {
                this->assert_mapped_type<O>();
                auto rows = this->select(sqlite_orm::min(m), std::forward<Args>(args)...);
                if(!rows.empty()) {
                    return std::move(rows.front());
                } else {
                    return {};
                }
            }

            /**
             *  SUM(x) query.
             *  @param m is a class member pointer (the same you passed into make_column).
             *  @return std::unique_ptr with sum value or null if sqlite engine returned null.
             */
            template<class F, class O, class... Args, class Ret = column_result_of_t<self, F O::*>>
            std::unique_ptr<Ret> sum(F O::*m, Args&&... args) {
                this->assert_mapped_type<O>();
                std::vector<std::unique_ptr<double>> rows =
                    this->select(sqlite_orm::sum(m), std::forward<Args>(args)...);
                if(!rows.empty()) {
                    if(rows.front()) {
                        return std::make_unique<Ret>(std::move(*rows.front()));
                    } else {
                        return {};
                    }
                } else {
                    return {};
                }
            }

            /**
             *  TOTAL(x) query.
             *  @param m is a class member pointer (the same you passed into make_column).
             *  @return total value (the same as SUM but not nullable. More details here
             * https://www.sqlite.org/lang_aggfunc.html)
             */
            template<class F, class O, class... Args>
            double total(F O::*m, Args&&... args) {
                this->assert_mapped_type<O>();
                auto rows = this->select(sqlite_orm::total(m), std::forward<Args>(args)...);
                if(!rows.empty()) {
                    return std::move(rows.front());
                } else {
                    return {};
                }
            }

            /**
             *  Select a single column into std::vector<T> or multiple columns into std::vector<std::tuple<...>>.
             *  For a single column use `auto rows = storage.select(&User::id, where(...));
             *  For multicolumns use `auto rows = storage.select(columns(&User::id, &User::name), where(...));
             */
            template<class T, class... Args, class R = column_result_of_t<self, T>>
            std::vector<R> select(T m, Args... args) {
                static_assert(!is_base_of_template_v<T, compound_operator> ||
                                  std::tuple_size<std::tuple<Args...>>::value == 0,
                              "Cannot use args with a compound operator");
                auto statement = this->prepare(sqlite_orm::select(std::move(m), std::forward<Args>(args)...));
                return this->execute(statement);
            }

            template<class T, satisfies<is_prepared_statement, T> = true>
            std::string dump(const T& preparedStatement, bool parametrized = true) const {
                return this->dump(preparedStatement.expression, parametrized);
            }

            template<
                class E,
                class Ex = polyfill::remove_cvref_t<E>,
                std::enable_if_t<!is_prepared_statement_v<Ex> && !storage_traits::is_mapped_v<self, Ex>, bool> = true>
            std::string dump(E&& expression, bool parametrized = false) const {
                static_assert(is_preparable_v<self, Ex>, "Expression must be a high-level statement");

                decltype(auto) e2 = static_if<is_select_v<Ex>>(
                    [](auto expression) {
                        expression.highest_level = true;
                        return expression;
                    },
                    [](const auto& expression) -> decltype(auto) {
                        return (expression);
                    })(std::forward<E>(expression));
                using context_t = serializer_context<impl_type>;
                context_t context{this->impl};
                context.replace_bindable_with_question = parametrized;
                // just like prepare_impl()
                context.skip_table_name = false;
                return serialize(e2, context);
            }

            /**
             *  Returns a string representation of object of a class mapped to the storage.
             *  Type of string has json-like style.
             */
            template<class O, satisfies<storage_traits::is_mapped, self, O> = true>
            std::string dump(const O& object) const {
                auto& tImpl = this->get_impl<O>();
                std::stringstream ss;
                ss << "{ ";
                bool first = true;
                tImpl.table.for_each_column([&ss, &first, &object](auto& column) {
                    using column_type = std::decay_t<decltype(column)>;
                    using field_type = typename column_type::field_type;
                    constexpr std::array<const char*, 2> sep = {", ", ""};

                    ss << sep[std::exchange(first, false)] << column.name << " : '"
                       << field_printer<field_type>{}(polyfill::invoke(column.member_pointer, object)) << "'";
                });
                ss << " }";
                return ss.str();
            }

            /**
             *  This is REPLACE (INSERT OR REPLACE) function.
             *  Also if you need to insert value with knows id you should
             *  also you this function instead of insert cause inserts ignores
             *  id and creates own one.
             */
            template<class O>
            void replace(const O& o) {
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::replace(std::ref(o)));
                this->execute(statement);
            }

            template<class It>
            void replace_range(It from, It to) {
                using O = typename std::iterator_traits<It>::value_type;
                this->assert_mapped_type<O>();
                if(from == to) {
                    return;
                }

                auto statement = this->prepare(sqlite_orm::replace_range(from, to));
                this->execute(statement);
            }

            template<class T, class It, class L>
            void replace_range(It from, It to, L transformer) {
                this->assert_mapped_type<T>();
                if(from == to) {
                    return;
                }

                auto statement = this->prepare(sqlite_orm::replace_range<T>(from, to, std::move(transformer)));
                this->execute(statement);
            }

            template<class O, class... Cols>
            int insert(const O& o, columns_t<Cols...> cols) {
                constexpr size_t colsCount = std::tuple_size<std::tuple<Cols...>>::value;
                static_assert(colsCount > 0, "Use insert or replace with 1 argument instead");
                this->assert_mapped_type<O>();
                auto statement = this->prepare(sqlite_orm::insert(std::ref(o), std::move(cols)));
                return int(this->execute(statement));
            }

            /**
             *  Insert routine. Inserts object with all non primary key fields in passed object. Id of passed
             *  object doesn't matter.
             *  @return id of just created object.
             */
            template<class O>
            int insert(const O& o) {
                this->assert_mapped_type<O>();
                this->assert_insertable_type<O>();
                auto statement = this->prepare(sqlite_orm::insert(std::ref(o)));
                return int(this->execute(statement));
            }

            /**
             *  Raw insert routine. Use this if `insert` with object does not fit you. This insert is designed to be able
             *  to call any type of `INSERT` query with no limitations.
             *  @example
             *  ```sql
             *  INSERT INTO users (id, name) VALUES(5, 'Little Mix')
             *  ```
             *  will be
             *  ```c++
             *  storage.insert(into<User>, columns(&User::id, &User::name), values(std::make_tuple(5, "Little Mix")));
             *  ```
             *  One more example:
             *  ```sql
             *  INSERT INTO singers (name) VALUES ('Sofia Reyes')('Kungs')
             *  ```
             *  will be
             *  ```c++
             *  storage.insert(into<Singer>(), columns(&Singer::name), values(std::make_tuple("Sofia Reyes"), std::make_tuple("Kungs")));
             *  ```
             *  One can use `default_values` to add `DEFAULT VALUES` modifier:
             *  ```sql
             *  INSERT INTO users DEFAULT VALUES
             *  ```
             *  will be
             *  ```c++
             *  storage.insert(into<Singer>(), default_values());
             *  ```
             *  Also one can use `INSERT OR ABORT`/`INSERT OR FAIL`/`INSERT OR IGNORE`/`INSERT OR REPLACE`/`INSERT ROLLBACK`:
             *  ```c++
             *  storage.insert(or_ignore(), into<Singer>(), columns(&Singer::name), values(std::make_tuple("Sofia Reyes"), std::make_tuple("Kungs")));
             *  storage.insert(or_rollback(), into<Singer>(), default_values());
             *  storage.insert(or_abort(), into<User>, columns(&User::id, &User::name), values(std::make_tuple(5, "Little Mix")));
             *  ```
             */
            template<class... Args>
            void insert(Args... args) {
                auto statement = this->prepare(sqlite_orm::insert(std::forward<Args>(args)...));
                this->execute(statement);
            }

            /**
             *  Raw replace statement creation routine. Use this if `replace` with object does not fit you. This replace is designed to be able
             *  to call any type of `REPLACE` query with no limitations. Actually this is the same query as raw insert except `OR...` option existance.
             *  @example
             *  ```sql
             *  REPLACE INTO users (id, name) VALUES(5, 'Little Mix')
             *  ```
             *  will be
             *  ```c++
             *  storage.prepare(replace(into<User>, columns(&User::id, &User::name), values(std::make_tuple(5, "Little Mix"))));
             *  ```
             *  One more example:
             *  ```sql
             *  REPLACE INTO singers (name) VALUES ('Sofia Reyes')('Kungs')
             *  ```
             *  will be
             *  ```c++
             *  storage.prepare(replace(into<Singer>(), columns(&Singer::name), values(std::make_tuple("Sofia Reyes"), std::make_tuple("Kungs"))));
             *  ```
             *  One can use `default_values` to add `DEFAULT VALUES` modifier:
             *  ```sql
             *  REPLACE INTO users DEFAULT VALUES
             *  ```
             *  will be
             *  ```c++
             *  storage.prepare(replace(into<Singer>(), default_values()));
             *  ```
             */
            template<class... Args>
            void replace(Args... args) {
                auto statement = this->prepare(sqlite_orm::replace(std::forward<Args>(args)...));
                this->execute(statement);
            }

            template<class It>
            void insert_range(It from, It to) {
                using O = typename std::iterator_traits<It>::value_type;
                this->assert_mapped_type<O>();
                this->assert_insertable_type<O>();
                if(from == to) {
                    return;
                }
                auto statement = this->prepare(sqlite_orm::insert_range(from, to));
                this->execute(statement);
            }

            template<class T, class It, class L>
            void insert_range(It from, It to, L transformer) {
                this->assert_mapped_type<T>();
                this->assert_insertable_type<T>();
                if(from == to) {
                    return;
                }
                auto statement = this->prepare(sqlite_orm::insert_range<T>(from, to, std::move(transformer)));
                this->execute(statement);
            }

            /**
             * Change table name inside storage's schema info. This function does not
             * affect database
             */
            template<class O>
            void rename_table(std::string name) {
                this->assert_mapped_type<O>();
                auto& tImpl = this->get_impl<O>();
                tImpl.table.name = move(name);
            }

            using storage_base::rename_table;

            /**
             * Get table's name stored in storage's schema info. This function does not call
             * any SQLite queries
             */
            template<class O>
            const std::string& tablename() const {
                this->assert_mapped_type<O>();
                auto& tImpl = this->get_impl<O>();
                return tImpl.table.name;
            }

            template<class F, class O>
            [[deprecated("Use the more accurately named function `find_column_name()`")]] const std::string*
            column_name(F O::*memberPointer) const {
                return internal::find_column_name(this->impl, memberPointer);
            }

            template<class F, class O>
            const std::string* find_column_name(F O::*memberPointer) const {
                return internal::find_column_name(this->impl, memberPointer);
            }

          protected:
            template<class... Tss, class... Cols>
            sync_schema_result schema_status(const storage_impl<index_t<Cols...>, Tss...>&, sqlite3*, bool) {
                return sync_schema_result::already_in_sync;
            }

            template<class T, bool WithoutRowId, class... Cs, class... Tss>
            sync_schema_result schema_status(const storage_impl<table_t<T, WithoutRowId, Cs...>, Tss...>& tImpl,
                                             sqlite3* db,
                                             bool preserve,
                                             bool* attempt_to_preserve) {
                if(attempt_to_preserve) {
                    *attempt_to_preserve = true;
                }

                auto dbTableInfo = this->pragma.table_xinfo(tImpl.table.name);
                auto res = sync_schema_result::already_in_sync;

                //  first let's see if table with such name exists..
                auto gottaCreateTable = !this->table_exists(db, tImpl.table.name);
                if(!gottaCreateTable) {

                    //  get table info provided in `make_table` call..
                    auto storageTableInfo = tImpl.table.get_table_info();

                    //  this vector will contain pointers to columns that gotta be added..
                    std::vector<const table_xinfo*> columnsToAdd;

                    if(calculate_remove_add_columns(columnsToAdd, storageTableInfo, dbTableInfo)) {
                        gottaCreateTable = true;
                    }

                    if(!gottaCreateTable) {  //  if all storage columns are equal to actual db columns but there are
                        //  excess columns at the db..
                        if(!dbTableInfo.empty()) {
                            // extra table columns than storage columns
                            if(!preserve) {
#if SQLITE_VERSION_NUMBER >= 3035000  //  DROP COLUMN feature exists (v3.35.0)
                                res = sync_schema_result::old_columns_removed;
#else
                                    gottaCreateTable = true;
#endif
                            } else {
                                res = sync_schema_result::old_columns_removed;
                            }
                        }
                    }
                    if(gottaCreateTable) {
                        res = sync_schema_result::dropped_and_recreated;
                    } else {
                        if(!columnsToAdd.empty()) {
                            // extra storage columns than table columns
                            for(const table_xinfo* colInfo: columnsToAdd) {
                                const basic_generated_always::storage_type* generatedStorageType =
                                    tImpl.table.find_column_generated_storage_type(colInfo->name);
                                if(generatedStorageType) {
                                    if(*generatedStorageType == basic_generated_always::storage_type::stored) {
                                        gottaCreateTable = true;
                                        break;
                                    }
                                    //  fallback cause VIRTUAL can be added
                                } else {
                                    if(colInfo->notnull && colInfo->dflt_value.empty()) {
                                        gottaCreateTable = true;
                                        // no matter if preserve is true or false, there is no way to preserve data, so we wont try!
                                        if(attempt_to_preserve) {
                                            *attempt_to_preserve = false;
                                        };
                                        break;
                                    }
                                }
                            }
                            if(!gottaCreateTable) {
                                if(res == sync_schema_result::old_columns_removed) {
                                    res = sync_schema_result::new_columns_added_and_old_columns_removed;
                                } else {
                                    res = sync_schema_result::new_columns_added;
                                }
                            } else {
                                res = sync_schema_result::dropped_and_recreated;
                            }
                        } else {
                            if(res != sync_schema_result::old_columns_removed) {
                                res = sync_schema_result::already_in_sync;
                            }
                        }
                    }
                } else {
                    res = sync_schema_result::new_table_created;
                }
                return res;
            }

            template<class... Tss, class... Cols>
            sync_schema_result sync_table(const storage_impl<index_t<Cols...>, Tss...>& tImpl, sqlite3* db, bool) {
                auto res = sync_schema_result::already_in_sync;
                using context_t = serializer_context<impl_type>;
                context_t context{this->impl};
                auto query = serialize(tImpl.table, context);
                perform_void_exec(db, query);
                return res;
            }

            template<class... Tss, class... Cols>
            sync_schema_result sync_table(const storage_impl<trigger_t<Cols...>, Tss...>& tImpl, sqlite3* db, bool) {
                auto res = sync_schema_result::already_in_sync;  // TODO Change accordingly
                using context_t = serializer_context<impl_type>;
                context_t context{this->impl};
                perform_void_exec(db, serialize(tImpl.table, context));
                return res;
            }

            template<class T, bool WithoutRowId, class... Args, class... Tss>
            sync_schema_result sync_table(const storage_impl<table_t<T, WithoutRowId, Args...>, Tss...>& tImpl,
                                          sqlite3* db,
                                          bool preserve);

            template<class C>
            void add_column(const std::string& tableName, const C& column, sqlite3* db) const {
                using context_t = serializer_context<impl_type>;

                context_t context{this->impl};
                std::stringstream ss;
                ss << "ALTER TABLE " << streaming_identifier(tableName) << " ADD COLUMN " << serialize(column, context)
                   << std::flush;
                perform_void_exec(db, ss.str());
            }

            template<typename S>
            prepared_statement_t<S> prepare_impl(S statement) {
                auto con = this->get_connection();
                sqlite3* db = con.get();
                sqlite3_stmt* stmt;
                using context_t = serializer_context<impl_type>;
                context_t context{this->impl};
                context.skip_table_name = false;
                context.replace_bindable_with_question = true;
                auto query = serialize(statement, context);
                if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    return prepared_statement_t<S>{std::forward<S>(statement), stmt, con};
                } else {
                    throw_translated_sqlite_error(db);
                }
            }

          public:
            /**
             *  This is a cute function used to replace migration up/down functionality.
             *  It performs check storage schema with actual db schema and:
             *  * if there are excess tables exist in db they are ignored (not dropped)
             *  * every table from storage is compared with it's db analog and
             *      * if table doesn't exist it is being created
             *      * if table exists its colums are being compared with table_info from db and
             *          * if there are columns in db that do not exist in storage (excess) table will be dropped and
             * recreated
             *          * if there are columns in storage that do not exist in db they will be added using `ALTER TABLE
             * ... ADD COLUMN ...' command
             *          * if there is any column existing in both db and storage but differs by any of
             * properties/constraints (pk, notnull, dflt_value) table will be dropped and recreated. Be aware that
             * `sync_schema` doesn't guarantee that data will not be dropped. It guarantees only that it will make db
             * schema the same as you specified in `make_storage` function call. A good point is that if you have no db
             * file at all it will be created and all tables also will be created with exact tables and columns you
             * specified in `make_storage`, `make_table` and `make_column` calls. The best practice is to call this
             * function right after storage creation.
             *  @param preserve affects function's behaviour in case it is needed to remove a column. If it is `false`
             * so table will be dropped if there is column to remove if SQLite version is < 3.35.0 and rmeove column if SQLite version >= 3.35.0,
             * if `true` -  table is being copied into another table, dropped and copied table is renamed with source table name.
             * Warning: sync_schema doesn't check foreign keys cause it is unable to do so in sqlite3. If you know how to get foreign key info please
             * submit an issue https://github.com/fnc12/sqlite_orm/issues
             *  @return std::map with std::string key equal table name and `sync_schema_result` as value.
             * `sync_schema_result` is a enum value that stores table state after syncing a schema. `sync_schema_result`
             * can be printed out on std::ostream with `operator<<`.
             */
            std::map<std::string, sync_schema_result> sync_schema(bool preserve = false) {
                auto con = this->get_connection();
                std::map<std::string, sync_schema_result> result;
                this->impl.for_each([this, db = con.get(), preserve, &result](auto& tImpl) {
                    auto res = this->sync_table(tImpl, db, preserve);
                    result.insert({tImpl.table.name, res});
                });
                return result;
            }

            /**
             *  This function returns the same map that `sync_schema` returns but it
             *  doesn't perform `sync_schema` actually - just simulates it in case you want to know
             *  what will happen if you sync your schema.
             */
            std::map<std::string, sync_schema_result> sync_schema_simulate(bool preserve = false) {
                auto con = this->get_connection();
                std::map<std::string, sync_schema_result> result;
                this->impl.for_each([this, db = con.get(), preserve, &result](auto& tImpl) {
                    auto schemaStatus = this->schema_status(tImpl, db, preserve, nullptr);
                    result.insert({tImpl.table.name, schemaStatus});
                });
                return result;
            }

            /**
             *  Checks whether table exists in db. Doesn't check storage itself - works only with actual database.
             *  Note: table can be not mapped to a storage
             *  @return true if table with a given name exists in db, false otherwise.
             */
            bool table_exists(const std::string& tableName) {
                auto con = this->get_connection();
                return this->table_exists(con.get(), tableName);
            }

            using storage_base::table_exists;  // now that it is in storage_base make it into overload set

            template<class T, class... Args>
            prepared_statement_t<select_t<T, Args...>> prepare(select_t<T, Args...> sel) {
                sel.highest_level = true;
                return prepare_impl<select_t<T, Args...>>(std::move(sel));
            }

            template<class T, class... Args>
            prepared_statement_t<get_all_t<T, Args...>> prepare(get_all_t<T, Args...> get_) {
                return prepare_impl<get_all_t<T, Args...>>(std::move(get_));
            }

            template<class T, class... Args>
            prepared_statement_t<get_all_pointer_t<T, Args...>> prepare(get_all_pointer_t<T, Args...> get_) {
                return prepare_impl<get_all_pointer_t<T, Args...>>(std::move(get_));
            }

            template<class... Args>
            prepared_statement_t<replace_raw_t<Args...>> prepare(replace_raw_t<Args...> ins) {
                return prepare_impl<replace_raw_t<Args...>>(std::move(ins));
            }

            template<class... Args>
            prepared_statement_t<insert_raw_t<Args...>> prepare(insert_raw_t<Args...> ins) {
                return prepare_impl<insert_raw_t<Args...>>(std::move(ins));
            }

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
            template<class T, class R, class... Args>
            prepared_statement_t<get_all_optional_t<T, R, Args...>> prepare(get_all_optional_t<T, R, Args...> get_) {
                return prepare_impl<get_all_optional_t<T, R, Args...>>(std::move(get_));
            }
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

            template<class... Args, class... Wargs>
            prepared_statement_t<update_all_t<set_t<Args...>, Wargs...>>
            prepare(update_all_t<set_t<Args...>, Wargs...> upd) {
                return prepare_impl<update_all_t<set_t<Args...>, Wargs...>>(std::move(upd));
            }

            template<class T, class... Args>
            prepared_statement_t<remove_all_t<T, Args...>> prepare(remove_all_t<T, Args...> rem) {
                return prepare_impl<remove_all_t<T, Args...>>(std::move(rem));
            }

            template<class T, class... Ids>
            prepared_statement_t<get_t<T, Ids...>> prepare(get_t<T, Ids...> get_) {
                return prepare_impl<get_t<T, Ids...>>(std::move(get_));
            }

            template<class T, class... Ids>
            prepared_statement_t<get_pointer_t<T, Ids...>> prepare(get_pointer_t<T, Ids...> get_) {
                return prepare_impl<get_pointer_t<T, Ids...>>(std::move(get_));
            }

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
            template<class T, class... Ids>
            prepared_statement_t<get_optional_t<T, Ids...>> prepare(get_optional_t<T, Ids...> get_) {
                return prepare_impl<get_optional_t<T, Ids...>>(std::move(get_));
            }
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

            template<class T>
            prepared_statement_t<update_t<T>> prepare(update_t<T> upd) {
                return prepare_impl<update_t<T>>(std::move(upd));
            }

            template<class T, class... Ids>
            prepared_statement_t<remove_t<T, Ids...>> prepare(remove_t<T, Ids...> statement) {
                using object_type = typename expression_object_type<decltype(statement)>::type;
                this->assert_mapped_type<object_type>();
                return this->prepare_impl<remove_t<T, Ids...>>(std::move(statement));
            }

            template<class T>
            prepared_statement_t<insert_t<T>> prepare(insert_t<T> statement) {
                using object_type = typename expression_object_type<decltype(statement)>::type;
                this->assert_mapped_type<object_type>();
                this->assert_insertable_type<object_type>();
                return this->prepare_impl<insert_t<T>>(std::move(statement));
            }

            template<class T>
            prepared_statement_t<replace_t<T>> prepare(replace_t<T> rep) {
                using object_type = typename expression_object_type<decltype(rep)>::type;
                this->assert_mapped_type<object_type>();
                return this->prepare_impl<replace_t<T>>(std::move(rep));
            }

            template<class It, class L, class O>
            prepared_statement_t<insert_range_t<It, L, O>> prepare(insert_range_t<It, L, O> statement) {
                using object_type = typename expression_object_type<decltype(statement)>::type;
                this->assert_mapped_type<object_type>();
                this->assert_insertable_type<object_type>();
                return this->prepare_impl<insert_range_t<It, L, O>>(std::move(statement));
            }

            template<class It, class L, class O>
            prepared_statement_t<replace_range_t<It, L, O>> prepare(replace_range_t<It, L, O> statement) {
                using object_type = typename expression_object_type<decltype(statement)>::type;
                this->assert_mapped_type<object_type>();
                return this->prepare_impl<replace_range_t<It, L, O>>(std::move(statement));
            }

            template<class T, class... Cols>
            prepared_statement_t<insert_explicit<T, Cols...>> prepare(insert_explicit<T, Cols...> ins) {
                using object_type = typename expression_object_type<decltype(ins)>::type;
                this->assert_mapped_type<object_type>();
                return this->prepare_impl<insert_explicit<T, Cols...>>(std::move(ins));
            }

            template<class... Args>
            void execute(const prepared_statement_t<replace_raw_t<Args...>>& statement) {
                sqlite3_stmt* stmt = reset(statement.stmt);
                iterate_ast(statement.expression.args, conditional_binder{statement.stmt});
                perform_step(stmt);
            }

            template<class... Args>
            void execute(const prepared_statement_t<insert_raw_t<Args...>>& statement) {
                sqlite3_stmt* stmt = reset(statement.stmt);
                iterate_ast(statement.expression.args, conditional_binder{stmt});
                perform_step(stmt);
            }

            template<class T, class... Cols>
            int64 execute(const prepared_statement_t<insert_explicit<T, Cols...>>& statement) {
                using statement_type = std::decay_t<decltype(statement)>;
                using expression_type = typename statement_type::expression_type;
                using object_type = typename expression_object_type<expression_type>::type;

                sqlite3_stmt* stmt = reset(statement.stmt);

                iterate_tuple(statement.expression.columns.columns,
                              [&tImpl = this->get_impl<object_type>(),
                               bind_value = field_value_binder{stmt},
                               &object = statement.expression.obj](auto& memberPointer) mutable {
                                  bind_value(tImpl.table.object_field_value(object, memberPointer));
                              });
                perform_step(stmt);
                return sqlite3_last_insert_rowid(sqlite3_db_handle(stmt));
            }

            template<class T,
                     std::enable_if_t<polyfill::disjunction_v<is_replace<T>, is_replace_range<T>>, bool> = true>
            void execute(const prepared_statement_t<T>& statement) {
                using statement_type = std::decay_t<decltype(statement)>;
                using expression_type = typename statement_type::expression_type;
                using object_type = typename expression_object_type<expression_type>::type;

                sqlite3_stmt* stmt = reset(statement.stmt);

                auto processObject = [&tImpl = this->get_impl<object_type>(),
                                      bind_value = field_value_binder{stmt}](auto& object) mutable {
                    tImpl.table.template for_each_column_excluding<is_generated_always>(
                        [&bind_value, &object](auto& column) {
                            bind_value(polyfill::invoke(column.member_pointer, object));
                        });
                };

                static_if<is_replace_range_v<T>>(
                    [&processObject](auto& statement) {
                        auto& transformer = statement.expression.transformer;
                        std::for_each(  ///
                            statement.expression.range.first,
                            statement.expression.range.second,
                            [&processObject, &transformer](auto& object) {
                                auto& realObject = transformer(object);
                                processObject(realObject);
                            });
                    },
                    [&processObject](auto& statement) {
                        auto& o = get_object(statement.expression);
                        processObject(o);
                    })(statement);
                perform_step(stmt);
            }

            template<class T, std::enable_if_t<polyfill::disjunction_v<is_insert<T>, is_insert_range<T>>, bool> = true>
            int64 execute(const prepared_statement_t<T>& statement) {
                using statement_type = std::decay_t<decltype(statement)>;
                using expression_type = typename statement_type::expression_type;
                using object_type = typename expression_object_type<expression_type>::type;

                sqlite3_stmt* stmt = reset(statement.stmt);

                auto processObject = [&tImpl = this->get_impl<object_type>(),
                                      bind_value = field_value_binder{stmt}](auto& object) mutable {
                    using table_type = std::decay_t<decltype(tImpl.table)>;
                    tImpl.table.template for_each_column_excluding<
                        mpl::conjunction<mpl::not_<mpl::always<typename table_type::is_without_rowid>>,
                                         mpl::disjunction_fn<is_generated_always, is_primary_key>>>(
                        [&tImpl, &bind_value, &object](auto& column) {
                            if(!tImpl.table.exists_in_composite_primary_key(column)) {
                                bind_value(polyfill::invoke(column.member_pointer, object));
                            }
                        });
                };

                static_if<is_insert_range_v<T>>(
                    [&processObject](auto& statement) {
                        auto& transformer = statement.expression.transformer;
                        std::for_each(  ///
                            statement.expression.range.first,
                            statement.expression.range.second,
                            [&processObject, &transformer](auto& object) {
                                auto& realObject = transformer(object);
                                processObject(realObject);
                            });
                    },
                    [&processObject](auto& statement) {
                        auto& o = get_object(statement.expression);
                        processObject(o);
                    })(statement);

                perform_step(stmt);
                return sqlite3_last_insert_rowid(sqlite3_db_handle(stmt));
            }

            template<class T, class... Ids>
            void execute(const prepared_statement_t<remove_t<T, Ids...>>& statement) {
                sqlite3_stmt* stmt = reset(statement.stmt);
                iterate_ast(statement.expression.ids, conditional_binder{stmt});
                perform_step(stmt);
            }

            template<class T>
            void execute(const prepared_statement_t<update_t<T>>& statement) {
                using statement_type = std::decay_t<decltype(statement)>;
                using expression_type = typename statement_type::expression_type;
                using object_type = typename expression_object_type<expression_type>::type;

                sqlite3_stmt* stmt = reset(statement.stmt);
                auto& tImpl = this->get_impl<object_type>();

                field_value_binder bind_value{stmt};
                auto& object = get_object(statement.expression);
                tImpl.table
                    .template for_each_column_excluding<mpl::disjunction_fn<is_generated_always, is_primary_key>>(
                        [&tImpl, &bind_value, &object](auto& column) {
                            if(!tImpl.table.exists_in_composite_primary_key(column)) {
                                bind_value(polyfill::invoke(column.member_pointer, object));
                            }
                        });
                tImpl.table.for_each_column([&tImpl, &bind_value, &object](auto& column) {
                    if(column.template is<is_primary_key>() || tImpl.table.exists_in_composite_primary_key(column)) {
                        bind_value(polyfill::invoke(column.member_pointer, object));
                    }
                });
                perform_step(stmt);
            }

            template<class T, class... Ids>
            std::unique_ptr<T> execute(const prepared_statement_t<get_pointer_t<T, Ids...>>& statement) {
                sqlite3_stmt* stmt = reset(statement.stmt);

                iterate_ast(statement.expression.ids, conditional_binder{stmt});

                std::unique_ptr<T> res;
                perform_step(stmt, [&tImpl = this->get_impl<T>(), &res](sqlite3_stmt* stmt) {
                    res = std::make_unique<T>();
                    object_from_column_builder<T> builder{*res, stmt};
                    tImpl.table.for_each_column(builder);
                });
                return res;
            }

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
            template<class T, class... Ids>
            std::optional<T> execute(const prepared_statement_t<get_optional_t<T, Ids...>>& statement) {
                sqlite3_stmt* stmt = reset(statement.stmt);

                iterate_ast(statement.expression.ids, conditional_binder{stmt});

                std::optional<T> res;
                perform_step(stmt, [&tImpl = this->get_impl<T>(), &res](sqlite3_stmt* stmt) {
                    object_from_column_builder<T> builder{res.emplace(), stmt};
                    tImpl.table.for_each_column(builder);
                });
                return res;
            }
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

            template<class T, class... Ids>
            T execute(const prepared_statement_t<get_t<T, Ids...>>& statement) {
                sqlite3_stmt* stmt = reset(statement.stmt);

                iterate_ast(statement.expression.ids, conditional_binder{stmt});

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
                std::optional<T> res;
                perform_step(stmt, [&tImpl = this->get_impl<T>(), &res](sqlite3_stmt* stmt) {
                    object_from_column_builder<T> builder{res.emplace(), stmt};
                    tImpl.table.for_each_column(builder);
                });
                if(!res.has_value()) {
                    throw std::system_error{orm_error_code::not_found};
                }
                return move(res).value();
#else
                    auto& tImpl = this->get_impl<T>();
                    auto stepRes = sqlite3_step(stmt);
                    switch(stepRes) {
                        case SQLITE_ROW: {
                            T res;
                            object_from_column_builder<T> builder{res, stmt};
                            tImpl.table.for_each_column(builder);
                            return res;
                        } break;
                        case SQLITE_DONE: {
                            throw std::system_error{orm_error_code::not_found};
                        } break;
                        default: {
                            throw_translated_sqlite_error(stmt);
                        }
                    }
#endif
            }

            template<class T, class... Args>
            void execute(const prepared_statement_t<remove_all_t<T, Args...>>& statement) {
                sqlite3_stmt* stmt = reset(statement.stmt);
                iterate_ast(statement.expression.conditions, conditional_binder{stmt});
                perform_step(stmt);
            }

            template<class... Args, class... Wargs>
            void execute(const prepared_statement_t<update_all_t<set_t<Args...>, Wargs...>>& statement) {
                sqlite3_stmt* stmt = reset(statement.stmt);
                conditional_binder bind_node{stmt};
                iterate_tuple(statement.expression.set.assigns, [&bind_node](auto& setArg) {
                    iterate_ast(setArg, bind_node);
                });
                iterate_ast(statement.expression.conditions, bind_node);
                perform_step(stmt);
            }

            template<class T, class... Args, class R = column_result_of_t<self, T>>
            std::vector<R> execute(const prepared_statement_t<select_t<T, Args...>>& statement) {
                sqlite3_stmt* stmt = reset(statement.stmt);

                iterate_ast(statement.expression, conditional_binder{stmt});

                std::vector<R> res;
                perform_steps(
                    stmt,
                    [rowExtractor = make_row_extractor<R>(lookup_table<R>(this->impl)), &res](sqlite3_stmt* stmt) {
                        res.push_back(rowExtractor.extract(stmt, 0));
                    });
                return res;
            }

            template<class T, class R, class... Args>
            R execute(const prepared_statement_t<get_all_t<T, R, Args...>>& statement) {
                sqlite3_stmt* stmt = reset(statement.stmt);

                iterate_ast(statement.expression, conditional_binder{stmt});

                R res;
                perform_steps(stmt, [&tImpl = this->get_impl<T>(), &res](sqlite3_stmt* stmt) {
                    T obj;
                    object_from_column_builder<T> builder{obj, stmt};
                    tImpl.table.for_each_column(builder);
                    res.push_back(std::move(obj));
                });
                return res;
            }

            template<class T, class R, class... Args>
            R execute(const prepared_statement_t<get_all_pointer_t<T, R, Args...>>& statement) {
                sqlite3_stmt* stmt = reset(statement.stmt);

                iterate_ast(statement.expression, conditional_binder{stmt});

                R res;
                perform_steps(stmt, [&tImpl = this->get_impl<T>(), &res](sqlite3_stmt* stmt) {
                    auto obj = std::make_unique<T>();
                    object_from_column_builder<T> builder{*obj, stmt};
                    tImpl.table.for_each_column(builder);
                    res.push_back(move(obj));
                });
                return res;
            }

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
            template<class T, class R, class... Args>
            R execute(const prepared_statement_t<get_all_optional_t<T, R, Args...>>& statement) {
                sqlite3_stmt* stmt = reset(statement.stmt);

                iterate_ast(statement.expression, conditional_binder{stmt});

                R res;
                perform_steps(stmt, [&tImpl = this->get_impl<T>(), &res](sqlite3_stmt* stmt) {
                    auto obj = std::make_optional<T>();
                    object_from_column_builder<T> builder{*obj, stmt};
                    tImpl.table.for_each_column(builder);
                    res.push_back(move(obj));
                });
                return res;
            }
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

            template<class O>
            bool has_dependent_rows(const O& object) {
                auto res = false;
                this->impl.for_each([this, &object, &res](auto& storageImpl) {
                    if(res) {
                        return;
                    }
                    storageImpl.table.for_each_foreign_key([this, &storageImpl, &object, &res](auto& foreignKey) {
                        using ForeignKey = std::decay_t<decltype(foreignKey)>;
                        using TargetType = typename ForeignKey::target_type;

#ifdef SQLITE_ORM_IF_CONSTEXPR_SUPPORTED
                        if constexpr(std::is_same<TargetType, O>::value) {
#else
                            call_if_constexpr<std::is_same<TargetType, O>::value>(
                                [this, &storageImpl, &res, &object](auto& foreignKey) {
#endif
                            std::stringstream ss;
                            ss << "SELECT COUNT(*)"
                               << " FROM " << streaming_identifier(storageImpl.table.name);
                            ss << " WHERE ";
                            auto columnIndex = 0;
                            iterate_tuple(foreignKey.columns, [&ss, &columnIndex, &storageImpl](auto& column) {
                                auto* columnName = storageImpl.table.find_column_name(column);
                                if(!columnName) {
                                    throw std::system_error{orm_error_code::column_not_found};
                                }

                                if(columnIndex > 0) {
                                    ss << " AND ";
                                }
                                ss << streaming_identifier(*columnName) << " = ?";
                                ++columnIndex;
                            });
                            ss.flush();
                            auto query = ss.str();
                            auto con = this->get_connection();
                            sqlite3* db = con.get();
                            sqlite3_stmt* stmt;
                            if(sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
                                throw_translated_sqlite_error(db);
                            }
                            statement_finalizer finalizer{stmt};

                            auto& tImpl = this->get_impl<O>();
                            iterate_tuple(
                                foreignKey.references,
                                [&tImpl, bind_value = field_value_binder{stmt}, &object](auto& memberPointer) mutable {
                                    bind_value(tImpl.table.object_field_value(object, memberPointer));
                                });
                            if(SQLITE_ROW != sqlite3_step(stmt)) {
                                throw_translated_sqlite_error(stmt);
                            }
                            auto countResult = sqlite3_column_int(stmt, 0);
                            res = countResult > 0;
                            if(SQLITE_DONE != sqlite3_step(stmt)) {
                                throw_translated_sqlite_error(stmt);
                            }
#ifdef SQLITE_ORM_IF_CONSTEXPR_SUPPORTED
                        }
#else
                                },
                                foreignKey);
#endif
                    });
                });
                return res;
            }
        };  // struct storage_t
    }

    template<class... Ts>
    internal::storage_t<Ts...> make_storage(const std::string& filename, Ts... tables) {
        return {filename, internal::storage_impl<Ts...>(std::forward<Ts>(tables)...)};
    }

    /**
     *  sqlite3_threadsafe() interface.
     */
    inline int threadsafe() {
        return sqlite3_threadsafe();
    }
}
#pragma once

#include <tuple>  //  std::tuple
#include <utility>  //  std::pair
#include <functional>  //  std::reference_wrapper
#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
#include <optional>  // std::optional
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

// #include "tuple_helper/tuple_filter.h"

// #include "conditions.h"

// #include "operators.h"

// #include "select_constraints.h"

// #include "prepared_statement.h"

// #include "optional_container.h"

// #include "core_functions.h"

// #include "function.h"

// #include "ast/excluded.h"

// #include "ast/upsert_clause.h"

// #include "ast/where.h"

// #include "ast/into.h"

// #include "ast/group_by.h"

namespace sqlite_orm {

    namespace internal {

        template<class T, class SFINAE = void>
        struct node_tuple;

        template<class T>
        using node_tuple_t = typename node_tuple<T>::type;

        template<class T, class SFINAE>
        struct node_tuple {
            using type = std::tuple<T>;
        };

        template<>
        struct node_tuple<void, void> {
            using type = std::tuple<>;
        };
#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
        template<class T>
        struct node_tuple<as_optional_t<T>, void> : node_tuple<T> {};
#endif  //  SQLITE_ORM_OPTIONAL_SUPPORTED
        template<class T>
        struct node_tuple<std::reference_wrapper<T>, void> : node_tuple<T> {};

        template<class... Args>
        struct node_tuple<group_by_t<Args...>, void> : node_tuple<std::tuple<Args...>> {};

        template<class T, class... Args>
        struct node_tuple<group_by_with_having<T, Args...>, void> {
            using args_tuple = node_tuple_t<std::tuple<Args...>>;
            using expression_tuple = node_tuple_t<T>;
            using type = conc_tuple_t<args_tuple, expression_tuple>;
        };

        template<class... TargetArgs, class... ActionsArgs>
        struct node_tuple<upsert_clause<std::tuple<TargetArgs...>, std::tuple<ActionsArgs...>>, void>
            : node_tuple<std::tuple<ActionsArgs...>> {};

        template<class... Args>
        struct node_tuple<set_t<Args...>, void> {
            using type = conc_tuple_t<node_tuple_t<Args>...>;
        };

        template<class T>
        struct node_tuple<excluded_t<T>, void> : node_tuple<T> {};

        template<class C>
        struct node_tuple<where_t<C>, void> : node_tuple<C> {};

        /**
         *  Column alias
         */
        template<class A>
        struct node_tuple<alias_holder<A>, void> : node_tuple<void> {};

        /**
         *  Literal
         */
        template<class T>
        struct node_tuple<literal_holder<T>, void> : node_tuple<void> {};

        template<class E>
        struct node_tuple<order_by_t<E>, void> : node_tuple<E> {};

        template<class T>
        struct node_tuple<T, std::enable_if_t<is_base_of_template_v<T, binary_condition>>> {
            using node_type = T;
            using left_type = typename node_type::left_type;
            using right_type = typename node_type::right_type;
            using left_node_tuple = node_tuple_t<left_type>;
            using right_node_tuple = node_tuple_t<right_type>;
            using type = conc_tuple_t<left_node_tuple, right_node_tuple>;
        };

        template<class L, class R, class... Ds>
        struct node_tuple<binary_operator<L, R, Ds...>, void> {
            using node_type = binary_operator<L, R, Ds...>;
            using left_type = typename node_type::left_type;
            using right_type = typename node_type::right_type;
            using left_node_tuple = node_tuple_t<left_type>;
            using right_node_tuple = node_tuple_t<right_type>;
            using type = conc_tuple_t<left_node_tuple, right_node_tuple>;
        };

        template<class... Args>
        struct node_tuple<columns_t<Args...>, void> {
            using type = conc_tuple_t<node_tuple_t<Args>...>;
        };

        template<class L, class A>
        struct node_tuple<dynamic_in_t<L, A>, void> {
            using left_tuple = node_tuple_t<L>;
            using right_tuple = node_tuple_t<A>;
            using type = conc_tuple_t<left_tuple, right_tuple>;
        };

        template<class L, class... Args>
        struct node_tuple<in_t<L, Args...>, void> {
            using left_tuple = node_tuple_t<L>;
            using right_tuple = conc_tuple_t<node_tuple_t<Args>...>;
            using type = conc_tuple_t<left_tuple, right_tuple>;
        };

        template<class T>
        struct node_tuple<T, std::enable_if_t<is_base_of_template_v<T, compound_operator>>> {
            using node_type = T;
            using left_type = typename node_type::left_type;
            using right_type = typename node_type::right_type;
            using left_tuple = node_tuple_t<left_type>;
            using right_tuple = node_tuple_t<right_type>;
            using type = conc_tuple_t<left_tuple, right_tuple>;
        };

        template<class T, class... Args>
        struct node_tuple<select_t<T, Args...>, void> {
            using columns_tuple = node_tuple_t<T>;
            using args_tuple = conc_tuple_t<node_tuple_t<Args>...>;
            using type = conc_tuple_t<columns_tuple, args_tuple>;
        };

        template<class... Args>
        struct node_tuple<insert_raw_t<Args...>, void> {
            using type = conc_tuple_t<node_tuple_t<Args>...>;
        };

        template<class... Args>
        struct node_tuple<replace_raw_t<Args...>, void> {
            using type = conc_tuple_t<node_tuple_t<Args>...>;
        };

        template<class T>
        struct node_tuple<into_t<T>, void> : node_tuple<void> {};

        template<class... Args>
        struct node_tuple<values_t<Args...>, void> {
            using type = conc_tuple_t<node_tuple_t<Args>...>;
        };

        template<class... Args>
        struct node_tuple<std::tuple<Args...>, void> {
            using type = conc_tuple_t<node_tuple_t<Args>...>;
        };

        template<class T, class R, class... Args>
        struct node_tuple<get_all_t<T, R, Args...>, void> {
            using type = conc_tuple_t<node_tuple_t<Args>...>;
        };

        template<class T, class... Args>
        struct node_tuple<get_all_pointer_t<T, Args...>, void> {
            using type = conc_tuple_t<node_tuple_t<Args>...>;
        };

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
        template<class T, class... Args>
        struct node_tuple<get_all_optional_t<T, Args...>, void> {
            using type = conc_tuple_t<node_tuple_t<Args>...>;
        };
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

        template<class... Args, class... Wargs>
        struct node_tuple<update_all_t<set_t<Args...>, Wargs...>, void> {
            using set_tuple = conc_tuple_t<node_tuple_t<Args>...>;
            using conditions_tuple = conc_tuple_t<node_tuple_t<Wargs>...>;
            using type = conc_tuple_t<set_tuple, conditions_tuple>;
        };

        template<class T, class... Args>
        struct node_tuple<remove_all_t<T, Args...>, void> {
            using type = conc_tuple_t<node_tuple_t<Args>...>;
        };

        template<class T>
        struct node_tuple<having_t<T>, void> : node_tuple<T> {};

        template<class T, class E>
        struct node_tuple<cast_t<T, E>, void> : node_tuple<E> {};

        template<class T>
        struct node_tuple<exists_t<T>, void> : node_tuple<T> {};

        template<class T>
        struct node_tuple<optional_container<T>, void> : node_tuple<T> {};

        template<class A, class T, class E>
        struct node_tuple<like_t<A, T, E>, void> {
            using arg_tuple = node_tuple_t<A>;
            using pattern_tuple = node_tuple_t<T>;
            using escape_tuple = node_tuple_t<E>;
            using type = conc_tuple_t<arg_tuple, pattern_tuple, escape_tuple>;
        };

        template<class A, class T>
        struct node_tuple<glob_t<A, T>, void> {
            using arg_tuple = node_tuple_t<A>;
            using pattern_tuple = node_tuple_t<T>;
            using type = conc_tuple_t<arg_tuple, pattern_tuple>;
        };

        template<class A, class T>
        struct node_tuple<between_t<A, T>, void> {
            using expression_tuple = node_tuple_t<A>;
            using lower_tuple = node_tuple_t<T>;
            using upper_tuple = node_tuple_t<T>;
            using type = conc_tuple_t<expression_tuple, lower_tuple, upper_tuple>;
        };

        template<class T>
        struct node_tuple<named_collate<T>, void> : node_tuple<T> {};

        template<class T>
        struct node_tuple<is_null_t<T>, void> : node_tuple<T> {};

        template<class T>
        struct node_tuple<is_not_null_t<T>, void> : node_tuple<T> {};

        template<class C>
        struct node_tuple<negated_condition_t<C>, void> : node_tuple<C> {};

        template<class R, class S, class... Args>
        struct node_tuple<built_in_function_t<R, S, Args...>, void> {
            using type = conc_tuple_t<node_tuple_t<Args>...>;
        };

        template<class R, class S, class... Args>
        struct node_tuple<built_in_aggregate_function_t<R, S, Args...>, void> {
            using type = conc_tuple_t<node_tuple_t<Args>...>;
        };

        template<class F, class W>
        struct node_tuple<filtered_aggregate_function<F, W>, void> {
            using left_tuple = node_tuple_t<F>;
            using right_tuple = node_tuple_t<W>;
            using type = conc_tuple_t<left_tuple, right_tuple>;
        };

        template<class F, class... Args>
        struct node_tuple<function_call<F, Args...>, void> {
            using type = conc_tuple_t<node_tuple_t<Args>...>;
        };

        template<class T, class O>
        struct node_tuple<left_join_t<T, O>, void> : node_tuple<O> {};

        template<class T>
        struct node_tuple<on_t<T>, void> : node_tuple<T> {};

        // note: not strictly necessary as there's no binding support for USING;
        // we provide it nevertheless, in line with on_t.
        template<class T, class M>
        struct node_tuple<using_t<T, M>, void> : node_tuple<column_pointer<T, M>> {};

        template<class T, class O>
        struct node_tuple<join_t<T, O>, void> : node_tuple<O> {};

        template<class T, class O>
        struct node_tuple<left_outer_join_t<T, O>, void> : node_tuple<O> {};

        template<class T, class O>
        struct node_tuple<inner_join_t<T, O>, void> : node_tuple<O> {};

        template<class R, class T, class E, class... Args>
        struct node_tuple<simple_case_t<R, T, E, Args...>, void> {
            using case_tuple = node_tuple_t<T>;
            using args_tuple = conc_tuple_t<node_tuple_t<Args>...>;
            using else_tuple = node_tuple_t<E>;
            using type = conc_tuple_t<case_tuple, args_tuple, else_tuple>;
        };

        template<class L, class R>
        struct node_tuple<std::pair<L, R>, void> {
            using left_tuple = node_tuple_t<L>;
            using right_tuple = node_tuple_t<R>;
            using type = conc_tuple_t<left_tuple, right_tuple>;
        };

        template<class T, class E>
        struct node_tuple<as_t<T, E>, void> : node_tuple<E> {};

        template<class T>
        struct node_tuple<limit_t<T, false, false, void>, void> : node_tuple<T> {};

        template<class T, class O>
        struct node_tuple<limit_t<T, true, false, O>, void> {
            using type = conc_tuple_t<node_tuple_t<T>, node_tuple_t<O>>;
        };

        template<class T, class O>
        struct node_tuple<limit_t<T, true, true, O>, void> {
            using type = conc_tuple_t<node_tuple_t<O>, node_tuple_t<T>>;
        };
    }
}
#pragma once

#include <type_traits>  //  std::is_same, std::decay, std::remove_reference

// #include "functional/static_magic.h"

// #include "prepared_statement.h"

// #include "ast_iterator.h"

// #include "expression_object_type.h"

namespace sqlite_orm {

    template<int N, class It, class L, class O>
    auto& get(internal::prepared_statement_t<internal::insert_range_t<It, L, O>>& statement) {
        return std::get<N>(statement.expression.range);
    }

    template<int N, class It, class L, class O>
    const auto& get(const internal::prepared_statement_t<internal::insert_range_t<It, L, O>>& statement) {
        return std::get<N>(statement.expression.range);
    }

    template<int N, class It, class L, class O>
    auto& get(internal::prepared_statement_t<internal::replace_range_t<It, L, O>>& statement) {
        return std::get<N>(statement.expression.range);
    }

    template<int N, class It, class L, class O>
    const auto& get(const internal::prepared_statement_t<internal::replace_range_t<It, L, O>>& statement) {
        return std::get<N>(statement.expression.range);
    }

    template<int N, class T, class... Ids>
    auto& get(internal::prepared_statement_t<internal::get_t<T, Ids...>>& statement) {
        return internal::get_ref(std::get<N>(statement.expression.ids));
    }

    template<int N, class T, class... Ids>
    const auto& get(const internal::prepared_statement_t<internal::get_t<T, Ids...>>& statement) {
        return internal::get_ref(std::get<N>(statement.expression.ids));
    }

    template<int N, class T, class... Ids>
    auto& get(internal::prepared_statement_t<internal::get_pointer_t<T, Ids...>>& statement) {
        return internal::get_ref(std::get<N>(statement.expression.ids));
    }

    template<int N, class T, class... Ids>
    const auto& get(const internal::prepared_statement_t<internal::get_pointer_t<T, Ids...>>& statement) {
        return internal::get_ref(std::get<N>(statement.expression.ids));
    }

#ifdef SQLITE_ORM_OPTIONAL_SUPPORTED
    template<int N, class T, class... Ids>
    auto& get(internal::prepared_statement_t<internal::get_optional_t<T, Ids...>>& statement) {
        return internal::get_ref(std::get<N>(statement.expression.ids));
    }

    template<int N, class T, class... Ids>
    const auto& get(const internal::prepared_statement_t<internal::get_optional_t<T, Ids...>>& statement) {
        return internal::get_ref(std::get<N>(statement.expression.ids));
    }
#endif  // SQLITE_ORM_OPTIONAL_SUPPORTED

    template<int N, class T, class... Ids>
    auto& get(internal::prepared_statement_t<internal::remove_t<T, Ids...>>& statement) {
        return internal::get_ref(std::get<N>(statement.expression.ids));
    }

    template<int N, class T, class... Ids>
    const auto& get(const internal::prepared_statement_t<internal::remove_t<T, Ids...>>& statement) {
        return internal::get_ref(std::get<N>(statement.expression.ids));
    }

    template<int N, class T>
    auto& get(internal::prepared_statement_t<internal::update_t<T>>& statement) {
        static_assert(N == 0, "get<> works only with 0 argument for update statement");
        return internal::get_ref(statement.expression.object);
    }

    template<int N, class T>
    const auto& get(const internal::prepared_statement_t<internal::update_t<T>>& statement) {
        static_assert(N == 0, "get<> works only with 0 argument for update statement");
        return internal::get_ref(statement.expression.object);
    }

    template<int N, class T, class... Cols>
    auto& get(internal::prepared_statement_t<internal::insert_explicit<T, Cols...>>& statement) {
        static_assert(N == 0, "get<> works only with 0 argument for insert statement");
        return internal::get_ref(statement.expression.obj);
    }

    template<int N, class T, class... Cols>
    const auto& get(const internal::prepared_statement_t<internal::insert_explicit<T, Cols...>>& statement) {
        static_assert(N == 0, "get<> works only with 0 argument for insert statement");
        return internal::get_ref(statement.expression.obj);
    }

    template<int N, class T>
    auto& get(internal::prepared_statement_t<internal::replace_t<T>>& statement) {
        static_assert(N == 0, "get<> works only with 0 argument for replace statement");
        return internal::get_ref(statement.expression.object);
    }

    template<int N, class T>
    const auto& get(const internal::prepared_statement_t<internal::replace_t<T>>& statement) {
        static_assert(N == 0, "get<> works only with 0 argument for replace statement");
        return internal::get_ref(statement.expression.object);
    }

    template<int N, class T>
    auto& get(internal::prepared_statement_t<internal::insert_t<T>>& statement) {
        static_assert(N == 0, "get<> works only with 0 argument for insert statement");
        return internal::get_ref(statement.expression.object);
    }

    template<int N, class T>
    const auto& get(const internal::prepared_statement_t<internal::insert_t<T>>& statement) {
        static_assert(N == 0, "get<> works only with 0 argument for insert statement");
        return internal::get_ref(statement.expression.object);
    }

    template<int N, class T>
    const auto& get(const internal::prepared_statement_t<T>& statement) {
        using statement_type = std::decay_t<decltype(statement)>;
        using expression_type = typename statement_type::expression_type;
        using node_tuple = internal::node_tuple_t<expression_type>;
        using bind_tuple = typename internal::bindable_filter<node_tuple>::type;
        using result_tupe = std::tuple_element_t<static_cast<size_t>(N), bind_tuple>;
        const result_tupe* result = nullptr;
        internal::iterate_ast(statement.expression, [&result, index = -1](auto& node) mutable {
            using node_type = std::decay_t<decltype(node)>;
            if(internal::is_bindable_v<node_type>) {
                ++index;
            }
            if(index == N) {
                internal::call_if_constexpr<std::is_same<result_tupe, node_type>::value>(
                    [](auto& r, auto& n) {
                        r = &n;
                    },
                    result,
                    node);
            }
        });
        return internal::get_ref(*result);
    }

    template<int N, class T>
    auto& get(internal::prepared_statement_t<T>& statement) {
        using statement_type = std::decay_t<decltype(statement)>;
        using expression_type = typename statement_type::expression_type;
        using node_tuple = internal::node_tuple_t<expression_type>;
        using bind_tuple = typename internal::bindable_filter<node_tuple>::type;
        using result_tupe = std::tuple_element_t<static_cast<size_t>(N), bind_tuple>;
        result_tupe* result = nullptr;

        internal::iterate_ast(statement.expression, [&result, index = -1](auto& node) mutable {
            using node_type = std::decay_t<decltype(node)>;
            if(internal::is_bindable_v<node_type>) {
                ++index;
            }
            if(index == N) {
                internal::call_if_constexpr<std::is_same<result_tupe, node_type>::value>(
                    [](auto& r, auto& n) {
                        r = const_cast<std::remove_reference_t<decltype(r)>>(&n);
                    },
                    result,
                    node);
            }
        });
        return internal::get_ref(*result);
    }
}
#pragma once

/*
 *  Note: This feature needs constexpr variables with external linkage.
 *  which can be achieved before C++17's inline variables, but differs from compiler to compiler.
 *  Hence we make it only available for compilers supporting inline variables.
 */

#ifdef SQLITE_ORM_INLINE_VARIABLES_SUPPORTED
#include <type_traits>  //  std::integral_constant
#include <utility>  //  std::move

// #include "functional/cxx_universal.h"

// #include "pointer_value.h"

namespace sqlite_orm {

    inline constexpr const char carray_pvt_name[] = "carray";
    using carray_pvt = std::integral_constant<const char*, carray_pvt_name>;

    template<typename P>
    using carray_pointer_arg = pointer_arg<P, carray_pvt>;
    template<typename P, typename D>
    using carray_pointer_binding = pointer_binding<P, carray_pvt, D>;
    template<typename P>
    using static_carray_pointer_binding = static_pointer_binding<P, carray_pvt>;

    /**
     *  Wrap a pointer of type 'carray' and its deleter function for binding it to a statement.
     *  
     *  Unless the deleter yields a nullptr 'xDestroy' function the ownership of the pointed-to-object
     *  is transferred to the pointer binding, which will delete it through
     *  the deleter when the statement finishes.
     */
    template<class P, class D>
    auto bindable_carray_pointer(P* p, D d) noexcept -> pointer_binding<P, carray_pvt, D> {
        return bindable_pointer<carray_pvt>(p, std::move(d));
    }

    /**
     *  Wrap a pointer of type 'carray' for binding it to a statement.
     *  
     *  Note: 'Static' means that ownership of the pointed-to-object won't be transferred
     *  and sqlite assumes the object pointed to is valid throughout the lifetime of a statement.
     */
    template<class P>
    auto statically_bindable_carray_pointer(P* p) noexcept -> static_pointer_binding<P, carray_pvt> {
        return statically_bindable_pointer<carray_pvt>(p);
    }

    /**
     *  Generalized form of the 'remember' SQL function that is a pass-through for values
     *  (it returns its argument unchanged using move semantics) but also saves the
     *  value that is passed through into a bound variable.
     */
    template<typename P>
    struct note_value_fn {
        P operator()(P&& value, carray_pointer_arg<P> pv) const {
            if(P* observer = pv) {
                *observer = value;
            }
            return std::move(value);
        }

        static constexpr const char* name() {
            return "note_value";
        }
    };

    /**
     *  remember(V, $PTR) extension function https://sqlite.org/src/file/ext/misc/remember.c
     */
    struct remember_fn : note_value_fn<int64> {
        static constexpr const char* name() {
            return "remember";
        }
    };
}
#endif
#pragma once

#include <string>  //  std::string

// #include "column.h"

// #include "table.h"

namespace sqlite_orm {
#ifdef SQLITE_ENABLE_DBSTAT_VTAB
    struct dbstat {
        std::string name;
        std::string path;
        int pageno = 0;
        std::string pagetype;
        int ncell = 0;
        int payload = 0;
        int unused = 0;
        int mx_payload = 0;
        int pgoffset = 0;
        int pgsize = 0;
    };

    inline auto make_dbstat_table() {
        return make_table("dbstat",
                          make_column("name", &dbstat::name),
                          make_column("path", &dbstat::path),
                          make_column("pageno", &dbstat::pageno),
                          make_column("pagetype", &dbstat::pagetype),
                          make_column("ncell", &dbstat::ncell),
                          make_column("payload", &dbstat::payload),
                          make_column("unused", &dbstat::unused),
                          make_column("mx_payload", &dbstat::mx_payload),
                          make_column("pgoffset", &dbstat::pgoffset),
                          make_column("pgsize", &dbstat::pgsize));
    }
#endif  //  SQLITE_ENABLE_DBSTAT_VTAB
}
/** @file Mainly existing to disentangle implementation details from circular and cross dependencies
 *  (e.g. column_t -> default_value_extractor -> serializer_context -> storage_impl -> table_t -> column_t)
 *  this file is also used to provide definitions of interface methods 'hitting the database'.
 */
#pragma once

// #include "implementations/column_definitions.h"
/** @file Mainly existing to disentangle implementation details from circular and cross dependencies
 *  (e.g. column_t -> default_value_extractor -> serializer_context -> storage_impl -> table_t -> column_t)
 *  this file is also used to provide definitions of interface methods 'hitting the database'.
 */

#include <memory>  //  std::make_unique

// #include "../functional/static_magic.h"

// #include "../tuple_helper/tuple_filter.h"

// #include "../tuple_helper/tuple_traits.h"

// #include "../default_value_extractor.h"

// #include "../column.h"

namespace sqlite_orm {
    namespace internal {

        template<class O, class T, class G, class S, class... Op>
        std::unique_ptr<std::string> column_t<O, T, G, S, Op...>::default_value() const {
            using default_op_index_sequence =
                filter_tuple_sequence_t<constraints_type, check_if_is_same_template<default_t>::template fn>;

            std::unique_ptr<std::string> value;
            call_if_constexpr<default_op_index_sequence::size()>(
                [&value](auto& constraints) {
                    constexpr size_t opIndex = first_index_sequence_value(default_op_index_sequence{});
                    value = std::make_unique<std::string>(serialize_default_value(get<opIndex>(constraints)));
                },
                this->constraints);
            return value;
        }

    }
}

// #include "implementations/table_definitions.h"
/** @file Mainly existing to disentangle implementation details from circular and cross dependencies
 *  (e.g. column_t -> default_value_extractor -> serializer_context -> storage_impl -> table_t -> column_t)
 *  this file is also used to provide definitions of interface methods 'hitting the database'.
 */

#include <type_traits>  //  std::decay_t
#include <utility>  //  std::move
#include <algorithm>  //  std::find_if

// #include "../type_printer.h"

// #include "../column.h"

// #include "../table.h"

namespace sqlite_orm {
    namespace internal {

        template<class T, bool WithoutRowId, class... Cs>
        std::vector<table_xinfo> table_t<T, WithoutRowId, Cs...>::get_table_info() const {
            std::vector<table_xinfo> res;
            res.reserve(size_t(this->elements_count));
            this->for_each_column([&res](auto& column) {
                using field_type = field_type_t<std::decay_t<decltype(column)>>;
                std::string dft;
                if(auto d = column.default_value()) {
                    dft = move(*d);
                }
                res.emplace_back(-1,
                                 column.name,
                                 type_printer<field_type>().print(),
                                 column.is_not_null(),
                                 dft,
                                 column.template is<is_primary_key>(),
                                 column.is_generated());
            });
            auto compositeKeyColumnNames = this->composite_key_columns_names();
            for(size_t i = 0; i < compositeKeyColumnNames.size(); ++i) {
                auto& columnName = compositeKeyColumnNames[i];
                auto it = std::find_if(res.begin(), res.end(), [&columnName](const table_xinfo& ti) {
                    return ti.name == columnName;
                });
                if(it != res.end()) {
                    it->pk = static_cast<int>(i + 1);
                }
            }
            return res;
        }

    }
}

// #include "implementations/storage_definitions.h"
/** @file Mainly existing to disentangle implementation details from circular and cross dependencies
 *  this file is also used to separate implementation details from the main header file,
 *  e.g. usage of the dbstat table.
 */

#include <type_traits>  //  std::is_same
#include <sstream>
#include <functional>  //  std::reference_wrapper, std::cref
#include <algorithm>  //  std::find_if

// #include "../dbstat.h"

// #include "../util.h"

// #include "../serializing_util.h"

// #include "../storage.h"

namespace sqlite_orm {
    namespace internal {

        template<class... Ts>
        template<class T, bool WithoutRowId, class... Args, class... Tss>
        sync_schema_result
        storage_t<Ts...>::sync_table(const storage_impl<table_t<T, WithoutRowId, Args...>, Tss...>& tImpl,
                                     sqlite3* db,
                                     bool preserve) {
#ifdef SQLITE_ENABLE_DBSTAT_VTAB
            if(std::is_same<T, dbstat>::value) {
                return sync_schema_result::already_in_sync;
            }
#endif  //  SQLITE_ENABLE_DBSTAT_VTAB
            auto res = sync_schema_result::already_in_sync;
            bool attempt_to_preserve = true;

            auto schema_stat = this->schema_status(tImpl, db, preserve, &attempt_to_preserve);
            if(schema_stat != sync_schema_result::already_in_sync) {
                if(schema_stat == sync_schema_result::new_table_created) {
                    this->create_table(db, tImpl.table.name, tImpl);
                    res = sync_schema_result::new_table_created;
                } else {
                    if(schema_stat == sync_schema_result::old_columns_removed ||
                       schema_stat == sync_schema_result::new_columns_added ||
                       schema_stat == sync_schema_result::new_columns_added_and_old_columns_removed) {

                        //  get table info provided in `make_table` call..
                        auto storageTableInfo = tImpl.table.get_table_info();

                        //  now get current table info from db using `PRAGMA table_xinfo` query..
                        auto dbTableInfo =
                            this->pragma.table_xinfo(tImpl.table.name);  // should include generated columns

                        //  this vector will contain pointers to columns that gotta be added..
                        std::vector<const table_xinfo*> columnsToAdd;

                        this->calculate_remove_add_columns(columnsToAdd, storageTableInfo, dbTableInfo);

                        if(schema_stat == sync_schema_result::old_columns_removed) {
#if SQLITE_VERSION_NUMBER >= 3035000  //  DROP COLUMN feature exists (v3.35.0)
                            for(auto& tableInfo: dbTableInfo) {
                                this->drop_column(db, tImpl.table.name, tableInfo.name);
                            }
                            res = sync_schema_result::old_columns_removed;
#else
                                //  extra table columns than storage columns
                                this->backup_table(db, tImpl, {});
                                res = sync_schema_result::old_columns_removed;
#endif
                        }

                        if(schema_stat == sync_schema_result::new_columns_added) {
                            for(const table_xinfo* colInfo: columnsToAdd) {
                                tImpl.table.for_each_column([this, colInfo, &tImpl, db](auto& column) {
                                    if(column.name != colInfo->name) {
                                        return;
                                    }
                                    this->add_column(tImpl.table.name, column, db);
                                });
                            }
                            res = sync_schema_result::new_columns_added;
                        }

                        if(schema_stat == sync_schema_result::new_columns_added_and_old_columns_removed) {

                            auto storageTableInfo = tImpl.table.get_table_info();
                            this->add_generated_cols(columnsToAdd, storageTableInfo);

                            // remove extra columns and generated columns
                            this->backup_table(db, tImpl, columnsToAdd);
                            res = sync_schema_result::new_columns_added_and_old_columns_removed;
                        }
                    } else if(schema_stat == sync_schema_result::dropped_and_recreated) {
                        //  now get current table info from db using `PRAGMA table_xinfo` query..
                        auto dbTableInfo =
                            this->pragma.table_xinfo(tImpl.table.name);  // should include generated columns
                        auto storageTableInfo = tImpl.table.get_table_info();

                        //  this vector will contain pointers to columns that gotta be added..
                        std::vector<const table_xinfo*> columnsToAdd;

                        this->calculate_remove_add_columns(columnsToAdd, storageTableInfo, dbTableInfo);

                        this->add_generated_cols(columnsToAdd, storageTableInfo);

                        if(preserve && attempt_to_preserve) {
                            this->backup_table(db, tImpl, columnsToAdd);
                        } else {
                            this->drop_create_with_loss(db, tImpl);
                        }
                        res = schema_stat;
                    }
                }
            }
            return res;
        }

        template<class... Ts>
        template<class I>
        void storage_t<Ts...>::copy_table(
            sqlite3* db,
            const std::string& sourceTableName,
            const std::string& destinationTableName,
            const I& tImpl,
            const std::vector<const table_xinfo*>& columnsToIgnore) const {  // must ignore generated columns
            std::vector<std::reference_wrapper<const std::string>> columnNames;
            columnNames.reserve(tImpl.table.count_columns_amount());
            tImpl.table.for_each_column([&columnNames, &columnsToIgnore](const auto& c) {
                auto& columnName = c.name;
                auto columnToIgnoreIt = std::find_if(columnsToIgnore.begin(),
                                                     columnsToIgnore.end(),
                                                     [&columnName](const table_xinfo* tableInfo) {
                                                         return columnName == tableInfo->name;
                                                     });
                if(columnToIgnoreIt == columnsToIgnore.end()) {
                    columnNames.push_back(cref(columnName));
                }
            });

            std::stringstream ss;
            ss << "INSERT INTO " << streaming_identifier(destinationTableName) << " ("
               << streaming_identifiers(columnNames) << ") "
               << "SELECT " << streaming_identifiers(columnNames) << " FROM " << streaming_identifier(sourceTableName)
               << std::flush;
            perform_void_exec(db, ss.str());
        }
    }
}

#pragma once

#if defined(_MSC_VER)
__pragma(pop_macro("max"))
__pragma(pop_macro("min"))
#endif  // defined(_MSC_VER)
