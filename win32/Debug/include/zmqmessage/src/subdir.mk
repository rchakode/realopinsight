################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../include/zmqmessage/src/ZmqMessage.cpp 

OBJS += \
./include/zmqmessage/src/ZmqMessage.o 

CPP_DEPS += \
./include/zmqmessage/src/ZmqMessage.d 


# Each subdirectory must supply rules for building sources it contributes
include/zmqmessage/src/%.o: ../include/zmqmessage/src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


