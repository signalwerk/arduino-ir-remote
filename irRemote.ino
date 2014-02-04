
// The PINs for the In-/Output
#define REMOTEPIN 11      // Pin for IR receiver (remote control)
#define LEDPIN 13         // Pin for LED

// the codes returned by getIRkey()
#define MENU 1
#define PLAY_PAUSE 2
#define VOLUME_UP 5
#define VOLUME_DOWN 6
#define REWIND 4
#define FAST_FORWARD 3
#define KEY_DELAY 100

// the keyboard keys sent to the usb-host
#define KEY_MENU KEY_ESC
#define KEY_PLAY_PAUSE 32 // space = 32
#define KEY_VOLUME_UP KEY_UP_ARROW
#define KEY_VOLUME_DOWN KEY_DOWN_ARROW
#define KEY_REWIND KEY_LEFT_ARROW
#define KEY_FAST_FORWARD KEY_RIGHT_ARROW

// pairing options
#define pairingTime 5000  // The time for the pairing during startup (in ms)
#define ledInterval 100   // The interval the LED is blinking during pairing

// The following variables for the remote control feature
int duration;             // Duration of the IR pulse
int mask;                 // Used to decode the remote signals
int c1;                   // Byte 1 of the 32-bit remote command code
int c2;                   // Byte 2 of the 32-bit remote command code
int c3;                   // Byte 3 of the 32-bit remote command code
int c4;                   // Byte 4 of the 32-bit remote command code
int IRkey;                // The unique code (Byte 3) of the remote key
int previousIRkey;        // The previous code (used for repeat)

int currentID = 0;        // the current id of the remote
int pairingID = -1;       // the id of the remote it got from the pairing


void setup() {

    Serial.begin(9600);
    pinMode(REMOTEPIN, INPUT);           // Pin for IR sensor
    pinMode(LEDPIN, OUTPUT);             // PIN for LED

    int ledState = LOW;                  // ledState used to set the LED
    unsigned long bootMillis=millis();   // Stores the time for the startup procedure
    unsigned long ledMillis = millis();  // Stores the time for the led-blink procedure
    unsigned long previousLedMillis = 0;

    digitalWrite(LEDPIN, LOW);
    Serial.println("start pairing");

    // on startup check during 5 sec for push button
    // if pushed = pairing to this remote
    while (millis() - bootMillis < pairingTime) {

        ledMillis = millis();

        if(ledMillis - previousLedMillis > ledInterval) {
            // save the last time you blinked the LED
            previousLedMillis = ledMillis;

            // if the LED is off turn it on and vice-versa:
            if (ledState == LOW) {

                ledState = HIGH;
            } else {

                ledState = LOW;
            }
            // set the LED with the ledState of the variable:
            digitalWrite(LEDPIN, ledState);
        }

        while(digitalRead(REMOTEPIN) == LOW){

            IRkey=getIRkey();

            if (IRkey == MENU) {
                pairingID = currentID;
                bootMillis = 0;
                Serial.println("pairing successful");
            }
        }
    }

    Serial.println("end pairing");

    if (pairingID != -1 ) {
        digitalWrite(LEDPIN, HIGH); // if pairing = led on
    }

    Keyboard.begin();
}


void loop() {

    while (digitalRead(REMOTEPIN) == LOW){

        if ((IRkey = getIRkey()) == 255){
            // Do nothing
        } else {
            if (IRkey==0) {            // Repeat code
                if (previousIRkey == VOLUME_UP || previousIRkey == VOLUME_DOWN) {
                    IRkey=previousIRkey;  // Repeat code, only for specified keys as indicated. Add additional
                    // keys to the "or" comparison in the if() statement if you want them
                    // to repeat
                } else {
                    // Do nothing. No repeat for the rest of the keys
                }
            } else {                    // Not a repeat code, it is a new command
                previousIRkey=IRkey;    // Remember the key in case we want to use the repeat code
            }
        }

        if (pairingID == -1 || pairingID == currentID) {

            switch(IRkey){
                    // case 0 and 255 are "valid" cases from the code, but we do nothing in this switch statement
                case MENU:
                    Serial.println("MENU");
                    Keyboard.press(KEY_MENU);
                    delay(KEY_DELAY);
                    Keyboard.releaseAll();
                    break;

                case PLAY_PAUSE:
                    Serial.println("PLAY_PAUSE");
                    Keyboard.press(KEY_PLAY_PAUSE);
                    delay(KEY_DELAY);
                    Keyboard.releaseAll();
                    break;

                case VOLUME_UP:
                    Serial.println("VOLUME_UP");
                    Keyboard.press(KEY_VOLUME_UP);
                    delay(KEY_DELAY);
                    Keyboard.releaseAll();
                    break;

                case VOLUME_DOWN:
                    Serial.println("VOLUME_DOWN");
                    Keyboard.press(KEY_VOLUME_DOWN);
                    delay(KEY_DELAY);
                    Keyboard.releaseAll();
                    break;

                case REWIND:
                    Serial.println("REWIND");
                    Keyboard.press(KEY_REWIND);
                    delay(KEY_DELAY);
                    Keyboard.releaseAll();
                    break;

                case FAST_FORWARD:
                    Serial.println("FAST_FORWARD");
                    Keyboard.press(KEY_FAST_FORWARD);
                    delay(KEY_DELAY);
                    Keyboard.releaseAll();
                    break;

                default:
                    Serial.println("UPPPS");
                    Serial.println(IRkey);
            } // end switch
        } else {
            if (pairingID != currentID) {
                Serial.println("other remote used!");
            }
        }

    } // End of remote control code
}


/*

The following function returns the code from the Apple Aluminum remote control. The Apple remote is
based on the NEC infrared remote protocol. Of the 32 bits (4 bytes) coded in the protocol, only the
third byte corresponds to the keys. The function also handles errors due to noise (returns 255) and
the repeat code (returs zero)

The Apple remote returns the following codes:

Up key:     238 135 011 089
Down key:   238 135 013 089
Left key:   238 135 008 089
Right key:  238 135 007 089
Center key: 238 135 093 089 followed by 238 135 004 089 (See blog for why there are two codes)
Menu key:   238 135 002 089
Play key:   238 135 094 089 followed by 238 135 004 089

(update) The value of the third byte varies from remote to remote. It turned out that the 8th bit
is not part of the command, so if we only read the 7 most significant bits, the value is the same
for all the remotes, including the white platic remote.

The value for the third byte when we discard the least significant bit is:

Up key:     238 135 005 089
Down key:   238 135 006 089
Left key:   238 135 004 089
Right key:  238 135 003 089
Center key: 238 135 046 089 followed by 238 135 002 089 (See blog for why there are two codes)
Menu key:   238 135 001 089
Play key:   238 135 047 089 followed by 238 135 002 089

More info here: http://hifiduino.wordpress.com/apple-aluminum-remote/

*/

int getIRkey() {

    c1=0;
    c2=0;
    c3=0;
    c4=0;
    duration=1;
    while((duration=pulseIn(REMOTEPIN, HIGH, 15000)) < 2000 && duration!=0)
    {
        // Wait for start pulse
    }
    if (duration == 0)           // This is an error no start or end of pulse
        return(255);             // Use 255 as Error

    else if (duration < 3000)    // This is the repeat
        return (0);              // Use zero as the repeat code

    else if (duration < 5000){   // This is the command get the 4 byte
        mask = 1;
        for (int i = 0; i < 8; i++){                   // get 8 bits
            if(pulseIn(REMOTEPIN, HIGH, 3000)>1000)    // If "1" pulse
                c1 |= mask;                            // Put the "1" in position
            mask <<= 1;                                // shift mask to next bit
        }
        mask = 1;
        for (int i = 0; i < 8; i++){                   // get 8 bits
            if(pulseIn(REMOTEPIN, HIGH, 3000)>1000)    // If "1" pulse
                c2 |= mask;                            // Put the "1" in position
            mask <<= 1;                                // shift mask to next bit
        }
        mask = 1;
        for (int i = 0; i < 8; i++){                   // get 8 bits
            if(pulseIn(REMOTEPIN, HIGH, 3000)>1000)    // If "1" pulse
                c3 |= mask;                            // Put the "1" in position
            mask <<= 1;                                // shift mask to next bit
        }
        mask = 1;
        for (int i = 0; i < 8; i++){                   // get 8 bits
            if(pulseIn(REMOTEPIN, HIGH, 3000)>1000)    // If "1" pulse
                c4 |= mask;                            // Put the "1" in position
            mask <<= 1;                                // shift mask to next bit
        }
        // Serial.println(c1, HEX); //For debugging
        // Serial.println(c2, HEX); //For debugging
        // Serial.println(c3, HEX); //For debugging
        // Serial.println(c4, HEX); //For debugging  -->> differ from remote to remote

        currentID = c4;

        c3 >>= 1;                                      // Discard the least significant bit
        return(c3);
    }
}
