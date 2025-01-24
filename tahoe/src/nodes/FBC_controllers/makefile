# $Id: makefile,v 1.13 2008-03-09 02:59:02 tdnguye Exp $
#
# This makefile is divided into four sections:
#    (1) objects
#    (2) sources
#    (3) headers
#    (4) subdirectories
#
# Instructions for how to define these is given in each section below
#
include $(MACRO_DIR)/suffix
include $(MACRO_DIR)/$(ARCH).macros

##################### (1) objects
#
# (1.1) If there are objects in this directory:
#    (1) define the OBJ macro as the list of object files
#    (2) uncomment the OBJ_LINK and DEP macros here
#    (3) comment the OBJ, OBJ_LINK, and DEP macros in the
#        section (1.1).
#
OBJ = \
	AugLagSphereT.o \
	AugLagWallT.o \
	FBC_ControllerT.o \
	MFPenaltySphereT.o \
	PenaltyRegionT.o \
	PenaltySphereT.o \
	PenaltyWallT.o \
	PenaltyCylinderT.o \
	AugLagCylinderT.o \
	MFAugLagMultT.o \
	PressureBCT.o \
	FieldMFAugLagMultT.o \
	Penalty_AngledBC.o 

OBJ_LINK = $(OBJ)
DEP = $(OBJ:.o=.d)

#
# (1.2) If there are not object in this directory:
#    (1) comment the OBJ, OBJ_LINK, and DEP definitions in section (1.1)
#    (2) uncomment the OBJ, OBJ_LINK, and DEP definitions below
#
#OBJ = dummy
#OBJ_LINK =
#DEP = $(MACRO_DIR)/empty

##################### (2) sources
#
# (2) If there are sources in this directory list them in the
#     SRC macro; otherwise, comment the SRC macro.
#
SRC = \
	AugLagSphereT.cpp \
	AugLagWallT.cpp \
	FBC_ControllerT.cpp \
	MFPenaltySphereT.cpp \
	PenaltyRegionT.cpp \
	PenaltySphereT.cpp \
	PenaltyWallT.cpp \
	PenaltyCylinderT.cpp \
	AugLagCylinderT.cpp \
	MFAugLagMultT.cpp \
	PressureBCT.cpp \
	FieldMFAugLagMultT.cpp \
	Penalty_AngledBC.cpp 

##################### (3) headers
#
# (3) If there are header files in this directory:
#    (1) list the header files in the HDR macro
#    (2) uncomment the HDR_LINK
# 
# If there are no header files in this directory comment both the
# HDR and HDR_LINK macro   
#
HDR = \
	AugLagSphereT.h \
	AugLagWallT.h \
	FBC_ControllerT.h \
	MFPenaltySphereT.h \
	PenaltyRegionT.h \
	PenaltySphereT.h \
	PenaltyWallT.h \
	PenaltyCylinderT.h \
	AugLagCylinderT.h \
	MFAugLagMultT.h \
	PressureBCT.h \
	FieldMFAugLagMultT.h \
	Penalty_AngledBC.h  

HDR_LINK = $(HDR:.h=.h_link)

##################### (4) sub-directories
#
# (4.1) If there are sub-directories contained in this directory:
#    (1) list the subdirectories in the SUB_DIR macro
#    (2) uncomment the subdir_driver macro
#    (3) comment the SUB_DIR and subdir_driver in section (4.2)
#
#SUB_DIR = 
#subdir_driver: subdir_loop

#
# (4.2) If there are no sub-directories in this directory:
#    (1) comment the SUB_DIR and subdir_driver in section (4.1)
#    (2) uncomment the subdir_driver macro below
#
SUB_DIR =
subdir_driver: 

#####################

# instructions for subdirectories
include $(MACRO_DIR)/subdir.targets

# dependencies
DEPEND = $(MACRO_DIR)/empty
include $(DEPEND)
