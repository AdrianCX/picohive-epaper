#!/usr/bin/env python3
# Generates docs/pictures/wiring.svg: cartoon wiring in pinout.xyz style, pin accurate
import sys

W, H = 1300, 500
S = 8.0                                  # px per mm
PITCH = 2.54 * S
PX, PY = 120, 60                         # Pico top-left
PW, PH = 21 * S, 51 * S
PILL_W, PILL_H, PILL_GAP = 84, 16, 8

LEFT = ["GP0", "GP1", "GND", "GP2", "GP3", "GP4", "GP5", "GND", "GP6", "GP7",
        "GP8", "GP9", "GND", "GP10", "GP11", "GP12", "GP13", "GND", "GP14", "GP15"]
RIGHT = ["VBUS", "VSYS", "GND", "3V3_EN", "3V3", "ADC_VREF", "GP28", "AGND", "GP27", "GP26",
         "RUN", "GP22", "GND", "GP21", "GP20", "GP19", "GP18", "GND", "GP17", "GP16"]
USED_LEFT = set()
USED_RIGHT = {1, 2, 4, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19}  # VSYS, GND, 3V3, GP27, GP26, GP22, GND, GP21-GP16

C_5V, C_GND = "#d7263d", "#222222"
C_3V3 = "#e36414"
C_SIG = {"RST": "#8e44ad", "BUSY": "#2e9e4f", "MOSI": "#1f77b4", "SCK": "#5fb3e8", "CS": "#e8890c", "DC": "#f2c14e"}
C_KEY = "#3a9a5b"

out = []
a = out.append


def py(i):
    return PY + (1.37 + i * 2.54) * S


def kind(n):
    if n in ("GND", "AGND"):
        return "#1a1a1a", "#ffffff"
    if n in ("VBUS", "VSYS"):
        return "#d7263d", "#ffffff"
    if n in ("3V3", "3V3_EN"):
        return "#e36414", "#ffffff"
    if n.startswith("GP"):
        return "#3a9a5b", "#ffffff"
    if n.startswith("OUT"):
        return "#b5651d", "#ffffff"
    if n in ("VCC",):
        return "#d7263d", "#ffffff"
    return "#6c757d", "#ffffff"


def text(x, y, s, size=12, color="#222", anchor="start", weight="normal"):
    a('<text x="%.1f" y="%.1f" font-size="%s" fill="%s" text-anchor="%s" font-weight="%s">%s</text>'
      % (x, y, size, color, anchor, weight, s))


def pill(edge_x, y, name, num, side, used):
    """Pin label sticking out of a board edge; returns the outer end x."""
    bg, fg = kind(name)
    x = edge_x - PILL_GAP - PILL_W if side < 0 else edge_x + PILL_GAP
    op = "1" if used else "0.35"
    a('<g opacity="%s">' % op)
    a('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="%s" stroke-width="2"/>'
      % (edge_x, y, x + (PILL_W if side < 0 else 0), y, bg))
    a('<rect x="%.1f" y="%.1f" width="%d" height="%d" rx="8" fill="%s"/>' % (x, y - PILL_H / 2, PILL_W, PILL_H, bg))
    if num is not None:
        nx = x + PILL_W - 9 if side < 0 else x + 9
        a('<circle cx="%.1f" cy="%.1f" r="7" fill="#ffffff" opacity="0.9"/>' % (nx, y))
        text(nx, y + 3.5, str(num), 8.5, bg, "middle", "bold")
        tx = x + (PILL_W - 20) / 2 if side < 0 else x + 20 + (PILL_W - 20) / 2
    else:
        tx = x + PILL_W / 2
    text(tx, y + 4.5, name, 11, fg, "middle", "bold" if used else "normal")
    a('</g>')
    return x if side < 0 else x + PILL_W


def wire(pts, color, w=3.5, over=False):
    d = " ".join("%.1f,%.1f" % p for p in pts)
    if over:                             # halo so it reads as passing over other wires
        a('<polyline fill="none" stroke="#fbfaf6" stroke-width="%.1f" stroke-linejoin="round" points="%s"/>' % (w + 8, d))
    a('<polyline fill="none" stroke="#00000033" stroke-width="%.1f" stroke-linejoin="round" stroke-linecap="round" points="%s"/>' % (w + 2, d))
    a('<polyline fill="none" stroke="%s" stroke-width="%.1f" stroke-linejoin="round" stroke-linecap="round" points="%s"/>' % (color, w, d))


def dot(x, y, color):
    a('<circle cx="%.1f" cy="%.1f" r="5" fill="%s" stroke="#ffffff" stroke-width="1.5"/>' % (x, y, color))


X0 = 0
a('<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="%d" viewBox="%d 0 %d %d" '
  'font-family="Helvetica, Arial, sans-serif">' % (W - X0, H, X0, W - X0, H))
a('<rect x="%d" width="%d" height="%d" rx="14" fill="#fbfaf6"/>' % (X0, W - X0, H))
# easter egg: faint blurred PicoHive in the empty middle
a('<defs><filter id="blur" x="-20%" y="-50%" width="140%" height="200%"><feGaussianBlur stdDeviation="1.4"/></filter></defs>')
a('<g filter="url(#blur)"><path transform="translate(800 128) scale(1.1)" d="M16 2l12 7v14l-12 7-12-7V9z" fill="#F2A516"/>'
  '<path transform="translate(800 128) scale(1.1)" d="M16 9l6 3.5v7L16 23l-6-3.5v-7z" fill="#2A1F0E"/>'
  '<text x="842" y="155" font-size="26" font-weight="bold" fill="#b8860b">PicoHive</text></g>')

# ---------------------------------------------------------------- Pico W (USB up, face up)
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" rx="10" fill="#2f8f4e" stroke="#1d5e32" stroke-width="3"/>' % (PX, PY, PW, PH))
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" rx="4" fill="#d5d8dc" stroke="#7f8c8d" stroke-width="2"/>'
  % (PX + PW / 2 - 4 * S, PY - 1.6 * S, 8 * S, 5.6 * S))
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" rx="2" fill="#95a5a6"/>' % (PX + PW / 2 - 3 * S, PY - 0.4 * S, 6 * S, 1.4 * S))
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" rx="4" fill="#f4f4f4" stroke="#b0b0b0" stroke-width="1.5"/>' % (PX + 4.6 * S, PY + 8.5 * S, 4 * S, 4.6 * S))
a('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="#ffffff" stroke="#c8c8c8"/>' % (PX + 6.6 * S, PY + 10.8 * S, 1.2 * S))
text(PX + 6.6 * S, PY + 15.4 * S, "BOOTSEL", 7.5, "#e8f5e9", "middle", "bold")
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" rx="6" fill="#2b2b2b" stroke="#111" stroke-width="2"/>' % (PX + 6.5 * S, PY + 18 * S, 8 * S, 8 * S))
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" rx="6" fill="#dfe4e8" stroke="#9aa4ab" stroke-width="2"/>' % (PX + 5.5 * S, PY + 31 * S, 10 * S, 9.5 * S))
text(PX + 10.5 * S, PY + 36.4 * S, "WiFi", 11, "#6b7880", "middle", "bold")
a('<path d="M%.1f %.1f q%.1f -%.1f %.1f 0 t%.1f 0 t%.1f 0 t%.1f 0" fill="none" stroke="#e9c46a" stroke-width="3" stroke-linecap="round"/>'
  % (PX + 4.5 * S, PY + 46.5 * S, 1.5 * S, 2.5 * S, 3 * S, 3 * S, 3 * S, 3 * S))
# PicoHive hex as a silkscreen logo, where the LED would be
a('<g transform="translate(%.1f %.1f) scale(0.75)"><path d="M16 2l12 7v14l-12 7-12-7V9z" fill="#F2A516"/>'
  '<path d="M16 9l6 3.5v7L16 23l-6-3.5v-7z" fill="#2A1F0E"/></g>' % (PX + 14 * S - 12, PY + 6.5 * S - 12))
text(PX + PW / 2, PY + 29.3 * S, "Pico 2 W", 10, "#ffffff", "middle", "bold")

def pad(x, y):
    """Plated through hole."""
    a('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="#e9c46a" stroke="#a07c1c" stroke-width="1.5"/>' % (x, y, 0.9 * S))
    a('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="#1b1b1b"/>' % (x, y, 0.45 * S))


# mounting holes
for mx, my in ((4.8, 2), (16.2, 2), (4.8, 49), (16.2, 49)):
    a('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="#fbfaf6" stroke="#c9a64a" stroke-width="2.5"/>' % (PX + mx * S, PY + my * S, 1.05 * S))

PL, PR = PX + 1.61 * S, PX + PW - 1.61 * S
LEND, REND = {}, {}
for i, n in enumerate(LEFT):
    pad(PL, py(i))
    LEND[i] = pill(PX, py(i), n, i + 1, -1, i in USED_LEFT)
for i, n in enumerate(RIGHT):
    pad(PR, py(i))
    REND[i] = pill(PX + PW, py(i), n, 40 - i, 1, i in USED_RIGHT)


# ---------------------------------------------------------------- Battery holder, 4x AA
y5, yg = py(1), py(2)
BX0, BY0, BW, BH = 1010, 36, 250, 96
wire([(REND[1], y5), (BX0, y5)], C_5V)
wire([(REND[2], yg), (BX0, yg)], C_GND)
a('<rect x="%d" y="%d" width="%d" height="%d" rx="10" fill="#26262b" stroke="#000" stroke-width="3"/>' % (BX0, BY0, BW, BH))
for k in range(4):
    cx, cy = BX0 + 22 + (k % 2) * 110, BY0 + 12 + (k // 2) * 40
    a('<rect x="%d" y="%d" width="98" height="32" rx="7" fill="#3ecf6a" stroke="#1d5e32" stroke-width="2"/>' % (cx, cy))
    a('<rect x="%d" y="%d" width="5" height="14" rx="2" fill="#c0c0c0"/>' % (cx + 98, cy + 9))
    text(cx + 49, cy + 21, "AA 1.2V", 11, "#ffffff", "middle", "bold")
text(BX0 + 10, y5 + 4, "+", 13, "#ff6b6b", "start", "bold")
text(BX0 + 10, yg + 18, "−", 13, "#ffffff", "start", "bold")
text(BX0 + BW / 2, BY0 + BH + 18, "4x AA NiMH holder", 12, "#222", "middle", "bold")
text(BX0 + BW / 2, BY0 + BH + 32, "4.8V into VSYS", 10, "#666", "middle")

# ---------------------------------------------------------------- Buttons (other leg to GND, internal pull-ups)
def button(cx, cy, label, sub):
    a('<rect x="%.1f" y="%.1f" width="40" height="34" rx="5" fill="#26262b" stroke="#000" stroke-width="2"/>' % (cx - 20, cy - 17))
    a('<circle cx="%.1f" cy="%.1f" r="9" fill="#d7263d"/>' % (cx, cy))
    a('<ellipse cx="%.1f" cy="%.1f" rx="4" ry="2.5" fill="#ffffff" opacity="0.45"/>' % (cx - 2, cy - 4))
    text(cx + 72, cy - 1, label, 12, "#222", "start", "bold")
    text(cx + 72, cy + 13, sub, 10, "#666", "start")


KX, GBUS = 510, 560
KEYS = [(8, 410, "KEY0 · GP27", "wake and refresh"),
        (9, 425, "KEY2 · GP26", "setup mode (hotspot)"),
        (11, 440, "KEY1 · GP22", "config mode (shows IP)")]
for n, (row, lane, label, sub) in enumerate(KEYS):
    ky = 178 + n * 52
    wire([(REND[row], py(row)), (lane, py(row)), (lane, ky), (KX - 20, ky)], C_KEY)
    wire([(KX + 20, ky), (GBUS, ky)], C_GND)
    if n < 2:
        dot(GBUS, ky, C_GND)
    button(KX, ky, label, sub)
wire([(REND[12], py(12)), (GBUS, py(12)), (GBUS, 178)], C_GND)

# ---------------------------------------------------------------- ePaper breakout board (XIAO)
BREAK = [(12, "3V3"), (13, "RST"), (14, "BUSY"), (15, "MOSI"), (16, "SCK"), (17, "GND"), (18, "CS"), (19, "DC")]
DX, DW = 900, 150
DY, DH = py(12) - 16, py(19) - py(12) + 32
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" rx="10" fill="#26262b" stroke="#000" stroke-width="3"/>' % (DX, DY, DW, DH))
for sx in (DX + 40, DX + 96):              # XIAO socket, unused
    a('<rect x="%.1f" y="%.1f" width="14" height="%.1f" rx="3" fill="#1a1a1e" stroke="#444" stroke-width="1.5"/>' % (sx, DY + 30, DH - 60))
text(DX + DW / 2 + 5, DY + 20, "ePaper breakout", 11, "#f0f0f0", "middle", "bold")
text(DX + DW / 2 + 5, DY + DH - 10, "for XIAO", 10, "#bbbbbb", "middle")
FX = DX + DW - 18                         # FPC connector on the right edge
a('<rect x="%.1f" y="%.1f" width="14" height="70" rx="3" fill="#f4f4f4" stroke="#999" stroke-width="1.5"/>' % (FX, DY + DH / 2 - 35))
DLEND = {}
for row, name in BREAK:
    pad(DX + 14, py(row))
    bg = {"3V3": C_3V3, "GND": C_GND}.get(name, "#3a9a5b")
    x = DX - PILL_GAP - PILL_W
    a('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="%s" stroke-width="2"/>' % (DX, py(row), x + PILL_W, py(row), bg))
    a('<rect x="%.1f" y="%.1f" width="%d" height="%d" rx="8" fill="%s"/>' % (x, py(row) - PILL_H / 2, PILL_W, PILL_H, bg))
    text(x + PILL_W / 2, py(row) + 4.5, name, 11, "#ffffff", "middle", "bold")
    DLEND[row] = x
wire([(REND[4], py(4)), (770, py(4)), (770, py(12)), (DLEND[12], py(12))], C_3V3)
for row, name in BREAK[1:]:
    wire([(REND[row], py(row)), (DLEND[row], py(row))], C_GND if name == "GND" else C_SIG[name])

# ---------------------------------------------------------------- E-ink panel + ribbon
# Panel turned 90 degrees so its ribbon edge faces the breakout: a short straight ribbon
EW, EH = 170, 135                         # panel drawn landscape, then rotated
mid = DY + DH / 2
ECX, ECY = FX + 10 + 88 + EH / 2, mid      # centre of the rotated panel
EX, EY = ECX - EW / 2, ECY - EH / 2
a('<path d="M%.1f %.1f L%.1f %.1f L%.1f %.1f L%.1f %.1f Z" fill="#e8a33d" stroke="#a86a10" stroke-width="1.5"/>'
  % (FX + 10, mid - 26, ECX - EH / 2 + 2, mid - 20, ECX - EH / 2 + 2, mid + 20, FX + 10, mid + 26))
a('<g transform="rotate(90 %.1f %.1f)">' % (ECX, ECY))
a('<rect x="%.1f" y="%.1f" width="%d" height="%d" rx="6" fill="#f2f2ee" stroke="#8a8a8a" stroke-width="3"/>' % (EX, EY, EW, EH))
a('<rect x="%.1f" y="%.1f" width="62" height="%d" rx="2" fill="#c9b9a8"/>' % (EX + 12, EY + 12, EH - 24))
for k in range(6):
    a('<rect x="%.1f" y="%.1f" width="%d" height="4" rx="2" fill="%s"/>' % (EX + 84, EY + 16 + k * 18, 60 - (k % 3) * 12, "#d7263d" if k % 3 == 0 else "#555"))
a('</g>')
text(ECX, ECY - EW / 2 - 22, "E-ink panel", 12, "#222", "middle", "bold")
text(ECX, ECY - EW / 2 - 8, "5.83\" B/W/R or 5.65\" 7 colour", 10, "#666", "middle")

a('</svg>')
open(sys.argv[1], "w").write("\n".join(out))
