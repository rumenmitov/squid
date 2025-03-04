TARGET   = squid
SRC_CC   = main.cc squid.cc benchmark.cc
LIBS     = vfs_lwext4 base format vfs lwext4

INC_DIR += $(call select_from_ports,lwext4)/include
INC_DIR += $(REP_DIR)/src/app/squid/include

CC_OPT += -DCONFIG_USE_DEFAULT_CFG=1
CC_OPT += -DCONFIG_HAVE_OWN_ERRNO=1
CC_OPT += -DCONFIG_HAVE_OWN_ASSERT=1
CC_OPT += -DCONFIG_BLOCK_DEV_CACHE_SIZE=256

vpath     %.c $(LWEXT4_DIR)/src
vpath     %.c $(LWEXT4_DIR)/blockdev
vpath qsort.c $(REP_DIR)/src/lib/lwext4/
vpath    %.cc $(REP_DIR)/src/lib/lwext4/

