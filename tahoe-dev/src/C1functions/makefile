# $Id: makefile,v 1.36 2013-11-07 19:29:54 tahoe.vickynguyen Exp $
# object directory makefile - compilation instructions

include $(MACRO_DIR)/suffix
include $(MACRO_DIR)/$(ARCH).macros

# objects
OBJ = \
	MajumdarBhushan.o \
	GreenwoodWilliamson.o \
	GWPlastic.o \
	PolyDistributionT.o \
	ConHypGeom.o \
	WormLikeChain.o \
	WLCwRep.o \
	FungwRep.o \
	FungType.o \
	VWType.o \
	ScaledCsch.o \
	ScaledSinh.o \
	PowerTrig.o \
	LanirFiber.o \
	LinearType.o 

OBJ_LINK = $(OBJ)
DEP = $(OBJ:.o=.d)
#OBJ = dummy
#OBJ_LINK =
#DEP = $(MACRO_DIR)/empty

# sources
SRC = \
	MajumdarBhushan.cpp \
	GreenwoodWilliamson.cpp \
	GWPlastic.cpp \
	PolyDistributionT.cpp \
	ConHypGeom.cpp \
	WormLikeChain.cpp \
	WLCwRep.cpp \
	FungwRep.cpp \
	FungType.cpp \
	VWType.cpp \
	ScaledCsch.cpp \
	ScaledSinh.cpp \
	PowerTrig.cpp \
	LanirFiber.cpp \
	LinearType.cpp  


# headers
HDR = \
	MajumdarBhushan.h \
	GreenwoodWilliamson.h \
	GWPlastic.h \
	PolyDistributionT.h \
	ConHypGeom.h \
	WormLikeChain.h \
	WLCwRep.h \
	FungwRep.h \
	FungType.h \
	VWType.h \
	ScaledCsch.h \
	ScaledSinh.h \
	PowerTrig.h \
	LanirFiber.h \
	LinearType.h 


HDR_LINK = $(HDR:.h=.h_link)

# subdirectories
SUB_DIR =  
#subdir_driver: subdir_loop
#SUB_DIR is empty
subdir_driver: 

# instructions for subdirectories
include $(MACRO_DIR)/subdir.targets

# dependencies
DEPEND = $(MACRO_DIR)/empty
include $(DEPEND)
