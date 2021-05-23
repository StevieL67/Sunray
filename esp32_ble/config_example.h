/*   ESP32 firmware providing:
     1. BLE UART bridge (GATT server UART)
     2. WiFi UART bridge (HTTP server UART)


Steps to install ESP32 for Arduino IDE:
     1. Arduino IDE: File->Preferences:  Add to board manager URLs: ",https://dl.espressif.com/dl/package_esp32_index.json"
     2. Choose "Tools->Board->Boards Manager"
     3. Add board "esp32" (IMPORTANT!!! choose v1.0.4, latest v1.0.6 does not seem to connect to WiFi access point! )
     4. Choose Board "ESP32 Dev Module"  (if upload does not work: PRESS EN+BOOT, release EN  on your ESP32)
     5. Choose Partition Scheme "Minimal SPIFFS"  (otherwise you may get 'memory space errors' in the Arduino IDE)
    (also see: https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md )
     6. Choose Port (Windows NOTE: if the port is not shown you may have to install drivers: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)

wiring (also see wiring image in Github folder):
  ESP32 Rx2 (GPIO16) ---  Ardumower PCB1.3 Bluetooth conn RX   (3.3v level)
  ESP32 Tx2 (GPIO17) ---  Ardumower PCB1.3 Bluetooth conn TX   (3.3v level)
  ESP32 GND          ---  Ardumower PCB1.3 Bluetooth conn GND
  ESP32 5V  (Vin)    ---  Ardumower PCB1.3 Bluetooth conn 5V


Note: ESP32 D1 Mini (china cheapo) can work as a comm/BLE chip.
Data is reported and mower can be steered using app.
Things to remember are:
- need to wait few secs after mower reboots before connecting (CRC issues)
- need to close/reopen app if the phone disconnects (CRC issues)


serial protocol:

---COMMANDS---                                                ---ANSWER---
                              AT\r\n                          OK\r\n
request version               AT+VERSION\r\n                  +VERSION=ESP32 firmware V0.1,Bluetooth V4.0 LE\r\n
change BLE name               AT+NAMEname\r\n                 +NAME=name\r\n
reset module                  AT+RESET\r\n                    +RESET\r\n
send BLE test packet          AT+TEST\r\n                     +TEST\r\n
connect to wifi               AT+WIFImode,ssid,pass\r\n       +WIFI=mode,ssid,pass\r\n


1. Rename file 'config_example.h' into 'config.h'

2. Configure the options below and finally compile and upload this project.

*/

// ---------- configuration ----------------------------------
#define VERSION "ESP32 firmware V0.2.2,Bluetooth V4.0 LE"
#define NAME "Ardumower"
#define BLE_MTU 20   // max. transfer bytes per BLE frame

#define BLE_MIN_INTERVAL 1    // connection parameters (tuned for high speed/high power consumption - see: https://support.ambiq.com/hc/en-us/articles/115002907792-Managing-BLE-Connection-Parameters)
#define BLE_MAX_INTERVAL 10
#define BLE_LATENCY      0
#define BLE_TIMEOUT      20

String ssid = "";  // WiFi SSID      (leave empty to not use WiFi)
String pass = "";  // WiFi password  (leave empty to not use WiFi)

#define WIFI_TIMEOUT_FIRST_RESPONSE  500   // fast response times, for more reliable choose: 800
#define WIFI_TIMEOUT_RESPONSE        100    // fast response times, for more reliable choose: 400

// -----------------------------------------------------------

#define pinGpioRx   16    // UART2 / GPIO16 / IO16
#define pinGpioTx   17    // UART2 / GPIO17 / IO17

//#define pinGpioRx   9   // UART1 / GPIO9  / SD2
//#define pinGpioTx   10  // UART1 / GPIO10 / SD3

//#define pinGpioRx   3   // UART0 / GPIO3  / RXD0
//#define pinGpioTx   1   // UART0 / GPIO1  / TXD0

#define pinLED   2

#define CONSOLE Serial  // where to send/receive console messages for debugging etc.
#define UART Serial2    // where to send/receive UART data
