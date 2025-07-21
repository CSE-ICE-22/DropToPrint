# DropToPrint

**DropToPrint** is a compact, Wi-Fi-enabled hardware module designed for 3D printers like the Ender 3 V3 SE. It allows you to:

- **Upload G-code wirelessly** to a real microSD card (used directly by the printer)
- **Control your printer remotely** by sending commands over the WIFI
- Do all of this without modifying the printer firmware or needing a Raspberry Pi

Built using an **ESP32**, DropToPrint creates a seamless bridge between your computer or phone and your printer.

# DropToPrint Project Timeline (ESP32) — 10 Weeks

### Weeks 1-2: Planning, Design & Hardware Setup
- Define feature scope and finalize ESP32 hardware selection  
- Design system architecture (Wi-Fi → SD card → UART)  
- Assemble prototype hardware: ESP32 + microSD + UART  
- Test basic microSD read/write and UART communication

---

### Weeks 3-5: Core Firmware Development
- Implement wireless G-code upload (Wi-Fi → microSD)  
- Develop UART serial command interface for printer control  
- Build basic web UI or REST API for uploading and sending commands  
- Initial testing of upload and command functionalities

---

### Weeks 6-7: Integration & Testing
- End-to-end workflow testing: slicing → uploading → printing  
- Validate file integrity and UART command reliability  
- Debug and improve stability

---

### Weeks 8-9: Advanced Features & Documentation
- Add real-time serial log display on web UI  
- Develop web G-code console and temperature monitoring (basic)  
- Write user guide, README, and setup instructions

---

### Week 10: Beta Testing & Final Touches
- Release beta version for feedback  
- Collect and address bugs or improvement requests  
- Prepare for final release and package hardware/firmware

---

# Summary Timeline (10 Weeks)

| Phase                      | Duration   | Timeline          |
|----------------------------|------------|-------------------|
| Planning & Hardware Setup  | 2 weeks    | Weeks 1-2         |
| Core Firmware Development  | 3 weeks    | Weeks 3-5         |
| Integration & Testing      | 2 weeks    | Weeks 6-7         |
| Advanced Features & Docs   | 2 weeks    | Weeks 8-9         |
| Beta Testing & Finalizing  | 1 week     | Week 10           |
