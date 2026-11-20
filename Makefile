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
PACKAGE_STAGE := $(BUILD_DIRECTORY)/package
PACKAGE_ARCH ?= $(shell getarch)
PACKAGE_FILE := $(CURDIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION)-$(PACKAGE_REVISION)-$(PACKAGE_ARCH).hpkg
INSTALL_DIRECTORY ?= $(shell finddir B_USER_NONPACKAGED_DIRECTORY)/apps

DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine

.PHONY: install
install: all
	@if [ -z "$(INSTALL_DIRECTORY)" ] || [ "$(INSTALL_DIRECTORY)" = "/apps" ]; then \
		echo "error: could not determine the user non-packaged applications directory" >&2; \
		exit 1; \
	fi; \
	binary=$$(find $(SOURCE_DIRECTORY) -maxdepth 2 -type f -name $(APP_NAME) -path '*/objects.*/*' -print -quit); \
	if [ -z "$$binary" ]; then \
		echo "error: could not find the built $(APP_NAME) binary" >&2; \
		exit 1; \
	fi; \
	mkdir -p "$(INSTALL_DIRECTORY)"; \
	install -m 755 "$$binary" "$(INSTALL_DIRECTORY)/$(APP_NAME)"; \
	echo "Installed $(APP_NAME) in $(INSTALL_DIRECTORY)"

hpkg: all
	@set -e; \
	if [ -z "$(PACKAGE_ARCH)" ]; then \
		echo "error: could not determine the Haiku package architecture" >&2; \
		exit 1; \
	fi; \
	command -v package >/dev/null 2>&1 || { \
		echo "error: Haiku's package command was not found" >&2; \
		exit 1; \
	}; \
	binary=$$(find $(SOURCE_DIRECTORY) -maxdepth 2 -type f -name $(APP_NAME) -path '*/objects.*/*' -print -quit); \
	if [ -z "$$binary" ]; then \
		echo "error: could not find the built $(APP_NAME) binary" >&2; \
		exit 1; \
	fi; \
	rm -rf "$(PACKAGE_STAGE)"; \
	mkdir -p "$(PACKAGE_STAGE)/apps"; \
	install -m 755 "$$binary" "$(PACKAGE_STAGE)/apps/$(APP_NAME)"; \
	sed 's/@ARCH@/$(PACKAGE_ARCH)/g' packaging/PackageInfo.in > "$(PACKAGE_STAGE)/.PackageInfo"; \
	package create -C "$(PACKAGE_STAGE)" "$(PACKAGE_FILE)"; \
	test -f "$(PACKAGE_FILE)"; \
	echo "Created $(PACKAGE_FILE)"
