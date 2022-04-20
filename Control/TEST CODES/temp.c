#include <WiFi.h>
#include <stdio.h>
#include <SoftwareSerial.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/un.h>
#include<stdlib.h>
#include<unistd.h>
#include<HardwareSerial.h>


#define PORTNUM 18000
//pin def for UART for energy
#define RXD2 16 //8
#define TXD2 17 //9
//pin def for UART for drive
#define TXD1 18 //9
#define RXD1 19 //10
//pin def for UART for vision
#define TXD3 15 //14
#define RXD3 2 //15

SoftwareSerial Serial3;

//internet settings to connect to server
int sd, answer;
char buf[256];
char msg[30];//change 30 into whichever max stat
char instruction[3];
char roverstatus[128];



const char* ssid = "Sindhu"; //change this to your wifi on integration side
const char* pw = "Butterchicken"; //likewise

unsigned long previousMillis = 0;
unsigned long interval = 30000;

//Setting Static IP address, may need to change on integration side
const IPAddress local_IP(192, 168, 1, 15);
//Setting Gateway IP address, may need to change on integration side
IPAddress gateway(192, 168, 1, 1);
//not required, remove if going over mem
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);



// rover parameters:
//uint8_t for 8bit wide
uint8_t batterylevel;
uint8_t dist;
uint8_t driveinstr;
uint8_t newdriveinstr; //temp for new incoming drive instr
uint8_t poweron = 1; //when 0, while loop breaks and control shuts down
uint8_t estop = 0; //when 1, Rover stops moving altogether in order to avoid accident. Direct Communication from Vision to Drive
uint8_t warn = 0; //decided by vision for above.
String newdriveinstrimm; //intermediate via http communication
int visionparam;


//fuction calls for rover functionality:

void initWiFi() { //initializes wifi connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pw);
  Serial.print("Connecting to WiFi:");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("");
  Serial.print("WiFi connected.");
  Serial.println("IP address: ");
  Serial.print(WiFi.localIP());
}


void setup() { //setups ESP32 controller connections: Wifi, SPI and UART
  Serial.begin(115200);
  while(!Serial)
  {}
  Serial.println("starting up");
  initWiFi();
  Serial.println("wifi connected");
  serverconnect();
  Serial.println("connected to server");
  Serial.println("setup finished");
  Serial2.begin(9600, SERIAL_8N1, RXD1, TXD1); //for energy, may need to change baud rate
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2); //for drive, figure out arduino uno max baud rate
  Serial3.begin(9600, SWSERIAL_8N1, RXD3, TXD3); //For Uart FPGA connection

}

void emergencystop() //direct communication between Vision and Drive. Used to stop rover if a blockage is detected
{
  if (warn == 1)
  {
    estop = 1;
  }
}

void receivevision()
{
if (Serial3.available()>0)
  {
    int visionparam = Serial3.read();
  }
}

void calculatevision()
{
  //omitted after Serial fail
}

//server connection and transfers:
void serverconnect(){


  struct sockaddr_in sin;

  if ((sd=socket(AF_INET,SOCK_STREAM,0))==-1){
    perror("socket");
    exit(1);
  }
  memset ((char*)&sin,'\0',sizeof(sin));
  sin.sin_family=AF_INET;
  sin.sin_port=htons(PORTNUM);
  sin.sin_addr.s_addr=inet_addr("104.45.192.134");

  if (connect(sd,(struct sockaddr *)&sin,sizeof(sin))){
    perror("connect");
    exit(1);
  }
}

 void serverhandshake()
 {
    if (read(sd,buf,sizeof(buf))==-1){
      perror("recv");
      exit(1);
    }

    Serial.println(buf);

    //copy out of this "instruction" variable to get the integer number that represents the instruction
    strcpy(instruction,buf);

    //msg is status of rover

    //roverstatus is the string of the status of the current rover, assign as shown below:
    char roverstatus[128];
    strcpy(roverstatus, "battery 10, blah blah");
    sprintf(msg,"%s",roverstatus);

   if (write(sd,msg,strlen(msg)+1)==-1){
      perror("send");
      exit(1);
    }

   if(read(sd,buf,sizeof(buf))==-1){
      perror("recv result");
      exit(1);
    }

    Serial.println(buf);


  close(sd);
}


void batterycheck() //Energy - ESP32 - Command. Used to update GUI of battery level of the rover
{
  if (Serial1.available > 0)
  {
  batterylevel = Serial1.read();
  Serial.print("Rover Battery level = ");
  Serial.println(batterylevel, DEC);
}
else{
  Serial.println("no new battery info receieved");
}
}

void poweroff() //turns off ESP32
{
  poweron = 1; //implement on/off from HTTPS server
}

void receivedrivedist() //get distance travelled from arduino
{
  if (Serial1.available > 0){
  dist = Serial1.read(); //do I require to decode this info? also if 8bit count am I limited to 2^8?
  Serial.print("Distance Travelled = ");
  Serial.println(dist, DEC);
}
else{
  Serial.println("no new drive distance received");
}
}


void senddriveinstr() //where n comes from receivenewdriveinstr, sends to Drive Arduino
{
  if(Serial1.available() > 0)
  {
  Serial1.write(driveinstr); //is this required because base value of int is 16 bits long
  }
  else
  {
  Serial.println("failed to send instruction");
  }
}

//main execute loop
void loop()
{


//rover function, will get stuck in while loop
  while (poweron)
  {
    receivedrivedist();
    //receivevision();
    //calculatevision();
    senddriveinstr();
    serverhandshake();
  }
  Serial.println("rover turned off");


}
