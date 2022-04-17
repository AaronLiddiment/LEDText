# Hacky script to turn the Adafruit GFX "classic" font into an LEDText font.
# Makes all sorts of assumptions, because it's only meant to run once.

# This program was made specifically to convert
# https://github.com/adafruit/Adafruit-GFX-Library/blob/master/glcdfont.c,
# and will probably require modification for other fonts.

import re
import sys

NUM_CHARS = 256
BYTES_PER_CHAR = 5

WIDTH = 5
HEIGHT = 8

FIRST = 32
LAST = 127

font_file = open("glcdfont.c", "r")
out_file = open("FontClassic.h", "w")

# Read in the source font
while True:
    l = font_file.readline()
    if not l:
        sys.exit("Error: never found bitmap")
    if re.search("font.*= {", l, re.IGNORECASE):
        break

# Extract the bitmaps
font = []
while True:
    l = font_file.readline()
    if not l:
        sys.exit("Unexpected EOF reading bitmaps")
    if l.find("};") != -1:
        break
    l = re.sub("//.*", "", l)
    for b in re.findall("0x[0-9a-f]{2}", l, re.IGNORECASE):
        font.append(int(b, 16))

expect_bytes = NUM_CHARS * BYTES_PER_CHAR
if len(font) != expect_bytes:
    sys.exit(f"Expected to read {expect_bytes} bytes, got {len(font)}")

# Render a given character into a two-dimensional grid
def render(ch):
    # Efficiency isn't important here, so we're literally using a matrix
    # of spaces and asterisks. Makes it easy to debug.
    cells = [list(" " * WIDTH) for i in range(HEIGHT)]

    # https://github.com/adafruit/Adafruit-GFX-Library/blob/6f4981e2f15dc7c80e53ecc6b721e9afed3f2815/Adafruit_GFX.cpp#L1150,
    # basically
    for i in range(WIDTH):
        line = font[ch * 5 + i]
        for j in range(HEIGHT):
            if line & 1:
                cells[j][i] = "*"
            line >>= 1

    return cells


# Print out character for debugging
def printchar(ch):
    cells = render(ch)

    for y in range(len(cells)):
        print("".join(cells[y]))


# Returns one line of LEDText font data
def convert(ch):
    cells = render(ch)

    s = ""

    for y in range(HEIGHT):
        byte = 0  # Assemble the byte one bit at a time

        bit = 7
        for x in range(WIDTH):
            if cells[y][x] != " ":
                byte |= 2 ** bit
            bit -= 1

        if len(s):
            s += ", "
        s += "0x{:02x}".format(byte)

    return s


def safechr(cnum):
    ch = chr(cnum)
    if ch == " ":
        return "Space"
    elif ch == "\\":
        return "Backslash"
    elif ch == "\x7f":
        return "DEL"
    else:
        return ch


HEADER = f"""#ifndef FontClassic_h
#define FontClassic_h

#include <stdint.h>

constexpr uint8_t ClassicFontData[] = {{
\t{WIDTH},\t// Font Width
\t{HEIGHT},\t// Font Height
\t{FIRST},\t// Font First Character
\t{LAST},\t// Font Last Character
"""

print(HEADER, end="", file=out_file)

for ch in range(FIRST, LAST + 1):
    comma = "," if ch != LAST else " "
    print(f"\t{convert(ch)}{comma} // {safechr(ch)}", file=out_file)

FOOTER = """};

#endif
"""

print(FOOTER, end="", file=out_file)
