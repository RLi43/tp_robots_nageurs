# What program(s) have to be built
PROGRAMS = ex8

# Libraries needed for the executable file
LIBS = -lwsock32
# Dependencies for the program(s) to build
ex8: ../common/netutil.o ../common/remregs.o ../common/robot.o ../common/wperror.o ../common/trkcli.o ../common/utils.o ex8.o

# Includes the common Makefile with the various rules
include ../common/Makefile.inc