#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coap-engine.h"
#include "os/dev/leds.h"
#include "contiki.h"
#include <cJSON.h>
#include "cuore.h" // emlearn generated model
#include "eml_net.h"


// Dichiarazioni delle funzioni handler
static void res_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

// Variabili per i valori dei parametri JSON
char command[10];
int trestbps;
int fbs;
int restecg;
int thalach;
int patient_heart = 0; // 0 = normale, 1 = anormale
float features[] = { 120, 1,1, 200 }; //monitoring features trestbps fbs restecg thalach
float outputs[2] = {0, 0}; //output features [0] normal probability [1] illness probability
int pat_status[]={0,0};
// Definizione della risorsa
RESOURCE(res_body,
	"title=\"MedicalMonitoring: ?body=0\";rt=\"Control\"", 
         res_get_handler,                     // GET handler
         NULL,                                // POST handler
         res_put_handler,                     // PUT handler
         NULL);                               // DELETE handler

// Funzione GET handler
static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  // Restituisce il valore di patient_heart
  const char* response_message = patient_heart == 1 ? "Anormale" : "Normale";
  size_t len = strlen(response_message);
  memcpy(buffer, response_message, len);
  coap_set_header_content_format(response, TEXT_PLAIN);
  coap_set_header_etag(response, (uint8_t *)&len, 1);
  coap_set_payload(response, buffer, len);
}

// Funzione per interpretare il payload JSON
static int parse_json_payload(const char *payload, int payload_len) {
  cJSON *json = cJSON_ParseWithLength(payload, payload_len);
  if (json == NULL) {
    return -1; // Errore di parsing
  }

  cJSON *command_item = cJSON_GetObjectItemCaseSensitive(json, "command");
  if (cJSON_IsString(command_item) && (command_item->valuestring != NULL)) {
    strncpy(command, command_item->valuestring, sizeof(command));
  }

  cJSON *trestbps_item = cJSON_GetObjectItemCaseSensitive(json, "trestbps");
  if (cJSON_IsNumber(trestbps_item)) {
    trestbps = trestbps_item->valuedouble;
  }

  cJSON *fbs_item = cJSON_GetObjectItemCaseSensitive(json, "fbs");
  if (cJSON_IsNumber(fbs_item)) {
    fbs = fbs_item->valuedouble;
  }

  cJSON *restecg_item = cJSON_GetObjectItemCaseSensitive(json, "restecg");
  if (cJSON_IsNumber(restecg_item)) {
    restecg = restecg_item->valuedouble;
  }

  cJSON *thalac_item = cJSON_GetObjectItemCaseSensitive(json, "thalach");
  if (cJSON_IsNumber(thalac_item)) {
    thalach = thalac_item->valuedouble;
  }

  cJSON_Delete(json);
  return 0;
}
//     sendBodyMessage("body", String.valueOf(fall),trestbps,fbs,restecg,thalach, databaseHandler.findSensIP(patientID, "body"));

// Funzione per classificare lo stato del paziente

static void response_status(char msg[]){

	if(pat_status[0]==0 && pat_status[1]==0){
			leds_on((LEDS_GREEN));
	 	sprintf(msg,"Patient stable");
	
	}
	if(pat_status[0]==0 && pat_status[1]==1){
				leds_on((LEDS_BLUE));
		sprintf(msg,"Patient is having heart problems, sending help request");

	
	}
	if(pat_status[0]==1 && pat_status[1]==0){
					leds_on((LEDS_BLUE));
		sprintf(msg,"Patient has fallen, sending help request");
	
	}
	if(pat_status[0]==1 && pat_status[1]==1){
					leds_on((LEDS_RED));
		sprintf(msg,"Patient is in danger, request is needed immediatly");
	
	}

}

static void heart_monitor(){
  printf("%p\n", eml_net_activation_function_strs); // This is needed to avoid compiler error (warnings == errors);
  features[0]= trestbps ;
  features[1]= fbs ;
  features[2]=  restecg;
  features[3]=  thalach;
  eml_net_predict_proba(&cuore, features, 4, outputs, 2);
  printf("CONDIZIONI:\nComand=%s\nTrestbps=%d\nFbs=%d\nRestecg=%d\nThalach=%d\nOutput1=%f\nOutput2=%f\n",command,trestbps,fbs,restecg,thalach,outputs[0],outputs[1]);
  if (outputs[0]<outputs[1]){
      patient_heart = 0; // Normale
      pat_status[1]=0;
      printf("Patient heart: normal\n");
    	leds_on(2); //turn on green light
    	leds_off(4);//turn off red light n2
    	//leds_on(LEDS_RED);
    	pat_status[1]=0;
    
    	}
  if(outputs[0]>outputs[1]){
    printf("[WARNING] Patient heart: anomalous \n");
    patient_heart = 1; // Malato
    pat_status[1]=1;
    leds_off(2); //turn off green light
    leds_on(4); //turn on red light n2
    pat_status[1]=1;
  }
  if (strcmp(command,"1")==0){
  	pat_status[0]=1;
  	printf("[WARNING] Patient has fallen\n");
  	    leds_off(2); //turn off green light
   	    leds_on(6); //turn on light n2
   	    pat_status[0]=1;
   	    
  }
  else{
    	pat_status[0]=0;
 	printf("Patient is normal\n");
  	    leds_on(2); //turn off green light
   	    leds_off(6); //turn on light n2}
   	    pat_status[0]=0;
   }
  
  
}


// Funzione PUT handler
static void res_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  unsigned int content_format = -1;  // Cambiato a unsigned int
      char reply[200];

  // Ottiene l'header del formato del contenuto dal messaggio CoAP
  coap_get_header_content_format(request, &content_format);

  // Verifica che il formato del contenuto sia JSON
  if (content_format == APPLICATION_JSON) {
    const char *payload = (const char *)request->payload;
    int payload_len = request->payload_len;

    // Parse JSON payload
    if (parse_json_payload(payload, payload_len) == 0) {
      // Classifica lo stato del paziente
      heart_monitor();

      // Invia risposta e Imposta il codice di stato della risposta per indicare il successo
    response_status(reply);

    coap_set_header_content_format(response, TEXT_PLAIN);
    coap_set_payload(response, buffer, snprintf((char *)buffer, preferred_size, "%s", reply));
    coap_set_status_code(response, CHANGED_2_04);
    } else {
      printf("Error in the payload received");
      // Imposta il codice di stato della risposta per indicare una richiesta errata
      coap_set_status_code(response, BAD_REQUEST_4_00);
    }
  } else {
    // Imposta il codice di stato della risposta per indicare un formato non supportato
    coap_set_status_code(response, UNSUPPORTED_MEDIA_TYPE_4_15);
  }
}

