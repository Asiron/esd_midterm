# Target specific macros
TOPPERS_KERNEL = OSEK_COM
TARGET = esd_midterm
TARGET_SOURCES = \
	esd_midterm.c
TOPPERS_OSEK_OIL_SOURCE = ./esd_midterm.oil

# Don't modify below part
O_PATH ?= build
include ../../ecrobot/ecrobot.mak