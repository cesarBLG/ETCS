# Find all .po files in the source directory and generate .mo files
PO_FILES := $(wildcard ./**/*.po)
MO_FILES := $(patsubst ./%.po, ./%.mo, $(PO_FILES))

# Default target: Compile all .po files to .mo files
all: $(MO_FILES)

# Rule to compile a .po file to a .mo file
%.mo: %.po
	msgfmt $< -o $@

# Clean up generated .mo files
clean:
	rm -f $(MO_FILES)

.PHONY: all clean 
