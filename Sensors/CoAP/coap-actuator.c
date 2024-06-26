#include "contiki.h"
#include "contiki-net.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"
#include <stdio.h>
#include "net/routing/routing.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-debug.h"
#include "net/ipv6/uiplib.h"
#include "sys/etimer.h"
#include "os/dev/leds.h"
#include "dev/button-hal.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

// Server IP and resource path
#define SERVER_EP "coap://[fd00::1]:5683"
#define TOGGLE_INTERVAL 5
char *service_url = "/registration"; //registration of the resource

// flag to exit the while cycle and start the server tasks
static bool registered = false;
static struct etimer e_timer;

// Define the resource
extern coap_resource_t res_body ,res_alarm;
//extern coap_resource_t res_body;
//extern coap_resource_t res_fall;
static struct etimer et;
static coap_endpoint_t server_ep;
static coap_message_t request[1]; /* This way the packet can be treated as pointer as usual. */
static char json_message[] = "{\"app\":\"MedicalMonitoring\",\n\"Patient_ID\":1}"; //Patient ID set to 1 



// Define a handler to handle the response from the server
void client_chunk_handler(coap_message_t *response){
    if(response == NULL) {
        puts("Request timed out");
        return;
    }
    registered = true;
}



/* Declare and auto-start this file's process */
PROCESS(coap_actuator, "coap-actuator");
AUTOSTART_PROCESSES(&coap_actuator);

PROCESS_THREAD(coap_actuator, ev, data){
    PROCESS_BEGIN();

    ///////////////// COAP CLIENT //////////////

    // Populate the coap_endpoint_t data structure
    coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP),&server_ep);
    
    etimer_set(&et, TOGGLE_INTERVAL * CLOCK_SECOND);
    leds_on(14);
    while(registered == false){
        PROCESS_YIELD();
        if(etimer_expired(&et)){
            printf("-- timer expired -- \n");
            // Prepare the message
                        printf("--             // Prepare the message -- \n");
            coap_init_message(request, COAP_TYPE_CON,COAP_POST, 0);
            coap_set_header_uri_path(request, service_url);

            // Set JSON as content format
                        printf("--             // Set JSON as content format-- \n");
            coap_set_header_content_format(request, APPLICATION_JSON);

            // Set the payload (if needed)
                        printf("--             // Set the payload (if needed) -- \n");
            coap_set_payload(request, (uint8_t *)json_message,strlen(json_message));

            // Issue the request in a blocking manner
            // The client will wait for the server to reply(or the transmission to timeout)
                        printf("--             // Issue the request in a blocking manner-- \n");
            COAP_BLOCKING_REQUEST(&server_ep, request,client_chunk_handler);
            printf("-- request sent --\n");
            etimer_set(&et, TOGGLE_INTERVAL * CLOCK_SECOND);
        }
    }
    leds_off(15);

    //////////////////// COAP SERVER //////////////////
    
    // Activation of a resource
    coap_activate_resource(&res_alarm, "alarm");
    coap_activate_resource(&res_body, "body");

    
    etimer_set(&e_timer, CLOCK_SECOND * 5);

    printf("Loop\n");
    //int modality=0; //0 good heart 1 bad heart
  while(1) {
    PROCESS_WAIT_EVENT();

	if(ev == PROCESS_EVENT_TIMER && data == &e_timer){
		    printf("Event triggered\n");
		  
			res_body.trigger();
			
			etimer_set(&e_timer, CLOCK_SECOND * 4);
	}

   }



    
    PROCESS_END();
}
