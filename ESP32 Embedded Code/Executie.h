#ifndef EXECUTIE_H
#define EXECUTIE_H



#include <driver/i2s.h>
#include <arduinoFFT.h>
#include "Config.h"


#define CUVANT_NEGASIT "__NEGASIT__"


int stareStart = 0;  unsigned long timpStart = 0;
int stareReset = 0;  unsigned long timpReset = 0;
int starePower = 0;  unsigned long timpPower = 0;


bool          sesiuneActiva  = false;   
bool          cuvantValidat  = false;   
unsigned long timpUltimaVoce = 0;       
unsigned long timpDebug      = 0;      


float zona_Joase  = 0;
float zona_Medii  = 0;
float zona_Inalte = 0;


void setupAudio() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = RATA_ESANTIONARE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
}


void calculeazaBenzi(float* data) {
  zona_Joase = 0;
  zona_Medii = 0;
  zona_Inalte = 0;

  for (int i = 2; i < Lungime_BUFFER / 2; i++) {
    if (i >= 12  && i <= 32)  zona_Joase  += data[i];  
    if (i >= 35  && i <= 80)  zona_Medii  += data[i];  
    if (i >= 180 && i <= 450) zona_Inalte += data[i];   
  }
}


void reseteazaMasini() {
  stareStart = 0;
  stareReset = 0;
  starePower = 0;
}

String ruleazaMasini() {


  if (stareStart == 0) {
    if (zona_Inalte > PRAG_INALTE && zona_Inalte > zona_Medii * 1.5) {
      stareStart = 1;
      timpStart = millis();
      if (MOD_DEBUG) Serial.println("Faza 1: START");
    }
  } else if (stareStart == 1) {
    if (millis() - timpStart > TIMP_MAX_FAZA) {
      stareStart = 0;
      if (MOD_DEBUG) Serial.println("Reset faza START");
    } else if (zona_Medii > PRAG_MEDII && zona_Medii > zona_Inalte * 1.2) {
      stareStart = 0;
      return "START";
    }
  }

 
  if (stareReset == 0) {
    if (zona_Medii > PRAG_MEDII && zona_Medii > zona_Inalte * 1.5 && zona_Medii > zona_Joase * 1.3) {
      stareReset = 1;
      timpReset = millis();
      if (MOD_DEBUG) Serial.println("Faza 1: RESET");
    }
  } else if (stareReset == 1) {
    if (millis() - timpReset > TIMP_MAX_FAZA) {
      stareReset = 0;
      if (MOD_DEBUG) Serial.println("Reset faza RESET");
    } else if (zona_Inalte > PRAG_INALTE && zona_Inalte > zona_Medii * 1.5) {
      stareReset = 0;
      return "RESET";
    }
  }

 
  if (starePower == 0) {
    if (zona_Joase > PRAG_JOASE && zona_Joase > zona_Medii * 1.5 && zona_Joase > zona_Inalte * 2.0) {
      starePower = 1;
      timpPower = millis();
      if (MOD_DEBUG) Serial.println("Faza 1: POWER");
    }
  } else if (starePower == 1) {
    if (millis() - timpPower > TIMP_MAX_FAZA) {
      starePower = 0;
      if (MOD_DEBUG) Serial.println("Reset faza POWER");
    } else if (zona_Medii > PRAG_MEDII && zona_Medii > zona_Joase * 1.2) {
      starePower = 0;
      return "POWER";
    }
  }

  return "";
}


String proceseazaCadru(float* data) {
  calculeazaBenzi(data);
  float energieTotala = zona_Joase + zona_Medii + zona_Inalte;
  bool  vocePrezenta  = (energieTotala > PRAG_VOCE);

  
  if (MOD_DEBUG && (millis() - timpDebug > 80)) {
    timpDebug = millis();
    Serial.print("Joase: ");  Serial.print(zona_Joase);
    Serial.print(" | Medii: ");  Serial.print(zona_Medii);
    Serial.print(" | Inalte: "); Serial.println(zona_Inalte);
  }

  
  if (vocePrezenta) {
    timpUltimaVoce = millis();
    if (!sesiuneActiva) {
      sesiuneActiva = true;
      cuvantValidat = false;
      reseteazaMasini();
      if (MOD_DEBUG) Serial.println(">> Rostire detectata...");
    }
  }

  
  String cuvant = ruleazaMasini();
  if (cuvant != "") {
    cuvantValidat = true;
    return cuvant;   
  }


  if (sesiuneActiva && (millis() - timpUltimaVoce > TIMP_LINISTE)) {
    sesiuneActiva = false;
    reseteazaMasini();
    if (!cuvantValidat) {
      return CUVANT_NEGASIT;   
    }
  }

  return "";
}

#endif 
