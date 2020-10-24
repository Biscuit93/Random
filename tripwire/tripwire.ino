#define PIN_SENSE_LASER   A6
#define PIN_SENSE_LIGHT   A7
#define PIN_BUTTON_MODE   2
#define PIN_BUTTON_ARM    3
#define PIN_EXTERN        4
#define PIN_LED_INACTIVE  5
#define PIN_LED_EXTERN    6
#define PIN_LED_AUTORESET 7
#define PIN_LED_ACTIVE    9
#define PIN_BUZZER        10
#define PIN_LASER         11
#define PIN_LED_BUZZER    12

#define LIGHT_THRESHOLD   10
#define SLEEP_TIME        50
#define RESET_TIME        1000 / SLEEP_TIME

enum STATES { STATE_INACTIVE, STATE_ACTIVE, STATE_TRIPPED };
enum MODES  { MODE_DISABLED = 0, MODE_ALARM = 1, MODE_EXTERN = 2, MODE_AUTORESET = 4, MODE_RESET = 8 };

int state;
int mode;
int pulse;
int reset;

int  laserTripped       ( );
void toggleState        ( );
void toggleMode         ( );
void setModeLEDs        ( );
void gotoInactiveState  ( );
void gotoActiveState    ( );
void gotoTrippedState   ( );
void trippedStateAction ( );
void readSensors        ( );



void setup() 
{
  Serial.begin( 9600 );

  //Photoresistor pins set to input.
  pinMode( PIN_SENSE_LASER,   INPUT  );
  pinMode( PIN_SENSE_LIGHT,   INPUT  );

  //All others set to output.
  pinMode( PIN_EXTERN,        OUTPUT );
  pinMode( PIN_BUZZER,        OUTPUT );
  pinMode( PIN_LASER,         OUTPUT );
  
  pinMode( PIN_LED_INACTIVE,  OUTPUT );
  pinMode( PIN_LED_ACTIVE,    OUTPUT );
  pinMode( PIN_LED_AUTORESET, OUTPUT );
  pinMode( PIN_LED_EXTERN,    OUTPUT );
  pinMode( PIN_LED_BUZZER,    OUTPUT );

  //Attach interrupts to button pins.
  attachInterrupt( digitalPinToInterrupt( PIN_BUTTON_ARM  ), toggleState, RISING );
  attachInterrupt( digitalPinToInterrupt( PIN_BUTTON_MODE ), toggleMode,  RISING );

  //Set to default state.
  gotoInactiveState( );

  //Set to default mode.
  mode = MODE_RESET;
  toggleMode( );

  pulse = 0;
}

void loop() 
{
  pulse = !pulse;

  noInterrupts( );

  //If active and tripped, go to tripped state. If inactive, active LED indicates if laser is tripped.
  if ( laserTripped( ) )
  {
    if ( state == STATE_ACTIVE )
      gotoTrippedState( );
    if ( state == STATE_INACTIVE )
      digitalWrite( PIN_LED_ACTIVE, HIGH );
  }
  else
  {
    if ( state == STATE_INACTIVE )
      digitalWrite( PIN_LED_ACTIVE, LOW  );
  }

  if ( state == STATE_TRIPPED )
    trippedStateAction( );

  interrupts( );

  //readSensors( );

  delay( SLEEP_TIME );
}

//Returns true if laser photoresistor reads at or below ambient photoresistor reading.
int laserTripped ( )
{
  return ( analogRead( PIN_SENSE_LIGHT ) + LIGHT_THRESHOLD ) > analogRead( PIN_SENSE_LASER );
}

//Cycle through the states when the button is pressed.
void toggleState ( )
{
  switch ( state )
  {
    //If inactive, set to active.
    case STATE_INACTIVE:
      gotoActiveState( );
    break;

    //If active or tripped, set to inactive.
    case STATE_ACTIVE:
    case STATE_TRIPPED:
    default:
      gotoInactiveState( );
    break;
  }
}

//Cycle through the modes when the button is pressed and inactive.
void toggleMode ( )
{
  if ( state == STATE_INACTIVE )
  {
    if ( ++mode >= MODE_RESET )
      mode = MODE_DISABLED;

    setModeLEDs( );  
  }
}

//Set the mode LEDs according to the mode variable.
void setModeLEDs ( )
{
  digitalWrite( PIN_LED_BUZZER,    LOW );
  digitalWrite( PIN_LED_EXTERN,    LOW );
  digitalWrite( PIN_LED_AUTORESET, LOW );

  if ( mode & MODE_ALARM )
    digitalWrite( PIN_LED_BUZZER,    HIGH );

  if ( mode & MODE_EXTERN )
    digitalWrite( PIN_LED_EXTERN,    HIGH );

  if ( mode & MODE_AUTORESET )
    digitalWrite( PIN_LED_AUTORESET, HIGH );
}

//Set the state to inactive and set the pins accordingly.
void gotoInactiveState ( )
{
  state = STATE_INACTIVE;
  setModeLEDs( );
  digitalWrite( PIN_LED_INACTIVE,  HIGH );
  digitalWrite( PIN_LED_ACTIVE,    LOW  );
  digitalWrite( PIN_LASER,         HIGH );
  digitalWrite( PIN_BUZZER,        LOW  );
  digitalWrite( PIN_EXTERN,        LOW  );
}

//Set the state to active and set the pins accordingly.
void gotoActiveState ( )
{
  state = STATE_ACTIVE;
  setModeLEDs( );
  digitalWrite( PIN_LED_INACTIVE,  LOW  );
  digitalWrite( PIN_LED_ACTIVE,    HIGH );
  digitalWrite( PIN_LASER,         HIGH );
  digitalWrite( PIN_BUZZER,        LOW  );
  digitalWrite( PIN_EXTERN,        LOW  );
}

//Set the state to tripped and set the pins accordingly.
void gotoTrippedState ( )
{
  state = STATE_TRIPPED;
  reset = 0;

  //Turn off laser when tripped.
  digitalWrite( PIN_LASER, LOW );

  //Turn on buzzer if mode enabled.
  if ( mode & MODE_ALARM )
    digitalWrite( PIN_BUZZER, HIGH );

  //Turn on external device if mode enabled.
  if ( mode & MODE_EXTERN )
    digitalWrite( PIN_EXTERN, HIGH );
}

//Blink the LEDs and track auto reset counter if active.
void trippedStateAction ( )
{
  digitalWrite( PIN_LED_ACTIVE, pulse );
    
  if ( mode & MODE_ALARM )
    digitalWrite( PIN_LED_BUZZER, pulse );

  if ( mode & MODE_EXTERN )
    digitalWrite( PIN_LED_EXTERN, pulse );

  if ( mode & MODE_AUTORESET )
  {
    digitalWrite( PIN_LED_AUTORESET, pulse );

    if ( ++reset >= RESET_TIME )
    {
      reset = 0;
      gotoActiveState( );
    }
  }
}

//Send a reading of the sensors over serial.
void readSensors ( )
{
  Serial.print( "Laser:   " );
  Serial.print( analogRead( PIN_SENSE_LASER ) );
  Serial.println( );
  Serial.print( "Ambient: " );
  Serial.print( analogRead( PIN_SENSE_LIGHT ) );
  Serial.println( );
  Serial.println( );
}
