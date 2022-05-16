#include "HR_SENSOR.h"

HR_SENSOR::HR_SENSOR()
{
    m_BPM = 0.00f;
    m_average_BPM = 0;

    m_patient_age = 0;
    m_blood_oxygen = 0.00f;
    m_average_blood_oxygen = 0;
}

HR_SENSOR::~HR_SENSOR()
{
    delete this;
}

void HR_SENSOR::initialise(MAX30105 particleSensor, int age)
{
    m_patient_age = age;
    m_particleSensor = particleSensor;

    // Initialize sensor
    if (!m_particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
    {
        Serial.println("MAX30105 was not found. Please check wiring/power. ");
        while (1);
    }

    // Notify sensor is ready
    Serial.println("Place your index finger on the sensor with steady pressure.");

    // Configure sensor with default settings
    m_particleSensor.setup();

    // Turn Red LED to low to indicate sensor is running
    m_particleSensor.setPulseAmplitudeRed(0x0A);
    
    // Turn off Green LED
    m_particleSensor.setPulseAmplitudeGreen(0);
}

bool HR_SENSOR::is_critical()
{
    // Calculate maximum heart-rate in reference to patient age
    int max_heart_rate = 220 - m_patient_age;

    // Compare to current BPM
    if(m_BPM > max_heart_rate || m_BPM < RELIABLE_MIN_THRESHOLD)
    {
        // State is critical
        return true;
    }
    else
    {
        // State is normal
        return false;
    }
}

void HR_SENSOR::calculate_heart_rate()
{
    // Set temporary IR value
    long irValue = m_particleSensor.getIR();

    if (checkForBeat(irValue) == true)
    {
        //We sensed a beat!
        long delta = millis() - lastBeat;
        lastBeat = millis();

        // Calculate beats per minute
        m_BPM = 60 / (delta / 1000.0);

        // Check if BPM is reliable
        if (m_BPM < RELIABLE_MAX_THRESHOLD && m_BPM > RELIABLE_MIN_THRESHOLD)
        {
            //Store this reading in the array
            rates[rateSpot++] = (byte)m_BPM;
            
            // Wrap variable
            rateSpot %= RATE_SIZE;

            //Take average of readings
            m_average_BPM = 0;

            // Sum up values in the array
            for (byte x = 0 ; x < RATE_SIZE ; x++){
                m_average_BPM += rates[x];
            }

            // Calculate average BPM
            m_average_BPM /= RATE_SIZE;
        }
    }
}

// TODO: Calculate blood oxygen
void HR_SENSOR::calculate_blood_oxygen()
{
    // TBA
}

float HR_SENSOR::get_heart_rate(){
    return m_BPM;
}

int HR_SENSOR::get_average_heart_rate(){
    return m_average_BPM;
}

float HR_SENSOR::get_blood_oxygen(){
    calculate_blood_oxygen();
    return m_blood_oxygen;
}

int HR_SENSOR::get_average_blood_oxygen(){
    return m_average_blood_oxygen;
}