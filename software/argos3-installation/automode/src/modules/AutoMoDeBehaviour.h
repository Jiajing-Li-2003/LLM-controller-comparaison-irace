/*
 * @file <src/modules/AutoMoDeBehaviour.h>
 *
 * @author Antoine Ligot - <aligot@ulb.ac.be>
 *
 * @package ARGoS3-AutoMoDe
 *
 * @license MIT License
 */

#ifndef AUTOMODEBEHAVIOUR_H
#define AUTOMODEBEHAVIOUR_H

#include <argos3/core/utility/math/vector2.h>
#include <argos3/core/utility/logging/argos_log.h>
#include <argos3/core/utility/configuration/argos_exception.h>

#include <argos3/plugins/robots/e-puck/control_interface/ci_epuck_proximity_sensor.h>
#include <argos3/plugins/robots/e-puck/control_interface/ci_epuck_range_and_bearing_sensor.h>

#include <argos3/demiurge/epuck-dao/EpuckDAO.h>

#include <map>

namespace argos {
	class AutoMoDeBehaviour {
		protected:
			/*
			 * Tells whether or not the behaviour is locked for a given
			 * number of steps. No conditions should be tested if it is locked.
			 */
			bool m_bLocked;

			/*
			 * Tells whether or not the behaviour is ready to be executed.
			 * This could happen if a previous reseting step takes more than
			 * one time step.
			 */
			bool m_bOperational;

			/*
			 * Contains the parameters of the behaviours.
			 */
			std::map<std::string, Real> m_mapParameters;

			/*
			 * The name of the behaviour.
			 */
			std::string m_strLabel;

			/*
			 * The index of the behaviour in the FSM behaviour list.
			 */
			UInt32 m_unIndex;

			/*
			 * The identifier of the behaviour.
			 */
			UInt32 m_unIdentifier;

			/*
			 * Pointer to the state of the robot. Shared with the controller AutoMoDeController
			 * and the finite state machine AutoMoDeFiniteStateMachine.
			 */
      EpuckDAO* m_pcRobotDAO;

		public:

		 virtual ~AutoMoDeBehaviour();
			/*
			 * Execution of the behaviour. Based on the state of the robot,
			 * the behaviour should update the output variables.
			 */
			virtual void ControlStep() = 0;

			/*
			 * Method used to reset the status (variables) of the behaviour.
			 */
			virtual void Reset() = 0;

			/*
			 * Method called at the reseting of the behaviour when more than one
			 * time step are needed to prepare before the execution of the behaviour
			 * (that is the call of ControlStep).
			 */
			virtual void ResumeStep() = 0;

			/*
			 * Initialize the behaviour.
			 */
			virtual void Init() = 0;

			/*
			 * Cloning function.
			 */
			virtual AutoMoDeBehaviour* Clone() = 0;

			/*
			 * Returns a string containing the DOT description of the behaviour.
			 */
			const std::string GetDOTDescription();

			/*
			 * Instert a pair <parameter, value> to the parameters map.
			 */
			void AddParameter(const std::string& str_identifier, const Real& f_value);

			/*
			 * Returns the value of a given parameter from the parameters map.
			 */
			const Real& GetParameter(const std::string& str_identifier);

			/*
			 * Returns the whole parameters map.
			 */
			const std::map<std::string, Real> GetParameters();

			/*
			 * Setter for the index of the behaviour.
			 */
			void SetIndex(const UInt32& un_index);

			/*
			 * Getter for the index of the behaviour.
			 */
			const UInt32& GetIndex() const;

			/*
			 * Setter for the identifier of the behaviour.
			 */
			void SetIdentifier(const UInt32& un_id);

			/*
			 * Getter for the identifier of the behaviour.
			 */
			const UInt32& GetIdentifier() const;

			/*
			 * Getter for the label (name) of the behaviour.
			 */
			const std::string& GetLabel() const;


			/*
			 * Returns the value of the locked flag.
			 * @see m_bLocked.
			 */
			const bool IsLocked() const;

			/*
			 * Returns the value of the operational flag.
			 * @see m_bOperational;
			 */
			const bool IsOperational() const;

			/*
			 * Utility function. Returns a vector containing the wheels velocity
			 * needed for the robot to follow the vector passed as parameter of the method.
			 */
			CVector2 ComputeWheelsVelocityFromVector(CVector2 c_vector_to_follow);

			/*
			 * Utility function. Returns a vector containing the sum of the
			 * proximity readings passed as parameter of the method.
			 */
			CVector2 SumProximityReadings(CCI_EPuckProximitySensor::TReadings s_prox);

			/*
			 * Setter for the shared pointer to the representation of the robot state.
			 */
			void SetRobotDAO(EpuckDAO* pc_robot_dao);

            /*
             * Data transform for color of the omnidirectional camera and LEDs.
             */
            CColor GetColorParameter(const UInt32& un_value, const bool& b_emiter);
	};
}

#endif
