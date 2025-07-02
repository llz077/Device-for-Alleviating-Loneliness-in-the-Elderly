
/*  
 
 */
#include <TFT_eSPI.h>  // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <SPIFFS.h>
#include <iostream>
#include <string>
#include <vector>
#include <Adafruit_NeoPixel.h>
#include <FastLED.h>



TFT_eSPI tft = TFT_eSPI(128, 160);  // Invoke library, pins defined in User_Setup.h

// define colours
#define TFT_WHITE 0xFFFF
// #define TFT_TRANSPARENT 0x0120
// #define TFT_BLUE 0x001F
// #define TFT_YELLOW 0xFFE0
// #define TFT_BLACK 0xFFFF // New colour

#define PIN_WS2812B  13   // ESP32 pin that connects to WS2812B
#define NUM_PIXELS     29  // The number of LEDs (pixels) on WS2812B

Adafruit_NeoPixel ws2812b(NUM_PIXELS, PIN_WS2812B, NEO_GRB + NEO_KHZ800);

// set pin numbers
const int enterButtonPin = 5;  // the number of enter pushbutton pin
const int homeButtonPin = 4;   // the number of home pushbutton pin
const int upButtonPin = 12;    // the number of home pushbutton pin
const int downButtonPin = 14;  // the number of home pushbutton pin

// variable for storing the pushbutton status
bool enterButtonState = false;
bool homeButtonState = false;
bool upButtonState = false;
bool downButtonState = false;

// Debounce Variables
unsigned long lastDebounceTime[4] = { 0, 0, 0, 0 };
const unsigned long debounceDelay = 50;

bool lastButtonStates[4] = { HIGH, HIGH, HIGH, HIGH };

int counter = 1;

int totalState = 0;
int locLen = 3;
int locState = 0;
std::string pageState = "home";
std::string contact = "";


// setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

class infoClass {
public:
  std::string myName;
  std::vector<std::string> myHobbies;

  infoClass()
    : myName("Default Name"), myHobbies({}) {}

  infoClass(const std::string& name, const std::vector<std::string>& hobbies)
    : myName(name), myHobbies(hobbies) {}

  void display() const {
    std::cout << "Name: " << myName << std::endl;
    std::cout << "Hobbies:" << std::endl;
    for (const auto& hobby : myHobbies) {
      std::cout << "  - " << hobby << std::endl;
    }
  }
};

infoClass myInfo1;
infoClass showInfo = myInfo1;

std::vector<std::string> Contacts = { "YB", "Nuraini", "Bruce", "Hayati" };
std::vector<std::shared_ptr<infoClass>> contactsInfo;

std::vector<std::string> newContacts{ "Jernice" };
std::vector<std::shared_ptr<infoClass>> newContactsInfo;


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
  //digitalWrite(enterButtonPin, HIGH);

  pinMode(homeButtonPin, INPUT);
  //digitalWrite(homeButtonPin, HIGH);

  pinMode(upButtonPin, INPUT);
  //digitalWrite(upButtonPin, HIGH);

  pinMode(downButtonPin, INPUT);
  //digitalWrite(downButtonPin, HIGH);

  for (int i = 0; i < 4; i++) {
    lastButtonStates[i] = HIGH; // Default state for pull-up is HIGH
  }

  // initialize LED strip
  ws2812b.begin();

  myInfo1.myName = "Sasha";
  myInfo1.myHobbies = { "Golf", "Gardening", "Vodka" };

  // myInfo1.myName = "YB";
  // myInfo1.myHobbies = { "Gacha", "Old", "IDK" };
  contactsInfo.push_back(std::make_shared<infoClass>("YB", std::vector<std::string>{ "Gacha", "Old", "IDK" }));
  contactsInfo.push_back(std::make_shared<infoClass>("Nuraini", std::vector<std::string>{ "Praying", "Gardening", "IDK" }));
  contactsInfo.push_back(std::make_shared<infoClass>("Bruce", std::vector<std::string>{ "Chinese", "Robots", "IDK" }));
  contactsInfo.push_back(std::make_shared<infoClass>("Hayati", std::vector<std::string>{ "Working", "Med", "IDK" }));

  newContactsInfo.push_back(std::make_shared<infoClass>("Jernice", std::vector<std::string>{ "K-POP", "Dogs", "IDK" }));


  Serial.begin(9600);
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
  tft.setTextColor(TFT_BLACK, TFT_WHITE);  // Black text on white background
  tft.setTextSize(2);                      // Adjust size as needed
  tft.drawString(text, xpos, ypos, 2);     // Position (10, 50), font size 2
}

void displayTextWithNewlines(String text, int startX, int startY, int lineSpacing) {
  int y = startY;  // Initial Y position
  while (text.length() > 0) {
    int newlineIndex = text.indexOf("\n");  // Find the next newline character
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
    tft.drawString(line, startX, y, 2);  // Adjust font size as needed
    y += lineSpacing;                    // Move to the next line
  }
}

void setupHomePage() {
  // Fill screen with grey so we can see the effect of printing with and without
  // a background colour defined
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);

  // Read text from file
  String savedText = readTextFromFile("/initial_db.txt");
  // String savedText = readTextFromFile("/initial_db.txt");

  // Display the text line by line on the screen
  displayTextWithNewlines(savedText, 0, 0, 30);  // Adjust line spacing as needed
  // displayName(savedText, 0, 0);

  // Display options
  tft.setCursor(20, 110, 150);
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setTextSize(2);
  tft.println("HOME");

  tft.setCursor(70, 110, 150);
  tft.setTextColor(TFT_WHITE, TFT_GOLD);
  tft.setTextSize(2);
  tft.println("ENTER");
}

void displayInfoPage() {
  // Fill screen with grey so we can see the effect of printing with and without
  // a background colour defined
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);

  displayName("New page", 0, 0);
}

void displayTable() {
  for (int i = 0; i < 3; i++) {
    tft.drawLine(0, 43 * i + 43, 160, 43 * i + 43, TFT_BLACK);
  }
}

int hover_state = 0;

void displayHover() {
  //tft.drawRect(0, hover_state*42, 160, hover_state*42+42, TFT_WHITE);
  if (locState > 2) {
    tft.fillRect(0, 2 * 43, 160, 43, TFT_YELLOW);
  } else {
    tft.fillRect(0, locState * 43, 160, 43, TFT_YELLOW);
  }
}

std::vector<std::string> home_options = { "My Info", "Contacts", "New Contacts" };

void displayHome() {
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(3);
  for (int i = 0; i < home_options.size(); i++) {
    tft.drawString(String(home_options[i].c_str()), 5, 43 * i + 3);  // Convert to String
  }
}

void setLEDBrightness(int brightness_level) {
  // set all pixel colors to 'off'. It only takes effect if pixels.show() is called
  //ws2812b.clear();  
  // turn pixels to green one-by-one with delay between each pixel
  for (int pixel = 0; pixel < NUM_PIXELS; pixel++) {         // for each pixel
    ws2812b.setPixelColor(pixel, ws2812b.Color(255, 255, 0));  // it only takes effect if pixels.show() is called
    ws2812b.setBrightness(brightness_level);
  //                                            // update to the WS2812B Led Strip
  }
  ws2812b.show();
}

void displayContacts() {
  const int maxVisibleContacts = 3;  // Number of contacts visible on the screen
  const int rowHeight = 43;          // Height of each row
  const int xOffset = 5;             // Horizontal offset for text
  const int yOffset = 3;             // Vertical offset for text

  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(3);

  // Calculate the starting index for the visible contacts
  int startIndex = (locState > maxVisibleContacts - 1) ? locState - (maxVisibleContacts - 1) : 0;

  // Render the visible contacts
  for (int i = 0; i < maxVisibleContacts; i++) {
    int contactIndex = startIndex + i;

    // Ensure we don't access out-of-bounds elements
    if (contactIndex < Contacts.size()) {
      int yPosition = rowHeight * i + yOffset;
      tft.drawString(String(Contacts[contactIndex].c_str()), xOffset, yPosition, 2);
    }
  }
}

void displayInfo(const infoClass& data) {
  tft.fillScreen(TFT_WHITE);

  tft.setTextColor(TFT_BLACK, TFT_WHITE);  // Black text on white background
  tft.setTextSize(2);
  if (data.myName != "Sasha") {
    tft.drawString("Contact Info", 3, 5);
  } else {
    tft.drawString("My Info", 3, 5);
  }
  tft.drawString("Name:", 3, 25);
  tft.drawString(String(data.myName.c_str()), 70, 25);  // Convert to String

  tft.drawString("Hobbies: ", 3, 45);
  for (int i = 0; i < data.myHobbies.size(); i++) {
    tft.drawString(String(data.myHobbies[i].c_str()), 3, (20 * i) + 65);  // Convert to String
  }
}


void displayContacts(const infoClass& data) {
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(3);

  tft.drawString("Call", 5, 3);       // Convert to String
  tft.drawString("Info", 5, 43 + 3);  // Convert to String
}

void displayCalling() {
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(3);

  tft.drawString("Calling", 5, 3);       // Convert to String
  setLEDBrightness(50);
}

void displayNewContacts() {
  const int maxVisibleContacts = 3;  // Number of contacts visible on the screen
  const int rowHeight = 43;          // Height of each row
  const int xOffset = 5;             // Horizontal offset for text
  const int yOffset = 3;             // Vertical offset for text

  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(3);

  // Calculate the starting index for the visible contacts
  int startIndex = (locState > maxVisibleContacts - 1) ? locState - (maxVisibleContacts - 1) : 0;

  // Render the visible contacts
  for (int i = 0; i < maxVisibleContacts; i++) {
    int contactIndex = startIndex + i;

    // Ensure we don't access out-of-bounds elements
    if (contactIndex < newContacts.size()) {
      int yPosition = rowHeight * i + yOffset;
      tft.drawString(String(newContacts[contactIndex].c_str()), xOffset, yPosition, 2);
    }
  }
}

void displayAddContact() {
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(3);

  tft.drawString("Add", 5, 3);        // Convert to String
  tft.drawString("Info", 5, 43 + 3);  // Convert to String
}

void displayAdded() {
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(3);

  tft.drawString("Contact", 5, 3);  // Convert to String
  tft.drawString("Added", 5, 43 + 3);
}


void handleEnter() {
  if (pageState == "home" && locState == 0) {
    pageState = "myInfo";
    locState = 0;
    showInfo = myInfo1;
    //setLEDBrightness(255);
  }
  else if (pageState == "home" && locState == 1) {
    pageState = "contacts";
    locState = 0;
    locLen = Contacts.size();
  }
  else if (pageState == "home" && locState == 2) {
    pageState = "newContacts";
    locState = 0;
    locLen = newContacts.size();
  }
  else if (pageState == "newContacts") {
    pageState = "addContact";
    locState = 0;
    locLen = 2;
    showInfo = myInfo1;
  }
  else if (pageState == "addContact" && locState == 0) {
    pageState = "added";
    locState = 0;
    contactsInfo.push_back(std::move(newContactsInfo[0]));
    newContactsInfo.erase(newContactsInfo.begin());

    Contacts.push_back(std::move(newContacts[0]));
    newContacts.erase(newContacts.begin());

    // Serial.println("Contact moved to contactsInfo");
    // Serial.print("contactsInfo size: ");
    // Serial.println(contactsInfo.size());
    // Serial.print("newContactsInfo size: ");
    // Serial.println(newContactsInfo.size());
  }
  else if (pageState == "addContact" && locState == 1) {
    pageState == "showInfo";
  }
  else if (pageState == "contacts" ) { // && locState == 1
    pageState = "contact";
    showInfo = *(contactsInfo[locState]);
    locState = 0;
    locLen = 2;
  }
  else if (pageState == "contact" && locState == 0){
    pageState = "calling";
  }
  else if (pageState == "contact" && locState == 1) {
    pageState = "showInfo";
  }
  counter = 1;
}

void handleHome() {
  if (pageState == "myInfo") {
    pageState = "home";
    locLen = home_options.size();
  }
  else if (pageState == "contacts") {
    pageState = "home";
    locLen = home_options.size();
  }
  else if (pageState == "contact") {
    pageState = "contacts";
    locState = 0;
    locLen = Contacts.size();
  }
  else if (pageState == "showInfo") {
    if (showInfo.myName == "Sasha") {
      pageState = "home";
      locState = 0;
      locLen = home_options.size();
    } else {
      pageState = "contact";
      locState = 0;
      locLen = 2;
    }
  }
  else if (pageState == "addContact") {
    pageState = "newContacts";
    locState = 0;
    locLen = newContacts.size();
  }
  else if (pageState == "newContacts") {
    pageState = "home";
    locLen = home_options.size();
  }
  else if (pageState == "added"){
    pageState = "newContacts";
    locLen = home_options.size();
  }
  else if (pageState == "calling"){
    pageState = "contact";
    setLEDBrightness(0);
  }
  locState = 0;
  counter = 1;
}

void handleUp() {
  if (locState > 0) {
    //std::cout << "hello world" << std::endl;
    locState -= 1;
  }
  counter = 1;
}

void handleDown() {
  if (locState < locLen) {
    locState += 1;
  }
  counter = 1;
}

void handleButton(int pin, int index, bool& buttonState, const char* buttonName) {
  int reading = digitalRead(pin);

  // Debounce Logic
  if (reading != lastButtonStates[index]) {
    lastDebounceTime[index] = millis();
  }

  if ((millis() - lastDebounceTime[index]) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == HIGH) {  // Button pressed
        Serial.print(buttonName);
        Serial.println(" button pressed");

        // Add your custom action for each button here
        if (strcmp(buttonName, "Enter") == 0) {
          handleEnter();
        } else if (strcmp(buttonName, "Home") == 0) {
          handleHome();
        } else if (strcmp(buttonName, "Up") == 0) {
          handleUp();
        } else if (strcmp(buttonName, "Down") == 0) {
          handleDown();
        }
      }
    }
  }

  lastButtonStates[index] = reading;
}

unsigned long lastUpdateTime = 0;  // Stores the last time the screen was updated
const unsigned long updateInterval = 200;

void loop() {
  //unsigned long currentMillis = millis();
  //locState = 0;
  // pageState = "added";
  // showInfo = myInfo1;

  // control LED off and on with specified brightness (0-255)
  // changeLEDBrightness(0);
  // delay(500);
  // changeLEDBrightness(50);
  // delay(500);
  // changeLEDBrightness(100);
  // delay(500);
  // changeLEDBrightness(255);

  // std::cout << totalState << std::endl;
  std::cout << "Page State: " << pageState << std::endl;
  std::cout << "LocState: " << locState << std::endl;

  // Read the state of the pushbutton value
  //enterButtonStateCurrent = digitalRead(enterButtonPin);

  handleButton(enterButtonPin, 0, enterButtonState, "Enter");
  handleButton(homeButtonPin, 1, homeButtonState, "Home");
  handleButton(upButtonPin, 2, upButtonState, "Up");
  handleButton(downButtonPin, 3, downButtonState, "Down");

  // if (currentMillis - lastUpdateTime >= updateInterval) {
  //   lastUpdateTime = currentMillis;

  if (counter == 1) {
    if (pageState == "home") {
      //setupHomePage();
      tft.fillScreen(TFT_WHITE);
      displayHover();
      displayTable();
      displayHome();
    }
    if (pageState == "myInfo") {
      displayInfo(showInfo);
    }
    if (pageState == "contacts") {
      tft.fillScreen(TFT_WHITE);
      displayHover();
      displayTable();
      displayContacts();
    }
    if (pageState == "contact") {
      tft.fillScreen(TFT_WHITE);
      displayHover();
      displayTable();
      displayContacts(*contactsInfo[0]);
    }
    if (pageState == "newContacts") {
      tft.fillScreen(TFT_WHITE);
      displayHover();
      displayTable();
      displayNewContacts();
    }
    if (pageState == "addContact") {
      tft.fillScreen(TFT_WHITE);
      displayHover();
      displayTable();
      displayAddContact();
    }
    if (pageState == "added") {
      tft.fillScreen(TFT_WHITE);
      displayAdded();
    }
    if (pageState == "showInfo") {
      displayInfo(showInfo);
    }
    if (pageState == "calling"){
      displayCalling();
    }
    counter = 0;
  }
  //}

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