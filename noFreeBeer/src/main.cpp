#include <Arduino.h>
#include <SoftwareSerial.h>
#include <MFRC522.h>
#include <SPI.h> //INCLUSÃO DE BIBLIOTECA

#define SS_PIN 10 // PINO SDA
#define RST_PIN 9 // PINO DE RESET

#define uidSize 4

MFRC522 rfid(SS_PIN, RST_PIN); // PASSAGEM DE PARÂMETROS REFERENTE AOS PINOS
void openValve()
{
  Serial.println("abrindo válvula");
}

void closeValve()
{
  Serial.println("Fechando válvula");
}

class consumption
{
  const static int cardsLimit = 50;

private:
  char cardUIDs[cardsLimit][uidSize+1];
  unsigned long consumptions[cardsLimit];
  int firstEmptyIndex;

  int findCardIndex(String uid)
  {
    for (int i = 0; i < cardsLimit; i++)
    {
      if (uid == cardUIDs[i])
      {
        return i;
      }
    }
    return 0;
  }

  int setNewCard(char uid[4])
  {
    strncpy(cardUIDs[firstEmptyIndex], uid, 4);
    consumptions[firstEmptyIndex] = 0;

    return (firstEmptyIndex++); // rreturn element and add one to the next one
  }

public:
  consumption()
  {
    firstEmptyIndex = 1;
  };
  void addConsunmption(char uid[4], unsigned long consumption)
  {
    int cardIndex = findCardIndex(uid);
    if (cardIndex == 0)
    { // New card setting new
      cardIndex = setNewCard(uid);
    }
    consumptions[cardIndex] += consumption;
  }
};

bool checkCardRemoval()
{
  control = 0;
  for (int i = 0; i < 3; i++)
  {
    if (!rfid.PICC_IsNewCardPresent())
    {
      if (rfid.PICC_ReadCardSerial())
      {
        // Serial.print('a');
        control |= 0x16;
      }
      if (rfid.PICC_ReadCardSerial())
      {
        // Serial.print('b');
        control |= 0x16;
      }
      // Serial.print('c');
      control += 0x1;
    }
    // Serial.print('d');
    control += 0x4;
  }

  // Serial.println(control);
  if (control == 13 || control == 14)
  {
    return false;
  }
  else
  {
    return true;
  }
}

bool cardDetected;
unsigned long cardStart;
String strID;
char bytesId[4];
consumption consumptions;
uint8_t control;

void setup()
{
  Serial.begin(9600); // INICIALIZA A SERIAL
  SPI.begin();        // INICIALIZA O BARRAMENTO SPI
  rfid.PCD_Init();    // INICIALIZA MFRC522

  cardDetected = false;
  cardStart = 0;
  consumptions = consumption();
  control = 0x00;
}

void loop()
{
  if (!cardDetected)
  {
    rfid.PICC_IsNewCardPresent();
    if (rfid.PICC_ReadCardSerial())
    {
      openValve();
      cardDetected = true;
      cardStart = millis();

      strID = "";
      for (byte i = 0; i < 4; i++)
      {
        strID +=
            (rfid.uid.uidByte[i] < 0x10 ? "0" : "") +
            String(rfid.uid.uidByte[i], HEX) +
            (i != 3 ? ":" : "");
        bytesId[i] = rfid.uid.uidByte[i];
      }
      strID.toUpperCase();
      /***FIM DO BLOCO DE CÓDIGO RESPONSÁVEL POR GERAR A TAG RFID LIDA***/

      Serial.print("Identificador (UID) da tag: "); // IMPRIME O TEXTO NA SERIAL
      Serial.println(strID);                        // IMPRIME NA SERIAL O UID DA TAG RFID
    }
  }
  else
  {
    if (checkCardRemoval())
    {
      closeValve();
      unsigned long totalTime = millis() - cardStart;
      consumptions.addConsunmption(bytesId, totalTime);
      cardStart = 0;
      Serial.println("Card saiu");
      Serial.println(strID);
      Serial.print("Tempo total: ");
      Serial.println(totalTime);

      rfid.PICC_HaltA();      // PARADA DA LEITURA DO CARTÃO
      rfid.PCD_StopCrypto1(); // PARADA DA CRIPTOGRAFIA NO PCD
    }
  }
  if (Serial.available())
  { // Enquanto a Serial receber dados
    delay(10);
    String comando = "";

    while (Serial.available())
    {                                 // Enquanto receber comandos
      comando += (char)Serial.read(); // Lê os caractéres
    }
    if (comando == "a")
    {
      Serial.println("recebi a");
    }
    if (comando == "b")
    {
    }
  }
}