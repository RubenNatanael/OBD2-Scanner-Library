# OBD2-Scanner-Library

A lightweight and flexible **multi-platform OBD-II communication library**.  
This project provides a single codebase that can be compiled for:

- **Arduino** (compatible with popular CAN shields)
- **Raspberry Pi Pico SDK**
- **Linux** (using [SocketCAN](https://www.kernel.org/doc/Documentation/networking/can.txt))

With this library, you can send OBD-II requests and parse ECU responses, making it easy to build custom diagnostic tools, loggers, or embedded applications.

**Attention, this project is still in development(not finished)** 
---

## ✨ Features
- Multi-platform support (Arduino, Pico SDK, Linux).
- CAN bus communication via SocketCAN or hardware CAN controllers.
- Easy interface for sending OBD requests (`mode`, `PID`).
- Ready for integration into diagnostic or telematics projects.
- Lightweight, no external dependencies beyond platform requirements.

---

## 🛠 Building & Usage

### Arduino  
🚧 Work in progress  

### Raspberry Pi Pico (W)  
🚧 Work in progress  

### Linux  

On Linux you have two options to build and run the examples:  

---

#### 🔹 Console Example  

The **console example** shows how to use the library in a simple C++ program.  
You can modify `example/main.cpp` to call your desired methods (for example `getPid`, `clearDTC`, `getDTC`, etc.).  

To build and run(Optional: Start the Car simulator before launching the application):  

```bash
make simple
./carSimulator.sh & #optional
./main
```

Or simply run the helper script that starts everything for you:

```bash
./run_example.sh
```

#### 🔹 Qt Example

The **Qt example** demonstrates how to build a graphical interface on top of the library.
Make sure you have the necessary Qt development packages installed before building.

To build and run(Optional: Start the Car simulator before launching the application):

```bash
make qt
./carSimulator.sh & #optional
./qt_example
```

Or use the helper script that does it automatically:

```bash
./run_exampleQT.sh
```
