/*
  UDPSendReceive.pde:
 This sketch receives UDP message strings, prints them to the serial port
 and sends an "acknowledge" string back to the sender

 A Processing sketch is included at the end of file that can be used to send
 and received messages for testing with a computer.

 created 21 Aug 2010
 by Michael Margolis

 This code is in the public domain.
 */


#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <IRremote.h>

IRsend irsend;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 10, 100);

unsigned int localPort = 8888;      // local port to listen on

// Storage for the recorded code
int codeType = -1; // The type of code
unsigned long codeValue; // The code value if not raw
unsigned long codeValueNEC; 
unsigned int rawCodes[RAWBUF]; // The durations if raw
int codeLen; // The length of the code
int toggle = 0; // The RC5/6 toggle state

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
char  ReplyBuffer[] = "acknowledged";       // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

void setup() {
  // start the Ethernet and UDP:
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);

  Serial.begin(9600);
}

void decodePacket(){
  char current;
  int i=0;
  String protocol;
  String command;
  String size;
  while((current=packetBuffer[i])!='/')
  {
    
    protocol+=current;
    i++;}
    i++;
      while((current=packetBuffer[i])!='/')
  {
    
    command+=current;
    i++;}
    i++;
      while((current=packetBuffer[i])!='\0')
  {
    
    size+=current;
    i++;}
    current=protocol[0];
    switch(current){
      case 'N':{codeType=NEC;break;}
      case 'R':{if(protocol[2]=='6')codeType=RC6;else
      codeType=RC5;break;}
      case 'S':{codeType=SONY;break;}
      
      
      }
    codeValue=command.toInt();
    codeLen=size.toInt();

Serial.print("Value=");Serial.println(codeValue);
Serial.print("Code length=");Serial.println(codeLen);

  
  }


void sendCode(int repeat) {
  if (codeType == NEC) {
//    if (repeat) {
//      irsend.sendNEC(REPEAT, codeLen);
//      Serial.println("Sent NEC repeat");
//    } 
//    else {
      irsend.sendNEC(codeValue, codeLen);
      Serial.print("Sent NEC ");
      Serial.println(codeValue, HEX);
//    }
  } 
  else if (codeType == SONY) {
    irsend.sendSony(codeValue, codeLen);
    Serial.print("Sent Sony ");
    Serial.println(codeValue, HEX);
  } 
  else if (codeType == RC5 || codeType == RC6) {
    if (!repeat) {
      // Flip the toggle bit for a new button press
      toggle = 1 - toggle;
    }
    // Put the toggle bit into the code to send
    codeValue = codeValue & ~(1 << (codeLen - 1));
    codeValue = codeValue | (toggle << (codeLen - 1));
    if (codeType == RC5) {
      Serial.print("Sent RC5 ");
      Serial.println(codeValue, HEX);
      irsend.sendRC5(codeValue, codeLen);
    } 
    else {
      irsend.sendRC6(codeValue, codeLen);
      Serial.print("Sent RC6 ");
      Serial.println(codeValue, HEX);
    }
  } 
  else if (codeType == UNKNOWN /* i.e. raw */) {
    // Assume 38 KHz
    irsend.sendRaw(rawCodes, codeLen, 38);
    Serial.println("Sent raw");
  }
}

void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i = 0; i < 4; i++)
    {
      Serial.print(remote[i], DEC);
      if (i < 3)
      {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
   // Serial.println("Contents:");
 ///   Serial.println(packetBuffer);
    String str(packetBuffer);
    Serial.println(str);
    decodePacket();
//codeType = NEC;
//codeValue=2160016324;
//codeLen=32;
sendCode(0);
    // send a reply, to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), 90);
    Udp.write(ReplyBuffer);
    Udp.endPacket();
  }
  delay(10);
}


/*
  Processing sketch to run with this example
 =====================================================

 // Processing UDP example to send and receive string data from Arduino
 // press any key to send the "Hello Arduino" message


 import hypermedia.net.*;

 UDP udp;  // define the UDP object


 void setup() {
 udp = new UDP( this, 6000 );  // create a new datagram connection on port 6000
 //udp.log( true ); 		// <-- printout the connection activity
 udp.listen( true );           // and wait for incoming message
 }

 void draw()
 {
 }

 void keyPressed() {
 String ip       = "192.168.1.177";	// the remote IP address
 int port        = 8888;		// the destination port

 udp.send("Hello World", ip, port );   // the message to send

 }

 void receive( byte[] data ) { 			// <-- default handler
 //void receive( byte[] data, String ip, int port ) {	// <-- extended handler

 for(int i=0; i < data.length; i++)
 print(char(data[i]));
 println();
 }
 */


