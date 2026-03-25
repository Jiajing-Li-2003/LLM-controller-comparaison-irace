#include "ReferenceModel2Dot3.h"

/****************************************/
/****************************************/

ReferenceModel2Dot3::ReferenceModel2Dot3() {
  m_pcRng = CRandom::CreateRNG("argos");
  m_pcRabMessageBuffer = RabMessageBuffer();
  m_pcRabMessageBuffer.SetTimeLife(10);
  m_fMaxVelocity = 12;
  m_fLeftWheelVelocity = 0;
  m_fRightWheelVelocity = 0;
}

/****************************************/
/****************************************/

ReferenceModel2Dot3::~ReferenceModel2Dot3() {}

/****************************************/
/****************************************/

void ReferenceModel2Dot3::Reset() {
  m_fLeftWheelVelocity = 0;
  m_fRightWheelVelocity = 0;
  m_pcRabMessageBuffer.Reset();
}

/****************************************/
/****************************************/

CCI_EPuckProximitySensor::SReading ReferenceModel2Dot3::GetProximityReading() {
  CCI_EPuckProximitySensor::SReading cOutputReading;
  CVector2 cSumProxi(0, CRadians::ZERO);
  for (UInt8 i = 0; i < m_sProximityInput.size(); i++) {
    cSumProxi += CVector2(m_sProximityInput[i].Value, m_sProximityInput[i].Angle.SignedNormalize());
  }

  cOutputReading.Value = (cSumProxi.Length() > 1) ? 1 : cSumProxi.Length();
  cOutputReading.Angle = cSumProxi.Angle().SignedNormalize();

  return cOutputReading;
}

/****************************************/
/****************************************/

void ReferenceModel2Dot3::SetProximityInput(CCI_EPuckProximitySensor::TReadings s_prox_input) {
  m_sProximityInput = s_prox_input;
}

/****************************************/
/****************************************/

CCI_EPuckLightSensor::SReading ReferenceModel2Dot3::GetLightReading() {
  CCI_EPuckLightSensor::SReading cOutputReading;
  CVector2 cSumLight(0, CRadians::ZERO);
	for (UInt8 i = 0; i < m_sLightInput.size(); i++) {
    //if (m_sLightInput[i].Value > 0.1) {
      cOutputReading.Value = m_sLightInput[i].Value;
    //}
		cSumLight += CVector2(m_sLightInput[i].Value, m_sLightInput[i].Angle.SignedNormalize());
	}
  //if (cOutputReading.Value == 1) {
    cOutputReading.Angle = cSumLight.Angle().SignedNormalize();
  //}
  return cOutputReading;
}

/****************************************/
/****************************************/

void ReferenceModel2Dot3::SetLightInput(CCI_EPuckLightSensor::TReadings s_light_input) {
  m_sLightInput = s_light_input;
}

/****************************************/
/****************************************/

Real ReferenceModel2Dot3::GetGroundReading() {
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

  if (unBlackWhiteCounter[0] > 10) {
    return 0.0f;
  }
  else if (unBlackWhiteCounter[1] > 10) {
    return 1.0f;
  }
  else {
    return 0.5f;
  }
}

/****************************************/
/****************************************/

void ReferenceModel2Dot3::SetGroundInput(CCI_EPuckGroundSensor::SReadings s_ground_input) {
  m_deqGroundInput.push_back(s_ground_input);
  if (m_deqGroundInput.size() > 5) {
    m_deqGroundInput.pop_front();
  }
}

/****************************************/
/****************************************/

const UInt8 ReferenceModel2Dot3::GetNumberNeighbors() const {
  return m_unNumberNeighbors;
}

/****************************************/
/****************************************/

void ReferenceModel2Dot3::SetNumberNeighbors(const UInt8& un_number_neighbors){
  m_unNumberNeighbors = un_number_neighbors;
}

/****************************************/
/****************************************/

CCI_EPuckRangeAndBearingSensor::SReceivedPacket ReferenceModel2Dot3::GetAttractionVectorToNeighbors(Real f_alpha_parameter) {
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

CCI_EPuckRangeAndBearingSensor::SReceivedPacket ReferenceModel2Dot3::GetAttractionVectorToMessagingNeighbors(Real f_alpha_parameter, UInt8 un_message) {
  CCI_EPuckRangeAndBearingSensor::TPackets sRabPackets = m_pcRabMessageBuffer.GetMessages();
  CCI_EPuckRangeAndBearingSensor::TPackets::iterator it;
  CVector2 sRabVectorSum(0,CRadians::ZERO);

  for (it = sRabPackets.begin(); it != sRabPackets.end(); it++) {
    if ( ((*it)->Data[0] != (UInt32) GetRobotIdentifier()) && ((*it)->Range > 0.0f) && (( (UInt8) (*it)->Data[1]) == un_message) ) {
      sRabVectorSum += CVector2(f_alpha_parameter / ((*it)->Range + 1),(*it)->Bearing.SignedNormalize());
      //sRabVectorSum += CVector2(f_alpha_parameter/std::pow(((*it)->Range/100),2),(*it)->Bearing.SignedNormalize());
      //sRabVectorSum += CVector2(f_alpha_parameter/((*it)->Range + 1),(*it)->Bearing.SignedNormalize());
    }
  }

  CCI_EPuckRangeAndBearingSensor::SReceivedPacket cRaBReading;
  cRaBReading.Range = sRabVectorSum.Length();
  cRaBReading.Bearing = sRabVectorSum.Angle().SignedNormalize();

  return cRaBReading;
}

/****************************************/
/****************************************/

std::vector<CCI_EPuckRangeAndBearingSensor::SReceivedPacket*> ReferenceModel2Dot3::GetRangeAndBearingMessages() {
  return m_pcRabMessageBuffer.GetMessages();
}

/****************************************/
/****************************************/

void ReferenceModel2Dot3::SetRangeAndBearingMessages(CCI_EPuckRangeAndBearingSensor::TPackets s_packets) {
  std::map<UInt32, CCI_EPuckRangeAndBearingSensor::SReceivedPacket*> mapRemainingMessages;
  std::map<UInt32, CCI_EPuckRangeAndBearingSensor::SReceivedPacket*>::iterator mapIt;
  CCI_EPuckRangeAndBearingSensor::TPackets::iterator it;
  m_unNumberNeighbors = 0;
  for (it = s_packets.begin(); it < s_packets.end(); ++it) {
    if ((*it)->Data[0] != m_unRobotIdentifier) {
      if (mapRemainingMessages.find((*it)->Data[0]) != mapRemainingMessages.end()) {  // If ID not in map, add message.
        mapRemainingMessages[(*it)->Data[0]] = (*it);
      } else if ((*it)->Bearing != CRadians::ZERO){  // If ID there, overwrite only if the message is valid (correct range and bearing information)
        mapRemainingMessages[(*it)->Data[0]] = (*it);
      }
    }
  }
  for (mapIt = mapRemainingMessages.begin(); mapIt != mapRemainingMessages.end(); ++mapIt) {
    m_pcRabMessageBuffer.AddMessage((*mapIt).second);
    m_unNumberNeighbors += 1;
  }
  m_pcRabMessageBuffer.Update();
}

/****************************************/
/****************************************/

void ReferenceModel2Dot3::SetRangeAndBearingMessageToSend(UInt8 un_message) {
    m_unMessageToSend = un_message;
}

/****************************************/
/****************************************/

const UInt8 ReferenceModel2Dot3::GetMessageToSend() const {
    return m_unMessageToSend;
}

/****************************************/
/****************************************/

UInt8 ReferenceModel2Dot3::GetNumberMessagingNeighbors(UInt8 un_message) {
    CCI_EPuckRangeAndBearingSensor::TPackets sLastPackets = GetRangeAndBearingMessages();
    CCI_EPuckRangeAndBearingSensor::TPackets::iterator it;
    UInt8 unNumberMessagingNeighbors = 0;

    // for (it = sLastPackets.begin(); it != sLastPackets.end(); it++) {
    //     if ((*it)->Data[1] != 0) {
    //         LOG << "mess: "<< "id:" << (*it)->Data[0] << "data:" << (*it)->Data[1] << std::endl;
    //     }
    // }

    for (it = sLastPackets.begin(); it != sLastPackets.end(); it++) {
        if ( (UInt8) ((*it)->Data[0] != GetRobotIdentifier()) && ( (UInt8) ((*it)->Data[1]) == un_message) ) {
            unNumberMessagingNeighbors+=1;
        }
    }

    return unNumberMessagingNeighbors;
}

/****************************************/
/****************************************/

SInt8 ReferenceModel2Dot3::GetDiffMessagingNeighbors(UInt8 un_message, UInt8 un_message2) {
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
