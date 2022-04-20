#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/un.h>
#include<stdlib.h>
#include<unistd.h>
#include <WiFi.h>
#define PORTNUM 18000


int sd, answer;
char buf[256];
char msg[30];//change 30 into whichever max stat
char instruction[3];

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



void setup() {
 Serial.begin(115200);
 initWiFi();
 serverconnect();
 Serial.println("connected to wifi and the server");
}

void loop() {
  serverhandshake();
  delay(500);
}
