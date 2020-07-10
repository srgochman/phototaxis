#include <MegunoLink.h>
#include <CommandHandler.h>
#include <TCPCommandHandler.h>
#include <ArduinoTimer.h>
#include <CircularBuffer.h>
#include <EEPROMStore.h>
#include <Filter.h>
ExponentialFilter<long> ADCFilter(0, 0);

//#include <MovingAverageFilter.h>
//MovingAverageFilter movingAverageFilter(5);

#include <RBDdimmer.h>
#include <Wire.h>
#include <LIDARLite.h>

#define USE_SERIAL  Serial
#define outputPin  10 
#define zerocross  2 // for boards with CHANGEBLE input pins

dimmerLamp dimmer(outputPin); //initialase port for dimmer for UNO, MEGA, Leonardo...

LIDARLite myLidarLite;

const float min_dist = 100;
const float max_dist = 445;
const float mid_dist = max_dist/1.1;
//const float wall_dist = 500; //set too high if you want the brightness to not be stored
const float min_brightness = 40; //40 functionally off
const float mid_brightness = 50;
const float max_brightness = 89; //functionally brightest
const float m = -1*(max_brightness-min_brightness)/(max_dist-min_dist);
const float m_far = -1*(mid_brightness-min_brightness)/(max_dist-mid_dist);

//float min_flicker = 350;
//float max_flicker = 450;

float temp1_brightness = 40;
float temp2_brightness = 40;
float temp3_brightness = 40;
float stored_brightness = 40;

bool PRESENCE = false;
bool LAST_PRESENCE = false;
unsigned long startTime;
const unsigned long period = 20000;
bool timing=false;
bool periodElapsed = false;
int time_count=0;
int sound_count=0;

void setup() {
  USE_SERIAL.begin(9600);

  myLidarLite.begin(0, true); // Set configuration to default and I2C to 400 kHz
  myLidarLite.configure(0); // Change this number to try out alternate configurations
  
  pinMode(outputPin, OUTPUT);
  pinMode(zerocross, INPUT);
  dimmer.begin(NORMAL_MODE, ON); //dimmer initialization: name.begin(MODE, STATE) 
  dimmer.toggleSettings(0, 100); //Name.toggleSettings(MIN, MAX);
  dimmer.setPower(min_brightness);
  dimmer.setState(ON); // state: dimmer1.setState(ON/OFF);
}

void setDimmerOnPresence(float dist) {
  
  float brightness = m*(dist-min_dist)+max_brightness; //linear dropoff
  float far_brightness = m_far*(dist-mid_dist)+mid_brightness;

  if(dist>max_dist){
    dimmer.setPower(stored_brightness);
    USE_SERIAL.print("A");
  }
  else{
    USE_SERIAL.print("B");
//    if(dist>max_dist){ //max_dist
//      dimmer.setPower(min_brightness); //min_brightness
//    }
    if(dist<min_dist){
      dimmer.setPower(max_brightness-2);
    }
    else if(dist>mid_dist){
      dimmer.setPower(far_brightness);
    }
    else{
      dimmer.setPower(brightness);
    }
    stored_brightness=temp3_brightness;
    temp3_brightness=temp2_brightness;
    temp2_brightness=temp1_brightness;
    temp1_brightness=dimmer.getPower();
  }
  
//  if(LAST_PRESENCE==true && PRESENCE==false){
//    
//  }
}

//void setDimmerToMin() {
//   dimmer.setPower(min_brightness);
////   Serial.println("No one is present and period elapsed, setting dimmer to min now");
//}

//bool presenceOrShortAbsence(bool someonePresentNow) {
//  if (someonePresentNow){
//    // someone is present
//    timing = false;
//    periodElapsed = false;
//
////    Serial.print("Someone is present now");
////    Serial.print("        ");
//    
//    return(true);
//  }
//    
//  // no one present
//  if (periodElapsed){    // no need for timing, no one present and period already elapsed
////    Serial.print("Period elapsed");
////    Serial.print("        ");
//    return(false);
//  }
//    
//  // no one present and period not elapsed, check time
//  if (!timing){
//    // start timing now 
//    startTime = millis();
//    timing = true;
//    return(true);
//  }
//  
//  // timing already started
//  if (millis() - startTime < period){
//    // period not elapsed
////    Serial.print("Timing");
////    Serial.print("        ");
//    return(true);
//  }
//
//  // period just elapsed      
//  timing = false;
//  periodElapsed = true;
//  return(false);        
//}

void loop() {  
  float dist = myLidarLite.distance();
  dist = dist-5.0;
  ADCFilter.Filter(dist);
//  dist = movingAverageFilter.process(dist);

//  float dist=400;

//  if(dist<max_flicker && dist>min_flicker){
//    dist = (max_flicker + min_flicker)/2 - 5;
//  }
  
  if(dist<max_dist){
    PRESENCE = true;
  }
  else{
    PRESENCE = false;
  }
  
  if((dist<2.0 || dist>999.0) && (time_count<15)){
    time_count=time_count+1;
    return;
  }
  time_count=0;
  
  setDimmerOnPresence(dist);

//  USE_SERIAL.print("    ");
//  USE_SERIAL.print(dimmer.getPower());
//  USE_SERIAL.print("    ");
  USE_SERIAL.println(dist);
}
