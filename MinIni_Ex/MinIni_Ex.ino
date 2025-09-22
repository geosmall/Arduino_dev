#include "src/geoSD/geoSD.h"
#include "src/minIni/minIni.h"
#include "../ci_log.h"

#define LED_1  LED_BUILTIN
#define LED_2  PB5
#define LED_3  PB4

void setup()
{
  pinMode(LED_1, OUTPUT);
  digitalWrite(LED_1, HIGH);

#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
#endif

  CI_LOG("MinIni_Ex Test Starting\n");
  CI_BUILD_INFO();

  if(!SD.begin(PD2)){
    CI_LOG("Card failed, or not present.\n");
    while(1);
  }
  CI_LOG("INI file test start\n");
}

void loop()
{
  minIni ini("setting.ini");
  
  // read INI file
  std::string ip = ini.gets( "network", "ip address" , "none" );
  CI_LOGF("IP Address: %s\n", ip.c_str());

  std::string name = ini.gets( "user", "name" , "none" );
  CI_LOGF("Name: %s\n", name.c_str());

  int id = ini.geti( "user", "id" , -1 );
  CI_LOGF("ID: %d\n", id);

  bool righthanded = ini.getbool( "user", "righthanded" , false );
  CI_LOGF("Handedness: %s\n", righthanded ? "Right" : "Left");

  float k1 = ini.getf( "calibration", "k1" , 0.0 );
  CI_LOG_FLOAT("k1: ", k1, 2);
  CI_LOG("\n");

  float k2 = ini.getf( "calibration", "k2" , 0.0 );
  CI_LOG_FLOAT("k2: ", k2, 2);
  CI_LOG("\n");

  // write INI file
  CI_LOG("New ID...\n");

  if(id == 1234){
    name = "Kida Taro";
    id = 4321;
  }else{
    name = "Isaac Asimov";
    id = 1234;
  }
  ini.put( "user", "name" , name );
  ini.put( "user", "ID" , id );

  name = ini.gets( "user", "name" , "none" );
  CI_LOGF("Name: %s\n", name.c_str());

  id = ini.geti( "user", "id" , -1 );
  CI_LOGF("ID: %d\n", id);

  CI_LOG("INI file test end\n");
  CI_LOG("*STOP*\n");

  while(true){;}

}
