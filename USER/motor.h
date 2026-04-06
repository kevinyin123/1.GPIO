
#ifndef _motor_h
#define _motor_h 




extern int Puls_cnt;

extern int Puls_cnt_CH1;
extern int Puls_cnt_CH2;

extern int Puls_cnt_CH_REF;  // 参考基准
extern int Puls_cnt_CH_SPD;  // 测速通道


extern int Puls_cnt_Coder;
extern Puls_cnt_CoderBackup;

extern int Puls_cnt_CoderBase;

extern u32  PulsLossMeasCnt_CH1; //通道1采集到的脉冲个数
extern u32  PulsLossMeasCnt_CH2; // 通道2采集到的脉冲个数
extern u32  PulsLossMeasCnt_CH1_backup; //通道1采集到的脉冲个数
extern u32  PulsLossMeasCnt_CH2_backup; // 通道2采集到的脉冲个数
extern u32  PulsLossMeasCnt_REF; // 参考基准通道采集到的脉冲个数
extern uchar  Flag_LossMeas; // 丢数检测状态标志


	

extern  u32 CircleNum;

extern u8  Flag_Get_Dat_Req; 
extern u8   SavePoint; 
extern u16 TimeCntBuf[6];
extern u16 TimeCntBufBackup[6];
extern u16 LowSpeedMeasCnt; 




extern void Delay(uint32_t nCount); 
extern void MotorContor(void);
extern void CalcuSpeed(void);
extern void LossManage(void);
extern void MotorRunManage(void); 





#endif 
