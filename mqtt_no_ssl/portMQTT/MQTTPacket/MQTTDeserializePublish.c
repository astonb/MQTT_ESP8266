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

#include "StackTrace.h"
#include "MQTTPacket.h"
#include <string.h>

#define min(a, b) ((a < b) ? 1 : 0)

/** pulish报文格式: 固定头部(byte1 + byte2剩余数据长度) + 可变报头(长度 +  Topic Name)  +  报文标识符
  * Deserializes the supplied (wire) buffer into publish data
  * @param dup returned integer - the MQTT dup flag
  * @param qos returned integer - the MQTT QoS value
  * @param retained returned integer - the MQTT retained flag
  * @param packetid returned integer - the MQTT packet identifier
  * @param topicName returned MQTTString - the MQTT topic in the publish
  * @param payload returned byte buffer - the MQTT publish payload
  * @param payloadlen returned integer - the length of the MQTT payload
  * @param buf the raw buffer data, of the correct length determined by the remaining length field
  * @param buflen the length in bytes of the data in the supplied buffer
  * @return error code.  1 is success  返回结果:  成功: 1   错误: 0  
  * 反序列化: 发布内容(解析buf内容，赋值给 dup， qos, retained, packtid, topicname, payload, payloadlen)
  */
int MQTTDeserialize_publish(unsigned char* dup, int* qos, unsigned char* retained, unsigned short* packetid, MQTTString* topicName,
		unsigned char** payload, int* payloadlen, unsigned char* buf, int buflen)
{
	MQTTHeader header = {0};
	unsigned char* curdata = buf;
	unsigned char* enddata = NULL;
	int rc = 0;
	int mylen = 0;

	FUNC_ENTRY;
	//1. 读取pulish固定头部
	header.byte = readChar(&curdata);
	if (header.bits.type != PUBLISH)
		goto exit;
	*dup = header.bits.dup;
	*qos = header.bits.qos;
	*retained = header.bits.retain;
	//2. 获取剩余数据长度->mylen
	curdata += (rc = MQTTPacket_decodeBuf(curdata, &mylen)); /* read remaining length */
	enddata = curdata + mylen;
	//3. 获取TopicName(如格式: a/b)
	if (!readMQTTLenString(topicName, &curdata, enddata) ||
		enddata - curdata < 0) /* do we have enough data to read the protocol version byte? */
		goto exit;
	//4. 获取报文标识符(QoS>0，才有意义)
	if (*qos > 0)
		*packetid = readInt(&curdata);
	//5. 有效负载及长度
	*payloadlen = enddata - curdata;
	*payload = curdata;
	rc = 1;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}



/**
  * Deserializes the supplied (wire) buffer into an ack
  * @param packettype returned integer - the MQTT packet type
  * @param dup returned integer - the MQTT dup flag
  * @param packetid returned integer - the MQTT packet identifier
  * @param buf the raw buffer data, of the correct length determined by the remaining length field
  * @param buflen the length in bytes of the data in the supplied buffer
  * @return error code.  1 is success, 0 is failure 返回结果:  成功: 1   错误: 0   
  * 反序列化: ack数据包(用于解析发布，订阅应答)
  */
int MQTTDeserialize_ack(unsigned char* packettype, unsigned char* dup, unsigned short* packetid, unsigned char* buf, int buflen)
{
	MQTTHeader header = {0};
	unsigned char* curdata = buf;
	unsigned char* enddata = NULL;
	int rc = 0;
	int mylen;

	FUNC_ENTRY;
	//1. 获取固定头部(byte1)
	header.byte = readChar(&curdata);
	*dup = header.bits.dup;
	*packettype = header.bits.type;
	//2. 获取剩余长度(byte2)
	curdata += (rc = MQTTPacket_decodeBuf(curdata, &mylen)); /* read remaining length */
	enddata = curdata + mylen;

	if (enddata - curdata < 2)
		goto exit;
	//3. 获取报文标识符
	*packetid = readInt(&curdata);

	rc = 1;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}

