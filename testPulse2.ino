/* Gowri Somanath.
2014.
Test program for pulseHeartRate.h for using Pulse Heart Rate 
sensor with Arduino
*/
#include "Timer.h"
Timer timer_obj;
int speakerPin=6;

#include "pulseHeartRate.h"


void setup() {
  system("rm *.txt");
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
  setupPulse2(0,5,PULSE_CHECK_TIME_GAP);
  
}//setup


void loop() {
  // put your main code here, to run repeatedly: 
  timer_obj.update();

}


