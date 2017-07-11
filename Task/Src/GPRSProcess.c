#include "GPRSProcess.h"
#include "gprs.h"

/*******************************************************************************
 *
 */
void GPRSPROCESS_Task(void)
{
	osEvent signal;
	GPRS_ModuleStatusEnum moduleStatus = SET_BAUD_RATE;		/* GPRSģ��״̬ */
	char* expectString;						/* Ԥ���յ����ַ��� */
	char* str;
//	GPRS_StructTypedef sendStruct;			/* ���ͽṹ */

	while(1)
	{

		/* ���Ͳ��� */
		switch (moduleStatus)
		{
		/* ���ģ����Ч������ִ�п��� */
		case MODULE_INVALID:
			printf("ģ�鿪��\r\n");
			/* ���� */
			GPRS_PWR_CTRL_ENABLE();
			osDelay(5000);
			GPRS_PWR_CTRL_DISABLE();
			expectString = AT_CMD_POWER_ON_READY_RESPOND;
			moduleStatus = MODULE_VALID;
			break;

		/* ���ò����� */
		case SET_BAUD_RATE:
			printf("���ò�����\r\n");
			GPRS_SendCmd(AT_CMD_SET_BAUD_RATE);
			expectString = AT_CMD_SET_BAUD_RATE_RESPOND;
			moduleStatus = SET_BAUD_RATE_FINISH;
			break;

		/* ��ѯSIM��״̬ */
		case CHECK_SIM_STATUS:
			printf("��ѯsim��״̬\r\n");
			GPRS_SendCmd(AT_CMD_CHECK_SIM_STATUS);
			expectString = AT_CMD_CHECK_SIM_STATUS_RESPOND;
			moduleStatus = CHECK_SIM_STATUS_FINISH;
			break;

		/* ��������״̬ */
		case SEARCH_NET_STATUS:
			printf("��������\r\n");
			GPRS_SendCmd(AT_CMD_SEARCH_NET_STATUS);
			expectString = AT_CMD_SEARCH_NET_STATUS_RESPOND;
			moduleStatus = SEARCH_NET_STATUS_FINISH;
			break;

		/* ����GPRS״̬ */
		case CHECK_GPRS_STATUS:
			printf("����GPRS״̬\r\n");
			GPRS_SendCmd(AT_CMD_CHECK_GPRS_STATUS);
			expectString = AT_CMD_CHECK_GPRS_STATUS_RESPOND;
			moduleStatus = CHECK_GPRS_STATUS_FINISH;
			break;

		/* ���õ����ӷ�ʽ */
		case SET_SINGLE_LINK:
			printf("���õ�����ʽ\r\n");
			GPRS_SendCmd(AT_CMD_SET_SINGLE_LINK);
			expectString = AT_CMD_SET_SINGLE_LINK_RESPOND;
			moduleStatus = SET_SINGLE_LINK_FINISH;
			break;

		/* ����Ϊ͸��ģʽ */
		case SET_SERIANET_MODE:
			printf("����͸��ģʽ\r\n");
			GPRS_SendCmd(AT_CMD_SET_SERIANET_MODE);
			expectString = AT_CMD_SET_SERIANET_MODE_RESPOND;
			moduleStatus = SET_SERIANET_MODE_FINISH;
			break;

		/* ����APN���� */
		case SET_APN_NAME:
			printf("����APN����\r\n");
			GPRS_SendCmd(AT_CMD_SET_APN_NAME);
			expectString = AT_CMD_SET_APN_NAME_RESPOND;
			moduleStatus = SET_APN_NAME_FINISH;
			break;

		/* ����PDP���� */
		case ACTIVE_PDP:
			printf("����PDP����\r\n");
			GPRS_SendCmd(AT_CMD_ACTIVE_PDP);
			expectString = AT_CMD_ACTIVE_PDP_RESPOND;
			moduleStatus = ACTIVE_PDP_FINISH;
			break;

		/* ��ȡ����IP��ַ */
		case GET_SELF_IP_ADDR:
			printf("��ȡ����IP��ַ\r\n");
			GPRS_SendCmd(AT_CMD_GET_SELF_IP_ADDR);
			expectString = AT_CMD_GET_SELF_IP_ADDR_RESPOND;
			moduleStatus = GET_SELF_IP_ADDR_FINISH;
			break;

		/* ���÷�������ַ */
		case SET_SERVER_IP_ADDR:
			printf("��ȡ��������ַ\r\n");
			GPRS_SendCmd(AT_CMD_SET_SERVER_IP_ADDR);
			expectString = AT_CMD_SET_SERVER_IP_ADDR_RESPOND;
			moduleStatus = SET_SERVER_IP_ADDR_FINISH;
			break;

		/* ģ��׼������ */
		case READY:
			printf("ģ��׼�����ˣ���������\r\n");
			/* �������ݵ�ƽ̨ */
//			GPRS_SendDetail();
			expectString = AT_CMD_DATA_SEND_SUCCESS_RESPOND;
			moduleStatus = DATA_SEND_FINISH;
			break;

		/* �˳�͸��ģʽ */
		case EXTI_SERIANET_MODE:
			printf("�˳�͸��ģʽ\r\n");
			GPRS_SendCmd(AT_CMD_EXIT_SERIANET_MODE);
			expectString = AT_CMD_EXIT_SERIANET_MODE_RESPOND;
			moduleStatus = EXTI_SERIANET_MODE_FINISH;
			break;

		/* �˳�����ģʽ */
		case EXTI_LINK_MODE:
			printf("�˳�����ģʽ\r\n");
			GPRS_SendCmd(AT_CMD_EXIT_LINK_MODE);
			expectString = AT_CMD_EXIT_LINK_MODE_RESPOND;
			moduleStatus = EXTI_LINK_MODE_FINISH;
			break;

		/* �ر��ƶ����� */
		case SHUT_MODULE:
			printf("�ر��ƶ�����\r\n");
			GPRS_SendCmd(AT_CMD_SHUT_MODELU);
			expectString = AT_CMD_SHUT_MODELU_RESPOND;
			moduleStatus = SHUT_MODULE_FINISH;
			break;

		default:
			break;
		}

		signal = osSignalWait(GPRS_PROCESS_TASK_RECV_ENABLE, 5000);
		/* ���ͳ�ʱ */
		if (signal.status == osEventTimeout)
		{
			/* ���͵�ƽ̨������û���յ��� */
			if (DATA_SEND_FINISH == moduleStatus)
			{
				/* �����������ݷ��ͣ���ģʽ�л����˳�͸��ģʽ */
				moduleStatus = EXTI_SERIANET_MODE;
			}
			else /* ��������û���յ�Ԥ�ڴ𸴣����ظ����� */
			{
				/* ģʽ�л���ǰһ���ٴδ������� */
				moduleStatus--;
			}

//			/* ���������̣߳�ִ�������������߳� */
//			osThreadYield();
			printf("�ȴ����ճ�ʱ\r\n");
		}
		else if ((signal.value.signals & GPRS_PROCESS_TASK_RECV_ENABLE)
				== GPRS_PROCESS_TASK_RECV_ENABLE)
		{
			/* Ѱ��Ԥ�ڽ��յ��ַ����Ƿ��ڽ��յ������� */
			str = strstr((char*)GPRS_BufferStatus.recvBuffer, expectString);
			if (NULL != str)
			{
				switch (moduleStatus)
				{
				/* ģ����� */
				case MODULE_VALID:
					printf("ģ�����\r\n");
					/* ������ɣ��Ͽ�power�������� */
					GPRS_PWR_CTRL_DISABLE();
					expectString = AT_CMD_MODULE_START_RESPOND;
					moduleStatus = MODULE_START;
					break;

					/* ģ������ */
				case MODULE_START:
					printf("ģ������\r\n");
					/* ��ʼ����ģ����� */
					moduleStatus = SET_BAUD_RATE;
					break;

					/* ���ò�������� */
				case SET_BAUD_RATE_FINISH:
					printf("���ò��������\r\n");
					moduleStatus = CHECK_SIM_STATUS;
					break;

					/* ���sim��״̬��� */
				case CHECK_SIM_STATUS_FINISH:
					printf("���sim��״̬���\r\n");
					moduleStatus = SEARCH_NET_STATUS;
					break;

					/* ��������״̬��� */
				case SEARCH_NET_STATUS_FINISH:
					printf("��������״̬���\r\n");
					moduleStatus = CHECK_GPRS_STATUS;
					break;

					/* ����GPRS״̬��� */
				case CHECK_GPRS_STATUS_FINISH:
					printf("����GPRS״̬���\r\n");
					moduleStatus = SET_SINGLE_LINK;
					break;

					/* ���õ�����ʽ��� */
				case SET_SINGLE_LINK_FINISH:
					printf("���õ�����ʽ���\r\n");
					moduleStatus = SET_SERIANET_MODE;
					break;

					/* ����͸��ģʽ��� */
				case SET_SERIANET_MODE_FINISH:
					printf("����͸��ģʽ���\r\n");
					moduleStatus = SET_APN_NAME;
					break;

					/* ����APN������� */
				case SET_APN_NAME_FINISH:
					printf("����APN�������\r\n");
					moduleStatus = ACTIVE_PDP;
					break;

					/* ����PDP������� */
				case ACTIVE_PDP_FINISH:
					printf("����PDP�������\r\n");
					moduleStatus = GET_SELF_IP_ADDR;
					break;

					/* ��ȡ����IP��ַ��� */
				case GET_SELF_IP_ADDR_FINISH:
					printf("��ȡ����IP��ַ���\r\n");
					moduleStatus = SET_SERVER_IP_ADDR;
					break;

					/* ���÷�������ַ��� */
				case SET_SERVER_IP_ADDR_FINISH:
					printf("���÷�������ַ���\r\n");
					moduleStatus = READY;
					break;

					/* ���ݷ������ */
				case DATA_SEND_FINISH:
					printf("���ݷ������\r\n");
					moduleStatus = EXTI_SERIANET_MODE;
					break;

					/* �˳�͸��ģʽ��� */
				case EXTI_SERIANET_MODE_FINISH:
					printf("�˳�͸��ģʽ���\r\n");
					moduleStatus = EXTI_LINK_MODE;
					break;

					/* �˳�����ģʽ��� */
				case EXTI_LINK_MODE_FINISH:
					printf("�˳�����ģʽ���\r\n");
					moduleStatus = SHUT_MODULE;
					break;

					/* �ر��ƶ�������� */
				case SHUT_MODULE_FINISH:
					printf("�ر��ƶ��������\r\n");
					/* ģ�鷢����ɣ���״̬���ó����ӷ�������ַ���´�����ֱ�����ӷ�������ַ���ɷ��� */
					moduleStatus = SET_SERVER_IP_ADDR;
					break;

				default:
					break;
				}
			}
		}
		else /* ���յ�����û����Ԥ�ڵ���ͬ */
		{
			/* �жϽ��յ������Ƿ���� */
			str = strstr((char*)GPRS_BufferStatus.recvBuffer, "Error");

			/* ���ݴ��󣬱������³�ʼ��ģ�� */
			if (str != RESET)
				moduleStatus = SET_BAUD_RATE;
		}

		memset(GPRS_BufferStatus.recvBuffer, 0, GPRS_BufferStatus.bufferSize);
		osDelay(1000);
	}
}


