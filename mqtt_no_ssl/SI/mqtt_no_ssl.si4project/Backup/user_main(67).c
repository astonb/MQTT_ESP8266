/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
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

#include "MQTTClient.h"
#include "MQTT_esp8266.h"

#define GPIO_LED    15

#define UDP_CLIENT
#undef UDP_CLIENT

#define MQTT_SERVER_ADDR		"192.168.1.115"		
#define MQTT_SERVER_PORT		1883

static char *TAG = "wifi";
static char *TEST_SSID = "test_lenv";
static char *TEST_PASSWORD = "continue";

static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id)
    {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(TAG, "wifi_station_start");
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "go ip is [%s]", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_AP_STACONNECTED:
            ESP_LOGI(TAG, "station:"MACSTR" join, AID=%d",
                 MAC2STR(event->event_info.sta_connected.mac),
                 event->event_info.sta_connected.aid);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

void wifi_sta_init(char *ssid, char *password)
{
    wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));

    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));

    wifi_config_t wifi_config;

    memset(&wifi_config, 0x0, sizeof(wifi_config_t));
    strncpy((char *)&wifi_config.sta.ssid, ssid, strlen(ssid));
    strncpy((char *)&wifi_config.sta.password, password, strlen(password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "connect to [%s, %s]....", ssid, password);
}

void led_flash_task(void *arg)
{
    gpio_config_t io_config;

    io_config.intr_type = GPIO_INTR_DISABLE;
    io_config.mode = GPIO_MODE_OUTPUT;
    io_config.pin_bit_mask = (1<<GPIO_LED);
    io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_config.pull_up_en = GPIO_PULLUP_DISABLE;

    gpio_config(&io_config);

    while(1)
    {
        gpio_set_level(GPIO_LED, true);
        vTaskDelay(1000/portTICK_RATE_MS);
        gpio_set_level(GPIO_LED, false);
        vTaskDelay(1000/portTICK_RATE_MS);
    }   
}

void messageArrived(MessageData* data)
{
	ESP_LOGI(TAG, "Message arrived on topic %.*s: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
		data->message->payloadlen, (char *)data->message->payload);
}


unsigned char sendbuf[512];
unsigned char readbuf[512];
extern int keepalive(MQTTClient* c);

void mqtt_client_task(void *arg)
{
	MQTTClient client;
	Network network;

	int rc = 0;
	int count = 0;

	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
	
	NewNetwork(&network);

	MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));
	
	rc = ConnectNetwork(&network, MQTT_SERVER_ADDR, MQTT_SERVER_PORT);
	if(rc != 0)
	{
		ESP_LOGI(TAG, "tcp connect failed");
	}

	connectData.MQTTVersion = 4;	//3.1.1
	connectData.clientID.cstring = "test123456789"; //ClientID

	if((rc = MQTTConnect(&client, &connectData)) != 0)
	{
		ESP_LOGI(TAG, "mqtt connect failed");
	}
	ESP_LOGI(TAG, "mqtt connect successful");
	
	if((rc = MQTTSubscribe(&client, "test/a", 0, messageArrived)) != 0)
	{
		ESP_LOGI(TAG, "MQTT subscribe failed");
	}
	ESP_LOGI(TAG, "mqtt subscribe successful");

	while(1)
	{
		if ((rc = MQTTYield(&client, 500)) != 0)
			ESP_LOGI(TAG, "Return code from yield ");

		count++;
		if(count == 20)
		{	//10秒心跳包
			count = 0;
			rc = keepalive(&client);
			if(rc != 0)
			{
				ESP_LOGI(TAG, "ping error");
			}
		}
		
	}
		
	
}

/******************************************************************************
 * FunctionName : app_main
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);


    wifi_sta_init(TEST_SSID, TEST_PASSWORD);

    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, true, false, portMAX_DELAY);

    ESP_LOGI(TAG, "***wifi connect successful***");
    xTaskCreate(led_flash_task,  "led_flash_task",  1024, NULL, 10, NULL);
	xTaskCreate(mqtt_client_task, "mqtt_client_task",8192, NULL, 10, NULL);
}
