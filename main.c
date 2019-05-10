#include <stdio.h>
#include <pthread.h>

#include "p2ptun.h"
#include "MQTTLinux.h"
#include "mqtt.h"
#include "cJSON.h"

int data_2_json(int cmd, int freq, int res, char *body)
{
	char *p;
	cJSON *pJsonRoot = NULL;

	pJsonRoot = cJSON_CreateObject();
	if (NULL == pJsonRoot)
	{
		return 0;
	}

	cJSON_AddNumberToObject(pJsonRoot, "cmd", cmd + 8000);
	cJSON_AddNumberToObject(pJsonRoot, "freq", freq);
	cJSON_AddNumberToObject(pJsonRoot, "res", res);
	cJSON_AddStringToObject(pJsonRoot, "body", body);

	p = cJSON_Print(pJsonRoot);

	if (NULL == p)
	{
		cJSON_Delete(pJsonRoot);
		return 0;
	}

	cJSON_Delete(pJsonRoot);
	printf("MQTT：%s \n", p);
	mqtt_data_write(p, strlen(p), 1);

	free(p);

	return 1;
	//
}

void json_2_data(char *in_json)
{
	cJSON *pSub;
	cJSON *pJson;

	if (NULL == in_json)
	{
		return;
	}
	pJson = cJSON_Parse(in_json);
	if (NULL == pJson)
	{
		return;
	}

	pSub = cJSON_GetObjectItem(pJson, "cmd");
	if (NULL != pSub)
	{
		int freq = 0;
		int cmd = pSub->valueint;
		pSub = cJSON_GetObjectItem(pJson, "freq");
		if (NULL != pSub)
		{
			freq = pSub->valueint;
		}
		switch (cmd)
		{
		case 1:

			pSub = cJSON_GetObjectItem(pJson, "url");
			if (NULL != pSub)
			{
				printf("url %s \n", pSub->valuestring);
				//CCommanObj.rtmp->setUrl(pSub->valuestring);
				//CCommanObj.rtmp->startPush();
			}
			break;
		case 2:
			//CCommanObj.rtmp->stopPush();
			break;
		default:
			break;
		}

		data_2_json(cmd, freq, 0, "");
	}

	cJSON_Delete(pJson);

	return;
	//
}

void MessageArrived_Fun(void *pbuf, int len)
{
	printf("data = %s\n", (unsigned char *)pbuf); //打印接收到的数据

	//json_2_data((char *)pbuf);
}

void mqttthread(void *p)
{
	cloud_mqtt_thread((void *)MessageArrived_Fun);
}

int main(int argc, char **argv)
{
	pthread_t mqttthreadid;
	if ((pthread_create(&mqttthreadid, NULL, mqttthread, (void *)NULL)) == -1)
	{
		printf("create error!\n");
		return -1;
	}
	struct P2PTUN_CONN_SESSION *p2psession_dev1 = p2ptun_alloc_session("device1", P2PTUN_WORKMODE_CLIENT);
	struct P2PTUN_CONN_SESSION *p2psession_dev2 = p2ptun_alloc_session("device2", P2PTUN_WORKMODE_SERVER);

	for(;;)
	{
		sleep(1);
	}
	return 0;
}
