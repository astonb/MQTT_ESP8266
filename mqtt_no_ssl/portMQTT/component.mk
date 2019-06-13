COMPONENT_ADD_INCLUDEDIRS := . 
COMPONENT_SRCDIRS := . 

COMPONENT_ADD_INCLUDEDIRS += . ./MQTTPacket ./MQTTCommon
COMPONENT_SRCDIRS += . ./MQTTPacket ./MQTTCommon

CFLAGS += -Wno-error=unused-value -Wno-error=format=  -Wno-error=char-subscripts -Wno-error=pointer-sign

