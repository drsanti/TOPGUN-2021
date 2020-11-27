#!/bin/bash
for f in MQTTClient-C/src/MQTTClient.c MQTTClient-C/src/MQTTClient.h; do
	sed -i -e 's/SUCCESS/MQSUCCESS/g' $f
done
