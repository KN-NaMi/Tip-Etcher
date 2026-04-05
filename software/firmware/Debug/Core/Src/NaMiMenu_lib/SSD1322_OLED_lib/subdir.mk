################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_API.c \
../Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_GFX.c \
../Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_HW_Driver.c 

OBJS += \
./Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_API.o \
./Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_GFX.o \
./Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_HW_Driver.o 

C_DEPS += \
./Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_API.d \
./Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_GFX.d \
./Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_HW_Driver.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/%.o Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/%.su Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/%.cyclo: ../Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/%.c Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-NaMiMenu_lib-2f-SSD1322_OLED_lib

clean-Core-2f-Src-2f-NaMiMenu_lib-2f-SSD1322_OLED_lib:
	-$(RM) ./Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_API.cyclo ./Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_API.d ./Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_API.o ./Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_API.su ./Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_GFX.cyclo ./Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_GFX.d ./Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_GFX.o ./Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_GFX.su ./Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_HW_Driver.cyclo ./Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_HW_Driver.d ./Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_HW_Driver.o ./Core/Src/NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_HW_Driver.su

.PHONY: clean-Core-2f-Src-2f-NaMiMenu_lib-2f-SSD1322_OLED_lib

