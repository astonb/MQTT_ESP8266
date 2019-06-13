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
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include "MQTTPacket.h"
#include "StackTrace.h"

#include <string.h>


/**
  * Deserializes the supplied (wire) buffer into unsubscribe data
  * @param dup integer returned - the MQTT dup flag
  * @param packetid integer returned - the MQTT packet identifier
  * @param maxcount - the maximum number of members allowed in the topicFilters and requestedQoSs arrays
  * @param count - number of members in the topicFilters and requestedQoSs arrays
  * @param topicFilters - array of topic filter names
  * @param buf the raw buffer data, of the correct length determined by the remaining length field
  * @param buflen the length in bytes of the data in the supplied buffer
  * @return the length of the serialized data.  <= 0 indicates error
  * 反序列化: 取消订阅数据包
  */
int MQTTDeserialize_unsubscribe(unsigned char* dup, unsigned short* packetid, int maxcount, int* count, MQTTString topicFilters[],
		unsigned char* buf, int len)
{
	MQTTHeader header = {0};
	unsigned char* curdata = buf;
	unsigned char* enddata = NULL;
	int rc = 0;
	int mylen = 0;

	FUNC_ENTRY;
	//1. 获取固定头部
	header.byte = readChar(&curdata);
	if (header.bits.type != UNSUBSCRIBE)
		goto exit;
	*dup = header.bits.dup;
	//2. 获取剩余长度
	curdata += (rc = MQTTPacket_decodeBuf(curdata, &mylen)); /* read remaining length */
	enddata = curdata + mylen;
	//3. 获取报文标识符
	*packetid = readInt(&curdata);

	*count = 0;
	while (curdata < enddata)
	{	//4.  获取 topicFilters
		if (!readMQTTLenString(&topicFilters[*count], &curdata, enddata))
			goto exit;
		(*count)++;
	}

	rc = 1;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}


/**
  * Serializes the supplied unsuback data into the supplied buffer, ready for sending
  * @param buf the buffer into which the packet will be serialized
  * @param buflen the length in bytes of the supplied buffer
  * @param packetid integer - the MQTT packet identifier
  * @return the length of the serialized data.  <= 0 indicates error
  * 序列化: 取消订阅ack
  */
int MQTTSerialize_unsuback(unsigned char* buf, int buflen, unsigned short packetid)
{
	MQTTHeader header = {0};
	int rc = 0;
	unsigned char *ptr = buf;

	FUNC_ENTRY;
	if (buflen < 2)
	{
		rc = MQTTPACKET_BUFFER_TOO_SHORT;
		goto exit;
	}
	//1. 组织 固定头部(包含第2步)
	header.byte = 0;
	header.bits.type = UNSUBACK;
	writeChar(&ptr, header.byte); /* write header */
	//2. 组织 剩余长度
	ptr += MQTTPacket_encode(ptr, 2); /* write remaining length */
	//3. 组织 报文标识符
	writeInt(&ptr, packetid);

	rc = ptr - buf;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}


