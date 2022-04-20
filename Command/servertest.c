#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/un.h>
#include<stdlib.h>
#include<unistd.h>
#include<math.h>
#include<string.h>
#include<time.h>
#define PORTNUM 18000

char *my_itoa(int num, char *str) { 
    if(str == NULL) { 
        return NULL; 
    } 
    sprintf(str, "%d", num); 
    return str; 
}

const static char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";

int main(void){
    char buf[256],msg[256];
    char fo[20],ba[20],st[20],cl[20],ccl[20];
    memset(fo,0,strlen(fo));
    strcpy(fo,"forward");
    memset(ba,0,strlen(ba));
    strcpy(ba,"backward");
    memset(st,0,strlen(st));
    strcpy(st,"stop");
    memset(cl,0,strlen(cl));
    strcpy(cl,"clockwise");
    memset(ccl,0,strlen(ccl));
    strcpy(ccl,"counterclockwise");
    struct sockaddr_in sin,cli;
    int sd,ns,clientlen=sizeof(cli);

    if((sd=socket(AF_INET,SOCK_STREAM,0))==-1){
        perror("socket");
        exit(1);
    }
    int optvalue=1;
    setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&optvalue,sizeof(optvalue));
    memset((char*)&sin, '\0',sizeof(sin));
    sin.sin_family=AF_INET;
    sin.sin_port=htons(PORTNUM);
    sin.sin_addr.s_addr=htonl(INADDR_ANY); //might have to change

    if(bind(sd,(struct sockaddr *)&sin,sizeof(sin))){
        perror("bind");
        exit(1);
    }

    if(listen(sd,5)){
        perror("listen");
        exit(1);
    }
    printf("waiting client...\n");

    if((ns=accept(sd,(struct sockaddr *)&cli,&clientlen))==-1){
        perror("accept");
        exit(1);
    }

    while(1){
        char move[20], rotate[20];
        memset(move,0,strlen(move));
        memset(rotate,0,strlen(rotate));
        int power, num;
        srand(time(NULL));
        scanf("%s",&move);
        if(strcmp(move,st)==0){
            break;
        }
        scanf("%s",&rotate);
        scanf("%d",&power);
        int instr[8];
        int instruction=0;
        if(strcmp(move, fo)==0){
            //move forward
            instr[0]=0;
            instr[1]=1;
        }
        else if(strcmp(move, ba)==0){
            //move backward
            instr[0]=1; 
            instr[1]=0;
        }
        else{
            //dont move
            instr[0]=0;
            instr[1]=0;
        }

        if(strcmp(rotate, cl)==0){
            //rotate clockwise
            instr[2]=0;
            instr[3]=1;
        }
        else if(strcmp(rotate, ccl)==0){
            //rotate counter clockwise
            instr[2]=1;
            instr[3]=0;
        }
        else{
            //dont move
            instr[2]=0;
            instr[3]=0;
        }

        if(11>power > 0){
            //power is power
            int n =power;
            instr[4]=0;
            instr[5]=0;
            instr[6]=0;
            instr[7]=0;
            for(int i=7;n>0;i--){    
                instr[i]=n%2;    
                n=n/2;    
            }    
        }
        else{
            //dont move
            instr[4]=0;
            instr[5]=0;
            instr[6]=0;
            instr[7]=0;
        }
        //make instruction into an integer
        for( int j = 0; j<8; j++){
            printf("%d",instr[j]);
            instruction = instruction + instr[j] * pow(2, 7-j);
            instr[j]=0;
        }

        printf("\n");

        //integer to string
        char temp [3];
        my_itoa(instruction, temp);

        sprintf(buf,"%s",temp);
        if(write(ns,buf,strlen(buf)+1)==-1){
            perror("send");
            exit(1);
        }

        if(read(ns,msg,sizeof(msg))==-1){
            perror("recv");
            exit(1);
        }

        printf("Status of rover: %s\n ",msg);
        memset(buf,0,sizeof(buf));

        if(write(ns,buf,strlen(buf)+1)==-1){
            perror("error no input");
            exit(1);
        }
        printf("Sending result: %d\n",instruction);
    }

    close(ns);
    close(sd);
    return 0;
}
