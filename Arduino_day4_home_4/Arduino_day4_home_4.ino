/*
  이름 : 이동준, 박준우
  날짜 : 220616
  내용 : Exercise 4 - 통합 모니터링 제어 시스템
        1. preset. IoT 스마트팜 v2로 변경
        2. 위젯 아이디에 따라 장치 연결 후 모니터링/제어 가능하게 연동
        3. 수동/자동 제어 위젯에 따른 장치 컨트롤 구현
*/

/*  라이브 선언 및 Port(pin) 번호 설정   */ 
#include <VitconBrokerComm.h> 
using namespace vitcon; 

//DHT (온도/습도) 
#include "DHT.h"        // DHT 사용 
#define DHTPIN A1       // 온/습도 - A1 pin
#define DHTTYPE DHT22   // DHT22 버전을 사용하겠다. 
DHT dht(DHTPIN, DHTTYPE); // 핀 = A1 , 모델 = DHT22

//SOIL-LINK (토양 습도)
#define SOILHUMI A6     // 토양 습도 - A6 pin

//PUMP (펌프)
#define PUMP 16

//LED (조명)
#define LAMP 17

//PWM (펜)
#include <SoftPWM.h>    // DC 모터 사용
SOFTPWM_DEFINE_CHANNEL(A3); // DC 모터 - A3 pin
/*---*/

/* 전역 변수 선언 */
float temp;
float humi;
int soil;

bool device; //장치 제어
bool lamp_timer; //램프 타이머
bool fan; // 펜
bool pump; // 펌프
bool lamp; // 조명
bool bool_hour; 
bool bool_minute; 

int hour = 0;
int minute = 0;

uint32_t TimeSum = 0; 
uint32_t TimeCompare; 
uint32_t TimePushDelay = 0; 
uint32_t TimerStartTime = 0; 
/*---*/

/* 함수 선언 */
void def_device(bool val) { device = val; }
void def_lamp_timer (bool val)  { lamp_timer = val;}
void def_fan(bool val)    { fan  = val; }
void def_pump(bool val)   { pump = val;}
void def_lamp(bool val)   { lamp = val;}

void def_Reset(bool val) { 
  if (!lamp_timer && val) { 
    hour = 0; 
    minute = 0; 
  } 
} 
void def_hour(bool val) { bool_hour = val; }
void def_minute(bool val) { bool_minute = val; }
/*---*/


/* Vitcom 위젯 변수명 선언*/
// num 1~2
IOTItemBin vit_device_status;
IOTItemBin vit_device(def_device);          //장치제어
IOTItemBin vit_lamp_timer_status;
IOTItemBin vit_lamp_timer(def_lamp_timer);  //조명 타이머 설정

// num 6~8 //led는 status값을 반환하여 작동 
IOTItemBin vit_fan_status;
IOTItemBin vit_fan(def_fan);                //펜 on/off
IOTItemBin vit_pump_status;
IOTItemBin vit_pump(def_pump);              //펌프 on/off
IOTItemBin vit_lamp_status;
IOTItemBin vit_lamp(def_lamp);              //램프 on/off

// num 9~11
IOTItemBin vit_hour_up(def_hour);           //시간 더하기
IOTItemBin vit_minute_up(def_minute);       //분 더하기
IOTItemBin vit_reset(def_Reset);            //0으로 초기화

// num 12~16
IOTItemInt to_vit_hour;
IOTItemInt to_vit_minute;
IOTItemFlo to_vit_temp;
IOTItemFlo to_vit_humi;
IOTItemInt to_vit_soil;
/* --- */


/* vitcom index를 아두이노에서 제어 가능하게 변경  */ 
#define ITEM_COUNT 18
IOTItem *items[ITEM_COUNT] = { 
      &vit_device_status,      &vit_device,
      &vit_lamp_timer_status,  &vit_lamp_timer,
      &vit_fan_status,         &vit_fan,
      &vit_pump_status,        &vit_pump,
      &vit_lamp_status,        &vit_lamp,
      &vit_hour_up,            &vit_minute_up,  &vit_reset,
      
      //데이터 전송
      &to_vit_hour,            &to_vit_minute,
      &to_vit_temp,            &to_vit_humi,    &to_vit_soil
      }; 

const char device_id[] = "c1915a33743a1b5d475211f0d195d85a"; // Change device_id to yours 
BrokerComm comm(&Serial, device_id, items, ITEM_COUNT); 

/* 메인 */
void setup() {
  Serial.begin(250000); 
  comm.SetInterval(200); 
  
  dht.begin();
  SoftPWM.begin(490);
  pinMode(SOILHUMI,INPUT);
  pinMode(PUMP, OUTPUT);
  pinMode(LAMP, OUTPUT);
 
}

/* 동작 */
void loop() {
  humi = dht.readHumidity();
  temp = dht.readTemperature();
  soil = map(analogRead(SOILHUMI),0,1023,100,0);  

  //램프 타이머
  InvervalSet(lamp_timer);
    
  //장치 제어
  if(device == true) {
    
    //램프 제어
    if(lamp == true ){ digitalWrite(LAMP,HIGH); }
    else{ digitalWrite(LAMP,LOW); }
    
    //램프 타이머 조절
    if (lamp_timer) { 
      if (TimeCompare % 2 == 0) { digitalWrite(LAMP, HIGH); }
      else if (TimeCompare % 2 == 1) { digitalWrite(LAMP, LOW); } 
    }
    else{
      digitalWrite(LAMP, LOW);
      }

    //펜 제어
    if(fan == true ){ SoftPWM.set(80); }
    else{ SoftPWM.set(0); }

    //펌프 제어
    if(pump == true ){ digitalWrite(PUMP,HIGH); }
    else{ digitalWrite(PUMP,LOW); }


  }
  else{
    lamp = false;
    fan = false;
    pump = false;
    digitalWrite(LAMP,LOW);
    digitalWrite(PUMP,LOW);
    SoftPWM.set(0);  }


  //데이터 전송
  to_vit_temp.Set(temp);
  to_vit_humi.Set(humi);
  to_vit_soil.Set(soil);
  to_vit_hour.Set(hour); 
  to_vit_minute.Set(minute); 
  vit_fan_status.Set(fan);
  vit_pump_status.Set(pump);
  vit_lamp_status.Set(lamp);
  vit_lamp_timer_status.Set(lamp_timer);
  comm.Run(); 
}

/* Interval time set */ 
void InvervalSet(bool lamp_timer){

  //램프가 off여야만 조작가능
  if (!lamp_timer){
  TimeSum = (uint32_t)(hour * 60 + minute) * 60 * 1000; // m * 60 * 1000[ms] 
  TimerStartTime = millis(); 
  
  if (millis() > TimePushDelay + 300) { 
    hour += bool_hour; 
    if (hour >= 24) hour = 0; 
    minute += bool_minute; 
    if (minute >= 60) minute = 0;

    TimePushDelay = millis();
    }
  }

  //on이면 카운트에 맞춰서 다음 실행
  else if (lamp_timer) { 
    TimeCompare = (millis() - TimerStartTime) / TimeSum; 
  }
}
