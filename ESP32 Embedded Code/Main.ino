

#include "Configuratie.h"
#include "Executie.h"


float partea_Reala[Lungime_BUFFER];
float partea_Imaginara[Lungime_BUFFER];

ArduinoFFT<float> FFT = ArduinoFFT<float>(partea_Reala, partea_Imaginara, Lungime_BUFFER, RATA_ESANTIONARE);

void setup() {
  Serial.begin(115200);
  delay(300);
  setupAudio();
  Serial.println("=================================================");
  Serial.println(" Recunoastere vocala locala (ESP32 + INMP441)");
  Serial.println(" Comenzi recunoscute: START, POWER, RESET");
  Serial.println("=================================================");
}

void loop() {
  int16_t Buffer_c[Lungime_BUFFER];
  size_t  bytesIn = 0;


  i2s_read(I2S_NUM_0, Buffer_c, sizeof(Buffer_c), &bytesIn, portMAX_DELAY);
  int samplesRead = bytesIn / 2;        
  if (samplesRead <= 0) return;

 
  for (int i = 0; i < Lungime_BUFFER; i++) {
    partea_Reala[i] = (i < samplesRead) ? (float)Buffer_c[i] : 0.0f;
    partea_Imaginara[i] = 0.0f;
  }


  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();


  String rezultat = proceseazaCadru(partea_Reala);

  if (rezultat == CUVANT_NEGASIT) {
    Serial.println("Cuvantul nu este gasit.");
  } else if (rezultat != "") {
    Serial.print("Comanda activata: ");
    Serial.println(rezultat);
    delay(DELAY_DUPA_DETECTIE);  
  }
}
