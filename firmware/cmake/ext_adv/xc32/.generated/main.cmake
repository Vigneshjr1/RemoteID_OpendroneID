include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(ext_adv_xc32_library_list )

# Handle files with suffix s, for group xc32_toolchain
if(ext_adv_xc32_xc32_toolchain_FILE_TYPE_assemble)
add_library(ext_adv_xc32_xc32_toolchain_assemble OBJECT ${ext_adv_xc32_xc32_toolchain_FILE_TYPE_assemble})
    ext_adv_xc32_xc32_toolchain_assemble_rule(ext_adv_xc32_xc32_toolchain_assemble)
    list(APPEND ext_adv_xc32_library_list "$<TARGET_OBJECTS:ext_adv_xc32_xc32_toolchain_assemble>")

endif()

# Handle files with suffix S, for group xc32_toolchain
if(ext_adv_xc32_xc32_toolchain_FILE_TYPE_assembleWithPreprocess)
add_library(ext_adv_xc32_xc32_toolchain_assembleWithPreprocess OBJECT ${ext_adv_xc32_xc32_toolchain_FILE_TYPE_assembleWithPreprocess})
    ext_adv_xc32_xc32_toolchain_assembleWithPreprocess_rule(ext_adv_xc32_xc32_toolchain_assembleWithPreprocess)
    list(APPEND ext_adv_xc32_library_list "$<TARGET_OBJECTS:ext_adv_xc32_xc32_toolchain_assembleWithPreprocess>")

endif()

# Handle files with suffix [cC], for group xc32_toolchain
if(ext_adv_xc32_xc32_toolchain_FILE_TYPE_compile)
add_library(ext_adv_xc32_xc32_toolchain_compile OBJECT ${ext_adv_xc32_xc32_toolchain_FILE_TYPE_compile})
    ext_adv_xc32_xc32_toolchain_compile_rule(ext_adv_xc32_xc32_toolchain_compile)
    list(APPEND ext_adv_xc32_library_list "$<TARGET_OBJECTS:ext_adv_xc32_xc32_toolchain_compile>")

endif()

# Handle files with suffix cpp, for group xc32_toolchain
if(ext_adv_xc32_xc32_toolchain_FILE_TYPE_compile_cpp)
add_library(ext_adv_xc32_xc32_toolchain_compile_cpp OBJECT ${ext_adv_xc32_xc32_toolchain_FILE_TYPE_compile_cpp})
    ext_adv_xc32_xc32_toolchain_compile_cpp_rule(ext_adv_xc32_xc32_toolchain_compile_cpp)
    list(APPEND ext_adv_xc32_library_list "$<TARGET_OBJECTS:ext_adv_xc32_xc32_toolchain_compile_cpp>")

endif()

# Handle files with suffix elf, for group xc32_toolchain
if(ext_adv_xc32_xc32_toolchain_FILE_TYPE_objcopy_ihex)
add_library(ext_adv_xc32_xc32_toolchain_objcopy_ihex OBJECT ${ext_adv_xc32_xc32_toolchain_FILE_TYPE_objcopy_ihex})
    ext_adv_xc32_xc32_toolchain_objcopy_ihex_rule(ext_adv_xc32_xc32_toolchain_objcopy_ihex)
    list(APPEND ext_adv_xc32_library_list "$<TARGET_OBJECTS:ext_adv_xc32_xc32_toolchain_objcopy_ihex>")

endif()

# Handle files with suffix elf, for group xc32_toolchain
if(ext_adv_xc32_xc32_toolchain_FILE_TYPE_objcopy_eep)
add_library(ext_adv_xc32_xc32_toolchain_objcopy_eep OBJECT ${ext_adv_xc32_xc32_toolchain_FILE_TYPE_objcopy_eep})
    ext_adv_xc32_xc32_toolchain_objcopy_eep_rule(ext_adv_xc32_xc32_toolchain_objcopy_eep)
    list(APPEND ext_adv_xc32_library_list "$<TARGET_OBJECTS:ext_adv_xc32_xc32_toolchain_objcopy_eep>")

endif()

# Handle files with suffix elf, for group xc32_toolchain
if(ext_adv_xc32_xc32_toolchain_FILE_TYPE_objcopy_lss)
add_library(ext_adv_xc32_xc32_toolchain_objcopy_lss OBJECT ${ext_adv_xc32_xc32_toolchain_FILE_TYPE_objcopy_lss})
    ext_adv_xc32_xc32_toolchain_objcopy_lss_rule(ext_adv_xc32_xc32_toolchain_objcopy_lss)
    list(APPEND ext_adv_xc32_library_list "$<TARGET_OBJECTS:ext_adv_xc32_xc32_toolchain_objcopy_lss>")

endif()

# Handle files with suffix elf, for group xc32_toolchain
if(ext_adv_xc32_xc32_toolchain_FILE_TYPE_objcopy_srec)
add_library(ext_adv_xc32_xc32_toolchain_objcopy_srec OBJECT ${ext_adv_xc32_xc32_toolchain_FILE_TYPE_objcopy_srec})
    ext_adv_xc32_xc32_toolchain_objcopy_srec_rule(ext_adv_xc32_xc32_toolchain_objcopy_srec)
    list(APPEND ext_adv_xc32_library_list "$<TARGET_OBJECTS:ext_adv_xc32_xc32_toolchain_objcopy_srec>")

endif()


# Main target for this project
add_executable(ext_adv_xc32_image_8gFM103u ${ext_adv_xc32_library_list})

set_target_properties(ext_adv_xc32_image_8gFM103u PROPERTIES
    OUTPUT_NAME "xc32"
    SUFFIX ".elf"
    ADDITIONAL_CLEAN_FILES "${output_extensions}"
    RUNTIME_OUTPUT_DIRECTORY "${ext_adv_xc32_output_dir}")
target_link_libraries(ext_adv_xc32_image_8gFM103u PRIVATE ${ext_adv_xc32_xc32_toolchain_FILE_TYPE_link})

#Add objcopy steps
ext_adv_xc32_objcopy_ihex_rule(ext_adv_xc32_image_8gFM103u)
ext_adv_xc32_objcopy_eep_rule(ext_adv_xc32_image_8gFM103u)
ext_adv_xc32_objcopy_lss_rule(ext_adv_xc32_image_8gFM103u)
ext_adv_xc32_objcopy_srec_rule(ext_adv_xc32_image_8gFM103u)
# Add the link options from the rule file.
ext_adv_xc32_link_rule( ext_adv_xc32_image_8gFM103u)


