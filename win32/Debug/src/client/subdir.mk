################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/client/Auth.cpp \
../src/client/Base.cpp \
../src/client/GraphView.cpp \
../src/client/MsgPanel.cpp \
../src/client/Parser.cpp \
../src/client/PieChart.cpp \
../src/client/PieChartItem.cpp \
../src/client/Preferences.cpp \
../src/client/ServiceEditor.cpp \
../src/client/Stats.cpp \
../src/client/StatsLegend.cpp \
../src/client/SvConfigCreator.cpp \
../src/client/SvNavigator.cpp \
../src/client/SvNavigatorTree.cpp \
../src/client/WebKit.cpp \
../src/client/ngrt4n.cpp 

OBJS += \
./src/client/Auth.o \
./src/client/Base.o \
./src/client/GraphView.o \
./src/client/MsgPanel.o \
./src/client/Parser.o \
./src/client/PieChart.o \
./src/client/PieChartItem.o \
./src/client/Preferences.o \
./src/client/ServiceEditor.o \
./src/client/Stats.o \
./src/client/StatsLegend.o \
./src/client/SvConfigCreator.o \
./src/client/SvNavigator.o \
./src/client/SvNavigatorTree.o \
./src/client/WebKit.o \
./src/client/ngrt4n.o 

CPP_DEPS += \
./src/client/Auth.d \
./src/client/Base.d \
./src/client/GraphView.d \
./src/client/MsgPanel.d \
./src/client/Parser.d \
./src/client/PieChart.d \
./src/client/PieChartItem.d \
./src/client/Preferences.d \
./src/client/ServiceEditor.d \
./src/client/Stats.d \
./src/client/StatsLegend.d \
./src/client/SvConfigCreator.d \
./src/client/SvNavigator.d \
./src/client/SvNavigatorTree.d \
./src/client/WebKit.d \
./src/client/ngrt4n.d 


# Each subdirectory must supply rules for building sources it contributes
src/client/%.o: ../src/client/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


