# LED Snake — Arduino Uno R4 WiFi

A classic Snake game for **Arduino Uno R4 WiFi** using the onboard LED matrix. Control is via an **HW-504** analog joystick; the score is shown on a **3461BS-1** 4-digit 7-segment display.

## Hardware

| Component | Description |
|-----------|-------------|
| **Board** | Arduino Uno R4 WiFi (with built-in 12×8 LED matrix) |
| **Joystick** | HW-504 (analog X/Y + push button) |
| **Score display** | 3461BS-1 (4-digit common-anode 7-segment) |

## Wiring

### Important: LED display pins

**Use analog pins A2–A5 for the 3461BS-1 digit (common) pins**, not A1–A4 as in some standalone display diagrams. Pins **A0** and **A1** are used by the HW-504 joystick (VRx, VRy).

### Resistors

- Use **220 Ω** resistors in series with each of the **eight segment lines** (a–g and dp) of the 3461BS-1 to limit LED current.

### Combined wiring diagram

```
                    ARDUINO UNO R4 WIFI
                    ┌─────────────────────────────────────────┐
                    │  POWER          DIGITAL         ANALOG  │
                    │  5V ────┬───   2 ◄─── SW        A0 ◄── VRx (joystick)
                    │  GND ───┼───   5 ───► seg a      A1 ◄── VRy (joystick)
                    │         │      6 ───► seg b      A2 ───► digit 1 (3461BS-1)
                    │  (shared│      7 ───► seg c      A3 ───► digit 2
                    │   GND/  │      8 ───► seg d      A4 ───► digit 3
                    │   5V)   │      9 ───► seg e      A5 ───► digit 4
                    │         │     10 ───► seg f                   │
                    │         │     11 ───► seg g                   │
                    │         │     12 ───► seg dp                  │
                    └─────────┼─────────────────────────────────────┘
                              │
         ┌────────────────────┼────────────────────┐
         │                    │                    │
         ▼                    ▼                    ▼
    ┌─────────┐          ┌─────────┐          ┌──────────────────┐
    │  HW-504 │          │  GND/   │          │ 3461BS-1         │
    │ Joystick│          │  5V     │          │ 4-digit 7-seg    │
    │         │          │ (bread- │          │                  │
    │ VRx ────┼──────────┼─► A0    │          │ Each segment     │
    │ VRy ────┼──────────┼─► A1    │          │ (a,b,c,d,e,f,g,  │
    │ SW  ────┼──────────┼─► D2    │          │  dp) ← 220Ω ←   │
    │ +5V ────┼──────────┼─► 5V    │          │ D5,D6..D12       │
    │ GND ────┼──────────┼─► GND   │          │                  │
    └─────────┘          └─────────┘          │ Digit 1..4 ──────┼─► A2,A3,A4,A5
                                              └──────────────────┘
```

**Summary table**

| HW-504 pin | Arduino pin | 3461BS-1 | Arduino pin |
|------------|-------------|----------|-------------|
| GND        | GND         | —        | —           |
| +5V        | 5V          | —        | —           |
| VRx        | **A0**      | —        | —           |
| VRy        | **A1**      | —        | —           |
| SW         | **D2**      | —        | —           |
| —          | —           | Segment a | D5 (via 220Ω) |
| —          | —           | Segment b | D6 (via 220Ω) |
| —          | —           | Segment c | D7 (via 220Ω) |
| —          | —           | Segment d | D8 (via 220Ω) |
| —          | —           | Segment e | D9 (via 220Ω) |
| —          | —           | Segment f | D10 (via 220Ω) |
| —          | —           | Segment g | D11 (via 220Ω) |
| —          | —           | Segment dp | D12 (via 220Ω) |
| —          | —           | **Digit 1** common | **A2** |
| —          | —           | **Digit 2** common | **A3** |
| —          | —           | **Digit 3** common | **A4** |
| —          | —           | **Digit 4** common | **A5** |

Individual wiring references (note: use **A2–A5** for the display, not A0–A3):

- [HW-504 connection](images/hw-504.png)
- [3461BS-1 connection](images/3461bs-1.png) — in this project, connect digit commons to **A2, A3, A4, A5** instead of A0–A3.

## Software

- **Sketch:** `sketches/snake.c` (use with Arduino IDE or similar; ensure the file is part of a `.ino` project or adapted to your build system).
- **Library:** `Arduino_LED_Matrix` (included with the Uno R4 WiFi board package).

## Gameplay

- **Move:** Joystick (up/down/left/right).
- **Pause:** Joystick button (SW).
- **Score:** Shown on the 3461BS-1 (0–9999). Game speed increases as the score goes up.

## License

Use and modify as you like; no warranty.
