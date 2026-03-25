/*
 * @file <src/AutoMoDeMain.cpp>
 *
 * @author Antoine Ligot - <aligot@ulb.ac.be>
 *
 * @package ARGoS3-AutoMoDe
 *
 * @license MIT License
 */

#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include <argos3/core/simulator/simulator.h>
#include <argos3/core/simulator/space/space.h>
#include <argos3/core/simulator/entity/entity.h>
#include <argos3/core/simulator/entity/controllable_entity.h>
#include <argos3/core/utility/plugins/dynamic_loading.h>
#include <argos3/core/simulator/argos_command_line_arg_parser.h>

#include <argos3/demiurge/loop-functions/CoreLoopFunctions.h>

#include "./core/AutoMoDeFiniteStateMachine.h"
#include "./core/AutoMoDeFsmBuilder.h"
#include "./core/AutoMoDeController.h"

using namespace argos;

const std::string ExplainParameters() {
	std::string strExplanation = "The possible parameters are: \n\n"
		" --fsm-config CONF \t A description of a finite state machine [OPTIONAL]\n"
		" --fsm-file FILE \t A file containing descriptions of finite state machines [OPTIONAL]\n";
	return strExplanation;
}

// Function Transforms character '"' into '%22'
std::string EncodeURL(const std::string &value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        std::string::value_type c = (*i);

				if (c == '"') {
            escaped << '%' << std::setw(2) << int((unsigned char) c);
            continue;
        }
				escaped << c;
    }

    return escaped.str();
}

/**
 * @brief
 *
 */
int main(int n_argc, char** ppch_argv) {


	std::vector<std::string> vecConfigFsm;
	bool bFsmControllerFound = false;
	bool bFsmFileFound = false;
	std::string strFsmFile;

	std::vector<AutoMoDeFiniteStateMachine*> vecFsm;

	try {
		// Cutting off the FSM configuration from the command line

		int nCurrentArgument = 1;
		while(!bFsmControllerFound && nCurrentArgument < n_argc) {
			if(strcmp(ppch_argv[nCurrentArgument], "--help") == 0) {
				std::cout << ExplainParameters() << std::endl;
				return 0;
			}
			nCurrentArgument++;
		}

		nCurrentArgument = 1;
		while(!bFsmControllerFound && nCurrentArgument < n_argc) {
			if(strcmp(ppch_argv[nCurrentArgument], "--fsm-config") == 0) {
				bFsmControllerFound = true;
				nCurrentArgument++;
				while (nCurrentArgument < n_argc) {
					vecConfigFsm.push_back(std::string(ppch_argv[nCurrentArgument]));
					nCurrentArgument++;
				}
				// Do not take the FSM configuration into account in the standard command line parsing.
				n_argc = n_argc - vecConfigFsm.size() - 1;
			}
			nCurrentArgument++;
		}

		nCurrentArgument = 1;
		while(!bFsmFileFound && nCurrentArgument < n_argc) {
			if(strcmp(ppch_argv[nCurrentArgument], "--fsm-file") == 0) {
				bFsmFileFound = true;
				nCurrentArgument++;
				strFsmFile = std::string(ppch_argv[nCurrentArgument]);
      }
		}

		AutoMoDeFsmBuilder cBuilder = AutoMoDeFsmBuilder();

		if (bFsmControllerFound) {
			AutoMoDeFiniteStateMachine* pcFiniteStateMachine = cBuilder.BuildFiniteStateMachine(vecConfigFsm);

			std::string strFiniteStateMachineURL = pcFiniteStateMachine->GetReadableFormat();
			//std::cout << strFiniteStateMachineURL << std::endl;

			std::string strBrowser = "firefox \"";
			strBrowser.append(EncodeURL(strFiniteStateMachineURL));
			strBrowser.append("\"");

			system(strBrowser.c_str());
		}

		if (bFsmFileFound) {
			std::ifstream file(strFsmFile.c_str()); // pass file name as argment
		  std::string linebuffer;

			while (file && getline(file, linebuffer)){
				if (linebuffer.length() == 0)continue;
				AutoMoDeFiniteStateMachine* pcFiniteStateMachine = cBuilder.BuildFiniteStateMachine(linebuffer.c_str());

				std::string strFiniteStateMachineURL = pcFiniteStateMachine->GetReadableFormat();
				//std::cout << strFiniteStateMachineURL << std::endl;

				std::string strBrowser = "firefox \"";
				strBrowser.append(EncodeURL(strFiniteStateMachineURL));
				strBrowser.append("\"");

				system(strBrowser.c_str());
			}
		}
	} catch(std::exception& ex) {
    // A fatal error occurred: dispose of data, print error and exit
    LOGERR << ex.what() << std::endl;
    return 1;
  }

	/* Everything's ok, exit */
  return 0;
}
