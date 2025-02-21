
function(setup_target name)
    set_target_properties(${name} PROPERTIES LINKER_LANGUAGE CXX)
    target_compile_features(${name} PUBLIC
        cxx_std_23
        cxx_auto_type
        cxx_lambdas
        cxx_constexpr
        cxx_variadic_templates
        cxx_nullptr
        cxx_attributes
        cxx_decltype
        cxx_generic_lambdas
        cxx_inline_namespaces
        cxx_lambda_init_captures
        cxx_noexcept
        cxx_range_for
        cxx_raw_string_literals
        cxx_static_assert
        )
endfunction()


function(setup_library name)
    setup_target(${name})
endfunction()


function(setup_executable name)
    setup_target(${name})
endfunction()
