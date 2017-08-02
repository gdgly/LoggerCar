#include "../Inc/MainProcess.h"

#include "exFlash.h"
#include "analog.h"
#include "input.h"
#include "gps.h"
#include "file.h"

#include "osConfig.h"
#include "RealTime.h"
#include "GPRSProcess.h"

/*******************************************************************************
 *
 */
void MAINPROCESS_Task(void)
{
	osEvent signal;

	RT_TimeTypedef time;
	GPS_LocateTypedef* location;
	ANALOG_ValueTypedef* AnalogValue;

	FILE_InfoTypedef saveInfo;
	FILE_InfoTypedef readInfo[GPRS_PATCH_PACK_NUMB_MAX];

	FILE_PatchPackTypedef patchPack;
	uint16_t curPatchPack;			/* 本次补传数据 */

	/* 文件名格式初始化 */
	FILE_Init();

	while(1)
	{		
		/* 获取时间 */
		signal = osMessageGet(realtimeMessageQId, 100);
		memcpy(&time, (uint32_t*)signal.value.v, sizeof(RT_TimeTypedef));

		/* 获取模拟量数据 */
		signal = osMessageGet(analogMessageQId, 100);
		AnalogValue = (ANALOG_ValueTypedef*)signal.value.v;

		printf("当前时间是%02X.%02X.%02X %02X:%02X:%02X\r\n", time.date.Year,
				time.date.Month, time.date.Date,
				time.time.Hours,time.time.Minutes,
				time.time.Seconds);

		printf("温度1 = %f\r\n", AnalogValue->temp1);
		printf("温度2 = %f\r\n", AnalogValue->temp2);
		printf("温度3 = %f\r\n", AnalogValue->temp3);
		printf("温度4 = %f\r\n", AnalogValue->temp4);
		printf("湿度1 = %f\r\n", AnalogValue->humi1);
		printf("湿度2 = %f\r\n", AnalogValue->humi2);
		printf("湿度3 = %f\r\n", AnalogValue->humi3);
		printf("湿度4 = %f\r\n", AnalogValue->humi4);
		printf("电池电量 = %d\r\n", AnalogValue->batVoltage);

		/* 获取定位数据 */
		/* 激活GPRSProcess任务，启动GPS转换 */
		osThreadResume(gprsprocessTaskHandle);
 		osSignalSet(gprsprocessTaskHandle, GPRSPROCESS_GPS_ENABLE);
		
		/* 等待GPS完成,因为这个过程可能要启动GSM模块，所以等待周期必须长点，60s */
		signal = osSignalWait(MAINPROCESS_GPS_CONVERT_FINISH, 60000);
		if ((signal.value.signals & MAINPROCESS_GPS_CONVERT_FINISH)
						!= MAINPROCESS_GPS_CONVERT_FINISH)
		{
			printf("GPS定位失败\r\n");
		}
		else
		{
			/* 获取定位值 */
			signal = osMessageGet(infoMessageQId, 100);
			location = (GPS_LocateTypedef*)signal.value.v;
		}

		/* 将数据格式转换成协议格式 */
		FILE_InfoFormatConvert(&saveInfo, &time, location, AnalogValue);

		/* 读取补传数据条数 */
		FILE_ReadPatchPackFile(&patchPack);
		/* 补传数据不为0，也不是磁盘格式化后全FFFF */
		if ((patchPack.patchPackNumb != 0) && (patchPack.patchPackNumb != 0xFFFF))
		{
			/* 补传的数据大于等于最大补传条数 */
			if (patchPack.patchPackNumb >= GPRS_PATCH_PACK_NUMB_MAX)
				curPatchPack = GPRS_PATCH_PACK_NUMB_MAX;
			else
				/* 小于最大补传条数，则把当前这条数据也上传上去 */
				curPatchPack = patchPack.patchPackNumb + 1;
		}
		else
		{
			/* 没有需要补传的数据，发送的条数为1 */
			curPatchPack = 1;

			/* 避免patchPack.patchPackNumb的值为0xFFFF */
			patchPack.patchPackNumb = 0;
		}
		printf("本次需要补传的数据为%d条", patchPack.patchPackNumb);

		/* 储存并读取数据 */
		FILE_SaveReadInfo(&saveInfo, readInfo, curPatchPack);

		/* 通过GPRS上传到平台 */
		/* 传递发送结构体 */
		osMessagePut(infoMessageQId, (uint32_t)&readInfo, 100);

		/* 传递本次发送的数据条数，注意：curPatchPack是以数据形式传递，不是传递指针 */
		osMessagePut(infoCntMessageQId, (uint16_t)curPatchPack, 100);

		/* 把当前时间传递到GPRS进程，根据回文校准时间 */
		osMessagePut(realtimeMessageQId, (uint32_t)&time, 100);

		/* 使能MainProcess任务发送数据 */
		osSignalSet(gprsprocessTaskHandle, GPRSPROCESS_SEND_DATA_ENABLE);

		/* 等待GPRSProcess完成 */
		signal = osSignalWait(MAINPROCESS_GPRS_SEND_FINISHED, 10000);
		if ((signal.value.signals & MAINPROCESS_GPRS_SEND_FINISHED)
						!= MAINPROCESS_GPRS_SEND_FINISHED)
		{
			printf("发送数据超时，说明数据发送失败，记录数据等待补传\r\n");

			/* 数据上传失败，标志位要清空，避免下次直接触发 */
//			osSignalClear(gprsprocessTaskHandle, GPRSPROCESS_SEND_DATA_ENABLE);

			/* 记录补传数据条数 */
			/* 发送失败，则补传数据+本次记录的一条数据 */
			patchPack.patchPackNumb++;

			FILE_WritePatchPackFile(&patchPack);
		}
		else
		{
			/* 数据发送成功 */
			printf("数据发送到平台成功！！\r\n");

			/* 有补传数据 */
			if (curPatchPack > 1)
			{
				if (curPatchPack >= GPRS_PATCH_PACK_NUMB_MAX)
				{
					/* 补传30条数据次数 */
					patchPack.patchPackOver_30++;
					patchPack.patchPackNumb -= curPatchPack;
				}
				else
				{
					if (curPatchPack >= 20)
						patchPack.patchPackOver_20++;
					else if (curPatchPack >= 10)
						patchPack.patchPackOver_10++;
					else if (curPatchPack >= 5)
						patchPack.patchPackOver_5++;

					/* 已经补传全部数据 */
					patchPack.patchPackNumb = 0;
				}

				FILE_WritePatchPackFile(&patchPack);
			}
		}

		/* 任务运行完毕，一定要将自己挂起 */
		osThreadSuspend(NULL);
	}
}


