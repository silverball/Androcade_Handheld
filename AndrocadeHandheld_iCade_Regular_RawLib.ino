
/*************************************************************************
    Silverball Software:
    Androcade Blutooth Controller, firmware revision 1.0
    Copyright (c) 2012 Silverball Software

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    
**************************************************************************/
#include <RN42_HIDRaw_Arduino.h>

RN42_HIDRaw_Arduino Raw = RN42_HIDRaw_Arduino();

//Shared DEFINES
#define DEBUG 0
#define THRESHOLD 100
#define NUM_KP 6
#define NUM_BUTTONS 12

#include "iCade_mappings.h"
#include "regular_hid_mappings.h"

//variable to hold which protocol to run
//pin 13 (LT_SHOULDER)is sampled during setup 
//to determine which protocol to run
byte iCade_or_Regular = 0; //0=iCade 1=Regular

// This array maps a button index to an arduino pin
byte map_pins[NUM_BUTTONS] = {
     2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13};  
  
// debounce counter
char debounce[NUM_BUTTONS];

//------------------------------------------------------------------------------

void setup()
{
   //initialize serial port
    Serial.begin(115200);
    
    // initializes the pins used for the buttons
    for (byte i = 0; i < NUM_BUTTONS; i++)
    {
        byte pin = map_pins[i];
        pinMode(pin, INPUT); // set pin to input
        digitalWrite(pin, HIGH); // turn on pullup resistor
        debounce[i] = 0; // initialize the debounce array
    }
    
    //let everything settle
    delay(500);
    
    //read pin 13 (LT_SHOULDER) if it is pressed use regular HID protocol
    if(digitalRead(13) == LOW)
      iCade_or_Regular = 1;
}



//Regular HID variables
char kp[NUM_KP]; //tracks the current keypresses
char kp_prior[NUM_KP]; //holds the prior keypresses 
byte kp_ix = 0; //index to indicate how many keys are curently pressed
byte kp_ix_prior; //holds the prior index to help determine if things have changed

void read_btns_regular()
{
    memcpy(kp_prior, kp, NUM_KP); // copy the current keypresses to a holder
    kp_ix_prior = kp_ix; // copy the keypress index to holder
    kp_ix = 0; // reset the current number of keys pressed to 0
    
    // iterate through the buttons until you have six to send
    for (byte i = 0; i < NUM_BUTTONS; i++)
    {
        //byte pin = map_pins[i]; 
        if (digitalRead(map_pins[i]) == LOW) // normally pulled-up, buttons connect to ground
        {
            if (debounce[i] < THRESHOLD)
            {
                debounce[i] += 1;
            }
            else
            {
                if (kp_ix < NUM_KP)// if the index has not reached 6 keypresses log that key
                {
                    kp[kp_ix] = map_char[i]; // put the ASCII value into the keypress variable
                    kp_ix += 1; // increase the number of keys currently pressed
                }
                else
                {
                    // if more than 6 keys are pressed break as this is the max
                    // that a raw record for HID BT can handle
                    break;
                }
            }
        }
        else
        {
            // if the key is not pressed reset its debounce variable
            debounce[i] = 0;
        }
    }
    
    // make and transmit an HID raw record, but only if something has changed
    if (kp_ix != kp_ix_prior) //if the indexes are not the same there is more keys to send
    {
        send_reg_raw();
    }
    else
    {
        // if the indexes are the same iterate and compare the old and new keypress
        // variables because one key may have been changed for another even if the
        // index did not change
        for (byte i = 0; i < kp_ix; i++)
        {
            if (kp[i] != kp_prior[i])
            {
                send_reg_raw();
                break;
            }
        }
    }  
  
}

void send_reg_raw(){
    char data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    for (byte i = 0; i < kp_ix; i++)
    {
        data[i] = kp[i];
    }
    Raw.SendKeyboardRaw(data, (byte)0x00);
}


//iCade variables
char kp1[NUM_KP];
char kp2[NUM_KP];
byte kp1_changed = 0;
byte kp2_changed = 0;
byte kp_ix1 = 0;
byte kp_ix2 = 0;


/********************************
* the iCade protocol is unique as you have to send a different key for both
* the key down and key up. 
* Since we are sending both a key down and key up event we have to assure we 
* are scanning every button each time and since there are 12 buttons but only
* 6 allowed for each raw report. We need to split the buttons into 2 groups.
* We also need to track the state of each button so we know if we need to send
* the up key code or the down keycode for that button
*
*** [normally the PC will assume that a key is
* still pressed until it receives a raw HID report without that key code in
* the report, therfore eliminating the need to send key down and key up. Only 
* key down or a blank report is typically sent as in the regular protocol]
********************************/

//iterators
#define FIRST_BTNS 6 //end of the first set
#define SECOND_BTNS 12 //end of the second set

void read_btns_icade()
{
    //reset the indexes
    kp_ix1 = 0;
    kp_ix2 = 0;

    //iterate over the first 6 buttons
    for (byte i = 0; i < FIRST_BTNS; i++)
    {
        if (digitalRead(map_pins[i]) == LOW) // normally pulled-up, buttons connect to ground
        {
            if (debounce[i] < THRESHOLD) // if the button is pressed but it hasn't reached the debounce move on
            {
                debounce[i] += 1;
                continue;
            }
      
            // we are using a multidementional array to hold the current and previous state of the button
            // index 0 of the array indicates the current state of the button and index 1 is the previous 
            // state of the button (1 indicates the button is pressed 0 indicates it is not pressed)
            btn_state_icade[i][0] = 1; // set the current state of the button to pressed
            if(btn_state_icade[i][0] != btn_state_icade[i][1]) // is it the same as the previous state
            {
              kp1_changed = 1; //indicate that the button states have changed for later to know whether or not to send a raw report
              kp1[kp_ix1] = map_char_icade[i][0]; // set the index of this key to the down key
              btn_state_icade[i][1] = btn_state_icade[i][0]; // set the previous state to the current state for next time around
              kp_ix1 += 1; // increase the index for iteration in the send loop
            }               
        }
        else
        {
          //if we are here the button was either released or was not pressed
            btn_state_icade[i][0] = 0; // set the current state of th button to not pressed
            if(btn_state_icade[i][0] != btn_state_icade[i][1]) // compare it with the previous state
            {
              kp1_changed = 1; //indicate that the buton state has changed
              kp1[kp_ix1] = map_char_icade[i][1];//set it to the up key
              btn_state_icade[i][1] = btn_state_icade[i][0]; // copy the new state to the old for next time around
              kp_ix1 += 1; // increase the index for iteration in the send loop
            }
            debounce[i] = 0; // reset the debounce counter as the button was probably released
        }
    }
    
    //same routine as the above except for it is iterating over the second 6 buttons
    for (byte i = 6; i < SECOND_BTNS; i++)
    {
        if (digitalRead(map_pins[i]) == LOW) // normally pulled-up, buttons connect to ground
        {
            if (debounce[i] < THRESHOLD)
            {
                debounce[i] += 1;
                continue;
            }
      
            btn_state_icade[i][0] = 1;
            if(btn_state_icade[i][0] != btn_state_icade[i][1])
            {
              kp2_changed = 1;
              kp2[kp_ix2] = map_char_icade[i][0];//set it to the down key
              btn_state_icade[i][1] = btn_state_icade[i][0];
              kp_ix2 += 1;
            }               
        }
        else
        {
            btn_state_icade[i][0] = 0;
            if(btn_state_icade[i][0] != btn_state_icade[i][1])
            {
              kp2_changed = 1;
              kp2[kp_ix2] = map_char_icade[i][1];//set it to the up key
              btn_state_icade[i][1] = btn_state_icade[i][0];
              kp_ix2 += 1;
            }
            debounce[i] = 0;
        }
    } 
     
    // make and transmit an HID raw record, but only if something has changed
    if(kp1_changed == 1) // has the first set of buttons changed
    {
      Raw.SendKeyboardRaw(kp1, (byte)0x00);
      Raw.SendKeyboardRawBlank();
      kp1_changed = 0;
    }
    
    if(kp2_changed == 1) // has the second set of buttons changed
    {
      Raw.SendKeyboardRaw(kp2, (byte)0x00);
      Raw.SendKeyboardRawBlank();
      kp2_changed = 0;
    }
  
}

/****************
* Main loop continually running
* Checks if it should run the regular
* or the iCade protocol set in the setup
****************/
void loop(void)
{
    if(iCade_or_Regular == 1)
      read_btns_regular();
    else
      read_btns_icade();

}

