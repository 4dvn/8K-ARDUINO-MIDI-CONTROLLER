//--------------------------------------------------------------------------------------------------------------------------
// Project:		8K
// Purpose:		Unique midi controller design for sending continuous controller messages
// Developer:	Matt Pfeiffer
//--------------------------------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------------------------------
// Global variables

int last_cc1;				//previous value for potentiometer #1 (0-127)
int last_cc2;				//previous value for potentiometer #2 (0-127)
int last_cc3;				//previous value for potentiometer #3 (0-127)
int last_cc4;				//previous value for potentiometer #4 (0-127)
int last_cc5;				//previous value for potentiometer #5 (0-127)
int last_cc6;				//previous value for potentiometer #6 (0-127)
int last_cc7;				//previous value for potentiometer #7 (0-127)
int last_cc8;				//previous value for potentiometer #8 (0-127)

byte CH1 = 1;				//cc number for potentiometer #1
byte CH2 = 2;				//cc number for potentiometer #2
byte CH3 = 3;				//cc number for potentiometer #3
byte CH4 = 4;				//cc number for potentiometer #4
byte CH5 = 5;				//cc number for potentiometer #5
byte CH6 = 6;				//cc number for potentiometer #6
byte CH7 = 7;				//cc number for potentiometer #7
byte CH8 = 8;				//cc number for potentiometer #8

int midi_threshold = 2; 	// This defines the threshold of change in the cc value before a new cc value is triggered
							// (Notice that a value of 2 will prevent jitter at the expense of resolution)

const int analogPin = A6;	// analog pin used for reading the button presses using a resistor ladder method

int b_state = 0;			//holds current button state
int last_b_state = 0;		//holds previous button state so that changes can be caught

int sevenSegValue = 0;		//holds current seven segment digit value (0-9)
int lastSevenSegValue = 0;	//holds previous seven segment digit value (0-9) so that changes can be caught

//time variables are used to debounce the button presses
long currentTime;			//current time stamp
long pastTime;				//past time stamp


// Common cathode seven segment patterns, from 0 - 9
//                    Arduino pin: 2,3,4,5,6,7,8
byte seven_seg_digits[10][7] = { { 1,1,1,1,1,1,0 },  // = 0
                                 { 0,1,1,0,0,0,0 },  // = 1
                                 { 1,1,0,1,1,0,1 },  // = 2
                                 { 1,1,1,1,0,0,1 },  // = 3
                                 { 0,1,1,0,0,1,1 },  // = 4
                                 { 1,0,1,1,0,1,1 },  // = 5
                                 { 1,0,1,1,1,1,1 },  // = 6
                                 { 1,1,1,0,0,0,0 },  // = 7
                                 { 1,1,1,1,1,1,1 },  // = 8
                                 { 1,1,1,0,0,1,1 }   // = 9
                                };
// For common anode displays, change the 1's to 0's and 0's to 1's

//--------------------------------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------------------------------
// FUNCTION setup: runs once at startup
void setup(){
	Serial1.begin(31250); 
	pinMode(3, OUTPUT);  
	pinMode(2, OUTPUT);
	pinMode(5, OUTPUT);
	pinMode(7, OUTPUT);
	pinMode(15, OUTPUT);
	pinMode(14, OUTPUT);
	pinMode(16, OUTPUT);
	UpdateSevenSegment(sevenSegValue);
}
//--------------------------------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------------------------------
// FUNCTION ControlChange: sends midi continuous controller message through usb
void ControlChange(byte channel, byte control, byte value){
	MIDIEvent event = {0x0B, 0xB0 | channel, control, value};
	MIDIUSB.write(event);
}
//--------------------------------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------------------------------
// FUNCTION UpdateSevenSegment: updates the value of the seven segment display
void UpdateSevenSegment(byte digit) {
	digitalWrite(3, seven_seg_digits[digit][5]);
	digitalWrite(2, seven_seg_digits[digit][6]);
	digitalWrite(5, seven_seg_digits[digit][4]);
	digitalWrite(7, seven_seg_digits[digit][3]);
	digitalWrite(15, seven_seg_digits[digit][0]);
	digitalWrite(14, seven_seg_digits[digit][1]);
	digitalWrite(16, seven_seg_digits[digit][2]);
}
//--------------------------------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------------------------------
// FUNCTION loop: loops infinitely
void loop() {
	pot_read();
	button_read();
}
//--------------------------------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------------------------------
// Function button_read: Updates sevenSegValue on button press (b_state indicates which button was pressed)
void button_read(){

	int pin_value = analogRead(analogPin);
	currentTime = millis();

	// If the pin_value is less than 100 no button is pressed. currentTime and pastTime make sure only 1 button press is registered.
	if ((pin_value > 100) && ((currentTime - pastTime) >= 300)) {
		pastTime = currentTime;
		
		if(pin_value > 400){
		b_state = 2;
		}
		else{
		b_state = 1;
		}

	}
	else{
		b_state = 0;
	}

	// catch if button state has changed
	if(b_state != last_b_state){

		// Do nothing (button not pressed)
		if(b_state==0){
		}
		
		// Button #1 is pressed (increment down - unless at 0)
		else if(b_state==1){
		
			// Subtract one from value if we are greater than 0
			if(sevenSegValue>0){ //subtract one
				sevenSegValue = sevenSegValue - 1;
			}
			
			// Reset to nine if we are at 0
			else{
				sevenSegValue = 9;
			}
		
		}
		
		// Button #2 is pressed (increment up - unless at 9)
		else{
			
			// Add one to value if we are less than 9
			if(sevenSegValue<9){
				sevenSegValue = sevenSegValue + 1;
			}
			
			// Reset to zero if we are at 9
			else{ //send back to zero
				sevenSegValue = 0;
			}
		}
		
		// Write the new value to the 7-segment display
		if(sevenSegValue != lastSevenSegValue){
			UpdateSevenSegment(sevenSegValue);
		}
		
	lastSevenSegValue = sevenSegValue;
	
	}
}
//--------------------------------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------------------------------
// Function pot_read: If any potentiometer positions have changed and it outputs new midi cc value
// inputs: analog signal from potentiometers
// outputs: null (midi cc is sent on potentiometer change)
void pot_read(){

	int pot1 = (1023.0-analogRead(A3));
	int pot2 = (1023.0-analogRead(A2));
	int pot3 = (1023.0-analogRead(A1));
	int pot4 = (1023.0-analogRead(A0));
	int pot5 = (1023.0-analogRead(A10));
	int pot6 = (1023.0-analogRead(A9));
	int pot7 = (1023.0-analogRead(A8));
	int pot8 = (1023.0-analogRead(A7));

	int cc1 = pot1 * (127.0 / 1023.0);
	int cc2 = pot2 * (127.0 / 1023.0);
	int cc3 = pot3 * (127.0 / 1023.0);
	int cc4 = pot4 * (127.0 / 1023.0);
	int cc5 = pot5 * (127.0 / 1023.0);
	int cc6 = pot6 * (127.0 / 1023.0);
	int cc7 = pot7 * (127.0 / 1023.0);
	int cc8 = pot8 * (127.0 / 1023.0);

	CH1 = 1 + sevenSegValue * 8;
	CH2 = 2 + sevenSegValue * 8;
	CH3 = 3 + sevenSegValue * 8;
	CH4 = 4 + sevenSegValue * 8;
	CH5 = 5 + sevenSegValue * 8;
	CH6 = 6 + sevenSegValue * 8;
	CH7 = 7 + sevenSegValue * 8;
	CH8 = 8 + sevenSegValue * 8;

	if(abs(cc1 - last_cc1) >= midi_threshold){
		  ControlChange(0, CH1, cc1);
		  MIDIUSB.flush();
		  last_cc1 = cc1;
	}

	if(abs(cc2 - last_cc2) >= midi_threshold){
		  ControlChange(0, CH2, cc2);
		  MIDIUSB.flush();
		  last_cc2 = cc2;
	}

	if(abs(cc3 - last_cc3) >= midi_threshold){
		  ControlChange(0, CH3, cc3);
		  MIDIUSB.flush();
		  last_cc3 = cc3;
	}

	if(abs(cc4 - last_cc4) >= midi_threshold){
		  ControlChange(0, CH4, cc4);
		  MIDIUSB.flush();
		  last_cc4 = cc4;
	}

	if(abs(cc5 - last_cc5) >= midi_threshold){
		  ControlChange(0, CH5, cc5);
		  MIDIUSB.flush();
		  last_cc5 = cc5;
	}

	if(abs(cc6 - last_cc6) >= midi_threshold){
		  ControlChange(0, CH6, cc6);
		  MIDIUSB.flush();
		  last_cc6 = cc6;
	}

	if(abs(cc7 - last_cc7) >= midi_threshold){
		  ControlChange(0, CH7, cc7);
		  MIDIUSB.flush();
		  last_cc7 = cc7;
	}

	if(abs(cc8 - last_cc8) >= midi_threshold){
		  ControlChange(0, CH8, cc8);
		  MIDIUSB.flush();
		  last_cc8 = cc8;
	}
}
//--------------------------------------------------------------------------------------------------------------------------