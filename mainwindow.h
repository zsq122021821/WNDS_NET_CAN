#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include    <QUdpSocket>
#include    <QLabel>

typedef struct common_frame
{
    unsigned short   Frame_header;
    unsigned short   Frame_length;
    unsigned char    Vender_code;
    unsigned char    Device_code;
    unsigned short   Life_signal;
    unsigned short   Target_address;
    unsigned char    Repeat_flag;
    unsigned char    Reply_flag;
    unsigned char    Package_num;
    unsigned char    Obligate[11];
}COMMON_FRAME_STRUCT;//每个包共有部分


typedef struct reply_frame
{
    unsigned short   Frame_header;
    unsigned short   Frame_length;
    unsigned char    Vender_code;
    unsigned char    Device_code;
    unsigned short   Life_signal;
    unsigned char    Right_flag;
    unsigned char    Obligate[7];
}Reply_FRAME_STRUCT;//每个包共有部分



typedef struct TimeStruct
{
    unsigned char    year;
    unsigned char    month;
    unsigned char    day;
    unsigned char    hour;
    unsigned char    minute;
    unsigned char    second;
}MyTimeStruct;//每个包共有部分


typedef union process_data
{
    struct
    {
        COMMON_FRAME_STRUCT Package_header;
        unsigned short   Software_version;
        unsigned char    Sensor_fault;
        unsigned char    x1_value;
        unsigned char    x2_value;
        unsigned char    z1_value;
        unsigned char    z2_value;
        unsigned char    y1_value;
        unsigned char    y2_value;
        unsigned char    AllStatus;
        unsigned char    AlarmStatus;
        unsigned char    Preserve;   //补1个字节与失稳一致
        unsigned char    Talk_state; //控制板A、B的CAN和Net通信状态
        MyTimeStruct     Time_net;//以太网收发的时间
        unsigned char    Train_type;//车型
        unsigned char    Car_number;//车厢号
        unsigned char    Motor_car;//动车、拖车识别标志
        unsigned char    dummy_one;//预留
        unsigned char    dummy_two;//预留
        unsigned char    Input_digit;//30公里数字量输入
        unsigned char    Output_dummy;//数字量输出预留
        unsigned char    Our_data[8];//我方可用作其他程序版本或者硬件版本。
        unsigned char    Marshalling_num1;//列车编组编号（个、十）
        unsigned char    Marshalling_num2;//列车编组编号（个、十）
        unsigned short   Speed_train;//全车速度
        unsigned char    Temperature_outer;//车外温度
        unsigned char    Control_mode;//控车模式
        unsigned char    Valid_bit;//空簧压力或者GPS等有效位
        unsigned char   Air_pressure[2];//空气弹簧压力
        unsigned char    Gps_data[10];//GPS数据
        unsigned char    dummy_thr;//预留
        unsigned short   Sacc_power;//系统累计上电次数
        unsigned int     Sacc_time;//系统累计运行时间
        unsigned int     Sthis_time;//系统本次运行时间
        unsigned char    Module_dummy[40];//失稳模块统一预留
        unsigned char    Self_dummy[126];//各厂家可自行使用预留
        unsigned short   Check_sum;//检验和
    };
    unsigned char pro_data[256];
}PROCESS_DATA_STRUCT;//第一包过程数据部分

typedef union raw_data
{
    struct
    {
        COMMON_FRAME_STRUCT Package_header;
        unsigned short AdcRawData[512];//
    };
    unsigned char pro_data[1048];
}RAW_DATA_STRUCT;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    QLabel  *LabSocketState;
    QUdpSocket  *MudpSocket;//用于与连接的客户端通讯的QTcpSocket
    QUdpSocket  *UudpSocket;//用于与连接的客户端通讯的QTcpSocket
    QHostAddress    groupAddress;//组播地址
    QString getLocalIP();//获取本机IP地址
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
void DispData(PROCESS_DATA_STRUCT *ptProcessData);
void DispDataRaw(RAW_DATA_STRUCT *ptProcessData);

void ReceiveThread(void);
private slots:
//自定义槽函数
    void    onSocketStateChange(QAbstractSocket::SocketState socketState);
    void    onMSocketReadyRead();//读取socket传入的数据
    void    onUSocketReadyRead();//读取socket传入的数据

    void on_actStart_triggered();
    void on_actStop_triggered();
    void on_actStartUdp_triggered();
    void on_actStopUdp_triggered();
    void on_CanConnect_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
