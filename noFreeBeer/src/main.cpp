#include <Arduino.h>
#include <SoftwareSerial.h>
#include <MFRC522.h>
#include <SPI.h> //INCLUSÃO DE BIBLIOTECA

#define SS_PIN 10 // PINO SDA
#define RST_PIN 9 // PINO DE RESET

MFRC522 rfid(SS_PIN, RST_PIN); // PASSAGEM DE PARÂMETROS REFERENTE AOS PINOS
class consumption
{
  const static int cardsLimit = 50;

private:
  String cardNames[cardsLimit];
  unsigned long consumptions[cardsLimit];
  int firstEmptyIndex;

  int findCardIndex(String name)
  {
    for (int i = 0; i < cardsLimit; i++)
    {
      if (name == cardNames[i])
      {
        return i;
      }
    }
    return 0;
  }

  int setNewCard(String name)
  {
    cardNames[firstEmptyIndex] = name;
    consumptions[firstEmptyIndex] = 0;

    return (firstEmptyIndex++); // rreturn element and add one to the next one
  }

public:
  consumption(){firstEmptyIndex = 1};
  void addConsunmption(String name, unsigned long consumption)
  {
    int cardIndex = findCardIndex(name);
    if (cardIndex == 0)
    { // New card setting new
      cardIndex = setNewCard(name);
    }
    consumptions[cardIndex] += consumption;
  }
};



bool cardDetected;
unsigned long cardStart;
String strID;
consumption consumptions;


void setup()
{
  Serial.begin(9600); // INICIALIZA A SERIAL
  SPI.begin();        // INICIALIZA O BARRAMENTO SPI
  rfid.PCD_Init();    // INICIALIZA MFRC522

  cardDetected = false;
  cardStart = 0;
  consumptions  = consumption();
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
      }
      strID.toUpperCase();
      /***FIM DO BLOCO DE CÓDIGO RESPONSÁVEL POR GERAR A TAG RFID LIDA***/

      Serial.print("Identificador (UID) da tag: "); // IMPRIME O TEXTO NA SERIAL
      Serial.println(strID);                        // IMPRIME NA SERIAL O UID DA TAG RFID

      rfid.PICC_HaltA();      // PARADA DA LEITURA DO CARTÃO
      rfid.PCD_StopCrypto1(); // PARADA DA CRIPTOGRAFIA NO PCD
    }
  }
  else
  {
    if (!rfid.PICC_ReadCardSerial())
    {
      closeValve();
      unsigned long totalTime = millis() - cardStart;
      consumptions.addConsunmption(strID, totalTime);
      cardStart = 0;
      Serial.println("Card saiu");
      Serial.println(strID);
      Serial.print("Tempo total: ");
      Serial.println(totalTime);
    }
  }
}

void openValve()
{
  // TODO: setar rele
}

void closeValve()
{
  // TODO: setar rele
}