
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C mainLCD(0x27, 20, 4);
LiquidCrystal_I2C subLCD(0x26, 16, 2);

File myFile;

const int buttA = 2;
const int buttB = 3;
const int buttC = 4;

const int buttReset = 5;
const int Reset = 9;

const int correct = 7;
const int wrong = 8;

String question = String("");
String choiceA = String("");
String choiceB = String("");
String choiceC = String("");
String ans = String("");

String key = String("");

bool readLine = true;

void setup() {
  digitalWrite(Reset, HIGH);
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Initializing SD card...");

  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("geninfo.csv");

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.println("File detected");
  } else {
    Serial.println("error opening gen info");
  }

  mainLCD.init();
  subLCD.init();

  mainLCD.backlight();
  subLCD.backlight();

  pinMode(buttA, INPUT);
  pinMode(buttB, INPUT);
  pinMode(buttC, INPUT);

  pinMode(buttReset, INPUT);
  pinMode(Reset, OUTPUT);
  
  pinMode(correct, OUTPUT);
  pinMode(wrong, OUTPUT);

  digitalWrite(correct, HIGH);
  digitalWrite(wrong, HIGH);
  delay(1000);
  digitalWrite(correct, LOW);
  digitalWrite(wrong, LOW);
}

void loop() {
  ReadCsv(myFile);

  displayMainLCD();
  displaySubLCD();

  Evaluation();
}

//read the row of a csv and stop when the terminating value ";" is encountered
void ReadCsv(File file) {

  String tempString = String("");
  int column = 0;

  while (readLine) {
    char text = file.read();

    //concatenate text
    if (text != ',' && text != ';') {
      tempString = tempString + text;
    }

    //Assign the value to the specified column
    if (text == ',' && tempString != "") {
      if (column == 0) question = tempString; //Question
      if (column == 1) choiceA = tempString; //Choice A
      if (column == 2) choiceB = tempString; //Choice B
      if (column == 3) choiceC = tempString; //Choice C
      if (column == 4) ans = tempString; //Ans

      Serial.println(column);
      Serial.println(tempString);
      tempString = String("");

      column++;
    }

    //Read only the current row at the moment
    if (text == ';') {
      readLine = false;
    }
  }
}

void Evaluation() {

  //Check if any of the button are pressed
  if (digitalRead(buttA) == LOW) {
    key = String("a");
  }
  if (digitalRead(buttB) == LOW) {
    key = String("b");
  }
  if (digitalRead(buttC) == LOW) {
    key = String("c");
  }

  if (digitalRead(buttC) == LOW) {
    key = String("c");
  }

  if (digitalRead(buttReset) == LOW) {
    Serial.print("Reset Button is pressed.. reseting...");
    digitalWrite(Reset, LOW);
  }

  //Evaluate the input value with reference to the current question

  //if the input is correct
  if (key != "" && key == ans) {
    digitalWrite(correct, HIGH);
    delay(250);
    digitalWrite(correct, LOW);

    key = String("");
    readLine = true; //set true to proceed reading the next row of CSV
    delay(250);
  }

  //if the answer is all of the above, accept any input value
  //and prompt the usert the correct answer
  if (key != "" && ans == "all") {
    digitalWrite(correct, HIGH);
    delay(250);
    digitalWrite(correct, LOW);

    //display the answer to notify the user
    displayAnswer();

    key = String(""); //reset the input variable
    readLine = true; //set true to proceed reading the next row of CSV
    delay(250);
  }

  //if the input is wrong prompt the user the correct answer
  if (key != "" && key != ans) {
    digitalWrite(wrong, HIGH);
    delay(250);
    digitalWrite(wrong, LOW);

    displayAnswer();

    key = String("");//reset the input variable
  }

  delay(250);
}

void displayAnswer() {
  mainLCD.setCursor(0, 3);
  //clear the 4th row to display the answer
  mainLCD.print("                    ");

  mainLCD.setCursor(0, 3);
  //display the answer
  mainLCD.print("THE ANSWER IS " + ans);

  delay(250);

  mainLCD.setCursor(0, 3);
  //clear the 4th row to display the answer
  mainLCD.print("                    ");
}

void displayMainLCD() {
  String lineOne = question.substring(0, 20);
  String lineTwo = question.substring(20, 40);
  String lineThree = question.substring(40, 60);

  mainLCD.clear();
  mainLCD.setCursor(0, 0);
  mainLCD.print(lineOne);
  mainLCD.setCursor(0, 1);
  mainLCD.print(lineTwo);
  mainLCD.setCursor(0, 2);
  mainLCD.print(lineThree);
}

void displaySubLCD() {
  String lineOne = String(" A) " + choiceA + "  C) " + choiceC);
  String lineTwo = String(" B) " + choiceB);

  int maxLength = 0;

  if (lineOne.length() > lineTwo.length())
    maxLength = lineOne.length();
  else
    maxLength = lineTwo.length();

  bool isScroll = maxLength > 16;

  if (isScroll) {
    maxLength = maxLength - 12;

    for (int i = 0; i < maxLength; i++) {
      subLCD.clear();
      subLCD.setCursor(0, 0);
      subLCD.print(lineOne.substring(i, i + 20));

      subLCD.setCursor(0, 1);
      subLCD.print(lineTwo.substring(i, i + 20));

      Evaluation();

    }
  }
  else {
    subLCD.clear();
    subLCD.setCursor(0, 0);
    subLCD.print(lineOne);

    subLCD.setCursor(0, 1);
    subLCD.print(lineTwo);

    Evaluation();

    delay(1000);
  }
}
