/**
 * @brief 出伞动作.
 * @param argc
 * @param argv
 */
void Borrow_In_Action(void) {
    uint8_t i;
    DBG_LOG("Borrow_In_Action:");
    MOTOR_FORWARD(1);
    /*光电检测，检测伞桶位置是否到位*/
    if(IR_CHECK() == 0) {
        MOTOR_STOP(1);
        DBG_LOG("伞桶到位，准备出伞");
    } else {
        //设备故障
        app_timer_stop(TimerId_Lock);
        LED_IR_OVER_FLASH();
        Stop_Action(1);
        Motor_staus = k_status_ir_stuck;
        DBG_LOG("error of device!");
    }
    /* 打开开关门电机 */
    MOTOR_FORWARD(4);
    if(IF_IS_TOUCH(3) == 0) {
        MOTOR_STOP(4);
        DBG_LOG("开关门电机打开完毕：");
    }
    /* 打开推伞电机 */
    // MOTOR_FORWARD(2);
    if(IF_IS_TOUCH(1) == 0) {
        MOTOR_STOP(2);
        Motor_staus = status_borrow_complite;
        DBG_LOG("推伞电机打开完毕：");
    }
    /**等待伞被拿走，策略为5秒后检测是否有遮挡，同时Frid读取器也检测是否还有伞，
     * 其中任意一项有反应就再提示＂请尽快取伞或不要遮挡出伞口＂，每隔5秒重复一次，
     * 提示5次就报后台柜子故障。若Frid及出伞口红外都没检测到，即认为成功借伞，
     * 就可以关闭出伞口门。若此柜伞已借完，在App上显示此柜不能借伞，客户仍点击借伞按钮，
     * 就语音提示“此柜伞已借完，请到别的柜借伞”。*/
    for(i = 0; i <= 4; i++) {
        nrf_delay_ms(5000);
        if(IR_CHECK() == 1) {
            DBG_LOG("不要遮挡出伞口");
        } else if(RFID_M26_NRF() == 0) {
            DBG_LOG("请尽快取伞");
        } else {
            DBG_LOG("break in for(i=5)");
            break;
        }
    }
    if(i == 4) {
        DBG_LOG("故障");
        //上传数据
    } else {
        DBG_LOG("出伞成功！");
    }
    /*准备回收伞桶*/
    MOTOR_BACK(2);
    if(IF_IS_TOUCH(2)) {
        MOTOR_STOP(2);
        DBG_LOG("回收出伞电机");
    }
    MOTOR_BACK(4);
    if(IF_IS_TOUCH(5)) {
        MOTOR_STOP(4);
        DBG_LOG("开关门电机关闭");
    }
}
