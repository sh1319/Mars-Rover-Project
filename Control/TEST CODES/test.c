#include <WiFi.h>
#include "SPI.h"
#include <stdio.h>
#include <SoftwareSerial.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/un.h>
#include<stdlib.h>
#include<unistd.h>

//pin def for UART for drive
#define TXD1 10
#define RXD1 9

void setup() { //setups ESP32 controller connections: Wifi, SPI and UART
  Serial.begin(115200);
  while(!Serial)
  {}
  Serial.println("starting up");
  //initWiFi();
//  Serial.println("wifi connected");
//  serversetup();
  //Serial.println("connected to server")
  //Serial.println("setup finished");
  Serial2.begin(9600, SERIAL_8N1, RXD1, TXD1); //for energy, may need to change baud rate
  //Serial1.begin(9600, SERIAL_8N1, RXD1, TXD1); //for drive, figure out arduino uno max baud rate
  //Serial3.begin(9600, SERIAL_8N1, RXD3, TXD3); //For Uart FPGA connection
  //server connection
  }

  void loop()
  {
  Serial.println("trial");
  Serial2.write("input");
  }
