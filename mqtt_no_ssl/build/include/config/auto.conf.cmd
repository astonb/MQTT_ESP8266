deps_config := \
	/home/40806/esp/ESP8266_RTOS_SDK/components/app_update/Kconfig \
	/home/40806/esp/ESP8266_RTOS_SDK/components/aws_iot/Kconfig \
	/home/40806/esp/ESP8266_RTOS_SDK/components/esp8266/Kconfig \
	/home/40806/esp/ESP8266_RTOS_SDK/components/freertos/Kconfig \
	/home/40806/esp/ESP8266_RTOS_SDK/components/libsodium/Kconfig \
	/home/40806/esp/ESP8266_RTOS_SDK/components/log/Kconfig \
	/home/40806/esp/ESP8266_RTOS_SDK/components/lwip/Kconfig \
	/home/40806/esp/ESP8266_RTOS_SDK/components/mdns/Kconfig \
	/home/40806/esp/ESP8266_RTOS_SDK/components/newlib/Kconfig \
	/home/40806/esp/ESP8266_RTOS_SDK/components/pthread/Kconfig \
	/home/40806/esp/ESP8266_RTOS_SDK/components/ssl/Kconfig \
	/home/40806/esp/ESP8266_RTOS_SDK/components/tcpip_adapter/Kconfig \
	/home/40806/esp/ESP8266_RTOS_SDK/components/wpa_supplicant/Kconfig \
	/home/40806/esp/ESP8266_RTOS_SDK/components/bootloader/Kconfig.projbuild \
	/home/40806/esp/ESP8266_RTOS_SDK/components/esptool_py/Kconfig.projbuild \
	/home/40806/esp/ESP8266_RTOS_SDK/components/partition_table/Kconfig.projbuild \
	/home/40806/esp/ESP8266_RTOS_SDK/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
