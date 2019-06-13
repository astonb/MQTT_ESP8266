#ifndef __MQTT_ESP8266_
#define __MQTT_ESP8266_

#define TIME_STAMP_MAX	0xffffffff

typedef struct Timer Timer;

struct Timer {
	unsigned long systick_period;	//系统当前时间
	unsigned long end_time;			//结束(超时)时间
	unsigned char over_flow;					//是否溢出
};


typedef struct Network Network;

struct Network
{
	int my_socket;
	int (*mqttread) (Network*, unsigned char*, int, int);
	int (*mqttwrite) (Network*, unsigned char*, int, int);
	void (*disconnect) (Network*);
};


char expired(Timer*);
void countdown_ms(Timer*, unsigned int);
void countdown(Timer*, unsigned int);
int left_ms(Timer*);

void InitTimer(Timer*);

int mqtt_read(Network*, unsigned char*, int, int);
int mqtt_write(Network*, unsigned char*, int, int);
void mqtt_disconnect(Network*);
void NewNetwork(Network*);

int ConnectNetwork(Network*, char*, int);




#endif
