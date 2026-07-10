# The following functions contains all the flags passed to the different build stages.

set(PACK_REPO_PATH "/home/administrator/.mchp_packs" CACHE PATH "Path to the root of a pack repository.")

function(ext_adv_xc32_xc32_toolchain_assemble_rule target)
    set(options
        "-g"
        "${ASSEMBLER_PRE}"
        "-gdwarf-2"
        "-mcpu=cortex-m4"
        "-mthumb"
        "-Wa,--defsym=__MPLAB_BUILD=1${MP_EXTRA_AS_POST},--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--gdwarf-2")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG=1"
        PRIVATE "__PIC32WM_BW1__")
endfunction()
function(ext_adv_xc32_xc32_toolchain_assembleWithPreprocess_rule target)
    set(options
        "-x"
        "assembler-with-cpp"
        "-g"
        "-gdwarf-2"
        "-mcpu=cortex-m4"
        "-mthumb"
        "-Wa,--defsym=__MPLAB_BUILD=1${MP_EXTRA_AS_POST},--defsym=__MPLAB_DEBUG=1,--gdwarf-2,-g,--defsym=__DEBUG=1")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG=1"
        PRIVATE "__PIC32WM_BW1__"
        PRIVATE "xc32=xc32")
endfunction()
function(ext_adv_xc32_xc32_toolchain_compile_rule target)
    set(options
        "-g"
        "-gdwarf-2"
        "-mcpu=cortex-m4"
        "-x"
        "c"
        "-mthumb"
        "-O1"
        "-ffunction-sections"
        "-mlong-calls"
        "-Wall")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "__PIC32WM_BW1__"
        PRIVATE "HAVE_CONFIG_H"
        PRIVATE "WOLFSSL_IGNORE_FILE_WARN"
        PRIVATE "xc32=xc32")
    target_include_directories(${target}
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/app_ble"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/app_odid"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/mavlink"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/ble/lib/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/ble/middleware_ble"
        PRIVATE "src/config/default/ble/profile_ble"
        PRIVATE "src/config/default/ble/service_ble"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/driver/pds/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/packs/CMSIS"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/packs/CMSIS/CMSIS/Core/Include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/packs/PIC32WM_BW1_DFP"
        PRIVATE "src/packs/PIC32WM_DFP"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/ARM_CM4F"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/wolfssl"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/wolfssl/wolfssl"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/app_ble"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/ble/lib/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/ble/middleware_ble"
        PRIVATE "ext_adv.X/../src/config/default/ble/profile_ble"
        PRIVATE "ext_adv.X/../src/config/default/ble/service_ble"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/driver/pds/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../ext_adv.X")
endfunction()
function(ext_adv_xc32_xc32_toolchain_compile_cpp_rule target)
    set(options
        "-g"
        "-gdwarf-2"
        "-x"
        "c++"
        "-mcpu=cortex-m4"
        "-mthumb"
        "-O1"
        "-ffunction-sections"
        "-fno-rtti"
        "-fno-exceptions"
        "-mlong-calls")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "__PIC32WM_BW1__"
        PRIVATE "xc32=xc32")
    target_include_directories(${target}
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/packs/CMSIS"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/packs/CMSIS/CMSIS/Core/Include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/packs/PIC32WM_BW1_DFP"
        PRIVATE "src/packs/PIC32WM_DFP"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/ARM_CM4F"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../ext_adv.X")
endfunction()
function(ext_adv_xc32_link_rule target)
    set(options
        "-gdwarf-2"
        "${MP_EXTRA_LD_PRE}"
        "-T/home/administrator/opendroneid/wireless_apps_pic32_bw1-master/apps/ble/peripheral/peripheral_ext_adv/firmware/src/config/default/PIC32WM_BW1.ld"
        "-mthumb"
        "-Wl,-Map=mem.map"
        "--specs=nosys.specs"
        "-Wl,--defsym=__MPLAB_BUILD=1${MP_EXTRA_LD_POST},--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1"
        "-L${CMAKE_CURRENT_SOURCE_DIR}/../../../ext_adv.X"
        "-Wl,--gc-sections")
    list(REMOVE_ITEM options "")
    target_link_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__PIC32WM_BW1__"
        PRIVATE "xc32=xc32")
endfunction()
function(ext_adv_xc32_objcopy_ihex_rule target)
    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${OBJCOPY}
        ARGS --output-target=ihex --remove-section=.eeprom --remove-section=.fuse --remove-section=.lock --remove-section=.signature ${ext_adv_xc32_image_name} ${ext_adv_xc32_image_base_name}.hex
        WORKING_DIRECTORY ${ext_adv_xc32_output_dir})
endfunction()
function(ext_adv_xc32_objcopy_eep_rule target)
    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${OBJCOPY}
        ARGS --only-section=.eeprom --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0 --no-change-warnings --output-target=binary ${ext_adv_xc32_image_name} ${ext_adv_xc32_image_base_name}.eep
        WORKING_DIRECTORY ${ext_adv_xc32_output_dir})
endfunction()
function(ext_adv_xc32_objcopy_lss_rule target)
    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${OBJDUMP}
        ARGS --disassemble --wide --demangle --line-numbers --section-headers --source ${ext_adv_xc32_image_name} > ${ext_adv_xc32_image_base_name}.lss
        WORKING_DIRECTORY ${ext_adv_xc32_output_dir})
endfunction()
function(ext_adv_xc32_objcopy_srec_rule target)
    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${OBJCOPY}
        ARGS --output-target=srec --remove-section=.eeprom --remove-section=.fuse --remove-section=.lock --remove-section=.signature ${ext_adv_xc32_image_name} ${ext_adv_xc32_image_base_name}.srec
        WORKING_DIRECTORY ${ext_adv_xc32_output_dir})
endfunction()
