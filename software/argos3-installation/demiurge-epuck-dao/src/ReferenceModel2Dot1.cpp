#include "ReferenceModel2Dot1.h"

/****************************************/
/****************************************/

ReferenceModel2Dot1::ReferenceModel2Dot1() {
  m_pcRng = CRandom::CreateRNG("argos");
  m_pcRabMessageBuffer = RabMessageBuffer();
  m_pcRabMessageBuffer.SetTimeLife(10);
  m_fMaxVelocity = 12;
  m_fLeftWheelVelocity = 0;
  m_fRightWheelVelocity = 0;
}

/****************************************/
/****************************************/

ReferenceModel2Dot1::~ReferenceModel2Dot1() {}

/****************************************/
/****************************************/

void ReferenceModel2Dot1::Reset() {
  m_fLeftWheelVelocity = 0;
  m_fRightWheelVelocity = 0;
  m_pcRabMessageBuffer.Reset();
}

/****************************************/
/****************************************/

CCI_EPuckProximitySensor::TReadings ReferenceModel2Dot1::GetProximityInput() const {
  return m_sProximityInput;
}

/****************************************/
/****************************************/

void ReferenceModel2Dot1::SetProximityInput(CCI_EPuckProximitySensor::TReadings s_prox_input) {
  m_sProximityInput = s_prox_input;
}

/****************************************/
/****************************************/

CCI_EPuckLightSensor::TReadings ReferenceModel2Dot1::GetLightInput() const{
  return m_sLightInput;
}

/****************************************/
/****************************************/

void ReferenceModel2Dot1::SetLightInput(CCI_EPuckLightSensor::TReadings s_light_input) {
  m_sLightInput = s_light_input;
}

/****************************************/
/****************************************/

CCI_EPuckGroundSensor::SReadings ReferenceModel2Dot1::GetGroundInput() {
  std::deque<CCI_EPuckGroundSensor::SReadings>::iterator it;
  UInt32 unBlackWhiteCounter[2] = {0,0};  //unBlackWhiteCounter[0] -> Black; unBlackWhiteCounter[1] -> White.
  float fBlackThreshold = 0.03;
  float fWhiteThreshold = 0.85;
  for (it = m_deqGroundInput.begin(); it != m_deqGroundInput.end(); it++) {
    if (it->Left < fBlackThreshold) {
      unBlackWhiteCounter[0] += 1;
    }
    else if (it->Left > fWhiteThreshold) {
      unBlackWhiteCounter[1] += 1;
    }
    if (it->Center < fBlackThreshold) {
      unBlackWhiteCounter[0] +=1;
    }
    else if (it->Center > fWhiteThreshold) {
      unBlackWhiteCounter[1] += 1;
    }
    if (it->Right < fBlackThreshold) {
      unBlackWhiteCounter[0] +=1;
    }
    else if (it->Right > fWhiteThreshold) {
      unBlackWhiteCounter[1] += 1;
    }
  }

  CCI_EPuckGroundSensor::SReadings readings;
  if (unBlackWhiteCounter[0] > 10) {
    readings.Left = 0;
    readings.Center = 0;
    readings.Right = 0;
  }
  else if (unBlackWhiteCounter[1] > 10) {
    readings.Left = 1;
    readings.Center = 1;
    readings.Right = 1;
  }
  else {
    readings.Left = 0.5;
    readings.Center = 0.5;
    readings.Right = 0.5;
  }

  return readings;
}

/****************************************/
/****************************************/

void ReferenceModel2Dot1::SetGroundInput(CCI_EPuckGroundSensor::SReadings s_ground_input) {
  m_deqGroundInput.push_back(s_ground_input);
  if (m_deqGroundInput.size() > 5) {
    m_deqGroundInput.pop_front();
  }
}

/****************************************/
/****************************************/

const UInt8 ReferenceModel2Dot1::GetNumberNeighbors() const {
  return m_unNumberNeighbors;
}

/****************************************/
/****************************************/

void ReferenceModel2Dot1::SetNumberNeighbors(const UInt8& un_number_neighbors){
  m_unNumberNeighbors = un_number_neighbors;
}

/****************************************/
/****************************************/

CCI_EPuckRangeAndBearingSensor::SReceivedPacket ReferenceModel2Dot1::GetNeighborsCenterOfMass() {
  CCI_EPuckRangeAndBearingSensor::TPackets sRabPackets = m_pcRabMessageBuffer.GetMessages();
  CCI_EPuckRangeAndBearingSensor::TPackets::iterator it;
  CVector2 sRabVectorSum(0,CRadians::ZERO);

  for (it = sRabPackets.begin(); it != sRabPackets.end(); it++) {
    if ((*it)->Data[0] != (UInt32) EpuckDAO::GetRobotIdentifier()) {
      sRabVectorSum += CVector2((*it)->Range ,(*it)->Bearing.SignedNormalize());
    }
  }
  //sRabVectorSum /= sRabPackets.size();

  CCI_EPuckRangeAndBearingSensor::SReceivedPacket cRaBReading;
  cRaBReading.Range = sRabVectorSum.Length();
  cRaBReading.Bearing = sRabVectorSum.Angle().SignedNormalize();

  return cRaBReading;
}

/****************************************/
/****************************************/

CCI_EPuckRangeAndBearingSensor::SReceivedPacket ReferenceModel2Dot1::GetAttractionVectorToNeighbors(Real f_alpha_parameter) {
  CCI_EPuckRangeAndBearingSensor::TPackets sRabPackets = m_pcRabMessageBuffer.GetMessages();
  CCI_EPuckRangeAndBearingSensor::TPackets::iterator it;
  CVector2 sRabVectorSum(0,CRadians::ZERO);

  for (it = sRabPackets.begin(); it != sRabPackets.end(); it++) {
    if (((*it)->Data[0] != (UInt32) EpuckDAO::GetRobotIdentifier()) && ((*it)->Range > 0.0f)) {
      sRabVectorSum += CVector2((f_alpha_parameter / (Real) (1 + (*it)->Range)), (*it)->Bearing.SignedNormalize());
    }
  }

  CCI_EPuckRangeAndBearingSensor::SReceivedPacket cRaBReading;
  cRaBReading.Range = sRabVectorSum.Length();
  cRaBReading.Bearing = sRabVectorSum.Angle().SignedNormalize();

  return cRaBReading;
}

/****************************************/
/****************************************/

CCI_EPuckRangeAndBearingSensor::SReceivedPacket ReferenceModel2Dot1::GetAttractionVectorToMessagingNeighbors(Real f_alpha_parameter, UInt8 un_message) {
  CCI_EPuckRangeAndBearingSensor::TPackets sRabPackets = m_pcRabMessageBuffer.GetMessages();
  CCI_EPuckRangeAndBearingSensor::TPackets::iterator it;
  CVector2 sRabVectorSum(0,CRadians::ZERO);

  for (it = sRabPackets.begin(); it != sRabPackets.end(); it++) {
    if ( ((*it)->Data[0] != (UInt32) GetRobotIdentifier()) && ((*it)->Range > 0.0f) && ( (UInt8) ((*it)->Data[1]) == un_message) ) {
      sRabVectorSum += CVector2((f_alpha_parameter / (Real) (1 + (*it)->Range)), (*it)->Bearing.SignedNormalize());
    }
  }

  CCI_EPuckRangeAndBearingSensor::SReceivedPacket cRaBReading;
  cRaBReading.Range = sRabVectorSum.Length();
  cRaBReading.Bearing = sRabVectorSum.Angle().SignedNormalize();

  return cRaBReading;
}

/****************************************/
/****************************************/

std::vector<CCI_EPuckRangeAndBearingSensor::SReceivedPacket*> ReferenceModel2Dot1::GetRangeAndBearingMessages() {
  return m_pcRabMessageBuffer.GetMessages();
}

/****************************************/
/****************************************/

void ReferenceModel2Dot1::SetRangeAndBearingMessages(CCI_EPuckRangeAndBearingSensor::TPackets s_packets) {
  std::map<UInt32, CCI_EPuckRangeAndBearingSensor::SReceivedPacket*> mapRemainingMessages;
  std::map<UInt32, CCI_EPuckRangeAndBearingSensor::SReceivedPacket*>::iterator mapIt;
  CCI_EPuckRangeAndBearingSensor::TPackets::iterator it;
  m_unNumberNeighbors = 0;
  for (it = s_packets.begin(); it < s_packets.end(); ++it) {
    if ((*it)->Data[0] != m_unRobotIdentifier) {
      if (mapRemainingMessages.find((*it)->Data[0]) != mapRemainingMessages.end()) {  // If ID not in map, add message.
        mapRemainingMessages[(*it)->Data[0]] = (*it);
      } else if ((*it)->Bearing != CRadians::ZERO) {  // If ID there, overwrite only if the message is valid (correct range and bearing information)
        mapRemainingMessages[(*it)->Data[0]] = (*it);
      }
    }
  }
  //m_unNumberMessagingNeighbors = 0;
  for (mapIt = mapRemainingMessages.begin(); mapIt != mapRemainingMessages.end(); ++mapIt) {
    m_pcRabMessageBuffer.AddMessage((*mapIt).second);
    m_unNumberNeighbors += 1;
    // if ((*mapIt).second->Data[1] == 85) { //check data1 message of gianduja
    //     m_unNumberMessagingNeighbors++;
    // }
  }
  m_pcRabMessageBuffer.Update();
}

/****************************************/
/****************************************/

void ReferenceModel2Dot1::SetRangeAndBearingMessageToSend(UInt8 un_message) {
    m_unMessageToSend = un_message;
}

/****************************************/
/****************************************/

const UInt8 ReferenceModel2Dot1::GetMessageToSend() const {
    return m_unMessageToSend;
}

/****************************************/
/****************************************/

UInt8 ReferenceModel2Dot1::GetNumberMessagingNeighbors(UInt8 un_message) {

    CCI_EPuckRangeAndBearingSensor::TPackets sLastPackets = GetRangeAndBearingMessages();
    CCI_EPuckRangeAndBearingSensor::TPackets::iterator it;
    UInt8 unNumberMessagingNeighbors = 0;

    for (it = sLastPackets.begin(); it != sLastPackets.end(); it++) {
        if ( ( (UInt8) (*it)->Data[0] != GetRobotIdentifier() ) && ( (UInt8) (*it)->Data[1] == un_message ) ) {
            unNumberMessagingNeighbors+=1;
        }
    }
    return unNumberMessagingNeighbors;
}

/****************************************/
/****************************************/

SInt8 ReferenceModel2Dot1::GetDiffMessagingNeighbors(UInt8 un_message, UInt8 un_message2) {
    CCI_EPuckRangeAndBearingSensor::TPackets sLastPackets = GetRangeAndBearingMessages();
    CCI_EPuckRangeAndBearingSensor::TPackets::iterator it;
    SInt8 FirstNumberMessagingNeighbors = 0;
    SInt8 SecondNumberMessagingNeighbors = 0;

    for (it = sLastPackets.begin(); it != sLastPackets.end(); it++) {
        if ( (UInt8) ((*it)->Data[0] != GetRobotIdentifier()) && (( (UInt8) ((*it)->Data[1])&0xF0) == un_message || ((UInt8) ((*it)->Data[1])&0x0F) == un_message ) ) {
            FirstNumberMessagingNeighbors += 1;
        }
        if ( (UInt8) ((*it)->Data[0] != GetRobotIdentifier()) && (( (UInt8) ((*it)->Data[1])&0xF0) == un_message2 || ((UInt8) ((*it)->Data[1])&0x0F) == un_message2 ) ) {
            SecondNumberMessagingNeighbors += 1;
        }
    }
    return (SecondNumberMessagingNeighbors-FirstNumberMessagingNeighbors);
}
