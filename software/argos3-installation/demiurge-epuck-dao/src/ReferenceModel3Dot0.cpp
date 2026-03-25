#include "ReferenceModel3Dot0.h"

/****************************************/
/****************************************/

ReferenceModel3Dot0::ReferenceModel3Dot0() {
  m_pcRng = CRandom::CreateRNG("argos");
  m_pcRabMessageBuffer = RabMessageBuffer();
  m_pcRabMessageBuffer.SetTimeLife(10);
  m_fMaxVelocity = 12;
  m_fLeftWheelVelocity = 0;
  m_fRightWheelVelocity = 0;
}

/****************************************/
/****************************************/

ReferenceModel3Dot0::~ReferenceModel3Dot0() {}

/****************************************/
/****************************************/

void ReferenceModel3Dot0::Reset() {
  m_fLeftWheelVelocity = 0;
  m_fRightWheelVelocity = 0;
  m_pcRabMessageBuffer.Reset();
}

/****************************************/
/****************************************/

CCI_EPuckProximitySensor::SReading ReferenceModel3Dot0::GetProximityReading() {
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

void ReferenceModel3Dot0::SetProximityInput(CCI_EPuckProximitySensor::TReadings s_prox_input) {
  m_sProximityInput = s_prox_input;
}

/****************************************/
/****************************************/

CCI_EPuckLightSensor::SReading ReferenceModel3Dot0::GetLightReading() {
  CCI_EPuckLightSensor::SReading cOutputReading;
  CVector2 cSumLight(0, CRadians::ZERO);
	for (UInt8 i = 0; i < m_sLightInput.size(); i++) {
    if (m_sLightInput[i].Value > 0.2) {
      cOutputReading.Value = 1;
    }
		cSumLight += CVector2(m_sLightInput[i].Value, m_sLightInput[i].Angle.SignedNormalize());
	}
  if (cOutputReading.Value == 1) {
    cOutputReading.Angle = cSumLight.Angle().SignedNormalize();
  }
  return cOutputReading;
}

/****************************************/
/****************************************/

void ReferenceModel3Dot0::SetLightInput(CCI_EPuckLightSensor::TReadings s_light_input) {
  m_sLightInput = s_light_input;
}

/****************************************/
/****************************************/

Real ReferenceModel3Dot0::GetGroundReading() {
  std::deque<CCI_EPuckGroundSensor::SReadings>::iterator it;
  UInt32 unBlackWhiteCounter[2] = {0,0};  //unBlackWhiteCounter[0] -> Black; unBlackWhiteCounter[1] -> White.
  float fBlackThreshold = 0.02;
  float fWhiteThreshold = 0.97;
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

  if (unBlackWhiteCounter[0] > 8) {
    return 0.0f;
  }
  else if (unBlackWhiteCounter[1] > 8) {
    return 1.0f;
  }
  else {
    return 0.5f;
  }
}

/****************************************/
/****************************************/

void ReferenceModel3Dot0::SetGroundInput(CCI_EPuckGroundSensor::SReadings s_ground_input) {
  m_deqGroundInput.push_back(s_ground_input);
  if (m_deqGroundInput.size() > 10) {
    m_deqGroundInput.pop_front();
  }
}

/****************************************/
/****************************************/

const UInt8 ReferenceModel3Dot0::GetNumberNeighbors() const {
  return m_unNumberNeighbors;
}

/****************************************/
/****************************************/

void ReferenceModel3Dot0::SetNumberNeighbors(const UInt8& un_number_neighbors){
  m_unNumberNeighbors = un_number_neighbors;
}

/****************************************/
/****************************************/

CCI_EPuckRangeAndBearingSensor::SReceivedPacket ReferenceModel3Dot0::GetAttractionVectorToNeighbors(Real f_alpha_parameter) {
  CCI_EPuckRangeAndBearingSensor::TPackets sRabPackets = m_pcRabMessageBuffer.GetMessages();
  CCI_EPuckRangeAndBearingSensor::TPackets::iterator it;
  CVector2 sRabVectorSum(0,CRadians::ZERO);

  for (it = sRabPackets.begin(); it != sRabPackets.end(); it++) {
    if (((*it)->Data[0] != (UInt32) EpuckDAO::GetRobotIdentifier())) {
      sRabVectorSum += CVector2(f_alpha_parameter/(1 + (*it)->Range),(*it)->Bearing.SignedNormalize());
    }
  }

  CCI_EPuckRangeAndBearingSensor::SReceivedPacket cRaBReading;
  cRaBReading.Range = sRabVectorSum.Length();
  cRaBReading.Bearing = sRabVectorSum.Angle().SignedNormalize();

  return cRaBReading;
}


/****************************************/
/****************************************/

std::vector<CCI_EPuckRangeAndBearingSensor::SReceivedPacket*> ReferenceModel3Dot0::GetRangeAndBearingMessages() {
  return m_pcRabMessageBuffer.GetMessages();
}

/****************************************/
/****************************************/

void ReferenceModel3Dot0::SetRangeAndBearingMessages(CCI_EPuckRangeAndBearingSensor::TPackets s_packets) {
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

CCI_EPuckOmnidirectionalCameraSensor::SReadings ReferenceModel3Dot0::GetCameraInput() const {
    return m_sCameraInput;
}

/****************************************/
/****************************************/

void ReferenceModel3Dot0::SetCameraInput(CCI_EPuckOmnidirectionalCameraSensor::SReadings s_cam_input) {
    m_sCameraInput = s_cam_input;
}

/****************************************/
/****************************************/

void ReferenceModel3Dot0::SetLEDsColor(const CColor& c_color) {
    m_cLEDsColor = c_color;
}

/****************************************/
/****************************************/

const CColor& ReferenceModel3Dot0::GetLEDsColor() const {
    return m_cLEDsColor;
}
