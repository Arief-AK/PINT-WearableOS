#include <Arduino.h>
#include <Wire.h>

struct ACCELEROMETER
{
    int16_t x;
    int16_t y;
    int16_t z;

    float res_x;
    float res_y;
    float res_z;
};

struct GYROSCOPE
{
    int16_t x;
    int16_t y;
    int16_t z;

    float res_x;
    float res_y;
    float res_z;
};

class GYRO_SENSOR
{
    public:

        GYRO_SENSOR();
        ~GYRO_SENSOR();

        void initialise();
        void calibrate();

        bool fall_detected();

    private:

        ACCELEROMETER m_accelrometer;
        GYROSCOPE m_gyroscope;

        void read_raw_values();
        void calculate_values();

        bool operate();

        int MPU_ADDRESS = 0x68;  // I2C address of the MPU-6050
        int m_trig1_count, m_trig2_count, m_trig3_count = 0;
        int m_amplitude, m_angle_change = 0;

        bool m_fell, m_trig1, m_trig2, m_trig3 = false;
};