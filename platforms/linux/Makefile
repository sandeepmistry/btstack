# Makefile for Linux based examples
BTSTACK_ROOT = ../..
POSIX_ROOT= ${BTSTACK_ROOT}/platforms/posix
LINUX_ROOT= ${BTSTACK_ROOT}/platforms/linux

CORE += main.c stdin_support.c

COMMON += hci_transport_linux_kernel.c run_loop_posix.c remote_device_db_fs.c

include ${BTSTACK_ROOT}/example/embedded/Makefile.inc

# CC = gcc-fsf-4.9
CFLAGS  += -g -Wall
# CFLAGS += -Werror

CFLAGS += -I${POSIX_ROOT}/src -I${LINUX_ROOT}/src
VPATH += ${BTSTACK_ROOT}/platforms/posix/src ${BTSTACK_ROOT}/platforms/linux/src

all: ${BTSTACK_ROOT}/include/btstack/version.h ${EXAMPLES}
