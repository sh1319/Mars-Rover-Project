#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/un.h>
#include<stdlib.h>
#include<unistd.h>
#define PORTNUM 18000

int main(void){
	int sd, answer;
	char *msg[30];//change 30 into whichever max stat
	char buf[256];
	char instruction[3];
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

	while(1){
		if (read(sd,buf,sizeof(buf))==-1){
			perror("recv");
			exit(1);
		}

		printf("%s\n",buf);

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

		printf("%s\n",buf);
	}

	close(sd);

	return 0;
}
