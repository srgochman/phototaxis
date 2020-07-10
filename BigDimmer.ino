//Exponential smoothing filter library
#include <MegunoLink.h>
#include <CommandHandler.h>
#include <TCPCommandHandler.h>
#include <ArduinoTimer.h>
#include <CircularBuffer.h>
#include <EEPROMStore.h>
#include <Filter.h>
ExponentialFilter<long> ADCFilter(0, 0);

//LidarLite library
#include <Wire.h>
#include <LIDARLite.h>
LIDARLite myLidarLite;

#define USE_SERIAL Serial
#define outputPin  10 //dimmer into arduino

const float min_dist = 100; //minimum distance to the sensor; closer is uniform maximum brightness
const float max_dist = 445; //threshold slightly closer than back wall, which indicates to store a brightness; set too high if you want the brightness to not be stored
const float mid_dist = max_dist/1.7; //distance at the transitions between linear functions
const float min_brightness = 0;
const float mid_brightness = 100;
const float max_brightness = 255;
const float m = -1*(max_brightness-mid_brightness)/(mid_dist-min_dist); //slope for changing brightness when closer than mid_dist
const float m_far = -1*(mid_brightness-min_brightness)/(max_dist-mid_dist); //slope for changing brightness when farther than mid_dist

float i=0.0; //initialize brightness variable
float temp1_brightness = 0; //variables for remembering the past few distance values which dictate brightness
float temp2_brightness = 0;
float temp3_brightness = 0;
float stored_brightness = 0; //least recent remembered value

//bool PRESENCE = false;

//unsigned long startTime;
//const unsigned long period = 10000;
//bool timing=false;
//bool periodElapsed = false;
//int time_count=0;

void setup() {
  USE_SERIAL.begin(9600);
  myLidarLite.begin(0, true); // Set configuration to default and I2C to 400 kHz
  myLidarLite.configure(0); // Change this number to try out alternate configurations
  pinMode(outputPin, OUTPUT);
}

void setDimmerOnPresence(float dist) {
  
  float brightness = m*(dist-min_dist)+max_brightness; //linear function for changing brightness when closer than mid_dist
  float far_brightness = m_far*(dist-mid_dist)+mid_brightness; //linear function for changing brightness when farther than mid_dist

  if(dist>max_dist){ //store a brightness when no one is present
    i=stored_brightness;
  }
  else{
    if(dist<min_dist){ //brightness is maximum when closer than min_dist
      i=max_brightness;
    }
    else if(dist>mid_dist){ //use 'far_brightness' linear function
      i=far_brightness;
    }
    else{ //use 'brightness' linear function
      i=brightness;
    }

    //stored values get pushed back
    stored_brightness=temp3_brightness;
    temp3_brightness=temp2_brightness;
    temp2_brightness=temp1_brightness;
    temp1_brightness=i;
  }
  analogWrite(outputPin, i);
}

//bool presenceOrShortAbsence(bool someonePresentNow) { //timer function for turning off lights after period
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
  float dist = myLidarLite.distance(); //read distance (in centimeters) from sensor
  dist = dist-5.0; //adjust read distance for accuracy
  ADCFilter.Filter(dist); //filter for smoothing data (reducing noise) for less jumpy quality of light

//  if(dist<max_dist){
//    PRESENCE = true;
//  }
//  else{
//    PRESENCE = false;
  }
  
  if((dist==2.0 || dist>999.0) && (time_count<15)){ //ignore readings if they are outliers (fewer than 15 weird readings)
    time_count=time_count+1;
    return;
  }
  time_count=0;

  setDimmerOnPresence(dist); //run function for dictating brightness
  
  USE_SERIAL.println(dist); //print distance to serial
}
