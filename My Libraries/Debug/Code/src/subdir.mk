################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Code/src/main.c \
../Code/src/messages.c \
../Code/src/modbus.c \
../Code/src/system.c \
../Code/src/virtualTimers.c 

OBJS += \
./Code/src/main.o \
./Code/src/messages.o \
./Code/src/modbus.o \
./Code/src/system.o \
./Code/src/virtualTimers.o 

C_DEPS += \
./Code/src/main.d \
./Code/src/messages.d \
./Code/src/modbus.d \
./Code/src/system.d \
./Code/src/virtualTimers.d 


# Each subdirectory must supply rules for building sources it contributes
Code/src/%.o: ../Code/src/%.c Code/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -c -I"C:/Users/Human/Documents/GitHub/repository2/My Libraries" -I"C:/Users/Human/Documents/GitHub/repository2/My Libraries/CMSIS/inc" -I"C:/Users/Human/Documents/GitHub/repository2/My Libraries/Code/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

