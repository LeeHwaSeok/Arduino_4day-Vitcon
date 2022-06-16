/*
  이름 : 이동준, 박준우
  날짜 : 220616
  내용 : 연습 3 - IoT 모니터링/제어
  FAN Remote Control

  1) FAN ON/OFF 토글 스위치로 입력받는다. v
  2) 트랙바로 세기를 입력받고 프로그래스 바로 입력된 세기를 출력한다. ㅍ
  3) 타이머 기반 LED 조명 제어와 마찬가지로 시간 조절 기능을 추가한다. 
      (LED와 달리 동작을 먼저 수행하고, 정해 놓은 시간이 지나면 꺼지는 형태)
  4) 세기는 60~100으로 제한을 둔다. v
*/

/* Vitcon 사용하기 */
#include <VitconBrokerComm.h>
using namespace vitcon;

/* 라이브러리 선언 및 Port(PIN) 번호 설정*/
//PWM [dc_ fan]
#include <SoftPWM.h>    // DC 모터 사용
SOFTPWM_DEFINE_CHANNEL(A3); // DC 모터 - A3 pin

/* Vitcon data types
IOTItemBin // binary
IOTItemInt // integer
IOTItemFlo // float
IOTItemStr // String */

/*widget label*/
IOTItemBin Fan(fan_out);  // 빛에서 ON/OFF읽어오기
IOTItemInt Fancotrol1(track_fan);     // 트랙바 값 받아오기
IOTItemInt FanStatus;     // 트랙바 값 주기
IOTItemBin Timer_switch(time_out);

IOTItemInt hour_interval; 
IOTItemInt minute_interval; 

IOTItemBin IntervalHUP(function_Interval_hour); 
IOTItemBin IntervalMUP(function_Interval_minute); 
IOTItemBin IntervaIRST(IntervalReset); 

/* Vitcon 위젯 사용을 위한 선언 */
#define ITEM_COUNT 9

IOTItem *items[ITEM_COUNT] = {&Fan, &Fancotrol1, &FanStatus, &Timer_switch, &hour_interval, &IntervalHUP, &minute_interval, &IntervalMUP, &IntervaIRST}; //서버 위젯 인덱스와 연동
                             /*인덱스 0~8*/
// 변수선언
bool global_fan_status;
bool global_time_switch;
int Fan_control;

bool minute_btn;
bool hour_btn;

int hour = 0;
int minute = 0;
int timeset = 0;

uint32_t TimeSum = 0; 
uint32_t TimeCompare; 

uint32_t TimePushDelay = 0; 
uint32_t TimerStartTime = 0; 


//fan on/off 받아온걸 전역변수에 할당
void fan_out(bool val){
  global_fan_status = val;
}

void time_out(bool val){
  global_time_switch = val;
}
void track_fan(int32_t  val){
  Fan_control = (int)val;
}


void timeset_out(bool val) { timeset = val; }

void function_Interval_hour(bool val) { minute_btn = val; }

void function_Interval_minute(bool val) { hour_btn = val; }

void IntervalReset(bool val) { 
  if (global_time_switch && val) { 
    hour = 0;
    minute = 0;
  } 
} 

/* IOT server communication manager */
const char device_id[]="2f2153c92702abe0bd613a79729a0d74"; //Change device_id to yours
BrokerComm comm(&Serial, device_id, items, ITEM_COUNT);

void setup() {
  Serial.begin(250000);
  comm.SetInterval(200);
  SoftPWM.begin(490);

}

void loop() {
  
  if(global_fan_status == true) {
    SoftPWM.set(Fan_control); // 60~80
    IntervalSet(global_time_switch);
  }
  else {
    SoftPWM.set(0);
    Fan_control = 0;
  }

  FanStatus.Set(Fan_control);
  
  hour_interval.Set(hour); 
  minute_interval.Set(minute); 
  comm.Run(); 

}

void IntervalSet(bool timeset){

  //timeset ture일 경우 
  if (timeset){
  TimeSum = (uint32_t)(hour*24)*(minute*60)*1000; // m * 60 * 1000[ms]
  TimerStartTime = millis(); 
  
  if (millis() > TimePushDelay + 500) { 
    hour += hour_btn; 
    minute += minute_btn; 
    
    if (hour >= 24) {hour = 0;}
    if (minute >= 60) {minute = 0;}

    TimePushDelay = millis();
    }
  }
   
  else if (!timeset) { 
    TimeCompare = (millis() - TimerStartTime) / TimeSum ; 
  }
}
