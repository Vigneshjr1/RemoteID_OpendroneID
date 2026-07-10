include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(ext_adv_default_library_list )

# Handle files with suffix s, for group default-XC32
if(ext_adv_default_default_XC32_FILE_TYPE_assemble)
add_library(ext_adv_default_default_XC32_assemble OBJECT ${ext_adv_default_default_XC32_FILE_TYPE_assemble})
    ext_adv_default_default_XC32_assemble_rule(ext_adv_default_default_XC32_assemble)
    list(APPEND ext_adv_default_library_list "$<TARGET_OBJECTS:ext_adv_default_default_XC32_assemble>")

endif()

# Handle files with suffix S, for group default-XC32
if(ext_adv_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
add_library(ext_adv_default_default_XC32_assembleWithPreprocess OBJECT ${ext_adv_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
    ext_adv_default_default_XC32_assembleWithPreprocess_rule(ext_adv_default_default_XC32_assembleWithPreprocess)
    list(APPEND ext_adv_default_library_list "$<TARGET_OBJECTS:ext_adv_default_default_XC32_assembleWithPreprocess>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(ext_adv_default_default_XC32_FILE_TYPE_compile)
add_library(ext_adv_default_default_XC32_compile OBJECT ${ext_adv_default_default_XC32_FILE_TYPE_compile})
    ext_adv_default_default_XC32_compile_rule(ext_adv_default_default_XC32_compile)
    list(APPEND ext_adv_default_library_list "$<TARGET_OBJECTS:ext_adv_default_default_XC32_compile>")

endif()

# Handle files with suffix cpp, for group default-XC32
if(ext_adv_default_default_XC32_FILE_TYPE_compile_cpp)
add_library(ext_adv_default_default_XC32_compile_cpp OBJECT ${ext_adv_default_default_XC32_FILE_TYPE_compile_cpp})
    ext_adv_default_default_XC32_compile_cpp_rule(ext_adv_default_default_XC32_compile_cpp)
    list(APPEND ext_adv_default_library_list "$<TARGET_OBJECTS:ext_adv_default_default_XC32_compile_cpp>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(ext_adv_default_default_XC32_FILE_TYPE_dependentObject)
add_library(ext_adv_default_default_XC32_dependentObject OBJECT ${ext_adv_default_default_XC32_FILE_TYPE_dependentObject})
    ext_adv_default_default_XC32_dependentObject_rule(ext_adv_default_default_XC32_dependentObject)
    list(APPEND ext_adv_default_library_list "$<TARGET_OBJECTS:ext_adv_default_default_XC32_dependentObject>")

endif()


# Main target for this project
add_executable(ext_adv_default_image_YSXSQMOy ${ext_adv_default_library_list})

set_target_properties(ext_adv_default_image_YSXSQMOy PROPERTIES
    OUTPUT_NAME "default"
    SUFFIX ".elf"
    RUNTIME_OUTPUT_DIRECTORY "${ext_adv_default_output_dir}")
target_link_libraries(ext_adv_default_image_YSXSQMOy PRIVATE ${ext_adv_default_default_XC32_FILE_TYPE_link})

# Add the link options from the rule file.
ext_adv_default_link_rule( ext_adv_default_image_YSXSQMOy)

# Add bin2hex target for converting built file to a .hex file.
string(REGEX REPLACE [.]elf$ .hex ext_adv_default_image_name_hex ${ext_adv_default_image_name})
add_custom_target(ext_adv_default_Bin2Hex ALL
    COMMAND ${MP_BIN2HEX} \"${ext_adv_default_output_dir}/${ext_adv_default_image_name}\"
    BYPRODUCTS ${ext_adv_default_output_dir}/${ext_adv_default_image_name_hex}
    COMMENT "Convert built file to .hex")
add_dependencies(ext_adv_default_Bin2Hex ext_adv_default_image_YSXSQMOy)



