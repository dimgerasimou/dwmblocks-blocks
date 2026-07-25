# See LICENSE file for copyright and license details.

# Installation prefix
PREFIX      ?= $(HOME)/.local/bin/dwmblocks

# Compiler settings
CC          := cc
PKG_CFLAGS  := $(shell pkg-config --cflags libnotify libnm libpulse dbus-1 glib-2.0)
PKG_LIBS    := $(shell pkg-config --libs libnotify libnm libpulse dbus-1 glib-2.0)
CFLAGS      := -Wall -Wextra -Wno-deprecated-declarations -I./src/include -I. -Os $(PKG_CFLAGS)
LDFLAGS     := -lm -lX11 -lxkbfile $(PKG_LIBS)

# Directory structure
SRC_DIR     := src
BUILD_DIR   := build
BIN_DIR     := $(BUILD_DIR)/bin
OBJ_DIR     := $(BUILD_DIR)/obj
INCLUDE     := src/include

# Source files
UTILS_SRC   := $(SRC_DIR)/utils.c $(SRC_DIR)/colors.c
UTILS_OBJ   := $(OBJ_DIR)/utils.o $(OBJ_DIR)/colors.o

# Block configuration - comment out blocks you don't need
BLOCKS      := time \
               keyboard \
               battery \
               date \
               kernel \
               bluetooth \
               internet \
               memory \
               power \
               volume

# Generated file lists
BLOCK_SRCS  := $(addprefix $(SRC_DIR)/, $(addsuffix .c, $(BLOCKS)))
BLOCK_OBJS  := $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(BLOCKS)))
BINARIES    := $(addprefix $(BIN_DIR)/, $(BLOCKS))
INSTALL_TO  := $(addprefix $(PREFIX)/, $(BLOCKS))

# Pretty Output
ECHO := /bin/echo -e

COLOR_RESET   := \033[0m
COLOR_BOLD    := \033[1m
COLOR_DIM     := \033[2m

COLOR_RED     := \033[1;31m
COLOR_GREEN   := \033[1;32m
COLOR_YELLOW  := \033[1;33m
COLOR_BLUE    := \033[1;34m
COLOR_MAGENTA := \033[1;35m
COLOR_CYAN    := \033[1;36m
COLOR_GRAY    := \033[0;90m

# Uniform log helpers
# Usage: $(call MSG,TAG,COLOR,"message")
MSG = $(ECHO) "$(COLOR_BOLD)$(COLOR_GRAY)==>$(COLOR_RESET) $(3)$(COLOR_BOLD)[$(1)]$(COLOR_RESET) $(4)$(COLOR_RESET)"
OK  = $(call MSG,OK,$(COLOR_GREEN),$(COLOR_GREEN),$(1))
INFO= $(call MSG,INFO,$(COLOR_CYAN),$(COLOR_CYAN),$(1))
WARN= $(call MSG,WARN,$(COLOR_YELLOW),$(COLOR_YELLOW),$(1))
ERR = $(call MSG,ERR,$(COLOR_RED),$(COLOR_RED),$(1))
DO  = $(call MSG,DO,$(COLOR_BLUE),$(COLOR_BLUE),$(1))
GEN = $(call MSG,GEN,$(COLOR_MAGENTA),$(COLOR_MAGENTA),$(1))
RM  = $(call MSG,RM,$(COLOR_RED),$(COLOR_RED),$(1))

# Phony targets
.PHONY: all clean install uninstall

# Prevent deletion of intermediate files
.SECONDARY: $(BLOCK_OBJS)

# Default target
all: $(BINARIES)

# Link binaries from object files
$(BIN_DIR)/%: $(OBJ_DIR)/%.o $(UTILS_OBJ) config.h
	@mkdir -p $(BIN_DIR)
	@$(call DO,Link $@)
	@$(CC) -o $@ $< $(UTILS_OBJ) $(CFLAGS) $(LDFLAGS)
	@$(call OK,Built $@)

# Compile block object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c config.h
	@mkdir -p $(OBJ_DIR)
	@$(call DO,CC $<)
	@$(CC) -c $< -o $@ $(CFLAGS)

# Compile utils object file
$(OBJ_DIR)/utils.o: $(SRC_DIR)/utils.c $(INCLUDE)/utils.h config.h
	@mkdir -p $(OBJ_DIR)
	@$(call DO,CC $<)
	@$(CC) -c $< -o $@ $(CFLAGS)

# Compile colors object file
$(OBJ_DIR)/colors.o: $(SRC_DIR)/colors.c $(INCLUDE)/colors.h $(INCLUDE)/utils.h config.h
	@mkdir -p $(OBJ_DIR)
	@$(call DO,CC $<)
	@$(CC) -c $< -o $@ $(CFLAGS)

config.h:
	@$(call GEN,Install default config.h from config.def.h)
	@cp config.def.h config.h
	@$(call OK,Wrote config.h)

# Clean build artifacts
clean:
	@$(call RM,Remove build directory and generated theme state)
	@rm -rf $(BUILD_DIR)
	@$(call OK,Clean complete)

# Install binaries
install: all
	@$(call GEN,Installing blocks to $(PREFIX))
	@mkdir -p $(PREFIX)
	@for block in $(BLOCKS); do \
		$(ECHO) "$(COLOR_BOLD)$(COLOR_GRAY)==>$(COLOR_RESET) $(COLOR_GREEN)[COPY]$(COLOR_RESET) $(BIN_DIR)/$$block $(COLOR_GRAY)->$(COLOR_RESET) $(PREFIX)/"; \
		cp "$(BIN_DIR)/$$block" "$(PREFIX)/"; \
		chmod +x "$(PREFIX)/$$block"; \
	done
	@$(call OK,Installation complete)

# Uninstall binaries
uninstall:
	@$(call GEN,Removing blocks from $(PREFIX))
	@for block in $(BLOCKS); do \
		if [ -f "$(PREFIX)/$$block" ]; then \
			$(ECHO) "$(COLOR_BOLD)$(COLOR_GRAY)==>$(COLOR_RESET) $(COLOR_YELLOW)[DEL]$(COLOR_RESET) $(PREFIX)/$$block"; \
			rm -f "$(PREFIX)/$$block"; \
		else \
			$(ECHO) "$(COLOR_BOLD)$(COLOR_GRAY)==>$(COLOR_RESET) $(COLOR_DIM)$(COLOR_GRAY)[SKIP]$(COLOR_RESET) $(PREFIX)/$$block (not found)$(COLOR_RESET)"; \
		fi; \
	done
	@$(call OK,Uninstall complete)

# Dependency tracking for header changes
-include $(BLOCK_OBJS:.o=.d)

# Generate dependency files
$(BUILD_DIR)/%.d: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	@$(CC) -MM -MT '$(OBJ_DIR)/$*.o' $< -MF $@ $(CFLAGS)
