
#include "EpuckDAO.h"

namespace argos {

	EpuckDAO::~EpuckDAO() {}

	/****************************************/
	/****************************************/

	void EpuckDAO::SetWheelsVelocity(const Real& un_left_velocity, const Real& un_right_velocity) {
		m_fLeftWheelVelocity = un_left_velocity;
		m_fRightWheelVelocity = un_right_velocity;
	}

	/****************************************/
	/****************************************/

	void EpuckDAO::SetWheelsVelocity(const CVector2& c_velocity_vector) {
		m_fLeftWheelVelocity = c_velocity_vector.GetX();
		m_fRightWheelVelocity = c_velocity_vector.GetY();
	}

	/****************************************/
	/****************************************/

	const Real& EpuckDAO::GetRightWheelVelocity() const {
		return m_fRightWheelVelocity;
	}

	/****************************************/
	/****************************************/

	const Real& EpuckDAO::GetLeftWheelVelocity() const {
		return m_fLeftWheelVelocity;
	}

	/****************************************/
	/****************************************/

	void EpuckDAO::SetRobotIdentifier(const UInt32& un_robot_id) {
		m_unRobotIdentifier = un_robot_id;
	}

	/****************************************/
	/****************************************/

	const UInt32& EpuckDAO::GetRobotIdentifier() const{
		return m_unRobotIdentifier;
	}

	/****************************************/
	/****************************************/

	CRandom::CRNG* EpuckDAO::GetRandomNumberGenerator() const {
		return m_pcRng;
	}

	/****************************************/
	/****************************************/

	const Real& EpuckDAO::GetMaxVelocity() const{
		return m_fMaxVelocity;
	}
}
