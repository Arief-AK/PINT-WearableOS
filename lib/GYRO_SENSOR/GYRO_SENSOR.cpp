#include "GYRO_SENSOR.h"

GYRO_SENSOR::GYRO_SENSOR()
{
    m_accelrometer.x = 0;
    m_accelrometer.y = 0;
    m_accelrometer.z = 0;

    m_gyroscope.x = 0;
    m_gyroscope.y = 0;
    m_gyroscope.z = 0;
}

GYRO_SENSOR::~GYRO_SENSOR()
{
    delete this;
}

void GYRO_SENSOR::initialise()
{
    Wire.begin();
    Wire.beginTransmission(MPU_ADDRESS);
    Wire.write(0x6B);  // PWR_MGMT_1 register
    Wire.write(0);     // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);

    Serial.println("MPU-6050 is awake");
}

void GYRO_SENSOR::read_raw_values()
{
    Wire.beginTransmission(MPU_ADDRESS);
    Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);

    Wire.requestFrom(MPU_ADDRESS,14,true);  // request a total of 14 registers

    m_accelrometer.x = Wire.read() << 8 | Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
    m_accelrometer.y = Wire.read() << 8 | Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    m_accelrometer.z = Wire.read() << 8 | Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)

    m_gyroscope.x = Wire.read() << 8 | Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    m_gyroscope.y = Wire.read() << 8 | Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    m_gyroscope.z = Wire.read() << 8 | Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}

void GYRO_SENSOR::calculate_values()
{
    // Calculate the axis with modifying values in regards to sensor error of each axis
    m_accelrometer.res_x = (m_accelrometer.x - 2050) / 16384.00;
    m_accelrometer.res_y = (m_accelrometer.y - 77) / 16384.00;
    m_accelrometer.res_z = (m_accelrometer.z - 1947) / 16384.00;

    m_gyroscope.res_x = (m_gyroscope.x + 270) / 131.07;
    m_gyroscope.res_y = (m_gyroscope.y - 351) / 131.07;
    m_gyroscope.res_z = (m_gyroscope.z + 136) / 131.07;

    // Calculate the magnitude of the vector
    float raw_amplitude = pow(pow(m_accelrometer.res_x,2)+pow(m_accelrometer.res_y,2)+pow(m_accelrometer.res_z,2),0.5);

    // Amplify the values for better analysis
    m_amplitude = raw_amplitude * 10;  // Mulitiplied by 10 bcz values are between 0 to 1
}

bool GYRO_SENSOR::operate()
{
    bool fall = false;

    // Read raw values from sensor
    read_raw_values();

    // Calculate raw values and obtain amplitude
    calculate_values();

    // Perform check for fall detection
    if (m_amplitude <= 2 && m_trig2 == false)
    {
        //if AM breaks lower threshold (0.4g)
        m_trig1=true;

        Serial.println("TRIGGER 1 ACTIVATED");
    
    }
    if (m_trig1 == true)
    {
        m_trig1_count++;
        if (m_amplitude >= 12)
        { 
            //if AM breaks upper threshold (3g)
            m_trig2=true;
            m_trig1=false;
            m_trig1_count=0;

            Serial.println("TRIGGER 2 ACTIVATED");
        }
    }
    if (m_trig2 == true)
    {
        m_trig2_count++;
        m_angle_change = pow(pow(m_gyroscope.res_x,2)+pow(m_gyroscope.res_y,2)+pow(m_gyroscope.res_z,2),0.5);
        
        Serial.println(m_angle_change);

        if (m_angle_change >= 30 && m_angle_change <= 400)
        { 
            //if orientation changes by between 80-100 degrees
            m_trig3=true;
            m_trig2=false;
            m_trig2_count=0;

            Serial.println(m_angle_change);
            Serial.println("TRIGGER 3 ACTIVATED");
        }
    }
    if (m_trig3 == true)
    {
        m_trig3_count++;
        
        if (m_trig3_count >= 10)
        { 
            m_angle_change = pow(pow(m_gyroscope.res_x,2)+pow(m_gyroscope.res_y,2)+pow(m_gyroscope.res_z,2),0.5);
            //delay(10);
            
            Serial.println("THIS IS HERE: " + m_angle_change);

            if ((m_angle_change>=0) && (m_angle_change<=40))
            { 
                //if orientation changes remains between 0-10 degrees
                fall=true;
                m_trig3=false;
                m_trig3_count=0;

                Serial.println(m_angle_change);
            }
            else
            { 
                //user regained normal orientation
                m_trig3=false;
                m_trig3_count=0;
                
                Serial.println("TRIGGER 3 DEACTIVATED");
            }
        }
    }

    if (m_trig2_count >= 6)
    { 
        //allow 0.5s for orientation change
        m_trig2=false;
        m_trig2_count=0;
        
        //Serial.println("TRIGGER 2 DECACTIVATED");
    }

    if (m_trig1_count >= 6)
    { 
        //allow 0.5s for AM to break upper threshold
        m_trig1=false;
        m_trig1_count=0;
        
        //Serial.println("TRIGGER 1 DECACTIVATED");
    }

    // Provide a delay for processing
    delay(100);

    // Return falling flag
    return fall;
}

bool GYRO_SENSOR::fall_detected()
{
    m_fell = operate();

    if(m_fell)
    {
        // Reset falling flag
        m_fell = false;
        return true;
    }
    else
    {
        return false;
    }
}