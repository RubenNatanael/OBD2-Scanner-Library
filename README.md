# OBD2-Scanner-Library

A lightweight and flexible **multi-platform OBD-II communication library**.  
This project provides a single codebase that can be compiled for:

- **Arduino** (compatible with popular CAN shields)
- **Raspberry Pi Pico SDK**
- **Linux** (using [SocketCAN](https://www.kernel.org/doc/Documentation/networking/can.txt))
                   [ELM327] (https://cdn.sparkfun.com/assets/learn_tutorials/8/3/ELM327DS.pdf)

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

#### 🔹 Console Example(Only for SocketCAN)  

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
#### 🔹 ELM327 
 - Now the library also supports real ELM327 adapters over USB or Bluetooth. You can connect your ELM327 and send/receive CAN frames directly.
 - This allows you to test both with the car simulator (/dev/pts/X) or a real car via ELM327.
 - In the example file, you can see how to create an ELM327 transport layer instead of a socket one. (Unfortunately, the example currently works only with a socket CAN, but using the library does not change at all.)
 
