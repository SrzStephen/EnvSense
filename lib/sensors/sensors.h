#include "string.h"
#include "MHZ19.h"
#include "PMS.h"
#include "bsec.h"
#include "ArduinoJson.h"
#include <NTPClient.h>
#define MAX_ATTEMPTS 5

const char compile_datetime[] = __DATE__ " " __TIME__;
class Sensors
{
    public:
        Sensors(PMS& pms7003,Bsec& bme680,MHZ19& mhz19, NTPClient& ntpclient);
        String generate_payload (void);
        void set_payload_version(void);
        void set_payload_device_name(void);
        JsonObject generate_mhz19(JsonObject mhz19);
        JsonObject generate_pms(JsonObject pms7003);
        JsonObject generate_bme680(JsonObject bme680);
        bool check_all_sensors_status(void);
        bool check_pms(void);
        bool check_bme(void);
        bool check_mhz19(void);
        bool check_ntp(void);

    private:
        long get_unix_time_now(void);
        String _software_version = "undefined";
        String _device_name = "undefined";
        MHZ19* _MHZ19;
        Bsec* _BME680;
        PMS* _PMS7003;
        NTPClient* _ntpclient;
        long _first_report_time;
        long _messages;
};