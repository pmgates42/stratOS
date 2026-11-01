from PIL import Image, ImageDraw, ImageFont
import math
import os

# Regenerates wiring_bench.png next to this script. Requires: pip install pillow
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "wiring_bench.png")
W, H = 1760, 1520
img = Image.new("RGB", (W, H), "white")
d = ImageDraw.Draw(img)
F = lambda s, b=False: ImageFont.truetype(r"C:\Windows\Fonts\arial%s.ttf" % ("bd" if b else ""), s)
f_title, f_h, f_lbl, f_small, f_pin = F(32, True), F(22, True), F(19, True), F(16), F(13, True)

PIN_FUNC = {
    1: "3V3", 2: "5V", 3: "GPIO2 SDA", 4: "5V", 5: "GPIO3 SCL", 6: "GND",
    7: "GPIO4", 8: "GPIO14 TXD", 9: "GND", 10: "GPIO15 RXD", 11: "GPIO17",
    12: "GPIO18", 13: "GPIO27", 14: "GND", 15: "GPIO22", 16: "GPIO23",
    17: "3V3", 18: "GPIO24", 19: "GPIO10", 20: "GND", 21: "GPIO9",
    22: "GPIO25", 23: "GPIO11", 24: "GPIO8", 25: "GND", 26: "GPIO7",
    27: "ID_SD", 28: "ID_SC", 29: "GPIO5", 30: "GND", 31: "GPIO6",
    32: "GPIO12", 33: "GPIO13", 34: "GND", 35: "GPIO19", 36: "GPIO16",
    37: "GPIO26", 38: "GPIO20", 39: "GND", 40: "GPIO21",
}

# ---- Pi board in bench orientation: USB ports at top, header on left edge,
# pin 1 at the BOTTOM of the header, odd pins in the INNER column.
BRD = (300, 170, 1010, 1180)
X_EDGE, X_INNER = 335, 393       # even pins (board edge) / odd pins (inner)
Y_TOP, DY, R = 240, 46, 14       # pins 39/40 at top
def pin_xy(n):
    r = (n - 1) // 2
    return (X_INNER if n % 2 else X_EDGE, Y_TOP + (19 - r) * DY)

# ---- module to the right (same orientation as before: RJ45 up)
BX0, BX1, BY0, BY1 = 1230, 1540, 420, 950
JY0, JDY = 610, 62
J1 = ["G", "MO", "SCK", "CS", "INT"]
J2 = ["G", "V", "NC", "RST", "MI"]
def jpin(hdr, name):
    lst = J1 if hdr == 1 else J2
    return ((BX0 if hdr == 1 else BX1), JY0 + lst.index(name) * JDY)

CONN = [
    (17, 2, "V",   "3V3 (or pin 1)", (220, 30, 30), False),
    (39, 2, "G",   "GND",            (20, 20, 20), False),
    (40, 1, "SCK", "SCK <- GPIO21",  (240, 120, 0), False),
    (38, 1, "MO",  "MO <- GPIO20",   (30, 90, 220), False),
    (35, 2, "MI",  "GPIO19 -> MI",   (0, 150, 70), False),
    (36, 1, "CS",  "CS <- GPIO16",   (150, 40, 180), False),
    (37, 2, "RST", "GPIO26 -> RST",  (140, 90, 30), False),
    (33, 1, "INT", "GPIO13 -> INT (optional)", (120, 120, 120), True),
]
UART = {6, 8, 10}

def line(pts, color, width=5, dashed=False):
    if not dashed:
        d.line(pts, fill=color, width=width, joint="curve"); return
    for (x0, y0), (x1, y1) in zip(pts, pts[1:]):
        L = math.hypot(x1 - x0, y1 - y0); n = max(1, int(L // 14))
        for i in range(0, n, 2):
            t0, t1 = i / n, min(1, (i + 1) / n)
            d.line([(x0 + (x1 - x0) * t0, y0 + (y1 - y0) * t0),
                    (x0 + (x1 - x0) * t1, y0 + (y1 - y0) * t1)], fill=color, width=width)

# ---- title
d.text((40, 28), "W5500 Lite  <->  Pi 3B+  -  drawn in YOUR bench orientation", font=f_title, fill="black")
d.text((40, 74), "Pi as in the video: USB/Ethernet at the top, GPIO header down the left edge, HDMI on the right. "
                 "Pin 1 is the BOTTOM pin of the INNER column; pins 39/40 are at the top next to the USB ports.",
       font=f_small, fill=(70, 70, 70))

# ---- board + ports
d.rounded_rectangle(BRD, radius=18, fill=(205, 232, 212), outline=(60, 120, 80), width=3)
# USB stacks + Ethernet along the top edge (stick up past the board edge)
for x0, x1, name in [(520, 650, "USB"), (680, 810, "USB"), (850, 990, "ETH")]:
    d.rectangle([x0, BRD[1] - 70, x1, BRD[1] + 60], fill=(190, 190, 195), outline="black", width=2)
    d.text(((x0 + x1) / 2, BRD[1] - 5), name, font=f_lbl, fill="black", anchor="mm")
# HDMI + power on the right edge, SD slot at bottom
d.rectangle([BRD[2] - 20, 640, BRD[2] + 40, 760], fill=(190, 190, 195), outline="black", width=2)
d.text((BRD[2] + 50, 700), "HDMI", font=f_small, fill="black", anchor="lm")
d.rectangle([BRD[2] - 15, 930, BRD[2] + 30, 990], fill=(190, 190, 195), outline="black", width=2)
d.text((BRD[2] + 40, 960), "micro-USB power", font=f_small, fill="black", anchor="lm")
d.rectangle([560, BRD[3] - 10, 760, BRD[3] + 30], fill=(190, 190, 195), outline="black", width=2)
d.text((660, BRD[3] + 50), "SD card slot (underside)", font=f_small, fill="black", anchor="mm")
d.text((700, 700), "Raspberry Pi 3B+", font=f_h, fill=(50, 90, 60), anchor="mm")

# ---- header body
d.rounded_rectangle([X_EDGE - 32, Y_TOP - 30, X_INNER + 32, Y_TOP + 19 * DY + 30], radius=8, fill=(40, 40, 40))
d.text((X_EDGE, Y_TOP - 38), "edge", font=f_small, fill=(80, 80, 80), anchor="md")
d.text((X_INNER, Y_TOP - 38), "inner", font=f_small, fill=(80, 80, 80), anchor="md")
d.text((X_EDGE - 32, Y_TOP + 19 * DY + 42), "pin 1 / pin 2 end (SD-card end)", font=f_small, fill=(80, 80, 80), anchor="la")

# ---- wires
odd_k = even_k = 0
Y_BOT = BRD[3] + 90
for pi, hdr, mp, lbl, col, dashed in CONN:
    px, py = pin_xy(pi); tx, ty = jpin(hdr, mp)
    if pi % 2:  # odd / inner: run right across the board to the module
        xc = 1080 + odd_k * 18; odd_k += 1
        if hdr == 1:
            pts = [(px, py), (xc, py), (xc, ty), (tx, ty)]
        else:
            yb = BY1 + 40 + odd_k * 20; xr = BX1 + 50 + odd_k * 22
            pts = [(px, py), (xc, py), (xc, yb), (xr, yb), (xr, ty), (tx, ty)]
    else:       # even / edge: run left, under the board, then up to the module
        xc = 60 + even_k * 16; yb = Y_BOT + even_k * 20; even_k += 1
        xr = 1130 + even_k * 18
        pts = [(px, py), (xc, py), (xc, yb), (xr, yb), (xr, ty), (tx, ty)]
    line(pts, col, dashed=dashed)

# UART bracket
ux = X_EDGE - 95
d.line([(ux + 14, pin_xy(6)[1]), (ux, pin_xy(6)[1]), (ux, pin_xy(10)[1]), (ux + 14, pin_xy(10)[1])], fill=(150, 150, 150), width=3)
d.text((ux - 10, pin_xy(8)[1]), "UART debugger\n(already connected)", font=f_small, fill=(120, 120, 120), anchor="rm", align="right")

# ---- pins + labels
used = {c[0]: c for c in CONN}
for n in range(1, 41):
    x, y = pin_xy(n)
    col = used[n][4] if n in used else ((200, 200, 200) if n in UART else (255, 215, 0))
    d.ellipse([x - R, y - R, x + R, y + R], fill=col, outline="black", width=2)
    d.text((x, y), str(n), font=f_pin, fill="white" if n in used else "black", anchor="mm")
    if n in used:
        lbl, c = used[n][3], used[n][4]
        tw = d.textlength(lbl, font=f_lbl) + 20
        if n % 2:
            bx0 = X_INNER + R + 12
            d.rounded_rectangle([bx0, y - 15, bx0 + tw, y + 15], radius=6, fill=c)
            d.text((bx0 + 10, y), lbl, font=f_lbl, fill="white", anchor="lm")
        else:
            bx1 = X_EDGE - R - 12
            d.rounded_rectangle([bx1 - tw, y - 15, bx1, y + 15], radius=6, fill=c)
            d.text((bx1 - 10, y), lbl, font=f_lbl, fill="white", anchor="rm")
    else:
        if n % 2:
            d.text((X_INNER + R + 14, y), PIN_FUNC[n], font=f_small, fill=(90, 90, 90), anchor="lm")
        else:
            d.text((X_EDGE - R - 14, y), PIN_FUNC[n], font=f_small, fill=(110, 110, 110), anchor="rm")

# square pad marker on pin 1
x1, y1 = pin_xy(1)
d.rectangle([x1 - R - 4, y1 - R - 4, x1 + R + 4, y1 + R + 4], outline=(220, 30, 30), width=3)
d.text((x1 + R + 150, y1 + 28), "pin 1 has the SQUARE solder pad", font=f_small, fill=(220, 30, 30), anchor="lm")

# ---- module
d.rounded_rectangle([BX0, BY0, BX1, BY1], radius=12, fill=(20, 110, 60), outline="black", width=3)
d.rectangle([BX0 + 60, BY0 + 15, BX1 - 60, BY0 + 140], fill=(200, 200, 205), outline="black", width=2)
d.text(((BX0 + BX1) / 2, BY0 + 77), "RJ45", font=f_h, fill="black", anchor="mm")
d.text(((BX0 + BX1) / 2, BY0 + 170), "W5500 Lite", font=f_h, fill="white", anchor="mm")
d.text(((BX0 + BX1) / 2, BY0 + 198), "3.3 V ONLY", font=f_lbl, fill=(255, 220, 90), anchor="mm")
d.text((BX0 + 14, BY1 - 30), "J1", font=f_lbl, fill="white")
d.text((BX1 - 42, BY1 - 30), "J2", font=f_lbl, fill="white")
for name in J1:
    x, y = jpin(1, name)
    d.ellipse([x - 13, y - 13, x + 13, y + 13], fill=(240, 240, 240), outline="black", width=2)
    d.text((x + 24, y), name, font=f_lbl, fill="white", anchor="lm")
for name in J2:
    x, y = jpin(2, name)
    d.ellipse([x - 13, y - 13, x + 13, y + 13], fill=(240, 240, 240), outline="black", width=2)
    d.text((x - 24, y), name, font=f_lbl, fill="white", anchor="rm")
d.text(((BX0 + BX1) / 2, BY0 - 30), "W5500 Lite, RJ45 up (G is at the RJ45 end of each header)", font=f_small, fill="black", anchor="mm")

# ---- notes
nx, ny = 60, 1345
for i, t in enumerate([
    "Finding pin 17:",
    "  Start at pin 1 (bottom of the INNER column, square pad),",
    "  count 9 pins UP the inner column: 1, 3, 5, ... 17.",
    "  Pin 18 (GPIO24) is directly to its left on the edge column.",
    "Alternatively use pin 1 itself for 3V3.",
    "Only ONE module G pin needs GND. INT is optional.",
]):
    d.text((nx, ny + i * 27), t, font=f_lbl if i == 0 else f_small, fill="black")

img.save(OUT); print(OUT)
