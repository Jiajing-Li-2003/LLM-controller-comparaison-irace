/*
 * @brief Container that keep range-and-bearing messages for a fixed
 *        period of time. After the time to live of a message is exceded, the
 *        message is discarded.
 */

#ifndef RAB_MESSAGE_BUFFER_H
#define RAB_MESSAGE_BUFFER_H

#include <vector>
#include <algorithm>
#include <argos3/core/utility/logging/argos_log.h>
#include <argos3/core/utility/datatypes/datatypes.h>
#include <argos3/plugins/robots/e-puck/control_interface/ci_epuck_range_and_bearing_sensor.h>

namespace argos {
  class RabMessageBuffer {
    public:
      /*
			 * Class constructor.
       */
      RabMessageBuffer();

      /*
			 * Class destructor.
       */
      ~RabMessageBuffer();

      /*
			 * Setter for the time to live of the messages contained in the buffer.
       */
      void SetTimeLife(const UInt32& un_max_time_to_live);

      /*
			 * Removes the messages that are too old.
       */
      void Update();

      /*
			 * Add a range-and-bearing message to the buffer.
       */
      void AddMessage(CCI_EPuckRangeAndBearingSensor::SReceivedPacket* c_packet);

      /*
			 * Returns all range-and-bearing messages of the buffer.
       */
      std::vector<CCI_EPuckRangeAndBearingSensor::SReceivedPacket*> GetMessages();

      /*
			 * Clears the content of the buffer.
       */
      void Reset();


    private:
      /*
			 * Current time step.
       */
      UInt32 m_unCurrentTime;

      /*
			 * Time to live of the messages of the buffer.
       */
      UInt32 m_unMaxTimeToLive;

      /*
			 * List of pairs <range-and-bearing message, timestamp>. Represents the buffer.
       */
      std::vector<std::pair<CCI_EPuckRangeAndBearingSensor::SReceivedPacket, UInt32> > m_vecBufferElements;
  };
}

#endif
