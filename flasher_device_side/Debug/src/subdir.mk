################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Backup_driver.c \
../src/Backup_manager.c \
../src/Boot_jumper.c \
../src/_write.c \
../src/main.c \
../src/stm32f4xx_it.c \
../src/timer_driver.c 

OBJS += \
./src/Backup_driver.o \
./src/Backup_manager.o \
./src/Boot_jumper.o \
./src/_write.o \
./src/main.o \
./src/stm32f4xx_it.o \
./src/timer_driver.o 

C_DEPS += \
./src/Backup_driver.d \
./src/Backup_manager.d \
./src/Boot_jumper.d \
./src/_write.d \
./src/main.d \
./src/stm32f4xx_it.d \
./src/timer_driver.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DUSE_FULL_ASSERT -DTRACE -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DHSE_VALUE=8000000 -DUSE_STDPERIPH_DRIVER -DSTM32F401xx -I"../include" -I"../system/include" -I"../system/include/cmsis" -I../system/include/stm32f4-spl -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


