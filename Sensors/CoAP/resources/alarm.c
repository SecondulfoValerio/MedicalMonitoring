#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coap-engine.h"
#include "os/dev/leds.h"
#include "contiki.h"
#include <cJSON.h>



#define BUFFER_SIZE 512
#define MAX_PAYLOAD_SIZE 1000

//static char client_id[BUFFER_SIZE];
// Dichiarazioni delle funzioni handler

static void res_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);


// Definizione della risorsa
   
RESOURCE(
    res_alarm,
    "title=\"MedicalMonitoring: ?alarm=0\";rt=\"alarm\"", 
    NULL,
    NULL,
    res_put_handler,
    NULL
); 



// Funzione per interpretare il payload JSON
// Dichiarazione della struttura status come variabile globale
typedef struct {
    char fall[10]; // fall detection 0 No 1 Yes
    char heart[10]; // is used to switch the heart warning status to good
} Status;

Status status; // Variabile globale

// set alarm off
void set_alarm_off() {
    strcpy(status.fall, "0");
    strcpy(status.heart, "0");
}

// Funzione per interpretare il payload JSON
static int parse_json_payload(const char *payload, int payload_len) {
    int ret = 0;
    cJSON *json = cJSON_ParseWithLength(payload, payload_len);
    if (json == NULL) {
        printf("Empty message\n");
        ret= -1; // Errore di parsing
    }

    char temp_fall[10]; // fall detection 0 No 1 Yes
    char temp_heart[10];
    strcpy(temp_fall, status.fall);
    strcpy(temp_heart, status.heart);

    cJSON *fall_item = cJSON_GetObjectItemCaseSensitive(json, "command");
    if (cJSON_IsString(fall_item) && (fall_item->valuestring != NULL)) {
        strncpy(status.fall, fall_item->valuestring, sizeof(status.fall) - 1);
        status.fall[sizeof(status.fall) - 1] = '\0'; // Ensure null termination
        printf("RECEIVED FALL %s\n", status.fall);
        ret = 0;
    }
    if (strcmp(status.fall, "3") == 0) {
    	printf("ALLARM OFF\n");
        strcpy(status.fall, temp_fall);
        ret = 0;
    }

    cJSON *heart_item = cJSON_GetObjectItemCaseSensitive(json, "alarm");
    if (cJSON_IsString(heart_item) && (heart_item->valuestring != NULL)) {
        strncpy(status.heart, heart_item->valuestring, sizeof(status.heart) - 1);
        status.heart[sizeof(status.heart) - 1] = '\0'; // Ensure null termination
        printf("RECEIVED HEART %s\n", status.heart);
        ret = 0;
    }
    if (strcmp(status.heart, "3") == 0) {
        strcpy(status.heart, temp_heart);
        ret = 0;
    }

    if (strcmp(status.heart, "4") == 0) {
        set_alarm_off();
        ret = 1;
    }

    cJSON_Delete(json);
    printf("\nReturning: %d\n",ret);
    return ret;
}

// Funzione per classificare lo stato del paziente


static void response_status(char msg[]){


    printf("comand received: %s\ncomand alarm received: %s\n",status.fall,status.heart);

	if(strcmp(status.fall,"0")==0 && strcmp(status.heart,"0")==0){
			leds_off(14);
			leds_on((LEDS_GREEN));
	 	sprintf(msg,"Patient stable");
	
	}
	if(strcmp(status.fall,"0")==0 && strcmp(status.heart,"1")==0){
				leds_off(14);
				leds_on((LEDS_BLUE));
		sprintf(msg,"Patient is having heart problems, sending help request");

	
	}
	if(strcmp(status.fall,"1")==0 && strcmp(status.heart,"0")==0){
					leds_off(14);
					leds_on((LEDS_BLUE));
		sprintf(msg,"Patient has fallen, sending help request");
	
	}
	if(strcmp(status.fall,"1")==0 && strcmp(status.heart,"1")==0){
					leds_off(14);
					leds_on((LEDS_RED));
		sprintf(msg,"Patient is in danger, request is needed immediatly");
	
	}

}


// Funzione PUT handler
static void res_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  unsigned int content_format = -1;  // Cambiato a unsigned int
      char reply[200];
  printf("PUT request receiver!\n");
  // Ottiene l'header del formato del contenuto dal messaggio CoAP
  coap_get_header_content_format(request, &content_format);

  // Verifica che il formato del contenuto sia JSON
  if (content_format == APPLICATION_JSON) {
    const char *payload = (const char *)request->payload;
    int payload_len = request->payload_len;
    int exit= parse_json_payload(payload, payload_len);
    printf("response: %d\n",exit);
    // Parse JSON payload
    if (exit == 0) { 	//0 when only fall value is sent - 1 if alarm set off request is sent
      // Invia risposta e Imposta il codice di stato della risposta per indicare il successo
    	response_status(reply);
	coap_set_header_content_format(response, TEXT_PLAIN);
	coap_set_payload(response, buffer, snprintf((char *)buffer, preferred_size, "%s", reply));
	coap_set_status_code(response, CHANGED_2_04);
    }
    if (exit == 1) { //0 when only fall value is sent - 1 if alarm set off request is sent

      // Invia risposta e Imposta il codice di stato della risposta per indicare il successo
    sprintf(reply,"Alarm off");
    response_status(reply);
    coap_set_header_content_format(response, TEXT_PLAIN);
    coap_set_payload(response, buffer, snprintf((char *)buffer, preferred_size, "%s", reply));
    coap_set_status_code(response, CHANGED_2_04);
    } 
    if(exit== -1) {
      printf("Error in the payload received");
      // Imposta il codice di stato della risposta per indicare una richiesta errata
      coap_set_status_code(response, BAD_REQUEST_4_00);
    }
  } else {
    // Imposta il codice di stato della risposta per indicare un formato non supportato
    coap_set_status_code(response, UNSUPPORTED_MEDIA_TYPE_4_15);
  }
} 

