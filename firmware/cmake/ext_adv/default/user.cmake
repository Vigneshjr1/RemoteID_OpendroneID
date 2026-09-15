# Override compiler to XC32 v5.10
set(CMAKE_C_COMPILER "/opt/microchip/xc32/v5.10/bin/xc32-gcc" CACHE FILEPATH "" FORCE)
set(CMAKE_CXX_COMPILER "/opt/microchip/xc32/v5.10/bin/xc32-g++" CACHE FILEPATH "" FORCE)
set(CMAKE_ASM_COMPILER "/opt/microchip/xc32/v5.10/bin/xc32-gcc" CACHE FILEPATH "" FORCE)
set(CMAKE_AR "/opt/microchip/xc32/v5.10/bin/xc32-ar" CACHE FILEPATH "" FORCE)
set(MP_CC "/opt/microchip/xc32/v5.10/bin/xc32-gcc" CACHE PATH "" FORCE)
set(MP_CC_DIR "/opt/microchip/xc32/v5.10/bin" CACHE PATH "" FORCE)
set(MP_LD "/opt/microchip/xc32/v5.10/bin/xc32-ld" CACHE FILEPATH "" FORCE)
set(OBJCOPY "/opt/microchip/xc32/v5.10/bin/xc32-objcopy" CACHE FILEPATH "" FORCE)
set(OBJDUMP "/opt/microchip/xc32/v5.10/bin/xc32-objdump" CACHE FILEPATH "" FORCE)

option(ODID_BUILD_WIFI "Compile WINCS02 Wi-Fi support" OFF)

if(ODID_BUILD_WIFI)
# WINCS02 headers are generated in three sibling include directories.
target_include_directories(ext_adv_default_default_XC32_compile PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../../../src/config/default/driver/wifi/wincs02/include"
    "${CMAKE_CURRENT_LIST_DIR}/../../../src/config/default/driver/wifi/wincs02/include/dev"
    "${CMAKE_CURRENT_LIST_DIR}/../../../src/config/default/driver/wifi/wincs02/include/nc_driver"
)

# Add the WINCS02 and supporting peripheral sources omitted by the generator.
file(GLOB ODID_WINC_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/../../../src/config/default/driver/wifi/wincs02/*.c"
    "${CMAKE_CURRENT_LIST_DIR}/../../../src/config/default/driver/wifi/wincs02/dev/*.c"
    "${CMAKE_CURRENT_LIST_DIR}/../../../src/config/default/driver/wifi/wincs02/nc_driver/*.c"
)
target_sources(ext_adv_default_default_XC32_compile PRIVATE
    ${ODID_WINC_SOURCES}
    "${CMAKE_CURRENT_LIST_DIR}/../../../src/config/default/peripheral/eic/plib_eic.c"
    "${CMAKE_CURRENT_LIST_DIR}/../../../src/config/default/peripheral/qspi/plib_qspi_spi.c"
    "${CMAKE_CURRENT_LIST_DIR}/../../../src/config/default/peripheral/tc/plib_tc0.c"
)
endif()
