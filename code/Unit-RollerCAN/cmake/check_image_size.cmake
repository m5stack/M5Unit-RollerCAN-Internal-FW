if(NOT DEFINED OBJDUMP_TOOL OR NOT EXISTS "${OBJDUMP_TOOL}")
    message(FATAL_ERROR "arm-none-eabi-objdump was not found: ${OBJDUMP_TOOL}")
endif()

if(NOT DEFINED IMAGE_FILE OR NOT EXISTS "${IMAGE_FILE}")
    message(FATAL_ERROR "ELF image was not found: ${IMAGE_FILE}")
endif()

foreach(required_value FLASH_ORIGIN FLASH_CAPACITY_BYTES RELEASE_RESERVE_BYTES BUILD_TYPE)
    if(NOT DEFINED ${required_value})
        message(FATAL_ERROR "Missing required value: ${required_value}")
    endif()
endforeach()

execute_process(
    COMMAND "${OBJDUMP_TOOL}" -h "${IMAGE_FILE}"
    RESULT_VARIABLE objdump_result
    OUTPUT_VARIABLE objdump_output
    ERROR_VARIABLE objdump_error
)

if(NOT objdump_result EQUAL 0)
    message(FATAL_ERROR "Failed to inspect ${IMAGE_FILE}: ${objdump_error}")
endif()

# Use each section's load memory address (LMA), not just the sum of section
# sizes. This includes alignment gaps and the FLASH initialization image of
# sections such as .data whose run address is in RAM.
math(EXPR flash_limit "${FLASH_ORIGIN} + ${FLASH_CAPACITY_BYTES}")
set(flash_image_end "${FLASH_ORIGIN}")
string(REPLACE "\r\n" "\n" objdump_output "${objdump_output}")
string(REPLACE "\n" ";" objdump_lines "${objdump_output}")

foreach(line IN LISTS objdump_lines)
    string(STRIP "${line}" line)
    if(line MATCHES "^[0-9]+ +[^ ]+ +([0-9A-Fa-f]+) +[0-9A-Fa-f]+ +([0-9A-Fa-f]+) +")
        set(pending_size_hex "${CMAKE_MATCH_1}")
        set(pending_lma_hex "${CMAKE_MATCH_2}")
    elseif(DEFINED pending_size_hex)
        if(line MATCHES "(^|, )LOAD(,|$)")
            math(EXPR section_size "0x${pending_size_hex}")
            math(EXPR section_lma "0x${pending_lma_hex}")
            math(EXPR section_end "${section_lma} + ${section_size}")

            if(section_size GREATER 0
               AND section_lma GREATER_EQUAL FLASH_ORIGIN
               AND section_lma LESS flash_limit
               AND section_end GREATER flash_image_end)
                set(flash_image_end "${section_end}")
            endif()
        endif()
        unset(pending_size_hex)
        unset(pending_lma_hex)
    endif()
endforeach()

math(EXPR flash_used_bytes "${flash_image_end} - ${FLASH_ORIGIN}")
if(flash_used_bytes EQUAL 0)
    message(FATAL_ERROR "No FLASH load sections were found in arm-none-eabi-objdump output")
endif()

math(EXPR flash_free_bytes "${FLASH_CAPACITY_BYTES} - ${flash_used_bytes}")
math(EXPR flash_used_tenths "(${flash_used_bytes} * 1000 + ${FLASH_CAPACITY_BYTES} / 2) / ${FLASH_CAPACITY_BYTES}")
math(EXPR flash_used_percent "${flash_used_tenths} / 10")
math(EXPR flash_used_decimal "${flash_used_tenths} % 10")

message(STATUS
    "ROLLERCAN FLASH: ${flash_used_bytes}/${FLASH_CAPACITY_BYTES} bytes "
    "(${flash_used_percent}.${flash_used_decimal}%), free ${flash_free_bytes} bytes"
)

if(flash_used_bytes GREATER FLASH_CAPACITY_BYTES)
    math(EXPR flash_overflow_bytes "${flash_used_bytes} - ${FLASH_CAPACITY_BYTES}")
    message(FATAL_ERROR
        "ROLLERCAN image exceeds the application FLASH partition by "
        "${flash_overflow_bytes} bytes"
    )
endif()

if(BUILD_TYPE STREQUAL "Release" AND flash_free_bytes LESS RELEASE_RESERVE_BYTES)
    message(FATAL_ERROR
        "ROLLERCAN Release image leaves only ${flash_free_bytes} bytes; "
        "at least ${RELEASE_RESERVE_BYTES} bytes are reserved for planned extensions"
    )
endif()
