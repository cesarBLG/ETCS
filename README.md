# ETCS
European Train Control System implementation
ETCS is a standard European safety system for railway interoperability between countries. This code tries to implement some of its features. Most DMI graphical functions work, as well as speed and distance monitoring.

The DMI is the graphical interface. The EVC is the computer the DMI connects to. For connecting to ETCS within games like Simrail you should only need the DMI. 

# Building
Please note that you can also get builds from the [release page on github](https://github.com/cesarBLG/ETCS/releases).

## Linux
1. Install dependencies:
    * Ubuntu: `sudo apt install git cmake`
    * Arch Linux: `sudo pacman -S git cmake`
2. Download the ETCS repository:
    ```bash
    git clone https://github.com/cesarBLG/ETCS
    cd ETCS
    ```

3. Compile:
   ```bash
   cmake -S . -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5
   cmake --build build -j$(nproc --all)
   ```