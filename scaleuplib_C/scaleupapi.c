/*	dReDBox Scale-Up API
	Copyright (C) 2017, IBM Corporation
   	Kostas Katrinis, katrinisk@ie.ibm.com
*/

#include <zmq.h>
#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scaleupapi.h"
#include "jsmn.h"

int init(){
	config_t cfg, *cf;	
	
	cf = &cfg;
	config_init(cf);

    seqnr=1;

	if (!config_read_file(cf, "scaleupapi.ini")) {
		fprintf(stderr, "%s:%d - %s\n",
			config_error_file(cf),
			config_error_line(cf),
			config_error_text(cf));
		config_destroy(cf);
		return(EXIT_FAILURE);
	}

	if (config_lookup_string(cf, "uuid", &uuid)){
		;//printf("uuid: %s\n", uuid);
    }
    else
        return(EXIT_FAILURE); 

	if (config_lookup_string(cf, "scaleupclientport", &scaleupclientport))
		;//printf("scaleupclientport: %s\n", scaleupclientport);
    else
		return(EXIT_FAILURE);
        
	if (config_lookup_string(cf, "scaleupclienthost", &scaleupclienthost))
		;//printf("scaleupclienthost: %s\n", scaleupclienthost);			
    else
		return(EXIT_FAILURE);
}

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) 
		return 0;
}

void parseResponse(char* jsonresponse,scaleupresponse* response){
	int i;
	int r;
	jsmn_parser p;
	jsmntok_t t[512]; /* We expect no more than 128 tokens */

	jsmn_init(&p);
	r = jsmn_parse(&p, jsonresponse, strlen(jsonresponse), t, sizeof(t)/sizeof(t[0]));
	if (r < 0) {
		printf("Error: Failed to parse JSON: %d\n", r);
	}

	/* Assume the top-level element is an object */
	if (r < 1 || t[0].type != JSMN_OBJECT) {
		printf("Error: JSON Object expected\n");
	}

	/* Loop over all keys of the root object */
	for (i = 1; i < r; i++) {
		if (jsoneq(jsonresponse, &t[i], "memory") == 0) {
			sprintf(response->memory,"%.*s\n", t[i+1].end-t[i+1].start,
					jsonresponse + t[i+1].start);
	        //printf("Memory=%s",response->memory);
			i++;
		} else if (jsoneq(jsonresponse, &t[i], "vcpus") == 0) {
			sprintf(response->vcpus,"%.*s\n", t[i+1].end-t[i+1].start,
					jsonresponse + t[i+1].start);
			//printf("Vcpus=%s",response->vcpus);
			i++;
		} else if (jsoneq(jsonresponse, &t[i], "reason-mem") == 0) {
			sprintf(response->reason_mem,"%.*s\n", t[i+1].end-t[i+1].start,
					jsonresponse + t[i+1].start);
			//printf("Reason-Mem=%s",response->reason_mem);
			i++;
		} else if (jsoneq(jsonresponse, &t[i], "reason-vcpu") == 0) {
			sprintf(response->reason_vcpu,"%.*s\n", t[i+1].end-t[i+1].start,
					jsonresponse + t[i+1].start);
			//printf("reason-vcpu=%s",response->reason_vcpu);
			i++;
		} else if (jsoneq(jsonresponse, &t[i], "vm-uuid") == 0) {
			sprintf(response->vm_uuid,"%.*s\n", t[i+1].end-t[i+1].start,
					jsonresponse + t[i+1].start);
			//printf("vm-uuid=%s",response->vm_uuid);
			i++;									
		} else if (jsoneq(jsonresponse, &t[i], "fulfilled-mem") == 0) {
			sprintf(response->fulfilled_mem,"%.*s\n", t[i+1].end-t[i+1].start,
					jsonresponse + t[i+1].start);
			//printf("fulfilled-mem=%s",response->fulfilled_mem);
			i++;
		} else if (jsoneq(jsonresponse, &t[i], "fulfilled-vcpu") == 0) {
			sprintf(response->fulfilled_vcpu,"%.*s\n", t[i+1].end-t[i+1].start,
					jsonresponse + t[i+1].start);
			//printf("fulfilled-vcpu=%s",response->fulfilled_vcpu);
			i++;		
		} else if (jsoneq(jsonresponse, &t[i], "req-seqnr") == 0) {
			sprintf(response->req_seqnr,"%.*s\n", t[i+1].end-t[i+1].start,
					jsonresponse + t[i+1].start);
			//printf("req-seqnr=%s",response->req_seqnr);
			i++;	
		} else {
			printf("Unexpected key: %.*s\n", t[i].end-t[i].start,
					jsonresponse + t[i].start);
		}
    }

}

void printResponseStruct(scaleupresponse* response){
    printf ("MEMORY=%s\n",response->memory);
    printf ("VCPUS=%s\n",response->vcpus);
    printf ("Fulfilled-Mem=%s\n",response->fulfilled_mem);
    printf ("Fulfilled-VCPU=%s\n",response->fulfilled_vcpu);
    printf ("Reason-Mem=%s\n",response->reason_mem);
    printf ("Reason-VCPU=%s\n",response->reason_vcpu);
    printf ("SeqNr=%s\n",response->req_seqnr);
    printf ("VM-UUID=%s\n",response->vm_uuid);
}

int rescale(int vcpus, const char* memory,scaleupresponse* response){
  char jsonrequest[2048];
  char jsonresponse[2048];
  int zerr = 0 ;
  int rc = 0 ;
  int rRes;
  char zeromquri[256];
  char seqnrstr[512];
  
  void *ctx = zmq_ctx_new ();
  void *socket = zmq_socket (ctx, ZMQ_DEALER);
  zmq_setsockopt (socket, ZMQ_IDENTITY, uuid, 255);
  sprintf(zeromquri,"tcp://%s:%s",scaleupclienthost,scaleupclientport);
  //printf("zeromq URI=%s\n",zeromquri);
  zmq_connect (socket, zeromquri);

  sprintf(seqnrstr,"%s-%d",uuid,seqnr);
  sprintf(jsonrequest,"{\"vcpus\": %d,\"memory\": \"%s\",\"req-seqnr\": \"%s\",\"vm-uuid\": \"%s\",\"fulfilled-vcpu\": \"0\",\"reason-vcpu\":\"null\",\"fulfilled-mem\": \"0\", \"reason-mem\": \"null\"}", vcpus, memory,seqnrstr,uuid);
  //printf("Request:%s\n",jsonrequest);
  seqnr++;
  
  zmq_send (socket, jsonrequest, (int)strlen(jsonrequest), 0);
  zmq_recv (socket, jsonresponse, 2048, 0);
  //printf("Response:%s\nLength=%d\n",jsonresponse,strlen(jsonresponse));
  parseResponse(jsonresponse,response);

  zmq_close (socket);
  zmq_ctx_destroy (ctx);
  
}

int main(int argc , char *argv [])
{
	

    if (argc==2){
    init();
    scaleupresponse* supresponse =(scaleupresponse*)malloc(sizeof(scaleupresponse));
    supresponse->memory = malloc(64*sizeof(char));
    supresponse->vcpus = malloc(8*sizeof(char));
    supresponse->reason_vcpu = malloc(256*sizeof(char));
    supresponse->reason_mem = malloc(256*sizeof(char));
    supresponse->fulfilled_vcpu = malloc(8*sizeof(char));
    supresponse->fulfilled_mem = malloc(8*sizeof(char));
    supresponse->req_seqnr = malloc(256*sizeof(char));
    supresponse->vm_uuid = malloc(256*sizeof(char));
    rescale(2,"3G",supresponse);
    printResponseStruct(supresponse);
   
    }else{	
	
    init();
    scaleupresponse* supresponse =(scaleupresponse*)malloc(sizeof(scaleupresponse));
    supresponse->memory = malloc(64*sizeof(char));    
    supresponse->vcpus = malloc(8*sizeof(char));
    supresponse->reason_vcpu = malloc(256*sizeof(char));    
    supresponse->reason_mem = malloc(256*sizeof(char));      
    supresponse->fulfilled_vcpu = malloc(8*sizeof(char));    
    supresponse->fulfilled_mem = malloc(8*sizeof(char));
    supresponse->req_seqnr = malloc(256*sizeof(char));    
    supresponse->vm_uuid = malloc(256*sizeof(char));     
    rescale(2,"1G",supresponse);
    printResponseStruct(supresponse);
    }
}

