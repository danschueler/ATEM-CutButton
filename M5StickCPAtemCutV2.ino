/**************************************************************************
  M5Stick C Plus Cut Button ver 2
  based on the work of Kasper Skarhoj.  It uses the Skaarhoj Arduino GNU
  libraries.  There's some code from onguyoneblog as well, but it's been 
  extensively modified

 **************************************************************************/
#include "M5StickCPlus.h"

#include <WiFi.h>
#include <SkaarhojPgmspace.h>
#include <ATEMbase.h>
#include <ATEMstd.h>

IPAddress switcherIp(192, 168, 1, 170);       // IP address of the ATEM switcher - set to yours
ATEMstd AtemSwitcher;

const char* ssid = "yourssid";
const char* password = "yourpass";

TaskHandle_t Task1;

int PreviewTallyPrevious = 1;
int ProgramTallyPrevious = 1;
int ProgSource = 1;
int PrevSource = 1;
int globalPreview;

#define DEBOUNCE_MS 5

Button BtnA = Button(BUTTON_A_PIN, true, DEBOUNCE_MS);
Button BtnB = Button(BUTTON_B_PIN, true, DEBOUNCE_MS);

void Task1code( void * pvParameters ){
//  Serial.print("Task1 running on core ");
//  Serial.println(xPortGetCoreID());
  M5.update();
  int button1State = LOW;
  int button2State = LOW;
    for(;;){
//      button1State = M5.BtnA.wasPressed();
//      button2State = M5.BtnB.wasPressed();
      if (BtnA.read()) {
        AtemSwitcher.doCut();
      }
      if (BtnB.read()) {
        globalPreview++;
//        Serial.println(String(globalPreview));
        if (globalPreview > 4)
            globalPreview = 1;
        AtemSwitcher.changePreviewInput(globalPreview);  
      }
      delay(200);
    }
}
//-----------------------------------------------------------------------]


void drawLabelTop(unsigned long int screenColor, unsigned long int labelColor, int localCameraNumber) {

  M5.lcd.fillRect(0,0,134,119, screenColor);
  M5.lcd.setTextColor(labelColor, labelColor);
  M5.lcd.setCursor(40, 20, 8);

  M5.lcd.print(String(localCameraNumber));

}
void drawLabelBottom(unsigned long int screenColor, unsigned long int labelColor, int localCameraNumber) {

  M5.lcd.fillRect(0,119,134,119, screenColor);

  M5.lcd.setTextColor(labelColor, screenColor);
  M5.lcd.setCursor(40, 140, 8);

  M5.lcd.print(String(localCameraNumber));

}


void setup(void) {

  Serial.begin(115200);
  Serial.println("M5StickCP ATEM CutButton");

  M5.begin();
  M5.lcd.setRotation(0);  // landscape
  delay(1000);

  Serial.println(F("Initialized"));

  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  xTaskCreatePinnedToCore(Task1code,"Task1", 10000, NULL, 2, &Task1, 0);
  
  AtemSwitcher.begin(switcherIp);
  AtemSwitcher.serialOutput(0x80);
  AtemSwitcher.connect();
}

void loop() {

  // Check for packets, respond to them etc. Keeping the connection alive!
  AtemSwitcher.runLoop();

  int ProgramTally = AtemSwitcher.getProgramInput();
  int PreviewTally = AtemSwitcher.getPreviewInput();

  if ((ProgramTallyPrevious != ProgramTally) || (PreviewTallyPrevious != PreviewTally)) { // changed?
            drawLabelTop(TFT_RED, TFT_BLACK, ProgramTally);
            drawLabelBottom(TFT_GREEN, TFT_BLACK, PreviewTally);  
            globalPreview = PreviewTally;
  }

  ProgramTallyPrevious = ProgramTally;
  PreviewTallyPrevious = PreviewTally;

}