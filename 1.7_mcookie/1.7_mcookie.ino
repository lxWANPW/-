#define SSID        "zzz" //改为你的热点名称, 不要有中文
#define PASSWORD    "zc200110"//改为你的WiFi密码Wi-Fi密码
#define DEVICEID    "575396636" //OneNet上的设备ID
#include <Microduino_ColorLED.h> //引用彩灯库

#define LED_NUM 1                 //定义彩灯数量
#define PIN_LED 10                //定义彩灯端口号
ColorLED strip = ColorLED(LED_NUM, PIN_LED);  //定义彩灯数量、彩灯引脚号，色彩编码格式缺省配置（默认为：NEO_GRB + NEO_KHZ800）
String apiKey = "0AJHkkYMUvlop42eApse26AQ42E=";//与你的设备绑定的APIKey

/***/
#define HOST_NAME   "api.heclouds.com"
#define HOST_PORT   (80)
#define INTERVAL_SENSOR   50000             //定义传感器采样时间间隔  597000
#define INTERVAL_NET      50000             //定义发送时间
//传感器部分================================   
#include <Wire.h>                                  //调用库  
#include <ESP8266.h>
#include <I2Cdev.h>                                //调用库  
/*******温湿度*******/
#include <Microduino_SHT2x.h>
/*******光照*******/
#define  sensorPin_1  A0
#define  mic_pin A6
#define pir_pin A2
#define IDLE_TIMEOUT_MS  1000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.
                                   

//WEBSITE     
char buf[10];

#define INTERVAL_sensor 1000
unsigned long sensorlastTime = millis();

float tempOLED, humiOLED, lightnessOLED,sounOLED;


#define INTERVAL_OLED 500

String mCottenData;
String jsonToSend;

//3,传感器值的设置 
float sensor_tem, sensor_hum, sensor_lux,sensor_sound,sensor_pir;   //传感器温度、湿度、光照 ;  
char sensor_tem_c[7], sensor_hum_c[7], sensor_lux_c[7],sensor_sound_c[7],sensor_pir_c[7] ;   //换成char数组传输
#include <SoftwareSerial.h>
#define EspSerial mySerial
#define UARTSPEED  9600
SoftwareSerial mySerial(2, 3); /* RX:D3, TX:D2 */
ESP8266 wifi(&EspSerial);
//ESP8266 wifi(Serial1);                                      //定义一个ESP8266（wifi）的对象
unsigned long net_time1 = millis();                          //数据上传服务器时间
unsigned long sensor_time = millis();                        //传感器采样时间计时器

//int SensorData;                                   //用于存储传感器数据
String postString;                                //用于存储发送数据的字符串
//String jsonToSend;                                //用于存储发送的json格式参数

Tem_Hum_S2 TempMonitor;

void setup(void)     //初始化函数  
{   
    Serial.begin(115200);//串口初始化
  
    strip.begin();      //彩灯初始化
    strip.setBrightness(0);//设置彩灯亮度
    strip.show();      //将彩灯点亮成设置的颜色
    strip.setPixelColor(0, 255, 255, 255); //设置第0号灯，R、G、B的值为255、255、255，即白色
    strip.show();      //将彩灯点亮成设置的颜色    
    Wire.begin();
    Serial.begin(115200);
    while (!Serial); // wait for Leonardo enumeration, others continue immediately
    Serial.print(F("setup begin\r\n"));
    delay(100);
    pinMode(sensorPin_1, INPUT), pinMode(mic_pin, INPUT);
    pinMode(10,OUTPUT);
  WifiInit(EspSerial, UARTSPEED);

  Serial.print(F("FW Version:"));
  Serial.println(wifi.getVersion().c_str());

  if (wifi.setOprToStationSoftAP()) {
    Serial.print(F("to station + softap ok\r\n"));
  } else {
    Serial.print(F("to station + softap err\r\n"));
  }

  if (wifi.joinAP(SSID, PASSWORD)) {
    Serial.print(F("Join AP success\r\n"));

    Serial.print(F("IP:"));
    Serial.println( wifi.getLocalIP().c_str());
  } else {
    Serial.print(F("Join AP failure\r\n"));
  }

  if (wifi.disableMUX()) {
    Serial.print(F("single ok\r\n"));
  } else {
    Serial.print(F("single err\r\n"));
  }

  Serial.print(F("setup end\r\n"));
    
  
}
void loop(void)     //循环函数  
{   
  if (sensor_time > millis())  sensor_time = millis();  
    
  if(millis() - sensor_time > INTERVAL_SENSOR)              //传感器采样时间间隔  
  {  
    getSensorData();                                        //读串口中的传感器数据
    sensor_time = millis();
  }  

    
  if (net_time1 > millis())  net_time1 = millis();
  
  if (millis() - net_time1 > INTERVAL_NET)                  //发送数据时间间隔
  {                
    updateSensorData();        //将数据上传到服务器的函数
    net_time1 = millis();
  }
  
  int n= analogRead(A0);//判断光照并调控灯的开关
  float m=digitalRead(A2);//获取室内是否有人
  float t=TempMonitor.getTemperature();//获取室内温度
  int v=analogRead(A6);//获取室内噪音大小
  if(n>300&&m==1.00)
  {
    strip.begin();      //彩灯初始化
    strip.setBrightness(50);//设置彩灯亮度
    strip.show();      //将彩灯点亮成设置的颜色
    strip.setPixelColor(0, 255, 255, 255); //设置第0号灯，R、G、B的值为255、255、255，即白色
    strip.show();      //将彩灯点亮成设置的颜色

    if(t>25||v>200)
    {
    strip.begin();      //彩灯初始化
    strip.setBrightness(50);//设置彩灯亮度
    strip.show();      //将彩灯点亮成设置的颜色
    strip.setPixelColor(0, 255, 0, 0); //设置第0号灯，R、G、B的值为255、0、0，即红色
    strip.show();      //将彩灯点亮成设置的颜色
    }
  }
  
}

void getSensorData(){  
    sensor_tem = TempMonitor.getTemperature();  
    sensor_hum = TempMonitor.getHumidity();   
    //获取温湿度
    sensor_sound = analogRead(A6);
    //获取音量
   for(int iii=1;iii<=10;iii++){ 
    sensor_pir = digitalRead(A2);
    //获取有无人
    if(sensor_pir==1)Serial.print("someone inside\r\n");
    else Serial.print("no one\r\n");
    delay(100);
    }
    sensor_lux = analogRead(A0);
    //获取光照   
    delay(1000);
    dtostrf(sensor_tem, 2, 1, sensor_tem_c);
    dtostrf(sensor_hum, 2, 1, sensor_hum_c);
    dtostrf(sensor_lux, 3, 1, sensor_lux_c);
    dtostrf(sensor_sound, 3, 1, sensor_sound_c);
    dtostrf(sensor_pir, 2, 1, sensor_pir_c);
}

void updateSensorData() {
  if (wifi.createTCP(HOST_NAME, HOST_PORT)) { //建立TCP连接，如果失败，不能发送该数据
    Serial.print("create tcp ok\r\n");

    jsonToSend="{\"Temperature\":";
    dtostrf(sensor_tem,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";
    
    jsonToSend+=",\"Humidity\":";
    dtostrf(sensor_hum,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";
    
    jsonToSend+=",\"Light\":";
    dtostrf(sensor_lux,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";
    
    jsonToSend+=",\"Noise\":";
    dtostrf(sensor_sound,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";

    jsonToSend+=",\"People\":";
    dtostrf(sensor_pir,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";
    
    jsonToSend+="}";



    postString="POST /devices/";
    postString+=DEVICEID;
    postString+="/datapoints?type=3 HTTP/1.1";
    postString+="\r\n";
    postString+="api-key:";
    postString+=apiKey;
    postString+="\r\n";
    postString+="Host:api.heclouds.com\r\n";
    postString+="Connection:close\r\n";
    postString+="Content-Length:";
    postString+=jsonToSend.length();
    postString+="\r\n";
    postString+="\r\n";
    postString+=jsonToSend;
    postString+="\r\n";
    postString+="\r\n";
    postString+="\r\n";

  const char *postArray = postString.c_str();                 //将str转化为char数组
  Serial.println(postArray);
  wifi.send((const uint8_t*)postArray, strlen(postArray));    //send发送命令，参数必须是这两种格式，尤其是(const uint8_t*)
  Serial.println("send success");   
     if (wifi.releaseTCP()) {                                 //释放TCP连接
        Serial.print("release tcp ok\r\n");
        } 
     else {
        Serial.print("release tcp err\r\n");
        }
      postArray = NULL;                                       //清空数组，等待下次传输数据
  
  } else {
    Serial.print("create tcp err\r\n");
  }
}
