# DropToPrint



**DropToPrint** is a compact, Wi-Fi-enabled hardware module designed for 3D printers like the Ender 3 V3 SE. It allows you to:

- **Upload G-code wirelessly** to a real microSD card (used directly by the printer)
- **Control your printer remotely** by sending commands over the WIFI
- Do all of this without modifying the printer firmware or needing a Raspberry Pi

Built using an **ESP32**, DropToPrint creates a seamless bridge between your computer or phone and your printer.

---

## 🔧 Features

### 📂 Wireless G-code File Transfer
- Send `.gcode` files via Wi-Fi from any device
- Files are written directly to the microSD card, readable by your printer

### 🖥️ Remote Printer Control
- Send G-code commands (e.g. `M105`, `G28`, `M112`) via a web UI
- Interface with the printer’s serial port over UART

### 🚫 No Firmware Modifications Required
- Works out of the box with stock firmware
- No need for OctoPrint, Klipper, or SD card swapping

---

## 🧠 Use Case Example

1. Slice your model on PC or phone  
2. Upload the G-code over Wi-Fi to the SD card  
3. Start the print using the **web interface** — sends `M23` + `M24` via serial  
4. Monitor and send commands (e.g. pause, home, stop)

---

## 📦 Coming Soon
- Real-time serial log display
- Web G-code console 
- Print queue manager
- Secure authentication (optional)

---

## 🛠️ Hardware Requirements

- ESP32
- MicroSD slot (FAT32)
- Connection to printer serial TX/RX (e.g., Creality mainboard)

---

## 💡 Why DropToPrint?

| Feature                     | DropToPrint | OctoPrint on Pi | SD Card Only |
|----------------------------|-------------|------------------|--------------|
| Wireless G-code Upload     | ✅          | ✅               | ❌           |
| Remote Serial Control      | ✅          | ✅               | ❌           |
| Uses Real SD Card          | ✅          | ❌               | ✅           |
| No Firmware Mods Needed    | ✅          | ❌ (sometimes)    | ✅           |
| Compact, Low-Power         | ✅          | ❌               | ✅           |

---

**DropToPrint is lightweight, open, and made for practical 3D printer users.**  
Feedback, issues, and PRs are always welcome!
