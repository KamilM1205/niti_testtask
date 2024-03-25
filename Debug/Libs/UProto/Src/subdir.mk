################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libs/UProto/Src/ulist.c \
../Libs/UProto/Src/uproto.c 

OBJS += \
./Libs/UProto/Src/ulist.o \
./Libs/UProto/Src/uproto.o 

C_DEPS += \
./Libs/UProto/Src/ulist.d \
./Libs/UProto/Src/uproto.d 


# Each subdirectory must supply rules for building sources it contributes
Libs/UProto/Src/%.o Libs/UProto/Src/%.su Libs/UProto/Src/%.cyclo: ../Libs/UProto/Src/%.c Libs/UProto/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xC -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"/home/muts/projects/stm32/niti_testtask/Libs/UProto/include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Libs-2f-UProto-2f-Src

clean-Libs-2f-UProto-2f-Src:
	-$(RM) ./Libs/UProto/Src/ulist.cyclo ./Libs/UProto/Src/ulist.d ./Libs/UProto/Src/ulist.o ./Libs/UProto/Src/ulist.su ./Libs/UProto/Src/uproto.cyclo ./Libs/UProto/Src/uproto.d ./Libs/UProto/Src/uproto.o ./Libs/UProto/Src/uproto.su

.PHONY: clean-Libs-2f-UProto-2f-Src
