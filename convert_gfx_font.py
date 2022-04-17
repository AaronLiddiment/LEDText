# This program was made specifically to convert
# https://github.com/o0shojo0o/PixelIt/blob/master/src/PixelItFont.h,
# and will probably require modification for other fonts.

from math import ceil
import re
import sys
import argparse

any_int = lambda x: int(x, 0)

parser = argparse.ArgumentParser(description="Convert Adafruit GFX font to LEDText.")
parser.add_argument("input_file", help="source .h file containing GFX font")
parser.add_argument("output_file", help="destination filename for LEDText .h font file")
parser.add_argument("--name", default="PixelIt", help="font name")
parser.add_argument(
    "--first", default=0x20, type=any_int, help="starting character number"
)
parser.add_argument(
    "--last", default=0x7F, type=any_int, help="ending character number"
)
parser.add_argument(
    "--max_height", default=6, type=any_int, help="maximum character height in pixels"
)

args = parser.parse_args()

FOOTER = """};

#endif
"""

max_width = 0

font_file = open(args.input_file, "r")
out_file = open(args.output_file, "w")

# Read in the source font
while True:
    l = font_file.readline()
    if not l:
        sys.exit("Error: never found bitmap")
    if re.search("bitmaps.*{", l, re.IGNORECASE):
        break

# Read the bitmap section
bitmaps = []
while True:
    l = font_file.readline()
    if not l:
        sys.exit("Unexpected EOF reading bitmaps")
    if l.find("}") != -1:
        break
    l = re.sub("//.*", "", l)
    for b in re.findall("0x[0-9a-f]{2}", l, re.IGNORECASE):
        bitmaps.append(int(b, 16))

while True:
    l = font_file.readline()
    if not l:
        sys.exit("Error: never found glyphs")
    if re.search("glyphs.*{", l, re.IGNORECASE):
        break

# Read the glyphs section
glyphs = []
while True:
    l = font_file.readline()
    if not l:
        sys.exit("Unexpected EOF reading glyphs")
    if l.find("};") != -1:
        break
    l = re.sub("//.*", "", l)
    m = re.search(
        "{.*?(-?\d+).*?,.*?(-?\d+).*?,.*?(-?\d+).*?,.*?(-?\d+).*?,.*?(-?\d+).*?,.*?(-?\d+).*?}",
        l,
    )
    if m:
        g = m.groups()
        if len(g) != 6:
            sys.exit(f"Expected 6 elements at: {l}")
        glyphs.append(g)
        if int(g[1]) > max_width:
            max_width = int(g[1])


def render(ch):
    n = ord(ch)
    glyph = glyphs[n - args.first]
    i = int(glyph[0])
    width = int(glyph[1])
    height = int(glyph[2])
    xadv = int(glyph[3])
    xoff = int(glyph[4])
    yoff = int(glyph[5])

    bytes_per_line = ceil(width / 8)

    cells = [list(" " * max_width) for i in range(args.max_height)]

    for y in range(height):
        for x in range(bytes_per_line):
            b = bitmaps[i]
            i += 1
            for bit in range(8):
                if bit < xadv and b & (2 ** (7 - bit)):
                    cells[y + args.max_height + yoff - 1][bit + xoff + x * 8] = "*"

    return xadv, cells


def printchar(ch):
    xadv, cells = render(ch)

    for y in range(args.max_height):
        print("".join(cells[y]))


def convert(ch):
    xadv, cells = render(ch)

    s = f"{xadv-1}"

    for y in range(args.max_height):
        b = 0
        bit = 7
        r = cells[y]
        while r:
            b |= 0 if r.pop(0) == " " else 2 ** bit
            bit -= 1
            if bit < 0:
                s += f", {hex(b)}"
                b = 0
                bit = 7
        if bit != 7:
            s += f", {hex(b)}"

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


HEADER = f"""#ifndef Font{args.name}_h
#define Font{args.name}_h

#include <stdint.h>

constexpr uint8_t {args.name}FontData[] = {{
	FONT_PROPORTIONAL | {max_width},  // Font Maximum Width
	{args.max_height},  // Font Height
	{args.first}, // Font First Character
	{args.last},// Font Last Character
"""

print(HEADER, end="", file=out_file)

for ch in range(args.first, args.last + 1):
    comma = "," if ch != args.last else " "
    print(f"\t{convert(chr(ch))}{comma} // {safechr(ch)}", file=out_file)

print(FOOTER, end="", file=out_file)
