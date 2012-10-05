################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../include/zmqmessage/examples/zasync.cpp \
../include/zmqmessage/examples/zbisort.cpp \
../include/zmqmessage/examples/zqueue.cpp \
../include/zmqmessage/examples/zserialize.cpp \
../include/zmqmessage/examples/zsort.cpp 

OBJS += \
./include/zmqmessage/examples/zasync.o \
./include/zmqmessage/examples/zbisort.o \
./include/zmqmessage/examples/zqueue.o \
./include/zmqmessage/examples/zserialize.o \
./include/zmqmessage/examples/zsort.o 

CPP_DEPS += \
./include/zmqmessage/examples/zasync.d \
./include/zmqmessage/examples/zbisort.d \
./include/zmqmessage/examples/zqueue.d \
./include/zmqmessage/examples/zserialize.d \
./include/zmqmessage/examples/zsort.d 


# Each subdirectory must supply rules for building sources it contributes
include/zmqmessage/examples/%.o: ../include/zmqmessage/examples/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


