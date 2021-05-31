#include "M5CoreInk.h"
#include "icon.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

Ink_Sprite InkPageSprite(&M5.M5Ink);

RTC_TimeTypeDef RTCtime;
RTC_DateTypeDef RTCDate;

//使用するWifiのSSIDとPasswordを記入する
const char* ssid = "SSID";
const char* password = "Password";

//Lineのtokenを記入する
const char* host = "notify-api.line.me";
const char* token = "token";
WiFiClientSecure client;

char timeStrbuff[64];
struct tm tm;
#define JST (3600L * 9)
#define num 3

//画像データの名前
char* pictList[] = {"hoge2.bin","hoge4.bin","hoge5.bin"};

uint8_t LightSensor = 36;
char pictCount = 0;
char distance = 18;
uint16_t tempo = 280;
                       
void drawImageToSprite(int posX,int posY,image_t* imagePtr,Ink_Sprite* sprite){
    sprite->drawBuff(posX, posY,imagePtr->width, imagePtr->height, imagePtr->ptr);
}

void drawDateAndTime( RTC_DateTypeDef *date, RTC_TimeTypeDef *time ){
    int posX = 3;
    drawImageToSprite(posX, 172, &num18x28[date->Month/10%10],&InkPageSprite);  posX += distance;
    drawImageToSprite(posX, 172, &num18x28[date->Month%10],&InkPageSprite);     posX += distance;
    drawImageToSprite(posX, 172, &num18x28[10],&InkPageSprite);                 posX += distance;
    drawImageToSprite(posX, 172, &num18x28[date->Date/10%10 ],&InkPageSprite);  posX += distance;
    drawImageToSprite(posX, 172, &num18x28[date->Date%10 ],&InkPageSprite);     posX += 30;
    
    drawImageToSprite(posX, 172, &num18x28[time->Hours/10],&InkPageSprite);     posX += distance;
    drawImageToSprite(posX, 172, &num18x28[time->Hours%10],&InkPageSprite);     posX += distance;
    drawImageToSprite(posX, 172, &num18x28[11],&InkPageSprite);                 posX += distance;
    drawImageToSprite(posX, 172, &num18x28[time->Minutes/10],&InkPageSprite);   posX += distance;
    drawImageToSprite(posX, 172, &num18x28[time->Minutes%10],&InkPageSprite);
}

void flushTime(){
    M5.rtc.GetTime(&RTCtime);
    M5.rtc.GetData(&RTCDate);
    drawDateAndTime(&RTCDate, &RTCtime);
    InkPageSprite.pushSprite();
}

void setupTime(){
    RTCtime.Hours = tm.tm_hour;
    RTCtime.Minutes = tm.tm_min;
    RTCtime.Seconds = tm.tm_sec;
    M5.rtc.SetTime(&RTCtime);
  
    RTCDate.Year = tm.tm_year + 1900;
    RTCDate.Month = tm.tm_mon + 1;
    RTCDate.Date = tm.tm_mday;
    M5.rtc.SetData(&RTCDate);
}

void sendMessage(){
    if (!client.connect(host, 443)) {
        Serial.println("connection failed");
        return;
    }
    Serial.println("Connected");

    String query = String("message=") + "お客さんがきました！";
    String request = String("") +
            "POST /api/notify HTTP/1.1\r\n" +
            "Host: " + host + "\r\n" +
            "Authorization: Bearer " + token + "\r\n" +
            "Content-Length: " + String(query.length()) +  "\r\n" + 
            "Content-Type: application/x-www-form-urlencoded\r\n\r\n" +
            query + "\r\n";
    client.print(request);
    Serial.println("SEND");

  //受信終了まで待つ 
    while (client.connected()) {
        String line = client.readStringUntil('\n');
        Serial.println(line);
        if (line == "\r") {
            break;
        }
    }

    String line = client.readStringUntil('\n');
    Serial.println(line);
}

uint8_t* getPictUrl(String url){
    HTTPClient http;

    if (WiFi.status() != WL_CONNECTED){
        log_e("Not connected");
        return 0;
    }

    http.begin(url);

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK){
        log_e("HTTP ERROR: %d\n", httpCode);
        http.end();
        return 0;
    }

    size_t size = http.getSize();

    log_d("jpg size = %d Bytes", size);

    WiFiClient *stream = http.getStreamPtr();
    uint8_t *p = (uint8_t*)malloc(size);
    
    if(p == NULL){
        log_e("Memory overflow.");
        return 0;
    }

    log_d("downloading...");
    size_t offset = 0;
    
    while (http.connected()){
        size_t len = stream->available();
        if (!len){
            delay(1);
            continue;
        }
        stream->readBytes(p + offset, len);
        offset += len;
        log_d("%d / %d", offset, size);
        if(offset == size){
            break;
        }
    }

    http.end();
    log_d("done");
    return p;
}

void setup() {
    uint8_t* ret;
    // put your setup code here, to run once:
    M5.begin();

    M5.M5Ink.isInit();
    M5.M5Ink.clear();
    InkPageSprite.creatSprite(0, 0, 200, 200, false);
    
    pinMode(LightSensor,INPUT);
  
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.print(".");
    }
    Serial.println(WiFi.localIP());
    ret = getPictUrl("https://raw.githubusercontent.com/dangoShippo/M5P/main/hoge2.bin");
   
    InkPageSprite.drawBuff(0, 0, 200, 200, ret);
    InkPageSprite.pushSprite();

    free(ret);
    configTime(JST,0,"ntp.nict.jp", "time.google.com","ntp.jst.mfeed.ad.jp");
    if(getLocalTime(&tm)){
      Serial.println("I catch get time");
    }
    setupTime();
    flushTime();
}

void loop() {
  // put your main code here, to run repeatedly:
    if( M5.BtnMID.wasPressed()){
        //ボタンを押すと、表示画像が切り替わる
        uint8_t* ret;
        String hoge = "https://raw.githubusercontent.com/dangoShippo/M5P/main/" + String(pictList[pictCount]);
        ret = getPictUrl(hoge);
        InkPageSprite.drawBuff(0, 0, 200, 200, ret);
        InkPageSprite.pushSprite();
        free(ret);
        if(pictCount < (num-1)){
            pictCount++;
        }else{
            pictCount = 0;
        }
    }
    
    int val = analogRead(LightSensor);
    //光センサーが光を検知したら、スマホにLINEメッセージを送る
    if(val > 150){
        uint8_t* ret;
        Serial.println("Detect Light Sensor");
        sendMessage();
        
        //お客様が来た時の画像を表示
        String hoge = "https://raw.githubusercontent.com/dangoShippo/M5P/main/okyaku.bin";
        ret = getPictUrl(hoge);
        InkPageSprite.drawBuff(0, 0, 200, 200, ret);
        InkPageSprite.pushSprite();
        free(ret);
        
        delay(15000);
        
        //通常時の画像を表示
        hoge = "https://raw.githubusercontent.com/dangoShippo/M5P/main/hoge5.bin";
        ret = getPictUrl(hoge);
        InkPageSprite.drawBuff(0, 0, 200, 200, ret);
        InkPageSprite.pushSprite();
        free(ret);
    }
    flushTime();
    M5.update();
}
