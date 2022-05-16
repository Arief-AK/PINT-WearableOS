#include <Wire.h>

#include "MAX30105.h"
#include "heartRate.h"

#define RATE_SIZE 4
#define RELIABLE_MAX_THRESHOLD 200
#define RELIABLE_MIN_THRESHOLD 60

class HR_SENSOR
{
    public:

        HR_SENSOR();
        ~HR_SENSOR();

        void initialise(MAX30105 particleSensor, int age);
        void calculate_heart_rate();
        
        float get_heart_rate();
        float get_blood_oxygen();

        int get_average_heart_rate();
        int get_average_blood_oxygen();

        bool is_critical();

    private:

        void calculate_blood_oxygen();

        MAX30105 m_particleSensor;

        byte rates[RATE_SIZE];      //Array of heart rates
        byte rateSpot = 0;
        long lastBeat = 0;          //Time at which the last beat occurred

        float m_BPM;
        float m_blood_oxygen;

        int m_patient_age;
        int m_average_BPM;
        int m_average_blood_oxygen;

};