PREFIX      := $(HOME)/.local/bin/dwmblocks
CC          := cc
CFLAGS      := -Wall -Wextra -Werror -Wno-deprecated-declarations -Os
PKG_FLAGS   := $(shell pkg-config --cflags --libs libnotify libnm libpulse dbus-1)
LDFLAGS     := -lm -lX11 -lxkbfile $(PKG_FLAGS)

SRC_DIR     := src
BUILD_DIR   := build
BIN_DIR     := bin
INCLUDE     := include
COMMON_SRC  := $(SRC_DIR)/common.c
BLOCKS      := time keyboard battery date kernel bluetooth internet memory power volume

BLOCK_SRCS  := $(addprefix $(SRC_DIR)/, $(addsuffix .c, $(BLOCKS)))
BLOCK_OBJS  := $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(BLOCKS)))
COMMON_OBJ  := $(BUILD_DIR)/common.o

BINARIES    := $(addprefix $(BIN_DIR)/, $(BLOCKS))
INSTALL_TO  := $(addprefix $(PREFIX)/, $(BLOCKS))

.SECONDARY: $(BLOCK_OBJS)
.PHONY: all clean install uninstall

all: $(BINARIES)

# Compile binaries by linking object files
$(BIN_DIR)/%: $(BUILD_DIR)/%.o $(COMMON_OBJ) $(INCLUDE)/colorscheme.h $(INCLUDE)/config.h
	@mkdir -p $(BIN_DIR)
	@echo "Linking $@"
	@$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

# Compile object files from block sources
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(INCLUDE)/colorscheme.h $(INCLUDE)/config.h
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<"
	@$(CC) -c $< -o $@ $(CFLAGS) $(LDFLAGS)

# Compile object file for shared code
$(COMMON_OBJ): $(COMMON_SRC)
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling common.c"
	@$(CC) -c $< -o $@ $(CFLAGS) $(LDFLAGS)

# Generate colorscheme.h
$(INCLUDE)/colorscheme.h: $(SRC_DIR)/loadresources.c
	@echo "Generating colorscheme.h..."
	@mkdir -p $(BIN_DIR)
	@$(CC) -o $(BIN_DIR)/loadresources $< $(CFLAGS) -lX11
	@$(BIN_DIR)/loadresources $@

clean:
	@echo "Cleaning up..."
	@rm -rf $(BUILD_DIR) $(BIN_DIR)

install: all
	@echo "Installing blocks to $(PREFIX)"
	@mkdir -p $(PREFIX)
	@cp $(BINARIES) $(PREFIX)/

uninstall:
	@echo "Removing installed blocks from $(PREFIX)"
	@rm -f $(INSTALL_TO)
