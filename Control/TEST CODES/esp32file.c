#include <WiFi.h>
#include <stdio.h>
#include <SoftwareSerial.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<stdlib.h>
#include<unistd.h>

#define PORTNUM 18000
//pin def for UART for energy
#define RXD2 16
#define TXD2 17
//pin def for UART for drive
#define TXD1 10
#define RXD1 9
//pin def for UART for vision
#define TXD3 23
#define RXD3 18
//pin def for SPI for Vision, try using standard pin allignment
/*
#define VSPI_MISO   2
#define VSPI_MOSI   4
#define VSPI_SCLK   0
#define VSPI_SS     33
*/

SoftwareSerial Serial3;

//internet settings to connect to server
int sd, answer;
char *msg[30];//change 30 into whichever max stat
char buf[256];
char instruction[3];
char roverstatus[128];
struct sockaddr_in sin;


const char* ssid = "Sindhu"; //change this to your wifi on integration side
const char* pw = "Butterchicken"; //likewise
//const char* ssid2 = "waplocal";
//const char* pw2 = "localwap";

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

void serversetup(); //taken from andy
{
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

void setup() { //setups ESP32 controller connections: Wifi, SPI and UART
  Serial.begin(115200);
  while(!Serial)
  {}
  Serial.println("starting up");
  initWiFi();
  Serial.println("wifi connected");
  serversetup();
  Serial.println("connected to server")
  Serial.println("setup finished");
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); //for energy, may need to change baud rate
  Serial1.begin(9600, SERIAL_8N1, RXD1, TXD1); //for drive, figure out arduino uno max baud rate
  Serial3.begin(9600, SWSERIAL_8N1, RXD3, TXD3); //For Uart FPGA connection
  //server connection
}

void emergencystop() //direct communication between Vision and Drive. Used to stop rover if a blockage is detected
{
  if (warn == 1)
  {
    estop = 1;
  }
}

void serverreceive();
{
  if (recv(sd,buf,sizeof(buf),0)==-1){
			perror("recv");
			exit(1);
		}

		printf("%s\n",buf);

		//copy out of this "instruction" variable to get the integer number that represents the instruction
		strcpy(instruction,buf);
    newdriveinstr = stoi(instruction);

}

void serversend();
{

		strcpy(roverstatus, "battery 10, blah blah");
		sprintf(msg,"%s",roverstatus);

		if (send(sd,msg,strlen(msg)+1,0)==-1){
			perror("send");
			exit(1);
		}

		if(recv(sd,buf,sizeof(buf),0)==-1){
			perror("recv result");
			exit(1);
		}

		printf("%s\n",buf);
}

void batterycheck() //Energy - ESP32 - Command. Used to update GUI of battery level of the rover
{
  batterylevel = Serial1.read();
  Serial.print("Rover Battery level = ");
  Serial.println(batterylevel, DEC);
  sendbatteryinfo(batterylevel);
}

void sendbatteryinfo (uint8_t n) //sends info to server of above function
{
  //implement send to server. Server will have to figure out how to display raw data
}
void poweroff() //turns off ESP32
{
  poweron = 1; //implement on/off from HTTPS server
}

void receivedrivedist() //get distance travelled from arduino
{
  dist = Serial1.read(); //do I require to decode this info? also if 8bit count am I limited to 2^8?
  Serial.print("Distance Travelled = ");
  Serial.println(dist, DEC);
  senddistinfo(dist);

}

void senddistinfo(uint8_t n) //sends distance to GUI
{

}

void receivenewdriveinstr() //gets new instruction from the GUI
{
  if (newdriveinstr != 0)
  {
    driveinstr = newdriveinstr;
  }
  else
    {
      driveinstr = 0;
    }

}

void httpget() //replace with new code
{


      http.begin(serverName);
      httpCode = http.GET();
      if (httpCode > 0) { //Check for the returning code

        //newdriveinstrimm = http.getString();
        //newdriveinstr = stoi(newdriveinstrimm);
        //Serial.println(httpCode);
        //Serial.println(newdriveinstr);
        payload = http.getString();

}else {

      Serial.println("Error with HTTP request, no instruction received.");
}
}


void senddriveinstr(uint8_t n) //where n comes from receivenewdriveinstr, sends to Drive Arduino
{
  Serial1.write(n); //is this required because base value of int is 16 bits long
}

void receivevision()
{

visioncalc = Serial3.read();
}

void calculatevision()
{}


//main execute loop
void loop())
{

  Serial.println("ESP32 READY");
//rover function, will get stuck in while loop
  while (poweron)
  {

    batterycheck();
    receivedrivedist();
    //put vision instruction where
    recievevision();
    calculatevision();
    serverreceive();
    senddriveinstr(driveinstr);
    delay(100);




    //emergencystop(warn);
    //httpget();
    //Serial.println(payload);
    //receivenewdriveinstr();
    //senddriveinstr(driveinstr);
  //  receivedrivedist();

  //  poweroff();
    //delay(2500);

  }
  Serial.println("rover turned off");


}
