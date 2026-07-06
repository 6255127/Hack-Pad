# Hackpad

A custom 3x4 macro keyboard I built from scratch - schematic, PCB, 3D printed case, and firmware, all done by me as a portfolio/hackathon project. Built around an Arduino Nano RP2040 Connect, with a rotary encoder, OLED status screen, and per-key RGB backlighting.

## Features
- 3x4 key matrix (12 keys)
- rotary encoder for scroll/navigation
- 0.91" OLED showing current layer, last key pressed, and time (synced over wifi)
- SK6812 addressable LEDs under each key
- two layers - numpad mode and a shortcuts mode (copy/paste/undo/save/etc)
- wired usb keyboard (bluetooth planned later, board already supports it)

---

## 1. Schematic

Designed in KiCad. Went through a few pin conflicts along the way (accidentally put the LED data line on the I2C bus at one point) before landing on the final wiring.

![schematic here](./images/schematic.png)

---

## 2. PCB routing

Routed as a 2-layer board - rows on the bottom copper, columns on top, vias where they cross. Ran into a mixed-up switch numbering issue during routing that caused some real weird ratsnest jumps, fixed by re-checking every switch against the row/column table by hand.

![pcb routing here](./images/pcb_routing.png)

---

## 3. 3D case

Modeled in Fusion 360 / Onshape, sandwich-mount style. Bottom half holds the PCB and MCU, top half is the switch plate with cutouts for keycaps, the encoder, and the OLED window. Top plate designed to be printed in clear/frosted material so the LEDs glow through it.

![case bottom here](./images/case_bottom.png)
![case top here](./images/case_top.png)

---

## 4. Firmware

Written in Arduino for the Nano RP2040 Connect. Handles matrix scanning, encoder input, LED feedback, OLED display, and layer switching (numpad vs shortcuts) via the encoder click.

![firmware demo here](./images/firmware_demo.png)

```
firmware/hackpad_firmware_v4.ino
```

---

## To-do
- bluetooth (board already has the chip, just needs the firmware side)
- battery + charging circuit
- silkscreen art on the back
