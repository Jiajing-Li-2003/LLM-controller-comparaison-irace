/*
 * @file <src/core/EpuckDAO.h>
 *
 * @author Antoine Ligot - <aligot@ulb.ac.be>
 *
 * @package ARGoS3-AutoMoDe
 *
 * @license MIT License
 *
 * @brief This class represents the status of the robot.
 * 				It contains the input variables (the sensor inputs) and
 * 				the output variables (the values for the wheel actuators),
 * 				as well as the setters and getters to access them.
 *
 * 				Only one object of this class should be instanciated, and
 * 				is to be used as a brigde between the AutoMoDeController and
 * 				the AutoMoDeFiniteStateMachine classes. In AutoMoDeController,
 * 				the variables of the object shall be updated at each time step.
 * 				The different modules of the  AutoMoDeFiniteStateMachine will
 * 				then use the input variables and update the output variables
 * 				accordingly.
 */


#ifndef EPUCK_DAO_H
#define EPUCK_DAO_H

#include <vector>
#include <deque>

#include <argos3/core/utility/math/rng.h>
#include <argos3/core/utility/math/vector2.h>
#include <argos3/core/utility/logging/argos_log.h>
#include <argos3/plugins/robots/e-puck/control_interface/ci_epuck_wheels_actuator.h>
#include <argos3/plugins/robots/e-puck/control_interface/ci_epuck_range_and_bearing_sensor.h>
#include <argos3/plugins/robots/e-puck/control_interface/ci_epuck_range_and_bearing_actuator.h>
#include <argos3/plugins/robots/e-puck/control_interface/ci_epuck_rgb_leds_actuator.h>
#include <argos3/plugins/robots/e-puck/control_interface/ci_epuck_proximity_sensor.h>
#include <argos3/plugins/robots/e-puck/control_interface/ci_epuck_light_sensor.h>
#include <argos3/plugins/robots/e-puck/control_interface/ci_epuck_ground_sensor.h>
#include <argos3/plugins/robots/e-puck/control_interface/ci_epuck_omnidirectional_camera_sensor.h>

namespace argos {
	class EpuckDAO {
		public:

			virtual ~EpuckDAO();

			/*
			 * Reset function.
			 */
			virtual void Reset() = 0;

			/*
			 * Setter for the wheels velocity.
			 */
			void SetWheelsVelocity(const Real& un_left_velocity, const Real& un_right_velocity);

			/*
			 * Setter for the wheels velocity.
			 */
			void SetWheelsVelocity(const CVector2& c_velocity_vector);

			/*
			 * Getter for the right wheel velocity.
			 */
			const Real& GetRightWheelVelocity() const;

			/*
			 * Getter for the left wheel velocity.
			 */
			const Real& GetLeftWheelVelocity() const;

			/*
			 * Setter for the robot idientifier.
			 */
			void SetRobotIdentifier(const UInt32& un_robot_id);

			/*
			 * Getter for the robot identifier.
			 */
			const UInt32& GetRobotIdentifier() const;

			/*
			 * Getter for the maximal wheels velocity.
			 */
			const Real& GetMaxVelocity() const;

			/*
			 * Getter for the random number generetor.
			 */
			CRandom::CRNG* GetRandomNumberGenerator() const;


	/*******************/
	/* Virtual classes */
	/*******************/

			/*
			 * Getter for the proximity input.
			 */
			virtual CCI_EPuckProximitySensor::TReadings GetProximityInput() const {			// RM 1.1
				CCI_EPuckProximitySensor::TReadings emptyReadings;
				return emptyReadings;
			};

			virtual CCI_EPuckProximitySensor::SReading GetProximityReading() {					// RM 1.2
				return CCI_EPuckProximitySensor::SReading();
			};

			/*
			 * Setter for the proximity input.
			 */
			virtual void SetProximityInput(CCI_EPuckProximitySensor::TReadings s_prox_input) {};

			/*
			 * Getter for the light input.
			 */
			virtual CCI_EPuckLightSensor::TReadings GetLightInput() const { 		// RM 1.1
				CCI_EPuckLightSensor::TReadings emptyReadings;
				return emptyReadings;
			};

			virtual CCI_EPuckLightSensor::SReading GetLightReading() {					// RM 1.2
				return CCI_EPuckLightSensor::SReading();
			};

			/*
			 * Setter for the light input.
			 */
			virtual void SetLightInput(CCI_EPuckLightSensor::TReadings s_light_input) {};

			/*
			 * Getter for the ground input.
			 */
			virtual CCI_EPuckGroundSensor::SReadings GetGroundInput() {    // RM 1.1
				CCI_EPuckGroundSensor::SReadings emptyReadings;
				return emptyReadings;
			};

			virtual Real GetGroundReading() {																 // RM 1.2
				return 0.0f;
			};

			/*
			 * Setter for the ground input.
			 */
			virtual void SetGroundInput(CCI_EPuckGroundSensor::SReadings s_ground_input) {};

			/*
			 * Getter for the number of surrounding robots.
			 */
			virtual const UInt8 GetNumberNeighbors() const {
				UInt8 unEmptyVariable = 0;
				return unEmptyVariable;
			};

			/*
			 * Getter for the message to send.
			 */
			virtual const UInt8 GetMessageToSend() const{
				UInt8 unEmptyVariable = 0;
				return unEmptyVariable;
			};

			/*
			 * Getter for the range-and-bearing messages.
			 */
			virtual std::vector<CCI_EPuckRangeAndBearingSensor::SReceivedPacket*> GetRangeAndBearingMessages() {
				std::vector<CCI_EPuckRangeAndBearingSensor::SReceivedPacket*> emptyReadings;
				return emptyReadings;
			};

			/*
			 * Getter for the number of messaging neighbors
			 */
			virtual UInt8 GetNumberMessagingNeighbors(UInt8 un_message) {
				UInt8 unEmptyVariable = 0;
				return unEmptyVariable;
			};

			/*
			 * Getter for the difference of number between the messages that the robot got
			 */
			virtual SInt8 GetDiffMessagingNeighbors(UInt8 un_message, UInt8 un_message2) {
				SInt8 unEmptyVariable = 0;
				return unEmptyVariable;
			};

			/*
			 * Getter for attraction force to the neighbors computed with RaB messages
			 */
			virtual CCI_EPuckRangeAndBearingSensor::SReceivedPacket GetAttractionVectorToNeighbors(Real f_alpha_parameter) {			// RM 1.2
				return CCI_EPuckRangeAndBearingSensor::SReceivedPacket();
			};
			                    
				
			/*
			 * Getter for the vector representing the attraction force to the neighbors that are sending a message computed with RaB messages
			 */
			virtual CCI_EPuckRangeAndBearingSensor::SReceivedPacket GetAttractionVectorToMessagingNeighbors(Real f_alpha_parameter, UInt8 un_message) { //RM2.0
				return CCI_EPuckRangeAndBearingSensor::SReceivedPacket();
			};

			/*
			 * Getter for the center of mass of neighbors computed with RaB messages
			 */
			virtual CCI_EPuckRangeAndBearingSensor::SReceivedPacket GetNeighborsCenterOfMass() {
				return CCI_EPuckRangeAndBearingSensor::SReceivedPacket();
			};

			/*
			 * Getter for the center of mass of messaging neighbors computed with RaB messages
			 */
			virtual CCI_EPuckRangeAndBearingSensor::SReceivedPacket GetMessagingNeighborsCenterOfMass(UInt8 un_message) {
				return CCI_EPuckRangeAndBearingSensor::SReceivedPacket();
			};

			/*
			 * Setter for the number of surrounding robots.
			 */
			virtual void SetNumberNeighbors(const UInt8& un_number_neighbors) {};

			/*
			 * Setter for the range-and-bearing input.
			 */
			virtual void SetRangeAndBearingMessages(CCI_EPuckRangeAndBearingSensor::TPackets s_packets) {};

			/*
			 * Setter for the message to send with range and bearing
			 */
			virtual void SetRangeAndBearingMessageToSend(UInt8 un_message) {};

            /*
             * Getter for the camera input.
             */
            virtual CCI_EPuckOmnidirectionalCameraSensor::SReadings GetCameraInput() const {
                CCI_EPuckOmnidirectionalCameraSensor::SReadings emptyReadings;
                return emptyReadings;
            }

            /*
             * Setter for the camera input.
             */
            virtual void SetCameraInput(CCI_EPuckOmnidirectionalCameraSensor::SReadings s_cam_input) {};

            /*
             * Setter for the RGB LEDs color.
             */
            virtual void SetLEDsColor(const CColor& c_color) {};

            /*
             * Getter for the RGB LEDs color.
             */
            virtual const CColor& GetLEDsColor() const {
                CColor& ccEmptyVariable = CColor::BLACK;
                return ccEmptyVariable;

            }

		protected:
			/*
			 * The left wheel velocity (output variable).
			 */
			Real m_fLeftWheelVelocity;

			/*
			 * The right wheel velocity (output variable).
			 */
			Real m_fRightWheelVelocity;

			/*
			 * The maximal wheels velocity.
			 */
			Real m_fMaxVelocity;

			/*
			 * The robot identifier.
			 */
			UInt32 m_unRobotIdentifier;

			/*
			 * Pointer to the random number generator.
			 */
			CRandom::CRNG* m_pcRng;

	};
}


#endif
