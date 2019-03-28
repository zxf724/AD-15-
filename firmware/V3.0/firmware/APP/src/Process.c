
/**
 * *********************************************************************
 *             Copyright (c) 2016 temp. All Rights Reserved.
 * @file Process.c
 * @version V1.0
 * @date 2016.8.31
 * @brief ҵ���߼���������.
 *
 * *********************************************************************
 * @note
 *
 * *********************************************************************
 * @author ����
 */



/* Includes ------------------------------------------------------------------*/
#include "includes.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static char subscribeTopic[20];

/* Private function prototypes -----------------------------------------------*/
static void ArrivePath(uint8_t* dat, uint16_t len);
static void ArriveDataProc(char* cmd, cJSON* desired);
static BOOL CMD_Confirm_Rsp(uint32_t msgid, char* ret);
static void process_Console(int argc, char* argv[]);

/* Exported functions --------------------------------------------------------*/

/**
 * ҵ��������ʼ��
 */
void Process_Init(void) {
    sprintf(subscribeTopic, "/AD-15/%u", WorkData.DeviceID);
    CMD_ENT_DEF(process, process_Console);
    Cmd_AddEntrance(CMD_ENT(process));
    Subscribe_MQTT(subscribeTopic, QOS1, ArrivePath);
    DBG_LOG("Process Start.");
}

/**
 * ���������������?
 *
 * @param cmd     ���͵�����
 * @param desired �ӽṹ
 * @return ���ط��ͽ��?
 */
BOOL CMD_Updata(char* cmd, cJSON* desired) {
    BOOL ret = FALSE;
    char* s = NULL;
    cJSON* root = NULL;
    uint32_t tick = 0;
    root = cJSON_CreateObject();
    if (root != NULL) {
        app_timer_cnt_get(&tick);
        cJSON_AddNumberToObject(root, "messageid", tick);
        cJSON_AddNumberToObject(root, "timestamp", RTC_ReadCount());
        cJSON_AddStringToObject(root, "cmd", cmd);
        cJSON_AddNumberToObject(root, "deviceid", WorkData.DeviceID);
        cJSON_AddItemToObjectCS(root, "desired", desired);
        s = cJSON_PrintBuffered(root, 280, 0);
        if (s != NULL) {
            ret = Publish_MQTT("/AD-15", QOS0, (uint8_t*)s, strlen(s)) ? TRUE : FALSE;
            free(s);
        }
        cJSON_Delete(root);
    } else {
        cJSON_Delete(desired);
    }
    return ret;
}

/**
 * �ϱ���ɡ��״̬
 *
 * @param rfid   ɡ��RFID
 * @param status �����״�?
 */
BOOL Report_Umbrella_Borrow_Status(uint32_t rfid, motor_status_t status) {
    cJSON* desired = NULL;
    desired = cJSON_CreateObject();
    if (desired != NULL) {
        cJSON_AddNumberToObject(desired, "Umbrella_ID", rfid);
        switch (status) {
            case status_output_unbrella_success:
                cJSON_AddStringToObject(desired, "Result", "ok");
                break;
            case status_ir_stuck:
                cJSON_AddStringToObject(desired, "Result", "irstuck");
                break;
            case status_motor_stuck:
                cJSON_AddStringToObject(desired, "Result", "motorstuck");
                break;
            case status_timeout:
                cJSON_AddStringToObject(desired, "Result", "timeout");
                break;
            case status_have_no_unbrella:
                cJSON_AddStringToObject(desired, "Result", "empty");
                break;
            case status_restart_ouput:
                cJSON_AddStringToObject(desired, "Result", "motorbroken");
                break;
            default:
                break;
        }
        return CMD_Updata("CMD-23", desired);
    }
    return FALSE;
}

/**
 * �ϱ���ɡ��¼
 *
 * @param rfid   ɡ��RFID
 * @param status ��ɡ��״̬
 * @param ts     ʱ���?
 */
BOOL Report_Umbrella_Repy_Status(uint32_t rfid, motor_status_t status, uint32_t ts) {
    cJSON* desired = NULL;
    desired = cJSON_CreateObject();
    if (desired != NULL) {
        cJSON_AddNumberToObject(desired, "Umbrella_ID", rfid);
        cJSON_AddNumberToObject(desired, "timestamp", ts);
        switch (status) {
            case status_input_unbrella_success:
                cJSON_AddStringToObject(desired, "Result", "ok");
                break;
            case status_ir_stuck:
                cJSON_AddStringToObject(desired, "Result", "irstuck");
                break;
            case status_motor_stuck:
                cJSON_AddStringToObject(desired, "Result", "motorstuck");
                break;
            case status_timeout:
                cJSON_AddStringToObject(desired, "Result", "timeout");
                break;
            case status_full_unbrella:
                cJSON_AddStringToObject(desired, "Result", "full");
                break;
            default:
                break;
        }
        return CMD_Updata("CMD-22", desired);
    }
    return  FALSE;
}

/**
 * ���մ���
 *
 * @param dat    ���յ�������ָ��
 * @param len    ���ݳ���
 */
static void ArrivePath(uint8_t* dat, uint16_t len) {
    uint32_t tsdiff = 0, ts = 0;
    static uint32_t msgidOld = 0;
    cJSON* root = NULL, *msgid = NULL, *timestamp = NULL, *cmd = NULL, *desired = NULL, *deviceid = NULL;
    *(dat + len) = 0;
    DBG_LOG("ArrivePath:%s", dat);
    root = cJSON_Parse((const char*)dat);
    if (root != NULL) {
        msgid = cJSON_GetObjectItem(root, "messageid");
        deviceid = cJSON_GetObjectItem(root, "deviceid");
        if (deviceid != NULL && (deviceid->valueint == WorkData.DeviceID || deviceid->valueint == 0)) {
            timestamp = cJSON_GetObjectItem(root, "timestamp");
            desired = cJSON_GetObjectItem(root, "desired");
            cmd = cJSON_GetObjectItem(root, "cmd");
            if (msgid != NULL && msgid->type == cJSON_Number
                    && msgid->valueint != msgidOld) {
                msgidOld = msgid->valueint;
                if (timestamp != NULL && timestamp->type == cJSON_Number) {
                    ts = timestamp->valueint;
                    tsdiff = RTC_ReadCount();
                    tsdiff = abs(ts - tsdiff);
                }
                /*�Ƚ�ʱ���?*/
                if (tsdiff < 30 || STR_EQUAL(cmd->valuestring, "CMD-02")) {
                    DBG_LOG("receive cmd:%s", cmd->valuestring);
                    CMD_Confirm_Rsp(msgid->valueint, "ok");
                    ArriveDataProc(cmd->valuestring, desired);
                } else {
                    DBG_LOG("timestamp error:%d", tsdiff);
                    CMD_Confirm_Rsp(msgid->valueint, "ts error");
                }
            } else {
                DBG_LOG("messageid error");
                if (msgid == NULL) {
                    CMD_Confirm_Rsp(0, "messageid error");
                } else {
                    CMD_Confirm_Rsp(msgid->valueint, "messageid error");
                }
            }
        } else {
            DBG_LOG("deviceid error");
            CMD_Confirm_Rsp(msgid->valueint, "deviceid error");
        }
        cJSON_Delete(root);
    }
}

/**
 * ͨѶ�������״�?�ϴ�
 */
void Status_Updata(void) {
    cJSON* desired = NULL;
    desired = cJSON_CreateObject();
    if (desired != NULL) {
        cJSON_AddNumberToObject(desired, "timestamp", RTC_ReadCount());
        cJSON_AddStringToObject(desired, "ip", WorkData.MQTT_Server);
        cJSON_AddNumberToObject(desired, "port", WorkData.MQTT_Port);
        cJSON_AddNumberToObject(desired, "heartbeat", WorkData.MQTT_PingInvt);
        CMD_Updata("CMD-102", desired);
    }
}

/**
 * ��Ϣ����
 *
 * @param desired ���յ�����Ϣ����
 * @return ����ִ�н��?
 */
static  void ArriveDataProc(char* cmd, cJSON* desired) {
    cJSON* child = NULL;
    uint8_t save = 0;
    if (STR_EQUAL(cmd, "CMD-01")) {
        child = cJSON_GetObjectItem(desired, "devicereset");
        if (child != NULL && child->type == cJSON_True) {
            NVIC_SystemReset();
        }
        child = cJSON_GetObjectItem(desired, "devicefactoryreset");
        if (child != NULL && child->type == cJSON_True) {
            WorkData.StockCount = 0;
            WorkData.StockMax = UMBRELLA_STOCK_DEF;
            strcpy(WorkData.MQTT_Server, SERVER_DEF);
            WorkData.MQTT_Port = PORT_DEF;
            WorkData.MQTT_PingInvt = HEARTBEAT_DEF;
            WorkData.StoreLog_In = 0;
            WorkData.StoreLog_Report = 0;
            WorkData_Update();
        }
        child = cJSON_GetObjectItem(desired, "deviceparamget");
        if (child != NULL && child->type == cJSON_True) {
            Status_Updata();
        }
    } else if (STR_EQUAL(cmd, "CMD-02")) {
        child = cJSON_GetObjectItem(desired, "timestamp");
        if (child != NULL && child->type == cJSON_Number) {
            timeRTC_t time;
            RTC_TickToTime(child->valueint, &time);
            RTC_SetTime(&time);
        }
        child = cJSON_GetObjectItem(desired, "ip");
        if (child != NULL && child->type == cJSON_String
                && !STR_EQUAL(WorkData.MQTT_Server, child->valuestring)) {
            strcpy(WorkData.MQTT_Server, child->valuestring);
            save++;
        }
        child = cJSON_GetObjectItem(desired, "port");
        if (child != NULL && child->type == cJSON_Number
                && WorkData.MQTT_Port != child->valueint) {
            WorkData.MQTT_Port = child->valueint;
            save++;
        }
        child = cJSON_GetObjectItem(desired, "heartbeat");
        if (child != NULL && child->type == cJSON_Number
                && WorkData.MQTT_PingInvt != child->valueint) {
            WorkData.MQTT_PingInvt = child->valueint;
            save++;
        }
        child = cJSON_GetObjectItem(desired, "stockmax");
        if (child != NULL && child->type == cJSON_Number
                && WorkData.StockMax != child->valueint) {
            WorkData.StockMax = child->valueint;
            save++;
        }
        if (save > 0) {
            WorkData_Update();
        }
    }
    /*��ɡ*/
    else if (STR_EQUAL(cmd, "CMD-03")) {
        child = cJSON_GetObjectItem(desired, "Umbrella");
        if (child != NULL && child->type == cJSON_String) {
            if (STR_EQUAL(child->valuestring, "Borrow")) {
                Borrow_Action();
            }
        }
    }
}

/**
 * �ϴ�����
 *
 * @param ordermsgid ���е���ϢID
 * @param ret        ִ�еĽ��?
 * @return ��������ϴ�����ִ�н��
 */
static BOOL CMD_Confirm_Rsp(uint32_t msgid, char* ret) {
    BOOL r = FALSE;
    cJSON* bodydesired;
    bodydesired = cJSON_CreateObject();
    cJSON_AddNumberToObject(bodydesired, "messageid", msgid);
    cJSON_AddStringToObject(bodydesired, "ret", ret);
    r = CMD_Updata("CMD-99", bodydesired);
    return r;
}

/**
 * ��������
 * @param argc ����������
 * @param argv �����б�
 */
static void process_Console(int argc, char* argv[]) {
    cJSON* desired = NULL;
    argv++;
    argc--;
    if (ARGV_EQUAL("receive")) {
        ArrivePath((uint8_t*)argv[1], strlen(argv[1]));
    } else if (ARGV_EQUAL("send")) {
        desired = cJSON_Parse((const char*)argv[2]);
        if (desired != NULL) {
            CMD_Updata(argv[1], desired);
        }
    } else if (ARGV_EQUAL("statusup")) {
        DBG_LOG("Status_Updata OK.");
        Status_Updata();
    }
}
