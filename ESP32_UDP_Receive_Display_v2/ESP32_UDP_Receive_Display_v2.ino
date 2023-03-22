//这是一个接收端程序
//本程序仅用来测试串口
//本程序使用的是双线程
//GIF文件名称
#define GIF_FILENAME "/JiNiTai.gif"
#include <Arduino_GFX_Library.h>
#define GFX_BL DF_GFX_BL
/* More dev device declaration: https://github.com/moononournation/Arduino_GFX/wiki/Dev-Device-Declaration */
Arduino_DataBus *bus = new Arduino_ESP32SPI(25 /* DC */, 14 /* CS */, 18 /* SCK */, 23 /* MOSI */, GFX_NOT_DEFINED /* MISO */, VSPI /* spi_num */);  
Arduino_GFX *gfx = new Arduino_ST7789(  bus, 26 /* RST */, 0 /* rotation */, true /* IPS */,  172 /* width */, 320 /* height */,  34/* col offset 1 */, 0 /* row offset 1 */,  0/* col offset 2 */,0/* row offset 2 */);
#include <SPIFFS.h>
#include "GifClass.h"
static GifClass gifClass;




//引用UDP的库
#include <WiFi.h>
#include <WiFiUdp.h>
const char* ssid = "SunrayUDP";
const char* password = "00001598";
WiFiUDP udp;
// "RNTMESH", "MESHpassword", &userScheduler, 5555
#include <Arduino_JSON.h>


//定义马达引脚
#define LED_PWM D10
int ledChannel = 0;
int motorLeve = 0;//申明马达强度变量
//创建一个信号量
//#include <Semaphore.h>
//Semaphore printMutex = Semaphore(1);




void setup()
{

  //初始化震动马达引脚
  ledcSetup(ledChannel, 5000, 8);
  ledcAttachPin(LED_PWM, ledChannel);
  //初始化串口
  Serial.begin(115200); 
  delay(1000);
  

  
 //初始化屏幕
  gfx->begin();
  //gfx->fillScreen(BLACK);
  SPIFFS.begin();
  //设置字体的大小
  gfx->setTextSize(2,2,2/* pixel_margin */);




    //初始化UDP 
   WiFi.begin(ssid, password);  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    gfx->println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");
  udp.begin(1234);  
  Serial.println("UDP server started on port 1234");

  //两个线程
  xTaskCreatePinnedToCore(displayMessage, "displayMessage", 10000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(receiveUDP, "receiveUDP", 10000, NULL, 2, NULL, 0);
}

void loop()
{  
}



void receiveUDP(void * parameter) 
{
    //设置断开时间
  unsigned long previousMillis = 0;
  unsigned long interval = 30000;
  while (true) 
  {
    //检查是否断开
      unsigned long currentMillis = millis();
    // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
    gfx->println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }
    //下面为接收消息程序
    int packetSize = udp.parsePacket();
    if (packetSize) 
    {
      char packetBuffer[packetSize];
      int len = udp.read(packetBuffer, 256);
      if (len > 0) 
       {
         packetBuffer[len] = 0;
        //Serial.print("Received packet: ");
        //Serial.println(packetBuffer);
        //打印到屏幕
        gfx->setCursor(100, 150);
        gfx->setTextColor(random(0xffff));
        gfx->println(packetBuffer);
        //将json转换为int
        JSONVar myObject = JSON.parse(packetBuffer);
        int temp = myObject["motorLeve"];      
        //设置震动马达震动等级
        if(motorLeve!=temp){motorLeve = temp;ledcWrite(ledChannel,motorLeve);}      
       }
      } 
    vTaskDelay(100);  
  }
}
//下面为屏幕显示
void displayMessage(void * parameter) {
  int tempMotorLeve = 0;
  uint16_t temp_Colors[256];
  while (true) {
  //读取文件
  File gifFile = SPIFFS.open(GIF_FILENAME, "r");
    //下面是屏幕程序
  if (!gifFile || gifFile.isDirectory())
  {
    Serial.println(F("ERROR: open gifFile Failed!"));
    gfx->println("ERROR: open gifFile Failed!");
  }
  else
  {
    // read GIF file header
    gd_GIF *gif = gifClass.gd_open_gif(&gifFile);
    if (!gif)
    {
      Serial.println(F("gd_open_gif() failed!"));
    }
    else
    {
      uint8_t *buf = (uint8_t *)malloc(gif->width * gif->height);
      if (!buf)
      {
        Serial.println(F("buf malloc failed!"));
      }
      else
      {
        //居中绘画
        int16_t x = (gfx->width() - gif->width) / 2;
        int16_t y = (gfx->height() - gif->height) / 2;
        
        Serial.println(F("GIF video start"));
        int32_t t_fstart, t_delay = 0, t_real_delay, delay_until;
        int32_t res = 1;
        int32_t duration = 0, remain = 0;        
        //改变颜色
        if(gif->palette->colors!=NULL){
          
          for (int i = 0; i < 256; i++) {
              temp_Colors[i] = gif->palette->colors[i];
               if (temp_Colors[i] +15*motorLeve > 65535) {gif->palette->colors[i] = 65535;}
               else{gif->palette->colors[i] = temp_Colors[i] +15*motorLeve;}
           }
        }
        
        while (res > 0)
        {
          t_fstart = millis();
          t_delay = gif->gce.delay * 10;
          res = gifClass.gd_get_frame(gif, buf);
          if (res < 0)
          {
            Serial.println(F("ERROR: gd_get_frame() failed!"));
            break;
          }
          else if (res > 0)
          {
            //改变颜色
            if(tempMotorLeve!= motorLeve)
            {
              for(int i=0; i<sizeof(gif->palette->colors)/sizeof(gif->palette->colors[0]);i++)
              {
               //Serial.println(gif->palette->colors[i]);
               if (temp_Colors[i] +15*motorLeve > 65535) {gif->palette->colors[i] = 65535;}
               else{
                gif->palette->colors[i] = temp_Colors[i] +15*motorLeve;
               }
               
              };
              Serial.println("changemotorLeve");
              Serial.println(motorLeve);
              tempMotorLeve = motorLeve;          
            }


            
            gfx->drawIndexedBitmap(x, y, buf, gif->palette->colors, gif->width, gif->height);

            t_real_delay = t_delay - (millis() - t_fstart);
            duration += t_delay;
            remain += t_real_delay;
            delay_until = millis() + t_real_delay;
            
            while (millis() < delay_until)
            {
              delay(1);
            }
          }
        }
        Serial.println(F("GIF video end"));
        gifClass.gd_close_gif(gif);
        free(buf);
      }
    }
  }
  vTaskDelay(100);
  }
}
