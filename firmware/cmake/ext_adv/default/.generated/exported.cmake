set(DEPENDENT_MP_BIN2HEXext_adv_default_YSXSQMOy "/opt/microchip/xc32/v5.10/bin/xc32-bin2hex")
set(DEPENDENT_DEPENDENT_TARGET_ELFext_adv_default_YSXSQMOy ${CMAKE_CURRENT_LIST_DIR}/../../../../out/ext_adv/default.elf)
set(DEPENDENT_TARGET_DIRext_adv_default_YSXSQMOy ${CMAKE_CURRENT_LIST_DIR}/../../../../out/ext_adv)
set(DEPENDENT_BYPRODUCTSext_adv_default_YSXSQMOy ${DEPENDENT_TARGET_DIRext_adv_default_YSXSQMOy}/${sourceFileNameext_adv_default_YSXSQMOy}.c)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRext_adv_default_YSXSQMOy}/${sourceFileNameext_adv_default_YSXSQMOy}.c
    COMMAND ${DEPENDENT_MP_BIN2HEXext_adv_default_YSXSQMOy} --image ${DEPENDENT_DEPENDENT_TARGET_ELFext_adv_default_YSXSQMOy} --image-generated-c ${sourceFileNameext_adv_default_YSXSQMOy}.c --image-generated-h ${sourceFileNameext_adv_default_YSXSQMOy}.h --image-copy-mode ${modeext_adv_default_YSXSQMOy} --image-offset ${addressext_adv_default_YSXSQMOy} 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRext_adv_default_YSXSQMOy}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFext_adv_default_YSXSQMOy})
add_custom_target(
    dependent_produced_source_artifactext_adv_default_YSXSQMOy 
    DEPENDS ${DEPENDENT_TARGET_DIRext_adv_default_YSXSQMOy}/${sourceFileNameext_adv_default_YSXSQMOy}.c
    )
