#ifndef REFERENCE_MODEL_2_1_H
#define REFERENCE_MODEL_2_1_H

#include "EpuckDAO.h"
#include "RabMessageBuffer.h"

// Ref model 2.1 is the same as ref model 1.1 except it can send messages
// with gianduja1 (one message) (use for evocom1.x)

using namespace argos;

class ReferenceModel2Dot1: public EpuckDAO {
  public:
    /*
     *  Class constructor.
     */
    ReferenceModel2Dot1();

    /*
     * Class destructor.
     */
    virtual ~ReferenceModel2Dot1();

    /*
     * Reset function.
     */
    virtual void Reset();

    /*
     * Getter for the proximity input.
     */
    CCI_EPuckProximitySensor::TReadings GetProximityInput() const;

    /*
     * Setter for the proximity input.
     */
    void SetProximityInput(CCI_EPuckProximitySensor::TReadings s_prox_input);

    /*
     * Getter for the light input.
     */
    CCI_EPuckLightSensor::TReadings GetLightInput() const;

    /*
     * Setter for the light input.
     */
    void SetLightInput(CCI_EPuckLightSensor::TReadings s_light_input);

    /*
     * Getter for the ground input.
     */
    CCI_EPuckGroundSensor::SReadings GetGroundInput();

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
     * Getter for the center of mass of neighbors computed with RaB messages
     */
    CCI_EPuckRangeAndBearingSensor::SReceivedPacket GetNeighborsCenterOfMass();

    /*
     * Getter for the vector representing the attraction force to the neighbors computed with RaB messages
     */
    CCI_EPuckRangeAndBearingSensor::SReceivedPacket GetAttractionVectorToNeighbors(Real f_alpha_parameter);

    /*
     * Getter for the vector representing the attraction force to the neighbors that are sending a message computed with RaB messages
     */
    CCI_EPuckRangeAndBearingSensor::SReceivedPacket GetAttractionVectorToMessagingNeighbors(Real f_alpha_parameter, UInt8 un_message);

    /*
     * Setter for the range-and-bearing input.
     */
    void SetRangeAndBearingMessages(CCI_EPuckRangeAndBearingSensor::TPackets s_packets);

    /*
     * Getter for the message to send.
     */
    const UInt8 GetMessageToSend() const;

    /*
     * Setter for the message to send with range and bearing
     */
    void SetRangeAndBearingMessageToSend(UInt8 un_message);

    /*
     * Getter for the number of messaging neighbors
     */
    UInt8 GetNumberMessagingNeighbors(UInt8 un_message);

    /*
     * Getter for the difference of number between the messages that the robot got
     */
    SInt8 GetDiffMessagingNeighbors(UInt8 un_message, UInt8 un_message2);

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

    // /*
    //  * The number of surrounding robots messaging.
    //  */
    // UInt8 m_unNumberMessagingNeighbors;

    /*
     * Pointer to the range-and-bearing messages buffer.
     */
    RabMessageBuffer m_pcRabMessageBuffer;
    /*
     * Type of message to send.
     */
    UInt8 m_unMessageToSend;
};

#endif
