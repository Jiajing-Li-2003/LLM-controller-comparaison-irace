#ifndef REFERENCE_MODEL_3_0_H
#define REFERENCE_MODEL_3_0_H

#include "EpuckDAO.h"
#include "RabMessageBuffer.h"

using namespace argos;

class ReferenceModel3Dot0: public EpuckDAO {
  public:
    /*
     *  Class constructor.
     */
    ReferenceModel3Dot0();

    /*
     * Class destructor.
     */
    virtual ~ReferenceModel3Dot0();

    /*
     * Reset function.
     */
    virtual void Reset();

    /*
     * Getter for the proximity reading.
     */
    CCI_EPuckProximitySensor::SReading GetProximityReading();

    /*
     * Setter for the proximity input.
     */
    void SetProximityInput(CCI_EPuckProximitySensor::TReadings s_prox_input);

    /*
     * Getter for the light reading.
     */
    CCI_EPuckLightSensor::SReading GetLightReading();

    /*
     * Setter for the light input.
     */
    void SetLightInput(CCI_EPuckLightSensor::TReadings s_light_input);

    /*
     * Getter for the ground input.
     */
    Real GetGroundReading();

    /*
     * Setter for the ground input.
     */
    void SetGroundInput(CCI_EPuckGroundSensor::SReadings s_ground_input);

    /*
     * Getter for the number of surrounding robots.
     */
    const UInt8 GetNumberNeighbors() const;

    /*
     * Setter for the number of surrounding robots.
     */
    virtual void SetNumberNeighbors(const UInt8& un_number_neighbors);

    /*
     * Getter for the range-and-bearing input.
     */
    std::vector<CCI_EPuckRangeAndBearingSensor::SReceivedPacket*> GetRangeAndBearingMessages() ;

    /*
     * Getter for the vector representing the attraction force to the neighbors computed with RaB messages
     */
    CCI_EPuckRangeAndBearingSensor::SReceivedPacket GetAttractionVectorToNeighbors(Real f_alpha_parameter);

    /*
     * Setter for the range-and-bearing input.
     */
    void SetRangeAndBearingMessages(CCI_EPuckRangeAndBearingSensor::TPackets s_packets);

    /*
     * Getter for the camera input.
     */
    CCI_EPuckOmnidirectionalCameraSensor::SReadings GetCameraInput() const;

    /*
     * Setter for the camera input.
     */
    void SetCameraInput(CCI_EPuckOmnidirectionalCameraSensor::SReadings s_cam_input);

    /*
     * Setter for the RGB LEDs color.
     */
    void SetLEDsColor(const CColor& c_color);

    /*
     * Getter for the RGB LEDs color.
     */
    const CColor& GetLEDsColor() const;


  private:
    /*
     * The proximity sensors input.
     */
    CCI_EPuckProximitySensor::TReadings m_sProximityInput;

    /*
     * The light sensors input.
     */
    CCI_EPuckLightSensor::TReadings m_sLightInput;

    /*
     * The ground sensors input.
     */
    std::deque<CCI_EPuckGroundSensor::SReadings> m_deqGroundInput;

    /*
     * The ground sensors input.
     */
    CCI_EPuckGroundSensor::SReadings m_sGroundInput;

    /*
     * The number of surrounding robots.
     */
    UInt8 m_unNumberNeighbors;

    /*
     * Pointer to the range-and-bearing messages buffer.
     */
    RabMessageBuffer m_pcRabMessageBuffer;

    /*
     * The camera sensor input.
     */
    CCI_EPuckOmnidirectionalCameraSensor::SReadings m_sCameraInput;

    /*
     * The color of RGB LEDs  (output variable).
     */
    CColor m_cLEDsColor;

};

#endif
