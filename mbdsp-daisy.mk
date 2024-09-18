MBDSP_ROOT:=$(dir $(realpath $(lastword $(MAKEFILE_LIST))))

MBDSP_THIRDPARTY = $(MBDSP_ROOT)/mbdsp/thirdparty

C_INCLUDES += -I$(MBDSP_ROOT) \
			-I$(MBDSP_THIRDPARTY) \
			-I$(MBDSP_THIRDPARTY)/hiir-1.4.0 \
    		-I$(MBDSP_THIRDPARTY)/gcem/include \
			-I$(MBDSP_THIRDPARTY)/SG14 \
			-I$(MBDSP_THIRDPARTY)/etl/include \
			-I$(MBDSP_THIRDPARTY)/reflect-cpp/include \

CPP_SOURCES += $(MBDSP_THIRDPARTY)/reflect-cpp/src/reflectcpp.cpp

OPT += -ffast-math 

CPP_STANDARD ?= -std=gnu++2b