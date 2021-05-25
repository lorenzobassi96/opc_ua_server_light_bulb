#include "open62541.h"
//#include <open62541/plugin/log_stdout.h>
//#include <open62541/server.h>
//#include <open62541/server_config_default.h>

#include <signal.h>
#include <stdlib.h>

#include <time.h>

static volatile UA_Boolean running = true;
static void stopHandler(int sig) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
    running = false;
}

int main(int argc, char * argv[]) {
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    UA_Server *server = UA_Server_new();
   

    //Check for arguments
    if(argc > 2)       //hostname or ip address and a port number are avaiable
    {
	UA_Int16 port_number = atoi(argv[2]);
	UA_ServerConfig_setMinimal(UA_Server_getConfig(server), port_number, 0);
    }
    else
	 UA_ServerConfig_setDefault(UA_Server_getConfig(server));
	
    if(argc > 1)
    {                  //hostname ot ip address avaiable
	//copy the hostname from char * to an open62541 variable
	UA_String hostname;
	UA_String_init(&hostname);
	hostname.length = strlen(argv[1]);
	hostname.data= (UA_Byte *) argv[1];

	//Change the configuration
	UA_ServerConfig_setCustomHostname(UA_Server_getConfig(server), hostname);

     }

    //add a variable
    UA_VariableAttributes varAttr = UA_VariableAttributes_default;
    UA_Int32 random_var = 0;
    UA_Variant_setScalar(&varAttr.value, &random_var, &UA_TYPES[UA_TYPES_INT32]);
    UA_Server_addVariableNode(server, UA_NODEID_STRING(1, "Random_Number"), UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
		UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
		UA_QUALIFIEDNAME(1, "Random_Number"),
		UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), varAttr, NULL, NULL);

    UA_StatusCode retval = UA_Server_run_startup(server);

     if(retval != UA_STATUSCODE_GOOD){
		UA_Server_delete(server);
		return retval;
     	}


      int timestamp = time(0) + 1;

      while(running == true){
		// Handle Server
		UA_Server_run_iterate(server, true);

		//Update the variable Node
		if(time(0) > timestamp){
			timestamp = time(0) + 1;
			UA_Variant value;
			random_var = rand();
			UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "New random number: %d", random_var);
			UA_Variant_setScalar(&value, &random_var, &UA_TYPES[UA_TYPES_INT32]);
			UA_Server_writeValue(server, UA_NODEID_STRING(1, "Random_Number"), value);
		}

       }


       retval = UA_Server_run_shutdown(server);


    UA_Server_delete(server);
    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}
