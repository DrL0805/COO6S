#ifndef GSENSORDRIVE_H_
#define GSENSORDRIVE_H_

#define XHPL        (0x00)      // R
#define XHPH        (0x01)      // R
#define YHPL        (0x02)      // R
#define YHPH        (0x03)      // R
#define ZHPL        (0x04)      // R
#define ZHPH        (0x05)      // R
#define XOUTL       (0x06)      // R
#define XOUTH       (0x07)      // R
#define YOUTL       (0x08)      // R
#define YOUTH       (0x09)      // R
#define ZOUTL       (0x0A)      // R
#define ZOUTH       (0x0B)      // R
#define COTR        (0x0C)      // R

#define WHO_AM_I    (0x0F)      // RW
#define TSCP        (0x10)      // R
#define TSPP        (0x11)      // R
#define INS1        (0x12)      // R
#define INS2        (0x13)      // R
#define INS3        (0x14)      // R
#define STAT        (0x15)      // R

#define INT_REL     (0x17)      // R
#define CNTL1       (0x18)      // RW
#define CNTL2       (0x19)      // RW
#define CNTL3       (0x1A)      // RW
#define ODCNTL      (0x1B)      // RW
#define INC1        (0x1C)      // RW
#define INC2        (0x1D)      // RW
#define INC3        (0x1E)      // RW
#define INC4        (0x1F)      // RW
#define INC5        (0x20)      // RW
#define INC6        (0x21)      // RW
#define TILT_TIMER  (0x22)      // RW
#define WUFC        (0x23)      // RW
#define TDTRC       (0x24)      // RW
#define TDTC        (0x25)      // RW
#define TTH         (0x26)      // RW
#define TTL         (0x27)      // RW
#define FTD         (0x28)      // RW
#define STD         (0x29)      // RW
#define TLT         (0x2A)      // RW
#define TWS         (0x2B)      // RW

#define ATH         (0x30)      // RW
#define TILT_ANGLE_LL   (0x32)  // RW
#define TILT_ANGLE_HL   (0x33)  // RW
#define HYST_SET    (0x34)      // RW
#define LP_CNTL     (0x35)      // RW

#define BUF_CNTL1   (0x3A)      // RW
#define BUF_CNTL2   (0x3B)      // RW
#define BUF_STATUS_1    (0x3C)      // R
#define BUF_STATUS_2    (0x3D)      // R
#define BUF_CLEAR   (0x3E)      // W
#define BUF_READ    (0x3F)      // R
#define SELF_TEST   (0x60)      // RW

#define READ(x) (x|0x80)
#define WRITE(x) (x&0x7f)

#define BIN2CHAR(a,b,c,d,e,f,g,h) (((a&1)<<7)|((b&1)<<6)|((c&1)<<5)|((d&1)<<4)|((e&1)<<3)|((f&1)<<2)|((g&1)<<1)|(h&1))

#pragma pack(1)

typedef struct
{
    char x;
    char y;
    char z;
    char len;
}sAXIS;

#pragma pack()

unsigned char GetGsensorSamplingRateTpy(void);

unsigned char  UpdataGsensorData(void);
void GsensorSwitch(unsigned char tpy);

void GsensorInit(void);

void g_sensor_start(void);

void g_sensor_stop(void);


#endif /* GSENSORDRIVE_H_ */
