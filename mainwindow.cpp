#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "can.h"
#include    <QtNetwork>
#include <synchapi.h>
unsigned long  m_devtype=4;//USBCAN2类型号
unsigned long m_devind=0;
unsigned long m_cannum=0;
int m_connect=0;

QString MainWindow::getLocalIP()
{
    QString hostName=QHostInfo::localHostName();//本地主机名
    QHostInfo   hostInfo=QHostInfo::fromName(hostName);
    QString   localIP="";

    QList<QHostAddress> addList=hostInfo.addresses();//

    if (!addList.isEmpty())
    for (int i=0;i<addList.count();i++)
    {
        QHostAddress aHost=addList.at(i);
        if (QAbstractSocket::IPv4Protocol==aHost.protocol())
        {
            localIP=aHost.toString();
            break;
        }
    }
    return localIP;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    LabSocketState=new QLabel("Socket状态：");//
    LabSocketState->setMinimumWidth(200);
    ui->statusBar->addWidget(LabSocketState);

    QString localIP=getLocalIP();//本地主机名
    this->setWindowTitle(this->windowTitle()+"----本机IP："+localIP);
    ui->UcomboIP->addItem(localIP);

    MudpSocket=new QUdpSocket(this);//用于与连接的客户端通讯的QTcpSocket
//Multicast路由层次，1表示只在同一局域网内
    //组播TTL: 生存时间，每跨1个路由会减1，多播无法跨过大多数路由所以为1
    //默认值是1，表示数据包只能在本地的子网中传送。
    MudpSocket->setSocketOption(QAbstractSocket::MulticastTtlOption,1);
    connect(MudpSocket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this,SLOT(onSocketStateChange(QAbstractSocket::SocketState)));
    onSocketStateChange(MudpSocket->state());
    connect(MudpSocket,SIGNAL(readyRead()),this,SLOT(onMSocketReadyRead()));


    UudpSocket=new QUdpSocket(this);//用于与连接的客户端通讯的QTcpSocket
    connect(UudpSocket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this,SLOT(onSocketStateChange(QAbstractSocket::SocketState)));
    onSocketStateChange(UudpSocket->state());
    connect(UudpSocket,SIGNAL(readyRead()),this,SLOT(onUSocketReadyRead()));

    //connect(&CanThread,SIGNAL(started()),this,SLOT(onthreadA_started()));
    //connect(&CanThread,SIGNAL(finished()),this,SLOT(onthreadA_finished()));

    //connect(&CanThread,SIGNAL(newValue(int,int)),this,SLOT(onthreadA_newValue(int,int)));

    ui->StartStatus1->setStyleSheet("background-color: rgb(0, 255, 0);");
    ui->StartStatus2->setStyleSheet("background-color: rgb(0, 255, 0);");
    ui->RealStatus1->setStyleSheet("background-color: rgb(0, 255, 0);");
    ui->RealStatus2->setStyleSheet("background-color: rgb(0, 255, 0);");


    ui->BoardStatus->setStyleSheet("background-color: rgb(0, 255, 0);");

    ui->xAllSensorStatus->setStyleSheet("background-color: rgb(0, 255, 0);");
    ui->yAllSensorStatus->setStyleSheet("background-color: rgb(0, 255, 0);");
    ui->zAllSensorStatus->setStyleSheet("background-color: rgb(0, 255, 0);");

    ui->x_value1->setStyleSheet("background-color: rgb(0, 255, 0);");
    ui->y_value1->setStyleSheet("background-color: rgb(0, 255, 0);");
    ui->z_value1->setStyleSheet("background-color: rgb(0, 255, 0);");
    ui->x_value2->setStyleSheet("background-color: rgb(0, 255, 0);");
    ui->y_value2->setStyleSheet("background-color: rgb(0, 255, 0);");
    ui->z_value2->setStyleSheet("background-color: rgb(0, 255, 0);");


    QLibrary myLib("ControlCAN.dll");
    if(myLib.load())
    {
        //取得函数地址
        VCI_OpenDevice=(LPVCI_OpenDevice)myLib.resolve("VCI_OpenDevice");
    VCI_CloseDevice=(LPVCI_CloseDevice)myLib.resolve("VCI_CloseDevice");
    VCI_InitCAN=(LPVCI_InitCan)myLib.resolve("VCI_InitCAN");
    VCI_ReadBoardInfo=(LPVCI_ReadBoardInfo)myLib.resolve("VCI_ReadBoardInfo");
        VCI_ReadErrInfo=(LPVCI_ReadErrInfo)myLib.resolve("VCI_ReadErrInfo");
        VCI_ReadCanStatus=(LPVCI_ReadCanStatus)myLib.resolve("VCI_ReadCANStatus");
    VCI_GetReference=(LPVCI_GetReference)myLib.resolve("VCI_GetReference");
        VCI_SetReference=(LPVCI_SetReference)myLib.resolve("VCI_SetReference");
    VCI_GetReceiveNum=(LPVCI_GetReceiveNum)myLib.resolve("VCI_GetReceiveNum");
    VCI_ClearBuffer=(LPVCI_ClearBuffer)myLib.resolve("VCI_ClearBuffer");
    VCI_StartCAN=(LPVCI_StartCAN)myLib.resolve("VCI_StartCAN");
    VCI_ResetCAN=(LPVCI_ResetCAN)myLib.resolve("VCI_ResetCAN");
        VCI_Transmit=(LPVCI_Transmit)myLib.resolve("VCI_Transmit");
        VCI_Receive=(LPVCI_Receive)myLib.resolve("VCI_Receive");
        VCI_GetReference2=(LPVCI_GetReference2)myLib.resolve("VCI_GetReference2");
        VCI_SetReference2=(LPVCI_SetReference2)myLib.resolve("VCI_SetReference2");
        VCI_ResumeConfig=(LPVCI_ResumeConfig)myLib.resolve("VCI_ResumeConfig");
        VCI_ConnectDevice=(LPVCI_ConnectDevice)myLib.resolve("VCI_ConnectDevice");
        VCI_UsbDeviceReset=(LPVCI_UsbDeviceReset)myLib.resolve("VCI_UsbDeviceReset");
    }

}

MainWindow::~MainWindow()
{
    MudpSocket->abort();
    delete MudpSocket;
    delete ui;
}

void MainWindow::onSocketStateChange(QAbstractSocket::SocketState socketState)
{
    switch(socketState)
    {
    case QAbstractSocket::UnconnectedState:
        LabSocketState->setText("scoket状态：UnconnectedState");
        break;
    case QAbstractSocket::HostLookupState:
        LabSocketState->setText("scoket状态：HostLookupState");
        break;
    case QAbstractSocket::ConnectingState:
        LabSocketState->setText("scoket状态：ConnectingState");
        break;

    case QAbstractSocket::ConnectedState:
        LabSocketState->setText("scoket状态：ConnectedState");
        break;

    case QAbstractSocket::BoundState:
        LabSocketState->setText("scoket状态：BoundState");
        break;

    case QAbstractSocket::ClosingState:
        LabSocketState->setText("scoket状态：ClosingState");
        break;

    case QAbstractSocket::ListeningState:
        LabSocketState->setText("scoket状态：ListeningState");
    }
}

void MainWindow::DispData(PROCESS_DATA_STRUCT *ptProcessData)
{
    //显示包头
    if (ptProcessData->Package_header.Frame_header == 0x50AA)
    {
        ui->FreamHeader->setText("AA50");
    }
    else
    {
        ui->FreamHeader->setText("    ");
    }
    //显示帧长度
    if (ptProcessData->Package_header.Frame_length == 0x0001)
    {
        ui->FreamLen->setText("256");
    }
    else
    {
        ui->FreamLen->setText("    ");
    }
    //显示厂家代码
    ui->VenderCode->setCurrentIndex(ptProcessData->Package_header.Vender_code-1);
    //显示设备代码
    ui->DeviceCode->setCurrentIndex((ptProcessData->Package_header.Device_code>>4) -1);
    //显示生命信号
    char cStr[] = {"00000000000"};
    sprintf((char*)cStr,"%d",ptProcessData->Package_header.Life_signal);
    ui->LifeSignal->setText(QString(cStr));

    //显示目标板地址
    uint16_t nTargeAddress = 0;
    nTargeAddress = ptProcessData->Package_header.Target_address >> 8;
    if ((nTargeAddress&0x0001 == 1)&&((nTargeAddress>>1)&0x0001 == 1)&&((nTargeAddress>>2)&0x0001==1))
    {
        ui->TargetAddress->setText("控制板A、B，记录板");
    }
    else
    {
        ui->TargetAddress->clear();
    }

    //显示重发标志
    if (ptProcessData->Package_header.Repeat_flag == 0x55)
    {
        ui->RepeatFlag->setCurrentIndex(0);
    }
    else if (ptProcessData->Package_header.Repeat_flag == 0xAA)
    {
        ui->RepeatFlag->setCurrentIndex(1);
    }
    else
    {
        ui->RepeatFlag->setCurrentIndex(2);
    }

    //显示目标板应答标志
    if (ptProcessData->Package_header.Reply_flag == 0x00)
    {
        ui->ReplyFlag->setCurrentIndex(0);
    }
    else if (ptProcessData->Package_header.Reply_flag == 0x5A)
    {
        ui->ReplyFlag->setCurrentIndex(1);
    }
    else
    {
        ui->ReplyFlag->setCurrentIndex(2);
    }

    //显示总包数
    sprintf((char*)cStr,"%d",ptProcessData->Package_header.Package_num);
    ui->PackageNum->setText(QString(cStr));

    //显示软件版本
    cStr[0] = 'V';
    char tsoftware[2] = {0};
    tsoftware[1] = ptProcessData->Software_version >> 8;
    tsoftware[0] = ptProcessData->Software_version;
    sprintf((char*)(cStr+1),"%02d",tsoftware[0]);
    cStr[3] = '.';
    sprintf((char*)(cStr+4),"%02d",tsoftware[1]);
    ui->SoftwareVersion->setText(QString(cStr));

   //显示传感器1实时检测状态
    if (((ptProcessData->Sensor_fault >> 0)&0x01) == 0x01)
    {
        ui->RealStatus1->setText("实时检测断线");
        ui->RealStatus1->setStyleSheet("background-color: rgb(255, 0, 0);");
    }
    else
    {
        ui->RealStatus1->setText("实时检测正常");
        ui->RealStatus1->setStyleSheet("background-color: rgb(0, 255, 0);");
    }
    //显示传感器1上电检测状态
    if (((ptProcessData->Sensor_fault >> 2)&0x01) == 0x01)
    {
        ui->StartStatus1->setText("上电检测故障");
        ui->StartStatus1->setStyleSheet("background-color: rgb(255, 0, 0);");
    }
    else
    {
        ui->StartStatus1->setText("上电检测正常");
        ui->StartStatus1->setStyleSheet("background-color: rgb(0, 255, 0);");
    }

    //显示传感器2实时检测状态
     if (((ptProcessData->Sensor_fault >> 1)&0x01) == 0x01)
     {
         ui->RealStatus2->setText("实时检测断线");
         ui->RealStatus2->setStyleSheet("background-color: rgb(255, 0, 0);");
     }
     else
     {
         ui->RealStatus2->setText("实时检测正常");
         ui->RealStatus2->setStyleSheet("background-color: rgb(0, 255, 0);");
     }
     //显示传感器2上电检测状态
     if (((ptProcessData->Sensor_fault >> 3)&0x01) == 0x01)
     {
         ui->StartStatus2->setText("上电检测故障");
         ui->StartStatus2->setStyleSheet("background-color: rgb(255, 0, 0);");
     }
     else
     {
         ui->StartStatus2->setText("上电检测正常");
         ui->StartStatus2->setStyleSheet("background-color: rgb(0, 255, 0);");
     }


    if (((ptProcessData->Sensor_fault >> 4)&0x01) == 0x01)
    {
        ui->BoardStatus->setText("故障");
        ui->BoardStatus->setStyleSheet("background-color: rgb(255, 0, 0);");
    }
    else
    {
        ui->BoardStatus->setText("正常");
        ui->BoardStatus->setStyleSheet("background-color: rgb(0, 255, 0);");
    }

    float xyz_value = 0;
    xyz_value = ptProcessData->x1_value*0.1;
    sprintf((char*)(cStr),"%02.1f",xyz_value);
    ui->x_value1->setText(QString(cStr));

    xyz_value = ptProcessData->y1_value*0.1;
    sprintf((char*)(cStr),"%02.1f",xyz_value);
    ui->y_value1->setText(QString(cStr));

    xyz_value = ptProcessData->z1_value*0.1;
    sprintf((char*)(cStr),"%02.1f",xyz_value);
    ui->z_value1->setText(QString(cStr));

    xyz_value = ptProcessData->x2_value*0.1;
    sprintf((char*)(cStr),"%02.1f",xyz_value);
    ui->x_value2->setText(QString(cStr));

    xyz_value = ptProcessData->y2_value*0.1;
    sprintf((char*)(cStr),"%02.1f",xyz_value);
    ui->y_value2->setText(QString(cStr));

    xyz_value = ptProcessData->z2_value*0.1;
    sprintf((char*)(cStr),"%02.1f",xyz_value);
    ui->z_value2->setText(QString(cStr));

    if ((ptProcessData->AllStatus)&0x01 == 1)
    {
        ui->xAllSensorStatus->setText("报警");
        ui->xAllSensorStatus->setStyleSheet("background-color: rgb(255, 0, 0);");
    }
    else
    {
        ui->xAllSensorStatus->setText("正常");
        ui->xAllSensorStatus->setStyleSheet("background-color: rgb(0, 255, 0);");
    }

    if((ptProcessData->AlarmStatus)&0x01 == 1)
    {
        ui->x_value1->setStyleSheet("background-color: rgb(255, 0, 0);");
    }
    else
    {
        ui->x_value1->setStyleSheet("background-color: rgb(0, 255, 0);");
    }

    if((ptProcessData->AlarmStatus >>1)&0x01 == 1)
    {
        ui->x_value2->setStyleSheet("background-color: rgb(255, 0, 0);");
    }
    else
    {
        ui->x_value2->setStyleSheet("background-color: rgb(0, 255, 0);");
    }

    if(ptProcessData->AlarmStatus == 0x11)
    {
        ui->x_value1->setStyleSheet("background-color: rgb(255, 0, 0);");
        ui->x_value2->setStyleSheet("background-color: rgb(255, 0, 0);");
    }
    //显示北京时间
    sprintf((char*)(cStr),"%2d",20);
    sprintf((char*)(cStr+2),"%2d",ptProcessData->Time_net.year);
    cStr[4] = '.';
    sprintf((char*)(cStr+5),"%2d",ptProcessData->Time_net.month);
    cStr[7] = '.';
    sprintf((char*)(cStr+8),"%2d",ptProcessData->Time_net.day);
    cStr[10] = ',';
    sprintf((char*)(cStr+11),"%2d",ptProcessData->Time_net.hour);
    cStr[13] = ':';
    sprintf((char*)(cStr+14),"%2d",ptProcessData->Time_net.minute);
    cStr[16] = ':';
    sprintf((char*)(cStr+17),"%2d",ptProcessData->Time_net.second);
    ui->BeiJingTime->setText(QString(cStr));

    //显示车型
    if(ptProcessData->Train_type == 0x01)
    {
        ui->TrainStyle->setCurrentIndex(0);
    }
    else
    {
        ui->TrainStyle->setCurrentIndex(1);
    }

    //显示车厢号
    sprintf((char*)(cStr),"%2d",ptProcessData->Car_number+1);
    ui->TrainNum->setText(QString(cStr));

    //显示动拖车
    if(ptProcessData->Motor_car == 0x55)
    {
        ui->DongTuo->setCurrentIndex(0);
    }
    else if(ptProcessData->Motor_car == 0xAA)
    {
        ui->DongTuo->setCurrentIndex(1);
    }
    else
    {
        ui->DongTuo->setCurrentIndex(2);
    }
    //显示机车是否大于30Km/h
    if((ptProcessData->Input_digit)&0x01 == 0x01)
    {
        ui->KiloLimit->setCurrentIndex(0);
    }
    else
    {
        ui->KiloLimit->setCurrentIndex(1);
    }

    //显示编组编号
    sprintf((char*)(cStr),"%02x",ptProcessData->Marshalling_num1);
    sprintf((char*)(cStr+2),"%02x",ptProcessData->Marshalling_num2);
    ui->GroupNum->setText(QString(cStr));

    //显示速度
    float fSpeed = 0;
    fSpeed = ptProcessData->Speed_train*0.01;
    sprintf((char*)(cStr),"%05.2f",fSpeed);
    ui->TrainSpeed->setText(QString(cStr));

    //显示温度
    int16_t ucTemp = 0;
    ucTemp = ptProcessData->Temperature_outer - 50;
    if (ucTemp < 0)
    {
        ucTemp = -ucTemp;
        cStr[0] = '-';
    }
    else
    {
        cStr[0] = ' ';
    }
    sprintf((char*)(cStr+1),"%02d",ucTemp);
    ui->TempOut->setText(QString(cStr));

    //显示控制模式
    if(ptProcessData->Control_mode == 0x55)
    {
        ui->ControlMode->setCurrentIndex(0);
    }
    else if(ptProcessData->Control_mode == 0xAA)
    {
        ui->ControlMode->setCurrentIndex(1);
    }
    else
    {
        ui->ControlMode->setCurrentIndex(2);
    }
    //显示GPS状态
    //显示机车是否大于30Km/h

    if((ptProcessData->Valid_bit >>1)&0x01 == 0x01)
    {
        ui->ValidGps->setCurrentIndex(0);
    }
    else
    {
        ui->ValidGps->setCurrentIndex(1);
    }

    //显示经纬度
    if(ptProcessData->Gps_data[4] == 'E')
    {
        ui->JingDuDir->setCurrentIndex(0);
    }
    else
    {
         ui->JingDuDir->setCurrentIndex(1);
    }
    for(uint8_t i = 0;i<4;i++)
    {
        sprintf((char*)(cStr+2*i),"%2x",ptProcessData->Gps_data[i]);
    }    
    ui->JingDu->setText(QString(cStr));

    if(ptProcessData->Gps_data[5] == 'S')
    {
        ui->WeiDuDir->setCurrentIndex(0);
    }
    else
    {
         ui->JingDuDir->setCurrentIndex(1);
    }
    for(uint8_t i = 0;i<4;i++)
    {
        sprintf((char*)(cStr+2*i),"%2x",ptProcessData->Gps_data[i+6]);
    }
    ui->WeiDu->setText(QString(cStr));


    //应答
    Reply_FRAME_STRUCT replay_data = {0};
    replay_data.Frame_header = ptProcessData->Package_header.Frame_header;
    replay_data.Frame_length = 0x1000;
    replay_data.Vender_code = ptProcessData->Package_header.Vender_code;
    replay_data.Device_code = ptProcessData->Package_header.Device_code;
    replay_data.Life_signal = ptProcessData->Package_header.Life_signal;
    replay_data.Right_flag = 0xA5;

    QString     targetIP=ui->UcomboIP->currentText(); //目标IP
    QHostAddress    targetAddr(targetIP);
    quint16     targetPort=ui->UspinPort->value();//目标port

    UudpSocket->writeDatagram((char *)&replay_data.Frame_header,16,targetAddr,targetPort); //发出数据报

}




void MainWindow::onMSocketReadyRead()
{//读取数据报
    while(MudpSocket->hasPendingDatagrams())
    {
        char receivedata[256] = {0};
        QHostAddress    peerAddr;
        quint16 peerPort;
        MudpSocket->readDatagram(receivedata,256,&peerAddr,&peerPort);
        DispData((PROCESS_DATA_STRUCT*)receivedata);
    }
}
void MainWindow::DispDataRaw(RAW_DATA_STRUCT *ptProcessData)
{
    char cStr[] = {"00000000000"};
    char RawcStr[] = {"00000000000"};
    //uint32_t Sum = 0;
    //for (uint16_t i = 0;i < 512;i++)
    //{
    //    Sum += (ptProcessData->AdcRawData[i] << 8) |(ptProcessData->AdcRawData[i]>>8);
    //}
    //uint16_t AdcData = (uint16_t)(Sum/512.0);
    uint16_t AdcData = (ptProcessData->AdcRawData[0] << 8) |(ptProcessData->AdcRawData[0]>>8);
    sprintf((char*)cStr,"%d",ptProcessData->Package_header.Life_signal);
    sprintf((char*)RawcStr,"%4X",AdcData);
    if(ptProcessData->Package_header.Frame_header == 0x51AA)
    {
        ui->x1RawFream->setText("AA51");
        ui->x1RawLife->setText(QString(cStr));
        ui->x1RawData->setText(QString(RawcStr));
    }
    if(ptProcessData->Package_header.Frame_header == 0x52AA)
    {
        ui->z1RawFream->setText("AA52");
        ui->z1RawLife->setText(QString(cStr));
        ui->z1RawData->setText(QString(RawcStr));
    }
    if(ptProcessData->Package_header.Frame_header == 0x53AA)
    {
        ui->y1RawFream->setText("AA53");
        ui->y1RawLife->setText(QString(cStr));
        ui->y1RawData->setText(QString(RawcStr));
    }
    if(ptProcessData->Package_header.Frame_header == 0x54AA)
    {
        ui->x2RawFream->setText("AA54");
        ui->x2RawLife->setText(QString(cStr));
        ui->x2RawData->setText(QString(RawcStr));
    }
    if(ptProcessData->Package_header.Frame_header == 0x55AA)
    {
        ui->z2RawFream->setText("AA55");
        ui->z2RawLife->setText(QString(cStr));
        ui->z2RawData->setText(QString(RawcStr));
    }
    if(ptProcessData->Package_header.Frame_header == 0x56AA)
    {
        ui->y2RawFream->setText("AA56");
        ui->y2RawLife->setText(QString(cStr));
        ui->y2RawData->setText(QString(RawcStr));
    }

    //应答
    Reply_FRAME_STRUCT replay_data = {0};
    replay_data.Frame_header = ptProcessData->Package_header.Frame_header;
    replay_data.Frame_length = 0x1000;
    replay_data.Vender_code = ptProcessData->Package_header.Vender_code;
    replay_data.Device_code = ptProcessData->Package_header.Device_code;
    replay_data.Life_signal = ptProcessData->Package_header.Life_signal;
    replay_data.Right_flag = 0xA5;

    QString     targetIP=ui->UcomboIP->currentText(); //目标IP
    QHostAddress    targetAddr(targetIP);
    quint16     targetPort=ui->UspinPort->value();//目标port

    UudpSocket->writeDatagram((char *)&replay_data.Frame_header,16,targetAddr,targetPort); //发出数据报



}
void MainWindow::onUSocketReadyRead()
{//读取数据报
    while(UudpSocket->hasPendingDatagrams())
    {
        char receivedata[1048] = {0};
        QHostAddress    peerAddr;
        quint16 peerPort;
        UudpSocket->readDatagram(receivedata,256,&peerAddr,&peerPort);
        DispDataRaw((RAW_DATA_STRUCT*)receivedata);
    }
}

void MainWindow::on_actStart_triggered()
{//加入组播
    QString     IP=ui->McomboIP->currentText();
    groupAddress=QHostAddress(IP);//多播组地址
    quint16     groupPort=ui->MspinPort->value();//端口

    if (MudpSocket->bind(QHostAddress::AnyIPv4, groupPort, QUdpSocket::ShareAddress))//先绑定端口
    {
        MudpSocket->joinMulticastGroup(groupAddress); //加入多播组
        ui->actStart->setEnabled(false);
        ui->actStop->setEnabled(true);
        ui->McomboIP->setEnabled(false);
    }
}

void MainWindow::on_actStop_triggered()
{//退出组播
    MudpSocket->leaveMulticastGroup(groupAddress);//退出组播
    MudpSocket->abort(); //解除绑定
    ui->actStart->setEnabled(true);
    ui->actStop->setEnabled(false);
    ui->McomboIP->setEnabled(true);
}

void MainWindow::on_actStartUdp_triggered()
{//绑定端口

    quint16     port=ui->LspinPort->value(); //本机UDP端口
    if (UudpSocket->bind(port))//绑定端口成功
    {
        ui->actStartUdp->setEnabled(false);
        ui->actStopUdp->setEnabled(true);
    }

}

void MainWindow::on_actStopUdp_triggered()
{//解除绑定
    UudpSocket->abort(); //不能解除绑定
    ui->actStartUdp->setEnabled(true);
    ui->actStopUdp->setEnabled(false);
}




void MainWindow::on_CanConnect_clicked()
{

    if(m_connect==1)
   {
           ui->CanConnect->setText("连接");
           m_connect=0;
           //WaitForSingleObject(m_readhandle,2000);
           //m_readhandle=NULL;
           VCI_CloseDevice(m_devtype,m_devind);
           //EnableUI(TRUE);
           return;
   }
   int index = 0;
   int cannum = ui->CanRoute->currentIndex();
   VCI_INIT_CONFIG initconfig;
   //initconfig.AccCode=ui->AccCode->text();
   initconfig.AccCode = 0;
   //initconfig.AccMask=ui->AccMask->text();
   initconfig.AccMask= 0xFFFFFFFF;

   //initconfig.Timing0=ui->Timing0->text();
   initconfig.Timing0 = 0;
   //initconfig.Timing1=ui->Timing0->text();
   initconfig.Timing1 = 0x14;

   initconfig.Filter=ui->FilterType->currentIndex();
   initconfig.Mode=ui->WorkMode->currentIndex();

   if(index>=0&&cannum>=0)
   {
       ui->CanConnect->setText("断开");
       if(VCI_OpenDevice(m_devtype,index,0)==1)
           {

               if(VCI_InitCAN(m_devtype,index,cannum,&initconfig)==1)
                   {

                           m_connect=1;
                           m_devind=0;
                           m_cannum=cannum;
    //threadA.start();
                   }

                   //else
                   //{
                           //ShowMessage("初始化CAN错误");
                   //}
           }
           //else
           //{
                   //ShowMessage("打开端口错误");
          //}

   }

   //EnableUI(FALSE);
}


