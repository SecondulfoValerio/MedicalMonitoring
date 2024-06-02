#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coap-engine.h"
#include "os/dev/leds.h"
#include "contiki.h"

static void res_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

char fall_status[2][10] = {"normal", "fall"};

RESOURCE(res_fall,
         "title=\"MedicalMonitoring: ?fall=0\";rt=\"Control\"",
         res_get_handler,                                 // GET HANDLER
         NULL,                                            // POST HANDLER
         res_put_handler,                                 // PUT HANDLER
         NULL);                                           // DELETE HANDLER

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  const char *status = NULL;
  int index;
  int length;

  if(coap_get_query_variable(request, "fall", &status)) {
    index = atoi(status);
    if(index != 0 && index != 1) {
      return;
    }

    length = strlen(fall_status[index]);
    if(length > REST_MAX_CHUNK_SIZE) {
      length = REST_MAX_CHUNK_SIZE;
    }
    memcpy(buffer, fall_status[index], length);
  } else {
  //if no query status returns both
    char message[20]; 
    strcpy(message, fall_status[0]); 
    strcat(message, " "); 
    strcat(message, fall_status[1]); 
     
    length = strlen(message); 

    memcpy(buffer, message, length);
  }
  coap_set_header_content_format(response, TEXT_PLAIN);
  coap_set_header_etag(response, (uint8_t *)&length, 1);
  coap_set_payload(response, buffer, length);
}

static void res_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  const char *status;
  if(coap_get_query_variable(request, "fall", &status) && request->payload_len > 0) {
    int index = atoi(status);

    if(index != 0 && index != 1) {
      return;
    }

    /* Get the payload data */
    const char *payload = (const char *)request->payload;
    printf("Received: %s\n", payload);

    /* Parse the payload to determine the fall status */
    if(strcmp(payload, "fallen") == 0) {
      /* Patient has fallen, turn on the RED LED */
      strcpy(fall_status[index], "fall");
      leds_off(LEDS_GREEN);
      leds_on(LEDS_RED);

    } else if(strcmp(payload, "normal") == 0) {
      /* Patient is normal, turn on the GREEN LED */
      strcpy(fall_status[index], "normal");
      leds_off(LEDS_RED);
      leds_on(LEDS_GREEN);
    }

    /* Set the response code to indicate success */
    coap_set_status_code(response, CHANGED_2_04);
  } else {
    /* Set the response code to indicate bad request */
    coap_set_status_code(response, BAD_REQUEST_4_00);
  }
}
