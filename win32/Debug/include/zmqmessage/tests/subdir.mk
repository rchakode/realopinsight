################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../include/zmqmessage/tests/PerfTest.cpp \
../include/zmqmessage/tests/SimpleTest.cpp 

OBJS += \
./include/zmqmessage/tests/PerfTest.o \
./include/zmqmessage/tests/SimpleTest.o 

CPP_DEPS += \
./include/zmqmessage/tests/PerfTest.d \
./include/zmqmessage/tests/SimpleTest.d 


# Each subdirectory must supply rules for building sources it contributes
include/zmqmessage/tests/%.o: ../include/zmqmessage/tests/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


