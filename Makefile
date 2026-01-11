# See LICENSE file for copyright and license details.

# Installation prefix
PREFIX      ?= $(HOME)/.local/bin/dwmblocks

# Compiler settings
CC          := cc
PKG_CFLAGS  := $(shell pkg-config --cflags libnotify libnm libpulse dbus-1 glib-2.0)
PKG_LIBS    := $(shell pkg-config --libs libnotify libnm libpulse dbus-1 glib-2.0)
CFLAGS      := -Wall -Wextra -Werror -Wno-deprecated-declarations -I./src/include -I. -Os $(PKG_CFLAGS)
LDFLAGS     := -lm -lX11 -lxkbfile $(PKG_LIBS)

# Directory structure
SRC_DIR     := src
BUILD_DIR   := build
BIN_DIR     := $(BUILD_DIR)/bin
OBJ_DIR     := $(BUILD_DIR)/obj
INCLUDE     := src/include
THEMES_DIR  := themes

# Theme selection (can override: make THEME=gruvbox)
THEME       ?= catppuccin
THEME_FILE  := $(THEMES_DIR)/$(THEME).h

# Source files
UTILS_SRC   := $(SRC_DIR)/utils.c
UTILS_OBJ   := $(OBJ_DIR)/utils.o

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
.PHONY: all clean install uninstall help theme list-themes rebuild save-theme check-theme

# Prevent deletion of intermediate files
.SECONDARY: $(BLOCK_OBJS)

# Default target
all: check-theme $(BINARIES)

# Check if theme has changed and regenerate colorscheme if needed
check-theme:
	@mkdir -p $(INCLUDE)
	@CURRENT_THEME=""; \
	if [ -f "$(INCLUDE)/.current_theme" ]; then \
		CURRENT_THEME=$$(cat "$(INCLUDE)/.current_theme"); \
	fi; \
	if [ "$$CURRENT_THEME" != "$(THEME)" ]; then \
		$(call WARN,Theme changed: '$$CURRENT_THEME' -> '$(THEME)'; will regenerate colorscheme) ; \
		rm -f "$(INCLUDE)/colorscheme.h"; \
		$(ECHO) "$(THEME)" > "$(INCLUDE)/.current_theme"; \
	else \
		$(call INFO,Theme unchanged: '$(THEME)') ; \
	fi

# Help target
help:
	@$(ECHO) ""
	@$(ECHO) "$(COLOR_BOLD)$(COLOR_BLUE)dwmblocks-blocks build system$(COLOR_RESET)"
	@$(ECHO) ""
	@$(ECHO) "$(COLOR_BOLD)Targets:$(COLOR_RESET)"
	@$(ECHO) "  $(COLOR_CYAN)all$(COLOR_RESET)           Build all blocks (default)"
	@$(ECHO) "  $(COLOR_CYAN)clean$(COLOR_RESET)         Remove build artifacts"
	@$(ECHO) "  $(COLOR_CYAN)install$(COLOR_RESET)       Install blocks to $(PREFIX)"
	@$(ECHO) "  $(COLOR_CYAN)uninstall$(COLOR_RESET)     Remove installed blocks"
	@$(ECHO) "  $(COLOR_CYAN)rebuild$(COLOR_RESET)       Clean and rebuild everything"
	@$(ECHO) "  $(COLOR_CYAN)theme$(COLOR_RESET)         Force regenerate colorscheme from Xresources"
	@$(ECHO) "  $(COLOR_CYAN)list-themes$(COLOR_RESET)   Show available themes"
	@$(ECHO) "  $(COLOR_CYAN)save-theme$(COLOR_RESET)    Save current Xresources as theme (NAME=...)"
	@$(ECHO) ""
	@$(ECHO) "$(COLOR_BOLD)Options:$(COLOR_RESET)"
	@$(ECHO) "  PREFIX=<path>        Installation directory"
	@$(ECHO) "  THEME=<name>         Color theme (default: $(THEME))"
	@$(ECHO) "  BLOCKS='block1 ...'  Build only specified blocks"
	@$(ECHO) ""
	@$(ECHO) "$(COLOR_BOLD)Examples:$(COLOR_RESET)"
	@$(ECHO) "  make THEME=gruvbox"
	@$(ECHO) "  make save-theme NAME=mytheme"
	@$(ECHO) "  make BLOCKS='battery volume'"
	@$(ECHO) "  make clean install PREFIX=/usr/local/bin/dwmblocks"
	@$(ECHO) ""

# List available themes
list-themes:
	@$(call INFO,Available themes in '$(THEMES_DIR)') ; \
	if [ -d "$(THEMES_DIR)" ]; then \
		for theme in $(THEMES_DIR)/*.h; do \
			[ -f "$$theme" ] && $(ECHO) "  $(COLOR_GREEN)-$(COLOR_RESET) $$(basename "$$theme" | sed 's/\.h//')"; \
		done; \
	else \
		$(ECHO) "  $(COLOR_YELLOW)(no themes directory found)$(COLOR_RESET)"; \
	fi
	@$(ECHO) ""
	@$(call INFO,Current theme: $(THEME)) ; \
	if [ ! -f "$(THEME_FILE)" ]; then \
		$(call WARN,Theme file not found; will generate from Xresources) ; \
	fi

# Link binaries from object files
$(BIN_DIR)/%: $(OBJ_DIR)/%.o $(UTILS_OBJ) $(INCLUDE)/colorscheme.h config.h
	@mkdir -p $(BIN_DIR)
	@$(call DO,Link $@)
	@$(CC) -o $@ $< $(UTILS_OBJ) $(CFLAGS) $(LDFLAGS)
	@$(call OK,Built $@)

# Compile block object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INCLUDE)/colorscheme.h config.h
	@mkdir -p $(OBJ_DIR)
	@$(call DO,CC $<)
	@$(CC) -c $< -o $@ $(CFLAGS)

# Compile utils object file
$(UTILS_OBJ): $(UTILS_SRC) $(INCLUDE)/utils.h config.h
	@mkdir -p $(OBJ_DIR)
	@$(call DO,CC $<)
	@$(CC) -c $< -o $@ $(CFLAGS)

# Generate or copy colorscheme
$(INCLUDE)/colorscheme.h:
	@mkdir -p $(INCLUDE) $(BIN_DIR)
	@if [ -f "$(THEME_FILE)" ]; then \
		$(call GEN,Use theme '$(THEME)' ($(THEME_FILE))) ; \
		rm -f "$@"; \
		cp "$(THEME_FILE)" "$@"; \
		$(call OK,Generated $@) ; \
	else \
		$(call WARN,Theme '$(THEME)' not found; generating from Xresources) ; \
		rm -f "$@"; \
		$(CC) -o "$(BIN_DIR)/loadresources" "$(SRC_DIR)/loadresources.c" -lX11; \
		"$(BIN_DIR)/loadresources" "$@"; \
		$(call OK,Generated $@ from Xresources) ; \
	fi

config.h:
	@$(call GEN,Install default config.h from config.def.h)
	@cp config.def.h config.h
	@$(call OK,Wrote config.h)

# Force regenerate colorscheme from Xresources
theme: $(SRC_DIR)/loadresources.c
	@$(call GEN,Regenerating colorscheme from Xresources)
	@mkdir -p $(BIN_DIR) $(INCLUDE)
	@$(CC) -o $(BIN_DIR)/loadresources $< -lX11
	@$(BIN_DIR)/loadresources $(INCLUDE)/colorscheme.h
	@$(call OK,Colorscheme generated)

# Save current Xresources as a theme
save-theme: $(SRC_DIR)/loadresources.c
	@if [ -z "$(NAME)" ]; then \
		$(call ERR,Usage: make save-theme NAME=mytheme) ; \
		exit 1; \
	fi
	@$(call GEN,Saving Xresources theme as '$(NAME)')
	@mkdir -p $(THEMES_DIR) $(BIN_DIR)
	@$(CC) -o $(BIN_DIR)/loadresources $< -lX11
	@$(BIN_DIR)/loadresources $(THEMES_DIR)/$(NAME).h
	@$(call OK,Theme saved: $(THEMES_DIR)/$(NAME).h)

# Clean build artifacts
clean:
	@$(call RM,Remove build directory and generated theme state)
	@rm -rf $(BUILD_DIR)
	@rm -f $(INCLUDE)/.current_theme
	@rm -f $(INCLUDE)/colorscheme.h
	@$(call OK,Clean complete)

# Full rebuild
rebuild: clean all

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

# Print variables (for debugging the Makefile)
print-%:
	@$(call INFO,$* = $($*))

# Dependency tracking for header changes
-include $(BLOCK_OBJS:.o=.d)

# Generate dependency files
$(BUILD_DIR)/%.d: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	@$(CC) -MM -MT '$(OBJ_DIR)/$*.o' $< -MF $@ $(CFLAGS)