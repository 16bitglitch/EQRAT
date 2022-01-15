/*  Version 1
 *  EQRAT_UNO (Eq Ra Tracker for arduino uno/328)
 *  By Charles Gershom 
 *  @charlesgershom 
 *     
 *  Original Built to run a Skywatcher EQ 3-2 for polar aligned RA only sidereal tracking.
 *  This was built in response to not being able to figure out why onstep wasnt tracking right
 *  and because I had no real idea what the drivers were doing, what the gear stuff was, or how 
 *  uno timers worked. 
 *  
 *  The intention is to have a super simple "just turn it on" sidereal motor driver for the mount since I enjoy
 *  finding the stars manually by eye instead of goto. This is a result of a couple nights of head scratching, info reading, 
 *  and learning that delayMicroseconds on arduino overflows and goes wonky above 16383... always read the documents... 
 *  
 *  Parts Used 
 *  1.8degree/200 Step Nema 17 Motors 
 *  DRV8825 Drivers set to 32 Micro Steps
 *  Arduino UNO
 *  
 *  To modify this for your setup, use the following maths
 *  
 *  Seconds_Earth_Rotate = 86164.09053
 *  Earth_Seconds_Per_Degree = Seconds_Earth_Rotate / 360
 *  MicroSteps_Per_Degree = (Mount_Worm_Gear_Ratio * Motor_Gear_Ratio * Steps_Per_Rev * Microsteps) / 360
 *  Step_Delay_Microseconds = (Earth_Seconds_Per_Degree / MicroSteps_Per_Degree) * 1000000
 *  Step_Delay_Timer_Half_Phase = Step_Delay_Microseconds / 2
 *  
 *  use Step_Delay_Timer_Half_Phase in the timer1 OCR1A calculations This alternates the 
 *  step pin Low and High to complete one full step/microstep in two phases
 *  
 *  You can calc the valuee of OCR1A with  
 *  
 *  OCR1A= ( 16000000 / (1/(Step_Delay_Timer_Half_Phase))*1000000)*8) ) -1
 *  
 *  For reasons, see below calcs    
 *  https://www.unitjuggler.com/convert-frequency-from-%C2%B5s(p)-to-Hz.html?val=17256
 *  and 
 *  http://www.arduinoslovakia.eu/application/timer-calculator
   
 *  Example
 *  Earth_Seconds_Per_Degree = 86164.09053 / 360 = 239.34
 *  MicroSteps_Per_Degree = (130 * 3 * 200 * 32) /360 = 6933.333333
 *  Step_Delay_Microseconds (239.34/6933.333333) * 1000000 = 34520.8
 *  Step_Delay_Timer_Half_Phase = 34520.8 / 2 = 17260.4
 *  
 *  OCR1A= ( 16000000 / (1/(17260.4))*1000000)*8) ) -1 = 34519 (rounded to an int)
 *
 *  Some of thie might be wrong or innacurate.. its working "good enough" for me, but thought it might be useful to share
 *  
*/
 
//Debug Stuff
const bool debugEnabled=true;
long lastTime=0;
long currentTime=0;


//Ra Stepper Config
const int dirPin = 5;   
const int stepPin = 2;    

//Ra Stepper State
uint8_t raStepState=LOW;

// Generated with http://www.arduinoslovakia.eu/application/timer-calculator
void setupTimer1() {
  noInterrupts();
  
  // Clear registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  //EDIT THIS TO SET TIMER CORRECTLY  - See above
  OCR1A = 34519;
  //EDIT THIS TO SET TIMER CORRECTLY - See above

  // CTC
  TCCR1B |= (1 << WGM12);
  // Prescaler 8
  TCCR1B |= (1 << CS11);
  // Output Compare Match A Interrupt Enable
  TIMSK1 |= (1 << OCIE1A);
  interrupts();
}

void setup() {  

  //Setup Pins  
  pinMode(stepPin, OUTPUT);   
  pinMode(dirPin, OUTPUT);    
  digitalWrite(dirPin, LOW);   // invert this (HIGH) if wrong direction    

  //Setup and start Timer
  setupTimer1();

  //If checking the timing, enable serial
  if(debugEnabled)
    Serial.begin(9600);
}   

ISR(TIMER1_COMPA_vect) {
  
  //Switch the step pin state
  if(raStepState==LOW){
    raStepState=HIGH;
  }
  else{
    raStepState=LOW;
  }

   //Write to the step pin
   digitalWrite(stepPin, raStepState); 

   //Do some counting if debug is enabled
   if(debugEnabled)
      timerCount();
}

void timerCount(){
  lastTime=currentTime;
  currentTime=micros();
}

void loop() {  
  //Spit out some debug stuff
  if(debugEnabled)
    Serial.println(String((currentTime-lastTime)*2));
}   