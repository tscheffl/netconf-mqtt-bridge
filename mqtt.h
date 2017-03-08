#define CHECK(sts,msg) if((sts) == -1) {perror(msg);exit(-1);}


//MQTT
#define MQTT_TOPIC "LIFX/#"
#define MQTT_HOST "localhost"
#define MQTT_PORT 1883


void die(char *ss)
{
	perror(ss);
	exit(1);
}
