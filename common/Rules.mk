d := $(dir $(lastword $(MAKEFILE_LIST)))

SRCS += $(addprefix $(d), transaction.cpp)

OBJS-common_transaction := $(o)transaction.o
