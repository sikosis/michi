## Haiku Generic Makefile ##

NAME = michi
TYPE = APP
APP_MIME_SIG = application/x-vnd.michi

SRCS = \
	src/main.cpp \
	src/MichiApp.cpp \
	src/AboutWindow.cpp \
	src/Project.cpp \
	src/ProjectItem.cpp \
	src/ProjectEditor.cpp \
	src/ProjectReplicantView.cpp \
	src/SettingsWindow.cpp \
	src/MainWindow.cpp \
	src/DeskbarView.cpp

RDEFS = resources/Michi.rdef
RSRCS =

LIBS = be $(STDCPPLIBS)
LIBPATHS =
SYSTEM_INCLUDE_PATHS =
LOCAL_INCLUDE_PATHS = src
OPTIMIZE = SOME
LOCALES = en
DEFINES =
WARNINGS = ALL
SYMBOLS =
DEBUGGER =
COMPILER_FLAGS = -std=c++17 -Woverloaded-virtual -Wwrite-strings
LINKER_FLAGS =
APP_VERSION =
DRIVER_PATH =

INSTALL_DIR ?= $(shell finddir B_USER_NONPACKAGED_DIRECTORY)/apps

# Keep the package version in sync with src/Constants.h and resources/Michi.rdef.
PACKAGE_NAME ?= $(NAME)
PACKAGE_VERSION ?= 0.42
PACKAGE_REVISION ?= 1
PACKAGE_ARCH ?= $(shell getarch)
PACKAGE_STAGE := $(CURDIR)/package
PACKAGE_FILE := $(CURDIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION)-$(PACKAGE_REVISION)-$(PACKAGE_ARCH).hpkg

.DEFAULT_GOAL := default
.PHONY: all install hpkg

# The makefile engine supplies the build and the copy half of `install`.
# Check the destination before that copy can create anything in the wrong place.
install ::
	@if [ -z "$(INSTALL_DIR)" ] || [ "$(INSTALL_DIR)" = "/apps" ]; then \
		echo "error: could not determine the user non-packaged applications directory" >&2; \
		exit 1; \
	fi

DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine

all: default

install ::
	@mimeset -f "$(INSTALL_DIR)/$(NAME)"

hpkg: default
	@set -e; \
	if [ -z "$(PACKAGE_ARCH)" ]; then \
		echo "error: could not determine the Haiku package architecture" >&2; \
		exit 1; \
	fi; \
	command -v package >/dev/null 2>&1 || { \
		echo "error: Haiku's package command was not found" >&2; \
		exit 1; \
	}; \
	test -f packaging/PackageInfo.in || { \
		echo "error: packaging/PackageInfo.in was not found" >&2; \
		exit 1; \
	}; \
	rm -rf "$(PACKAGE_STAGE)"; \
	mkdir -p "$(PACKAGE_STAGE)/apps"; \
	install -m 755 "$(TARGET)" "$(PACKAGE_STAGE)/apps/$(NAME)"; \
	sed -e 's/@ARCH@/$(PACKAGE_ARCH)/g' -e 's/@VERSION@/$(PACKAGE_VERSION)/g' -e 's/@REVISION@/$(PACKAGE_REVISION)/g' packaging/PackageInfo.in > "$(PACKAGE_STAGE)/.PackageInfo"; \
	package create -C "$(PACKAGE_STAGE)" "$(PACKAGE_FILE)"; \
	test -f "$(PACKAGE_FILE)"; \
	echo "Created $(PACKAGE_FILE)"
