#ifndef FRONTEND_H_
#define FRONTEND_H_

#include "rpc.h"

/**
 *  Connect to a server
 *
 *  @params:
 *    name:     A string that is the ip address of server
 *    port:     An integer in the range [0, 65536) that is the port.
 *  @return:    (Useless but demanded) On success, the file returns the name and port as struct
 */
rpc_t *RPC_Connect(char *name, int port);


#endif //FRONTEND_H_