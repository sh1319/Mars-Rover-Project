#include "motor_control.h"
#include "optical_lib.h"

void setup() {
  
  //OPTICAL SENSOR SETUP
  
  pinMode(PIN_SS,OUTPUT);
  pinMode(PIN_MISO,INPUT);
  pinMode(PIN_MOSI,OUTPUT);
  pinMode(PIN_SCK,OUTPUT);
  
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV32);
  SPI.setDataMode(SPI_MODE3);
  SPI.setBitOrder(MSBFIRST);
  
  Serial.begin(9600);

  if(mousecam_init()==-1)
  {
    Serial.println("Mouse cam failed to init");
    while(1);
  } 
  
  //--------------------------------------------------------------------------
  
  //MOTOR CONTROL SETUP
  //************************** Motor Pins Defining **************************//
  pinMode(DIRR, OUTPUT);
  pinMode(DIRL, OUTPUT);
  pinMode(pwmr, OUTPUT);
  pinMode(pwml, OUTPUT);
  digitalWrite(pwmr, LOW);       //setting right motor speed at minimum
  digitalWrite(pwml, LOW);       //setting left motor speed at minimum
  //*******************************************************************//

  //Basic pin setups
  
  noInterrupts(); //disable all interrupts
  pinMode(13, OUTPUT);  //Pin13 is used to time the loops of the controller
  pinMode(3, INPUT_PULLUP); //Pin3 is the input from the Buck/Boost switch
  pinMode(2, INPUT_PULLUP); // Pin 2 is the input from the CL/OL switch
  analogReference(EXTERNAL); // We are using an external analogue reference for the ADC

  // TimerA0 initialization for control-loop interrupt.
  
  TCA0.SINGLE.PER = 999; //
  TCA0.SINGLE.CMP1 = 999; //
  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc | TCA_SINGLE_ENABLE_bm; //16 prescaler, 1M.
  TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP1_bm; 

  // TimerB0 initialization for PWM output
  
  pinMode(6, OUTPUT);
  TCB0.CTRLA=TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm; //62.5kHz
  analogWrite(6,120); 

  interrupts();  //enable interrupts.
  Wire.begin(); // We need this for the i2c comms for the current sensor
  ina219.init(); // this initiates the current sensor
  Wire.setClock(700000); // set the comms speed for i2c
}

byte last_instr = 0;

void loop() {

  //SMPS Voltage Output ---> from sample 
  
  if(loopTrigger) { // This loop is triggered, it wont run unless there is an interrupt
    
    digitalWrite(13, HIGH);   // set pin 13. Pin13 shows the time consumed by each control cycle. It's used for debugging.
    
    // Sample all of the measurements and check which control mode we are in
    sampling();
    CL_mode = digitalRead(3); // input from the OL_CL switch
    Boost_mode = digitalRead(2); // input from the Buck_Boost switch

    if (Boost_mode){
      if (CL_mode) { //Closed Loop Boost
          pwm_modulate(1); // This disables the Boost as we are not using this mode
      }else{ // Open Loop Boost
          pwm_modulate(1); // This disables the Boost as we are not using this mode
      }
    }else{      
      if (CL_mode) { // Closed Loop Buck
          // The closed loop path has a voltage controller cascaded with a current controller. The voltage controller
          // creates a current demand based upon the voltage error. This demand is saturated to give current limiting.
          // The current loop then gives a duty cycle demand based upon the error between demanded current and measured
          // current
          current_limit = 3; // Buck has a higher current limit
          ev = vref - vb;  //voltage error at this time
          cv=pidv(ev);  //voltage pid
          cv=saturation(cv, current_limit, 0); //current demand saturation
          ei=cv-iL; //current error
          closed_loop=pidi(ei);  //current pid
          closed_loop=saturation(closed_loop,0.99,0.01);  //duty_cycle saturation
          pwm_modulate(closed_loop); //pwm modulation
      }else{ // Open Loop Buck
          current_limit = 3; // Buck has a higher current limit
          oc = iL-current_limit; // Calculate the difference between current measurement and current limit
          if ( oc > 0) {
            open_loop=open_loop-0.001; // We are above the current limit so less duty cycle
          } else {
            open_loop=open_loop+0.001; // We are below the current limit so more duty cycle
          }
          open_loop=saturation(open_loop,dutyref,0.02); // saturate the duty cycle at the reference or a min of 0.01
          pwm_modulate(open_loop); // and send it out
      }
    }
    // closed loop control path

    digitalWrite(13, LOW);   // reset pin13.
    loopTrigger = 0;
  }

  //-------------------------------------------------------------------------------------------------------------------------------


  //Rover Instruction Reading and Sensor Output

  //Position Feedback from Optical Sensor
  int val = mousecam_read_reg(ADNS3080_PIXEL_SUM);
  MD md;
  mousecam_read_motion(&md);

  distance_x = md.dx; //convTwosComp(md.dx);
  distance_y = md.dy; //convTwosComp(md.dy);

  total_x1 = (total_x1 + distance_x);
  total_y1 = (total_y1 + distance_y);
  
  total_x = 10*total_x1/157; //Conversion from counts per inch to mm (400 counts per inch)
  total_y = 10*total_y1/157; //Conversion from counts per inch to mm (400 counts per inch)


  //Receiving instruction from Serial
  
  /*  combined instruction of the form "xyz"
      z = 0 - 9 is the motor speed
      y = 0 -> no rotation;   y = 1 -> clockwise with speed z;  y = 2 -> anti-clockwise with speed z
      x = 0 -> no movement;   x = 1 -> move forward;            x = 2 -> move backwards;

      -----> rotation implies no front-back movement and vice-versa
  */
  if (Serial.available())
  {
    last_instr = Serial.read();
    Serial.println(last_instr);
  }
  
  int instr_combined = last_instr;
  int instr_speed = instr_combined % 10;
  instr_combined /= 10;
  
  int instr_rot = instr_combined %10;
  instr_combined /= 10;

  int instr_dir = instr_combined;

  //speed control
  sensorValue2 = 350 + ((1023-350) * instr_speed /9);
  Serial.print('\n');
  Serial.print(instr_speed);
  Serial.print(' ');
  Serial.print(instr_rot);
  Serial.print(' ');
  Serial.print(instr_dir);
  Serial.print('\n');

  if (instr_dir)
  {
    if (instr_dir == 1)
    {
      //front
      DIRRstate = HIGH;
      DIRLstate = LOW;
      digitalWrite(DIRR, DIRRstate);
      digitalWrite(DIRL, DIRLstate); 
      digitalWrite(pwmr, HIGH);
      digitalWrite(pwml, HIGH);
    }
    else
    {
      //back
      DIRRstate = LOW;
      DIRLstate = HIGH;
      digitalWrite(DIRR, DIRRstate);
      digitalWrite(DIRL, DIRLstate); 
      digitalWrite(pwmr, HIGH);
      digitalWrite(pwml, HIGH);
    }
  }
  else
  {
    if (instr_rot)
    {
      if (instr_rot == 1)
      {
        //clockwise
        DIRRstate = HIGH;
        DIRLstate = HIGH;
        digitalWrite(DIRR, DIRRstate);
        digitalWrite(DIRL, DIRLstate); 
        digitalWrite(pwmr, HIGH);
        digitalWrite(pwml, HIGH); 
      }
      else
      {
        //anti-clockwise
        DIRRstate = LOW;
        DIRLstate = LOW;
        digitalWrite(DIRR, DIRRstate);
        digitalWrite(DIRL, DIRLstate); 
        digitalWrite(pwmr, HIGH);
        digitalWrite(pwml, HIGH); 
      }
    }
    else
    {
      //stop motors
      digitalWrite(pwmr, LOW);
      digitalWrite(pwml, LOW);
    }
  }
    
  Serial.println("Distance_x = " + String(total_x));
  Serial.println("Distance_y = " + String(total_y));
  
  //-------------------------------------------------------------------------------------------------------------------------------
}
