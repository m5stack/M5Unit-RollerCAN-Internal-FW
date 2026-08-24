target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    can/can_protocol.c
    flash/flash.c
    i2c/i2c_ex.c
    i2c/i2c_protocol.c

    button/button.c
    encoder/encoder.c
    motordriver/motordriver.c
    adc/myadc.c
    sys/mysys.c
    display/display_app.c
    display/menu_encoder.c
    display/oled_u8g2.c
    pid/pid_controller.c
    smartknob/smart_knob.c
    encoder/tle5012b.c
    display/u8g2_disp_fun.c
    rgb/rgb.c
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/adc
    ${CMAKE_CURRENT_LIST_DIR}/button
    ${CMAKE_CURRENT_LIST_DIR}/can
    ${CMAKE_CURRENT_LIST_DIR}/display
    ${CMAKE_CURRENT_LIST_DIR}/encoder
    ${CMAKE_CURRENT_LIST_DIR}/flash
    ${CMAKE_CURRENT_LIST_DIR}/i2c
    ${CMAKE_CURRENT_LIST_DIR}/motordriver
    ${CMAKE_CURRENT_LIST_DIR}/pid
    ${CMAKE_CURRENT_LIST_DIR}/rgb
    ${CMAKE_CURRENT_LIST_DIR}/smartknob
    ${CMAKE_CURRENT_LIST_DIR}/sys
)
