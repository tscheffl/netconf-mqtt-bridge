
/*
* Compile with: 
*    gcc -lmosquitto mqtt.c lifx-lib.c
*    clang -lmosquitto -o mqtt mqtt.c lifx-lib.c
*
*  Execute:
*    /usr/local/sbin/mosquitto -v
*    mosquitto_pub -d  -h localhost  -t "LIFX/test" -m "BABY"
*    mosquitto_sub -d -h localhost -p 1883 -t "LIFX/#"
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <mosquitto.h>
#include "mqtt.h"
#include "lifx-lib.h"

//SOCKET
//#define SERVER "255.255.255.255"
#define SERVER "192.168.178.255"
#define PORT 56700
//#define BUFLEN 255
#define BUFF_SIZE 520

int sockfd;

void sendMessage(int sock, struct sockaddr_in servaddr, char* buffer, int length)
{    
	int l_servaddr=sizeof(servaddr); 
	//Sending
	CHECK(sendto(sock, buffer, length, 0, (struct sockaddr*) &servaddr, l_servaddr), "Send problem");

	printf("LIFX message sent\n");    
	return;
}


void setLIFXAction(char* action)
{
    int i = 0;
    struct sockaddr_in servaddr;
    char buffer[BUFF_SIZE];
    int length;
    
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVER, &servaddr.sin_addr);
	//servaddr.sin_addr.s_addr = inet_addr("192.168.0.255");
    servaddr.sin_port = htons(PORT);

    

  if(!strcmp(action, "RED"))
  {
	printf("Message erkannt -> RED\n");
	for (i=0; i<3 ; i++) 
	{
		length = buildLIFX_ColorMessage(buffer, "red", 50); //in percent
		sendMessage(sockfd, servaddr, buffer, length);
		usleep(50000);
	}

  } 
  else if(!strcmp(action, "BLUE"))
  {
	printf("Message erkannt -> BLUE\n");
	for (i=0; i<3 ; i++) 
	{
		length = buildLIFX_ColorMessage(buffer, "blue", 50); //in percent
		sendMessage(sockfd, servaddr, buffer, length);
		usleep(50000);
	}

  } 
  else if(!strcmp(action, "GREEN"))
  {
	printf("Message erkannt -> GREEN\n");
	for (i=0; i<3 ; i++) 
	{
		length = buildLIFX_ColorMessage(buffer, "green", 50); //in percent
		sendMessage(sockfd, servaddr, buffer, length);
		usleep(50000);

  	}
 	
  }
  else if(!strcmp(action, "OFF"))
    {
  	printf("Message erkannt -> OFF\n");
  	for (i=0; i<3 ; i++) 
  	{
  		length = buildLIFX_ColorMessage(buffer, "green", 0); //in percent
  		sendMessage(sockfd, servaddr, buffer, length);
  		usleep(50000);
  
    	}
  
    }

  else 
 {
	printf("Message nicht erkannt!\n");
	return;
  }
}

void initUDPsocket()
{
  int status;
  int broadcast = 1;

  if ( (sockfd=socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
	  die("socket");
	}

  status = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(int) );
  printf("Setsockopt Status = %d\n", status);

}

void message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
	if(message->payloadlen){
		printf("Mosquitto: %s %s\n", message->topic, message->payload);
		  setLIFXAction(message->payload);
		  sleep(1);

	}else{
		printf("%s (null)\n", message->topic);
	}
	fflush(stdout);
}

void connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
	if(!result){
		mosquitto_subscribe(mosq, NULL, MQTT_TOPIC, 2);
	}else{
		fprintf(stderr, "Connect failed\n");
	}
}

void subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
	int i;

	printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
	for(i=1; i<qos_count; i++){
		printf(", %d", granted_qos[i]);
	}
	printf("\n");
}

void log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
	printf("%s\n", str);
}

int main(void)
{
  char *host = MQTT_HOST;
  int port = MQTT_PORT;
  int keepalive = 60;
  bool clean_session = true;
  struct mosquitto *mosq = NULL;

  initUDPsocket();

  mosquitto_lib_init();
  mosq = mosquitto_new(NULL, clean_session, NULL);
  if(!mosq){
	fprintf(stderr, "Error: Out of memory.\n");
	return 1;
  }
  mosquitto_log_callback_set(mosq, log_callback);
  mosquitto_connect_callback_set(mosq, connect_callback);
  mosquitto_message_callback_set(mosq, message_callback);
  mosquitto_subscribe_callback_set(mosq, subscribe_callback);

  if(mosquitto_connect(mosq, host, port, keepalive)){
	fprintf(stderr, "Unable to connect.\n");
	return 1;
  }

  mosquitto_loop_forever(mosq, -1, 1);

  mosquitto_destroy(mosq);
  mosquitto_lib_cleanup();

  close(sockfd);
  return 0;
}
