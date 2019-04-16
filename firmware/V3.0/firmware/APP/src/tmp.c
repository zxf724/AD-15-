/**
 * @brief ��ɡ����.
 * @param argc
 * @param argv
 */
void Borrow_In_Action(void) {
    uint8_t i;
    DBG_LOG("Borrow_In_Action:");
    MOTOR_FORWARD(1);
    /*����⣬���ɡͰλ���Ƿ�λ*/
    if(IR_CHECK() == 0) {
        MOTOR_STOP(1);
        DBG_LOG("ɡͰ��λ��׼����ɡ");
    } else {
        //�豸����
        app_timer_stop(TimerId_Lock);
        LED_IR_OVER_FLASH();
        Stop_Action(1);
        Motor_staus = k_status_ir_stuck;
        DBG_LOG("error of device!");
    }
    /* �򿪿����ŵ�� */
    MOTOR_FORWARD(4);
    if(IF_IS_TOUCH(3) == 0) {
        MOTOR_STOP(4);
        DBG_LOG("�����ŵ������ϣ�");
    }
    /* ����ɡ��� */
    // MOTOR_FORWARD(2);
    if(IF_IS_TOUCH(1) == 0) {
        MOTOR_STOP(2);
        Motor_staus = status_borrow_complite;
        DBG_LOG("��ɡ�������ϣ�");
    }
    /**�ȴ�ɡ�����ߣ�����Ϊ5������Ƿ����ڵ���ͬʱFrid��ȡ��Ҳ����Ƿ���ɡ��
     * ��������һ���з�Ӧ������ʾ���뾡��ȡɡ��Ҫ�ڵ���ɡ�ڣ���ÿ��5���ظ�һ�Σ�
     * ��ʾ5�ξͱ���̨���ӹ��ϡ���Frid����ɡ�ں��ⶼû��⵽������Ϊ�ɹ���ɡ��
     * �Ϳ��Թرճ�ɡ���š����˹�ɡ�ѽ��꣬��App����ʾ�˹��ܽ�ɡ���ͻ��Ե����ɡ��ť��
     * ��������ʾ���˹�ɡ�ѽ��꣬�뵽��Ĺ��ɡ����*/
    for(i = 0; i <= 4; i++) {
        nrf_delay_ms(5000);
        if(IR_CHECK() == 1) {
            DBG_LOG("��Ҫ�ڵ���ɡ��");
        } else if(RFID_M26_NRF() == 0) {
            DBG_LOG("�뾡��ȡɡ");
        } else {
            DBG_LOG("break in for(i=5)");
            break;
        }
    }
    if(i == 4) {
        DBG_LOG("����");
        //�ϴ�����
    } else {
        DBG_LOG("��ɡ�ɹ���");
    }
    /*׼������ɡͰ*/
    MOTOR_BACK(2);
    if(IF_IS_TOUCH(2)) {
        MOTOR_STOP(2);
        DBG_LOG("���ճ�ɡ���");
    }
    MOTOR_BACK(4);
    if(IF_IS_TOUCH(5)) {
        MOTOR_STOP(4);
        DBG_LOG("�����ŵ���ر�");
    }
}
