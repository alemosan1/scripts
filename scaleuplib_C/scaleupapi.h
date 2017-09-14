/* dReDBox Scale-Up API
   Copyright (C) 2017, IBM Corporation
   Kostas Katrinis, katrinisk@ie.ibm.com
*/

#include <zmq.h>
#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char* vcpus;
    char* reason_vcpu;
    char* fulfilled_vcpu;
    char* memory;
    char* fulfilled_mem;
    char* reason_mem;                    
    char* req_seqnr;
    char* vm_uuid;
}scaleupresponse;

int init();

int rescale(int vcpus, const char* memory,scaleupresponse* );

  	const char *uuid;
	const char *scaleupclientport;
	const char *scaleupclienthost;
	unsigned long int seqnr;
