#include "Arduino.h"
#include "esp_http_client.h"
#include <WiFi.h>
/*********************tcp订阅相关配置**********************/
// const char* ssid = "ChinaUnicom-E5DFNM";           //WIFI名称
// const char* password = "123456789";     //WIFI密码
const char* ssid = "TP-LINK_221";           //WIFI名称
const char* password = "221000221x";     //WIFI密码
String uid = "c8c9d678bd2847e5e270c094286291a5";    //用户私钥，巴法云控制台获取
const char*  topic = "AmbientBrightness";     //主题名字，可在图存储控制台新建
#define server_ip "bemfa.com" //巴法云服务器地址默认即可
#define server_port "8344" //服务器端口，tcp创客云端口8344
/********************************************************/

/*********************tcp初始化**********************/
//最大字节数
#define MAX_PACKETSIZE 512
//设置心跳值60s
#define KEEPALIVEATIME 60*1000
//tcp客户端相关初始化，默认即可
WiFiClient TCPclient;
String TcpClient_Buff = "";//初始化字符串，用于接收服务器发来的数据
unsigned int TcpClient_BuffIndex = 0;
unsigned long TcpClient_preTick = 0;
unsigned long preHeartTick = 0;//心跳
unsigned long preTCPStartTick = 0;//连接
bool preTCPConnected = false;
//相关函数初始化
//连接WIFI
void doWiFiTick();
//TCP初始化连接
void doTCPClientTick();
void startTCPClient();
void sendtoTCPServer(String p);
/********************************************************/

/*
  *发送数据到TCP服务器
 */
void sendtoTCPServer(String p){
    if (!TCPclient.connected()) 
    {
        Serial.println("Client is not readly");
        return;
    }
    TCPclient.print(p);
    preHeartTick = millis();
}

/*
  *初始化和服务器建立连接
*/
void startTCPClient(){
    if(TCPclient.connect(server_ip, atoi(server_port)))
    {
        Serial.print("\nConnected to server:");
        Serial.printf("%s:%d\r\n",server_ip,atoi(server_port));

        String tcpTemp = "";  //初始化字符串
        tcpTemp = "cmd=1&uid="+uid+"&topic="+topic+"\r\n"; //构建订阅指令
        sendtoTCPServer(tcpTemp); //发送订阅指令
        tcpTemp="";//清空
        /*
        //如果需要订阅多个主题，可再次发送订阅指令
        tcpTemp = "cmd=1&uid="+UID+"&topic="+主题2+"\r\n"; //构建订阅指令
        sendtoTCPServer(tcpTemp); //发送订阅指令
        tcpTemp="";//清空
        */

        preTCPConnected = true;
        preHeartTick = millis();
        TCPclient.setNoDelay(true);
    }
    else
    {
        Serial.print("Failed connected to server:");
        Serial.println(server_ip);
        TCPclient.stop();
        preTCPConnected = false;
    }
    preTCPStartTick = millis();
}


/*
  *检查数据，发送心跳
*/
void doTCPClientTick()
{
    //检查是否断开，断开后重连
    if(WiFi.status() != WL_CONNECTED) return;

    if (!TCPclient.connected()) //断开重连
    {
        if(preTCPConnected == true)
        {
            preTCPConnected = false;
            preTCPStartTick = millis();
            Serial.println();
            Serial.println("TCP Client disconnected.");
            TCPclient.stop();
        }
        else if(millis() - preTCPStartTick > 1*1000)//重新连接
            startTCPClient();
    } 
    else
    {
        if (TCPclient.available()) //收数据
        {
            char c =TCPclient.read();
            TcpClient_Buff +=c;
            TcpClient_BuffIndex++;      //防止数组溢出
            TcpClient_preTick = millis();

            if(TcpClient_BuffIndex >= MAX_PACKETSIZE - 1)   //防止数组溢出
            {
                TcpClient_BuffIndex = MAX_PACKETSIZE-2;
                TcpClient_preTick = TcpClient_preTick - 200;
            }
        }

        if(millis() - preHeartTick >= KEEPALIVEATIME)//保持心跳
        {
            preHeartTick = millis();
            Serial.println("--Keep alive:");
            sendtoTCPServer("ping\r\n"); //发送心跳，指令需\r\n结尾，详见接入文档介绍
        }
    }
    if((TcpClient_Buff.length() >= 1) && (millis() - TcpClient_preTick>=200)) //第二个条件是给数据传输时间
    {
        TCPclient.flush();
        Serial.print("Rev string: ");
        TcpClient_Buff.trim(); //去掉首位空格
        Serial.println(TcpClient_Buff); //打印接收到的消息

        String getTopic = "";
        String getMsg = "";
        if(TcpClient_Buff.length() > 15)//注意TcpClient_Buff只是个字符串，在上面开头做了初始化 String TcpClient_Buff = "";
        {
            //此时会收到推送的指令，指令大概为 cmd=2&uid=xxx&topic=mypic&msg=on
            int topicIndex = TcpClient_Buff.indexOf("&topic=")+7; //c语言字符串查找，查找&topic=位置，并移动7位，不懂的可百度c语言字符串查找
            int msgIndex = TcpClient_Buff.indexOf("&msg=");//c语言字符串查找，查找&msg=位置
            getTopic = TcpClient_Buff.substring(topicIndex,msgIndex);//c语言字符串截取，截取到topic,不懂的可百度c语言字符串截取
            getMsg = TcpClient_Buff.substring(msgIndex+5);//c语言字符串截取，截取到消息
            Serial.print("topic:");
            Serial.println(getTopic); //打印截取到的主题值
            Serial.print("msg:");
            Serial.println(getMsg);   //打印截取到的消息值
        }
        if (getTopic == "AmbientBrightness")
        {
            if(getMsg  == "on")
            { 
                digitalWrite(2,HIGH);
            }
            else if(getMsg == "off")
            {
                digitalWrite(2,LOW);
            }
        }
        TcpClient_Buff="";
        TcpClient_BuffIndex = 0;
    }
}


/**************************************************************************
                                 WIFI
***************************************************************************/
/*
  WiFiTick
  检查是否需要初始化WiFi
  检查WiFi是否连接上，若连接成功启动TCP Client
  控制指示灯
*/
void doWiFiTick()
{
    static bool startSTAFlag = false;
    static bool taskStarted = false;
    static uint32_t lastWiFiCheckTick = 0;

    if (!startSTAFlag) 
    {
        startSTAFlag = true;
        WiFi.disconnect();
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
    }

    //未连接1s重连
    if ( WiFi.status() != WL_CONNECTED ) 
    {
        if (millis() - lastWiFiCheckTick > 1000) 
        {
            WiFi.disconnect();
            WiFi.mode(WIFI_STA);
            WiFi.begin(ssid, password);
            lastWiFiCheckTick = millis();
        }
    }
    //连接成功建立
    else 
    {
        if (taskStarted == false) 
        {
            taskStarted = true;
            Serial.print("\r\nGet IP Address: ");
            Serial.println(WiFi.localIP());
            startTCPClient();
        }
    }
}

/********初始化WIFI*********/
bool init_wifi()
{
    Serial.println("\r\nConnecting to: " + String(ssid));
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED ) 
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Internet connected");
    return true;
}

extern "C" void app_main()
{
    initArduino();
    // Do your own thing
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);
    Serial.begin(115200);
    init_wifi();

    while (true)
    {
        doWiFiTick();//检查是否需要初始化WiFi
        doTCPClientTick();//接收数据，发送心跳
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

