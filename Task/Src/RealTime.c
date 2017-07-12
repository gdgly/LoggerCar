#include "RealTime.h"
#include "osConfig.h"
#include "eeprom.h"

/******************************************************************************/
void REALTIME_Task(void)
{
	osEvent signal;
	RT_TimeTypedef realTime;

	RT_Init(&realTime);

	while(1)
	{
		signal = osSignalWait(REALTIME_TASK_SIGNAL_UPDATE, 2000);

		/* 收到使能事件组标志位 */
		if ((signal.value.signals & REALTIME_TASK_SIGNAL_UPDATE)
				== REALTIME_TASK_SIGNAL_UPDATE)
		{
			HAL_RTC_GetTime(&hrtc, &realTime.time, RTC_FORMAT_BCD);
			if (realTime.oldWeekDay != hrtc.DateToUpdate.WeekDay)
			{
				/* 更新日期 */
				HAL_RTC_GetDate(&hrtc, &realTime.date, RTC_FORMAT_BCD);

				/* 日期更新到备份 */
				RT_BKUP_UpdateDate(&realTime);
			}
			printf("当前时间是%02X.%02X.%02X %02X:%02X:%02X\r\n", realTime.date.Year,
				realTime.date.Month, realTime.date.Date,
				realTime.time.Hours, realTime.time.Minutes,
				realTime.time.Seconds);

			if ((realTime.time.Seconds == 0)
				&& (realTime.time.Minutes % EE_recordInterval == 0))
			{
				osMessagePut(realtimeMessageQId, (uint32_t)&realTime, 100);

				/* 激活MainProcess任务 */
				osThreadResume(mainprocessTaskHandle);
			}
		}
		else
		{
			printf("秒中断超时\r\n");
		}

	}
}




