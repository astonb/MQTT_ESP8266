/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "MQTTCommon.h"


unsigned int GetSystemTimeStamp(void) 
{
	return (unsigned int)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

/* 检测是否超时 */
char expired(Timer *timer)
{
	long left = 0;

	if(timer->over_flow)
		left = TIME_STAMP_MAX - GetSystemTimeStamp() + timer->end_time;
	else
		left = timer->end_time - GetSystemTimeStamp();

	return (left < 0);
}

/* 设置超时时间(ms) */
void countdown_ms(Timer * timer, unsigned int  timeout)
{
	unsigned int current_time = GetSystemTimeStamp();
	timer->end_time = current_time + timeout;
	if(timer->end_time < current_time)
	{
		timer->over_flow = 1;
	}
}


/* 设置超时时间(s) */
void countdown(Timer * timer, unsigned int timeout)
{
	countdown_ms(timer, timeout*1000);
}

/* 倒计时剩余时间(ms) */
int left_ms(Timer *timer)
{
	long left = 0;

    if (timer->over_flow)
    {
        left = 0xFFFFFFFF - GetSystemTimeStamp() + timer->end_time;
    }
    else
    {
        left = timer->end_time - GetSystemTimeStamp();
    }

    return (left < 0) ? 0 : left;
}

void InitTimer(Timer *timer)
{
    timer->end_time = 0;
    timer->systick_period = 0;
    timer->over_flow = 0;
}

int mqtt_read(Network *n, uint8_t *buffer, int len, int timeout_ms)
{
	int rc = 0;
	int recvLen = 0;
	struct timeval t;
	fd_set fdset;
		

	FD_ZERO(&fdset);
	FD_SET(n->my_socket, &fdset);

	t.tv_sec = 0;
	t.tv_usec = timeout_ms*1000;

	if (select(n->my_socket + 1, &fdset, NULL, NULL, &t) == 1) 
	{
		do {
#ifdef MQTT_SSL
				rc = ssl_recv(n->my_socket, buffer + recvLen, len - recvLen, 0);
#else
				rc = recv(n->my_socket, buffer + recvLen, len - recvLen, 0);
#endif
				recvLen += rc;
			} while(recvLen < len);
	}
	return recvLen;
	
}


int mqtt_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	struct timeval t;
	fd_set fdset;
	int rc = 0;
	int readySock;

	FD_ZERO(&fdset);
	FD_SET(n->my_socket, &fdset);

	t.tv_sec = 0;
	t.tv_usec = timeout_ms * 1000;

	do{
		readySock = select(n->my_socket + 1, NULL, &fdset, NULL, &t);
	} while(readySock != 1);
#ifdef MQTT_SSL
	rc = ssl_send(n->my_socket, buffer, len, 0);
#else
	rc = send(n->my_socket, buffer, len, 0);
#endif
	return rc;

}


void mqtt_disconnect(Network *n)
{
#ifdef MQTT_SSL
	ssl_close(n->my_socket);
#else
	close(n->my_socket);
#endif	
}


void NewNetwork(Network* n) 
{
	n->my_socket = 0;
	n->mqttread = mqtt_read;
	n->mqttwrite = mqtt_write;
	n->disconnect = mqtt_disconnect;
}

int ConnectNetwork(Network *n, char *addr, int port)
{
	struct sockaddr_in sAddr;
	int addrSize;
	int retVal;
//	unsigned long ipAddress;

	sAddr.sin_family = AF_INET;
	sAddr.sin_port = htons(port);
	sAddr.sin_addr.s_addr = inet_addr(addr);	//点分十进制

	addrSize = sizeof(sAddr);

	n->my_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(n->my_socket < 0)
	{
		close(n->my_socket);
		return -1;
	}
	retVal = connect(n->my_socket, (const struct sockaddr *)&sAddr, addrSize);
	if(retVal < 0)
	{
		close(n->my_socket);
		return retVal;
	}

	
	return retVal;
}

int TLSConnectNetwork(Network *n, char *addr, int port)
{
	return 0;
}


