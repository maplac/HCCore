#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/5b6daedd/DeviceBME280.o \
	${OBJECTDIR}/_ext/5b6daedd/DeviceGeneric.o \
	${OBJECTDIR}/_ext/5b6daedd/DeviceManager.o \
	${OBJECTDIR}/_ext/5b6daedd/Log.o \
	${OBJECTDIR}/_ext/5b6daedd/PacketManager.o \
	${OBJECTDIR}/_ext/5b6daedd/PeriodicTimer.o \
	${OBJECTDIR}/_ext/5b6daedd/Rf24Server.o \
	${OBJECTDIR}/_ext/5b6daedd/Runnable.o \
	${OBJECTDIR}/_ext/5b6daedd/Test.o \
	${OBJECTDIR}/_ext/5b6daedd/gpio.o \
	${OBJECTDIR}/_ext/5b6daedd/main.o \
	${OBJECTDIR}/_ext/5b6daedd/mainLoop.o \
	${OBJECTDIR}/_ext/5b6daedd/rf24.o \
	${OBJECTDIR}/_ext/5b6daedd/spi.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/homecontrol

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/homecontrol: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/homecontrol ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/5b6daedd/DeviceBME280.o: /home/pi/Cpp/HomeControl/DeviceBME280.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5b6daedd
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5b6daedd/DeviceBME280.o /home/pi/Cpp/HomeControl/DeviceBME280.cpp

${OBJECTDIR}/_ext/5b6daedd/DeviceGeneric.o: /home/pi/Cpp/HomeControl/DeviceGeneric.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5b6daedd
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5b6daedd/DeviceGeneric.o /home/pi/Cpp/HomeControl/DeviceGeneric.cpp

${OBJECTDIR}/_ext/5b6daedd/DeviceManager.o: /home/pi/Cpp/HomeControl/DeviceManager.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5b6daedd
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5b6daedd/DeviceManager.o /home/pi/Cpp/HomeControl/DeviceManager.cpp

${OBJECTDIR}/_ext/5b6daedd/Log.o: /home/pi/Cpp/HomeControl/Log.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5b6daedd
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5b6daedd/Log.o /home/pi/Cpp/HomeControl/Log.cpp

${OBJECTDIR}/_ext/5b6daedd/PacketManager.o: /home/pi/Cpp/HomeControl/PacketManager.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5b6daedd
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5b6daedd/PacketManager.o /home/pi/Cpp/HomeControl/PacketManager.cpp

${OBJECTDIR}/_ext/5b6daedd/PeriodicTimer.o: /home/pi/Cpp/HomeControl/PeriodicTimer.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5b6daedd
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5b6daedd/PeriodicTimer.o /home/pi/Cpp/HomeControl/PeriodicTimer.cpp

${OBJECTDIR}/_ext/5b6daedd/Rf24Server.o: /home/pi/Cpp/HomeControl/Rf24Server.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5b6daedd
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5b6daedd/Rf24Server.o /home/pi/Cpp/HomeControl/Rf24Server.cpp

${OBJECTDIR}/_ext/5b6daedd/Runnable.o: /home/pi/Cpp/HomeControl/Runnable.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5b6daedd
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5b6daedd/Runnable.o /home/pi/Cpp/HomeControl/Runnable.cpp

${OBJECTDIR}/_ext/5b6daedd/Test.o: /home/pi/Cpp/HomeControl/Test.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5b6daedd
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5b6daedd/Test.o /home/pi/Cpp/HomeControl/Test.cpp

${OBJECTDIR}/_ext/5b6daedd/gpio.o: /home/pi/Cpp/HomeControl/gpio.c
	${MKDIR} -p ${OBJECTDIR}/_ext/5b6daedd
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5b6daedd/gpio.o /home/pi/Cpp/HomeControl/gpio.c

${OBJECTDIR}/_ext/5b6daedd/main.o: /home/pi/Cpp/HomeControl/main.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5b6daedd
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5b6daedd/main.o /home/pi/Cpp/HomeControl/main.cpp

${OBJECTDIR}/_ext/5b6daedd/mainLoop.o: /home/pi/Cpp/HomeControl/mainLoop.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5b6daedd
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5b6daedd/mainLoop.o /home/pi/Cpp/HomeControl/mainLoop.cpp

${OBJECTDIR}/_ext/5b6daedd/rf24.o: /home/pi/Cpp/HomeControl/rf24.c
	${MKDIR} -p ${OBJECTDIR}/_ext/5b6daedd
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5b6daedd/rf24.o /home/pi/Cpp/HomeControl/rf24.c

${OBJECTDIR}/_ext/5b6daedd/spi.o: /home/pi/Cpp/HomeControl/spi.c
	${MKDIR} -p ${OBJECTDIR}/_ext/5b6daedd
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5b6daedd/spi.o /home/pi/Cpp/HomeControl/spi.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
