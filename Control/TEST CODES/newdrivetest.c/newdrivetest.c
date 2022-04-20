#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include <netinet/in.h>
#include<sys/un.h>
#include<stdlib.h>
#include<unistd.h>
#include<WiFi.h>
#define PORTNUM 18000

#define TXD1 26
#define RXD1 27

uint8_t dist;

void setup()
{
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RXD1, TXD1);
  while (!Serial1)
  {}

}

void loop()
{
  if(Serial1.available()>0)
  {
    dist = Serial1.read();
  }
  Serial.println(dist);
  delay(100);
}
