find_package(Qt6 REQUIRED)

# get absolute path to qmake, then use it to find rcc executable

get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)

function(rcc target input output)

    # PRE_BUILD step
    # - before build, compile the resource file and add it to the source tree

    add_custom_command(TARGET ${target} PRE_BUILD
        COMMAND "${_qt_bin_dir}/rcc.exe"         
                -g cpp
				-o ${output}
				${input}
        COMMENT "Compiling resource file using rcc for compilation target '${target}' ..."
    )

endfunction()