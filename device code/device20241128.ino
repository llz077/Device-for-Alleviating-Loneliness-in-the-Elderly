/*  
 
 */


#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <SPIFFS.h>
#include <iostream>
#include <string>
#include <vector>



TFT_eSPI tft = TFT_eSPI(128, 160);  // Invoke library, pins defined in User_Setup.h

// define colours
#define TFT_WHITE 0xFFFF
// #define TFT_TRANSPARENT 0x0120 
// #define TFT_BLUE 0x001F 
// #define TFT_YELLOW 0xFFE0 
// #define TFT_BLACK 0xFFFF // New colour

// set pin numbers
const int enterButtonPin = 5; // the number of enter pushbutton pin
const int homeButtonPin = 4; // the number of home pushbutton pin
const int upButtonPin = 12; // the number of home pushbutton pin
const int downButtonPin = 14; // the number of home pushbutton pin
const int ledPin = 13; // the number of home pushbutton pin

// variable for storing the pushbutton status
int enterButtonState = LOW;
int homeButtonState = LOW;

int totalState = 0;
int locState = 0;
int locLen = 3;
std::string pageState = "home";

// setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

class infoClass{
  public:
    std::string myName;
    std::vector<std::string> myHobbies;
};

infoClass myInfo1;

void setup(void) {
  tft.init();
  tft.setRotation(7);

  // Initialize ports
  Serial.begin(115200);

  // Initialize ROM
  if (!SPIFFS.begin(true)) {  // true formats the file system if mounting fails
    Serial.println("An error occurred while mounting SPIFFS");
    return;
  }
  Serial.println("SPIFFS mounted successfully");

  // Save text to file
  saveTextToFile("/initial_db.txt", "Adam\nBobby");
  
  // Initialize the pushbutton pin as an input
  pinMode(enterButtonPin, INPUT);
  pinMode(homeButtonPin, INPUT);
  pinMode(upButtonPin, INPUT);
  pinMode(downButtonPin, INPUT);

  // configure LED PWM functionalitites
  ledcSetup(ledChannel, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(ledPin, ledChannel);

  infoClass myInfo1;
  myInfo1.myName = "John Doe";

  myInfo1.myHobbies = {"Golf", "Gardening", "Vodka"};
}

void saveTextToFile(const char* filename, const char* text) {
  fs::File file = SPIFFS.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(text)) {
    Serial.println("File written successfully");
  } else {
    Serial.println("File write failed");
  }
  file.close();
}

// Function to read text from SPIFFS
String readTextFromFile(const char* filename) {
  fs::File file = SPIFFS.open(filename, FILE_READ);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return "Error: Cannot read file";
  }
  String text = file.readString();
  file.close();
  return text;
}

void displayName(String text, int xpos, int ypos) {
  tft.setTextColor(TFT_BLACK, TFT_WHITE); // Black text on white background
  tft.setTextSize(2); // Adjust size as needed
  tft.drawString(text, xpos, ypos, 2); // Position (10, 50), font size 2
}

void displayTextWithNewlines(String text, int startX, int startY, int lineSpacing) {
  int y = startY; // Initial Y position
  while (text.length() > 0) {
    int newlineIndex = text.indexOf("\n"); // Find the next newline character
    String line;
    if (newlineIndex == -1) {
      // No more newlines, take the remaining text
      line = text;
      text = "";
    } else {
      // Extract the line and remove it from the text
      line = text.substring(0, newlineIndex);
      text = text.substring(newlineIndex + 1);
    }
    // Draw the line on the display
    tft.drawString(line, startX, y, 2); // Adjust font size as needed
    y += lineSpacing; // Move to the next line
  }
}

void setupHomePage(){
  // Fill screen with grey so we can see the effect of printing with and without 
  // a background colour defined
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK,TFT_WHITE);

  // Read text from file
  String savedText = readTextFromFile("/initial_db.txt");
  // String savedText = readTextFromFile("/initial_db.txt");

  // Display the text line by line on the screen
  displayTextWithNewlines(savedText, 0, 0, 30); // Adjust line spacing as needed
  // displayName(savedText, 0, 0);
  
  // Display options
  tft.setCursor(20, 110, 150);
  tft.setTextColor(TFT_WHITE,TFT_BLUE);
  tft.setTextSize(2);
  tft.println("HOME");

  tft.setCursor(70, 110, 150);
  tft.setTextColor(TFT_WHITE,TFT_GOLD);
  tft.setTextSize(2);
  tft.println("ENTER");
  
  
}

void displayInfoPage(){
  // Fill screen with grey so we can see the effect of printing with and without 
  // a background colour defined
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK,TFT_WHITE);

  displayName("New page", 0, 0);
}

void displayTable(){
  for (int i = 0; i < 3; i++){
    tft.drawLine(0, 43*i+43, 160, 43*i+43, TFT_BLACK);
  }
}

int hover_state = 0;

void displayHover(){
  //tft.drawRect(0, hover_state*42, 160, hover_state*42+42, TFT_WHITE);
  if (locState > 2){
    tft.fillRect(0, 2*43, 160, 43, TFT_YELLOW);
  }
  else{
    tft.fillRect(0, locState*43, 160, 43, TFT_YELLOW);
  }
}

std::vector<std::string> home_options = {"My Info", "Contacts", "New Contacts"};

void home(){
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(3);
  for (int i = 0; i < home_options.size(); i++){
        tft.drawString(String(home_options[i].c_str()), 5, 43 * i +3); // Convert to String
  }
}

void changeLEDBrightness(int dutyCycle){
  // specify LED brightness
  ledcWrite(ledChannel, dutyCycle);
}

std::vector<std::string> Contacts = {"YB", "Nuraini", "Bruce", "Hayati", "Jernice"};

void displayContacts(){
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(3);
  if (locState > 2){
    for (int i = locState-2; i < Contacts.size(); i++){
        tft.drawString(String(Contacts[i].c_str()), 5, 43 * (locState-i) +3); // Convert to String
    }
  }
  else {
    for (int i = 0; i < Contacts.size(); i++){
        tft.drawString(String(Contacts[i].c_str()), 5, 43 * i +3); // Convert to String
    }
  }
}

void displayMyInfo(const infoClass& data){
  tft.fillScreen(TFT_WHITE);
  //tft.drawString(String(data.myName.c_str()), 10, 10); // Convert to String
  tft.drawLine(0, 100, 160, 100, TFT_BLACK);
  tft.drawString("Info Page", 20, 20);

}

int counter = 0;

void loop() {
  
  // control LED off and on with specified brightness (0-255)
  // changeLEDBrightness(0);
  // delay(500);
  // changeLEDBrightness(50);
  // delay(500);
  // changeLEDBrightness(100);
  // delay(500);
  // changeLEDBrightness(255);

  // std::cout << totalState << std::endl;
  std::cout << pageState << std::endl;
  std::cout << "LocState: " << locState << std::endl;

  // Read the state of the pushbutton value
  //enterButtonStateCurrent = digitalRead(enterButtonPin);
  
  if (digitalRead(enterButtonPin) == 1){
    //std::cout << "hello world" << std::endl;

    //  if (enterButtonState == LOW){
    //   enterButtonState = HIGH;
    //   // std::cout << "Change Detected" << std::endl;
    //   // std::cout << enterButtonState << std::endl;
    //  }
    if (pageState == "home" && locState == 0){
      pageState = "myInfo";
      locState = 0;
    }
    if (pageState == "home" && locState == 1){
      pageState = "contacts";
      locState = 0;
      locLen = Contacts.size();
    }

    counter = 1;
  }

  if (digitalRead(homeButtonPin) == 1){
    if (pageState == "myInfo"){
      pageState = "home";
      locLen = home_options.size();
    }
    if (pageState == "Contacts"){
      pageState = "home";
      locLen = home_options.size();
    }
    locState = 0;
    counter = 1;
  }

  if(digitalRead(upButtonPin) == 1){
    //std::cout << "hello world" << std::endl;

    if(locState > 0){
      //std::cout << "hello world" << std::endl;

      locState -= 1;
    }
    counter = 1;
  }

  if(digitalRead(downButtonPin) == 1){
    //std::cout << "hello world" << std::endl;

    if(locState < locLen-1){
      locState += 1;
    }
    counter = 1;
  }

  if (counter == 1){
    if (pageState == "home"){
    //setupHomePage();
    tft.fillScreen(TFT_WHITE);
    displayHover();
    displayTable();
    home();
    }
    if (pageState == "myInfo"){
      displayMyInfo(myInfo1);
    }
    if (pageState == "contacts"){
      tft.fillScreen(TFT_WHITE);
      displayHover();
      displayTable();
      displayContacts();
    }
    // if (totalState == 1){
    //   displayInfoPage();
    // }
    // if (totalState == 2){
    //   tft.fillScreen(TFT_WHITE);

    //   displayTable();
    //   displayHover();
    // }

    counter = 0;
    delay(1000);
  }  

  // check if buttonState is HIGH and if so go to another page
  // if (enterButtonStateCurrent == HIGH && enterButtonState == LOW) {
   
  //   enterButtonState = enterButtonStateCurrent;
  // }



  // // ISSUE: goes to new page once from Home and remains at Home page
  // // Read the state of the pushbutton value
  // int enterButtonStateCurrent = digitalRead(enterButtonPin);

  // // check if buttonState is HIGH and if so go to another page
  // if (enterButtonStateCurrent == HIGH && enterButtonState == LOW) {
  //   displayInfoPage();
  //   enterButtonState = enterButtonStateCurrent;
  // }

  // ISSUE: immediately goes to newpage
  // // Read the state of the pushbutton value
  // int enterButtonStateCurrent = digitalRead(enterButtonPin);
  // Serial.println(enterButtonStateCurrent);

  // if (enterButtonStateCurrent == HIGH){
  //   enterButtonState = HIGH;
  // }

  // // check if buttonState is HIGH and if so go to another page
  // if (enterButtonState == HIGH) {
  //   displayInfoPage();
  // }
  // else{  
  //   setupHomePage();
  // }


  //delay(1000); 
}