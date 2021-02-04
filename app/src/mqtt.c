#include "mqtt.h"
#include "sys.h"
#include "string.h"

char MQTT_ClientID[100];                    //客户端ID
char MQTT_UserName[100];                    //用户民
char MQTT_PassWord[100];                    //秘钥

u8 *mqtt_rxbuf;
u8 *mqtt_txbuf;
u16 mqtt_rxlen;
u16 mqtt_txlen;
u8 _mqtt_rxbuf[256];                        //接收缓存区
u8 _mqtt_txbuf[256];                        //发送缓存区

typedef enum{
    //名字        值           报文流动方向          描述
    
    M_RESERVED1 =0  ,   //      禁止                  保留
    M_CONNECT       ,   //      客户端到服务器         客户端请求连接服务端报文
    M_CONNACK       ,   //      服务器到客户端         连接报文应答
    M_PUBLISH       ,   //      两个方向都允许          发布消息
    M_PUBACK        ,   //      两个方向都允许         QoS 1消息发布确认
    M_PUBREC        ,   //      两个方向都允许	    发布收到（保证交付第一步）
    M_PUBREL        ,   //      两个方向都允许	    发布释放（保证交付第二步）
    M_PUBCOMP       ,   //      两个方向都允许	    QoS 2消息发布完成（保证交互第三步）
    M_SUBSCRIBE     ,   //      客户端到服务端	    客户端订阅请求
    M_SUBACK        ,   //      服务端到客户端	    订阅请求报文确认
    M_UNSUBSCRIBE   ,   //      客户端到服务端	    客户端取消订阅请求
    M_UNSUBACK      ,   //      服务端到客户端	    取消订阅报文确认
    M_PINGREQ       ,   //      客户端到服务端	    心跳请求
    M_PINGRESP      ,   //      服务端到客户端	    心跳响应
    M_DISCONNECT    ,   //      客户端到服务端	    客户端断开连接
    M_RESERVED2     ,   //      禁止	                保留
    
}_typdef_mqtt_message;

//连接成功服务器回应 20 02 00 00
//客户端主动断开连接 e0 00

const u8 parket_connetAck[] = {0x20,0x02,0x00,0x00};
const u8 parket_disconnect[] = {0xe0,0x00};
const u8 parket_heart[] = {0xc0,0x00};
const u8 parket_heart_reply[] ={0xc0,0x00};
const u8 parket_subAck[] ={0x90,0x03};

/*
函数功能: 初始化阿里云物联网服务器的登录参数
*/
 
//密码
//clientId*deviceName*productKey#
// *替换为DeviceName  #替换为ProductKey  加密密钥是DeviceSecret  加密方式是HmacSHA1  
//PassWord明文=clientIdmq2_iotdeviceNamemq2_iotproductKeya1WLC5GuOfx
//hmacsha1加密网站：http://encode.chahuo.com/
//加密的密钥：DeviceSecret


/*
函数功能: 登录服务器
函数返回值: 0表示成功 1表示失败
*/
u8 MQTT_Connect(char *ClientID,char *Username,char *Password)
{
    u8 i,j;
    int ClientIDlen =strlen(ClientID);
    int Usernamelen =strlen(Username);
    int Passwordlen =strlen(Password);
    int Datalen;
    mqtt_txlen =0;
    //可变报头+Payload  每个字段包含两个字节的长度标识
    Datalen = 10+(ClientIDlen+2)+(Usernamelen+2)+(Passwordlen+2);
    //固定报头
	//控制报文类型
    mqtt_txbuf[mqtt_txlen++]=0x10;                                
}

