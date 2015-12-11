/* Don Browne 
 *  Connect to a pebble watch using an Arduino and Bluetooth board
 *  Known to work with the Bluetooth Mate Gold
 *  Two step process:
 *  1.  change firstTimePair to true, update watch bluetooth address, then run 
 *  2.  after successful pairing, change firstTimePair to false for all subsequent runs
 *  
 *  12/11/2015
 */
// set this to the hardware serial port you wish to use
#define BlueTooth Serial1
#define Debug  Serial
//
// controls power to Bluetooth Mate, ensures you can get it into CMD mode
// don't connect directly, use a transistor 2n3904
// once you've successfully paired you don't need this
// you can get by without this by power cycling to bluetooth device manually
const int PIN_A0 =  14;
//
// set this to true the first time you pair with the pebble, or if you "forget" the device from the pebble
// then leave this as false
bool firstTimePair = false;
//
// Pebble Watch
char WatchAddress[] = "0017E9671145";

//endpoint
//2001=ping
//0x07D1
const uint8_t ping[9] = {0x00, 0x05, 0x07, 0xd1, 0x00, 0xde, 0xad, 0xbe, 0xef};  //ping

//                                    //phone version
const uint8_t phoneVer[6] = {0x00, 0x02, 0x00, 0x30, 0xff, 0x08};  //phone version

//
//
//
//                                       0x11 = 17 = Phone Version endpoint
// prefix, session, remote                                                     //session capabilities
//                                             phone version
//                                                 remote capabilities  Telephony.SMS.Android
//                                                                     //session capabilities............
const uint8_t cap[17] = {0x00, 0x0d, 0x00, 0x11, 0x02, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32};  //capabilities

//notification                                                   2 chars in        3 chars                    bytes
//                                                               header            in body                    till end
//this says BG 115                                                        B     G           1      1      5
const uint8_t         msg[30] = {0x00, 0x1A, 0x0B, 0xB8 , 0x01 , 0x02, 0x42, 0x47, 0x03, 0x31 , 0x31 , 0x35,  0x11, 0xFF, 0xFF , 0xFF , 0xFF , 0xFF, 0xFF, 0xFF, 0xFF , 0xFF , 0xFF, 0xFF, 0xFF , 0xFF, 0xFF , 0xFF , 0xFF, 0xFF};
//                                                                                C     o    n      n       e      c    t     e      d
const uint8_t  connectedMsg[30]={0x00, 0x1a, 0x0b, 0xB8,  0x01,  0x00, 0x09, 0x43, 0x6f, 0x6e,  0x6e , 0x65,  0x63, 0x74, 0x65,  0x64 , 0x0d  ,0xFF, 0xFF, 0xFF, 0xFF , 0xFF , 0xFF, 0xFF, 0xFF , 0xFF, 0xFF , 0xFF , 0xFF, 0xFF};

//this is the UUID of the pebble app
const uint8_t appUUID[16] = {0x7f, 0x7a, 0x38, 0x90, 0x1a, 0x1c, 0x43, 0xa3, 0xad, 0xf1, 0x21, 0x44, 0x9e, 0x4f, 0x35, 0x2d};

//glucose app message
//this sends a value to a custom pebble app
//the firt key in the dictionary is an integer, pass a value to it
//composed of Header and Body
//sends a value of 120 = 0x78
const uint8_t appMsg[31] = {0x00, 0x1b, 0x00, 0x30, 0x01, 0x00, 0x7f, 0x7a, 0x38, 0x90, 0x1a, 0x1c, 0x43, 0xa3, 0xad, 0xf1, 0x21, 0x44, 0x9e, 0x4f, 0x35, 0x2d, 0x04, 0x01, 0x00, 0x00, 0x00, 0x03, 0x01, 0x00, 0x78};


bool oneMsg = true;
bool pairwithwatch = true;
int counter = 0;
//
//
//
void setup() {
  //this allows us to powercycle the Bluetooth Mate
  //so we can control what state it's in
  pinMode(PIN_A0, OUTPUT);
  digitalWrite(PIN_A0, HIGH);

  Debug.begin(115200);
  BlueTooth.begin(115200);
  //hang out here so that I can get the serial monitor up
  delay(5000);
  Debug.println("starting");
}


void loop() {
  if (pairwithwatch) {
    pairwithwatch = false;
    if (! firstTimePair) {
      pebbleConnect();
    } else {
      pebblePair();
    }
  }

  if (oneMsg) {
    delay(5000);
    //pings will work always if you are paired
    //will even work if you messed up the version & cap packets
      Debug.println("Send Ping");
      BlueTooth.write(ping, 9);
      delay(5000);

    Debug.println("Send notification");
    BlueTooth.write(msg, 30);
    delay(5000);

    Debug.println("Send notification");
    BlueTooth.write(connectedMsg, 30);
    delay(5000);
    //
    //app message only works if you sent the version and cap packets
    Debug.println("Send appmessage");
    Debug.println("Write 0x22");
    BlueTooth.write(appMsg, 31);

    oneMsg = false;

  }

  delay(100);
  while (BlueTooth.available()) // If bluetooth sent any characters
  {
    // Send any characters the bluetooth prints to the serial monitor
    char c = BlueTooth.read();

    Debug.print(c, HEX);
    Debug.print(":");

    counter++;
    if (counter > 40) {
      Debug.println("");
      counter = 0;
    }
  }
}

void pebblePair() {
  //watch should be in Settings->Bluetooth
  Debug.println("Send $$$");
  BlueTooth.print("$$$");

  while (BlueTooth.read() != 'C') {};
  while (BlueTooth.read() != 'M') {};
  while (BlueTooth.read() != 'D') {};

  delay(100);

  BlueTooth.println("SP,9999"); //pin code 
  delay(500);
  BlueTooth.println("SM,6"); //automatic connections
  delay(500);
  BlueTooth.println("SA,4"); //pin code authentication
  delay(500);
  BlueTooth.println("SY,0000");//power
  delay(500);
  BlueTooth.print("C,");          //connect to bluetooth address
  BlueTooth.println(WatchAddress);//pebble bluetooth address
  delay(500);
  BlueTooth.println("SW,0640");//sniff mode
  delay(500);
  BlueTooth.println("R,1");//reset

  //this will pair almost immediately after reset
  //this section should not need to be run again for this watch
  while (BlueTooth.read() != 0x11) {}
  while (BlueTooth.read() != 0x00) {}
  Debug.println("Connected");
  delay(1000);
  Debug.println("Send Phone Ver");
  BlueTooth.write(phoneVer, 6);
  // too much delay here and you don't really get connected
  delay(100);
  Debug.println("Send Cap");
  BlueTooth.write(cap, 17);
}

void pebbleConnect() {
  Debug.println("Connecting...");
  //you can expect to be here for up to a minute
  //pebble sends a packet like this on successful connection
  //xx=number of previous connections
  //1:xx:7F:7A:38:90:1A:1C:43:A3:AD:F1:21:44:9E:4F:35:2D:1:1:0:0:0:2:4:0:1:0:0:0
  while (BlueTooth.read() != 0x7F) {}
  while (BlueTooth.read() != 0x7A) {}
  Debug.println("Connected");
}


