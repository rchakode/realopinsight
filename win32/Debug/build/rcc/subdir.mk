################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../build/rcc/qrc_ngrt4n.cpp 

OBJS += \
./build/rcc/qrc_ngrt4n.o 

CPP_DEPS += \
./build/rcc/qrc_ngrt4n.d 


# Each subdirectory must supply rules for building sources it contributes
build/rcc/%.o: ../build/rcc/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


