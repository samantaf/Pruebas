################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Test1b/Test1c.c 

OBJS += \
./Test1b/Test1c.o 

C_DEPS += \
./Test1b/Test1c.d 


# Each subdirectory must supply rules for building sources it contributes
Test1b/%.o: ../Test1b/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


