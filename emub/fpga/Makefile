

# Simple Makefile for Verilog simulation with Icarus Verilog and GTKWave

CPU_DIR  := cpu
TB_DIR   := testbed

# Collect every .v file from the source directories
SRCS     := $(wildcard $(CPU_DIR)/*.v) $(wildcard $(TB_DIR)/*.v)

# Top‑level testbench module
TOP      := tbench_top

# Output files
VVP_OUT  := sim.vvp
VCD_FILE := dump

# Default target ─ compile, run, and launch waveform viewer
all: wave

###############################################################################
# Compile Verilog sources into a VVP executable
$(VVP_OUT): $(SRCS)
	iverilog -g2012 -I$(CPU_DIR) -I$(TB_DIR) -s $(TOP) -o $@ $^

# Run the simulation (produces $(VCD_FILE) via $dumpfile in your testbench)
sim: $(VVP_OUT)
	vvp $(VVP_OUT)

# Open GTKWave on the generated VCD
wave: sim
#	# mkfifo /tmp/gtkwave.rc
	@if ! pgrep -x gtkwave > /dev/null; then \
		echo "Starting GTKWave..."; \
		gtkwave $(VCD_FILE).vcd $(VCD_FILE).gtkw 1>/dev/null 2>/dev/null & \
	else \
		echo "GTKWave is already running."; \
	fi

# Utility target to clean build artefacts
clean:
	rm -f $(VVP_OUT) $(VCD_FILE).vcd

.PHONY: all sim wave clean