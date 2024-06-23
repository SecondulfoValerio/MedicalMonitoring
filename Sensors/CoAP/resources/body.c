#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coap-engine.h"
#include "os/dev/leds.h"
#include "contiki.h"
#include <cJSON.h>
#include "cuore.h" // emlearn generated model
#include "eml_net.h"

#define BUFFER_SIZE 512
#define MAX_PAYLOAD_SIZE 1000

//static char client_id[BUFFER_SIZE];
// Dichiarazioni delle funzioni handler

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);

// Variabili per i valori dei parametri JSON

char alarm[10]; // is used to switch the heart warning status to good
int heart_warning; //to start heart desease modality

static int trestbps =0;
static int fbs =0;
static int restecg =0;
static int thalach =0;

int patient_heart = 0; // 0 = normale, 1 = anormale
float features[] = { 120, 1,1, 200 }; //monitoring features trestbps fbs restecg thalach
float outputs[2] = {0, 0}; //output features [0] normal probability [1] illness probability
int pat_status[]={0,0};
int timer_modality_switch=0; //every 3 msg reports (12 seconds) switch from good heart to bad heart values



// Definizione della risorsa
EVENT_RESOURCE(res_body,
	"title=\"MedicalMonitoring: ?body=0\";rt=\"Control\"", 
         res_get_handler,                     // GET handler
         NULL,                                // POST handler
         //res_put_handler,                     // PUT handler
         NULL,
         NULL,				      //DELETE handler
         res_event_handler		      //EVENT handler
);    

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
 // Creazione manuale del payload JSON
    char response_str[MAX_PAYLOAD_SIZE];
    char status_paziente[30];
    if (patient_heart==1)
    	strcpy(status_paziente,"1");
    else
    	strcpy(status_paziente,"0");
    	

    sprintf(response_str, 
             "{\"stat\":\"%s\",\"trestbps\":%d,\"fbs\":%d,\"restecg\":%d,\"thalach\":%d}", 
             status_paziente, trestbps, fbs, restecg, thalach);

    // Impostare il payload della risposta CoAP con il JSON
    coap_set_header_content_format(response, APPLICATION_JSON);
    coap_set_payload(response, buffer, snprintf((char *)buffer, preferred_size, "%s", response_str));
    // Impostare il payload della risposta CoAP con il JSON

    printf("Get received! JSON response: %s\n", response_str);

    // Deallocazione delle risorse
}


//set allarm off

//generate random values for heart desease detection
int generate_random_body(int min, int max) {
    return min + rand() % (max - min + 1);
    }
    //genera valori randomici per i dati presi dal cuore ecc
    void generate_values_body(int *mod, int *fbs, int *restecg, int *trestbps, int *thalach) {
    printf("mod %d",*mod);
    if (*mod == 1) {
        *fbs = 1;
        *restecg = 1;
        *trestbps = generate_random_body(200, 300);
        *thalach = generate_random_body(200, 300);
    } else if (*mod == 0) {
        *fbs = 0;
        *restecg = 0;
        *trestbps = generate_random_body(70, 110);
        *thalach = generate_random_body(70, 110);
    } else {
        printf("Error in heart values generation\n");
        exit(1);
    }
}
void generate_body(){ 
   //1-12 seconds good heart simulation 13-24 seconds bad heart simulation
   if(timer_modality_switch>=6){ //reset counter after 24 seconds to good heart simulation
   	printf("reset timer %d\n",timer_modality_switch);
  	timer_modality_switch=0;
  	}
  if(timer_modality_switch<4){ //first 12 seconds good heart simulation 
  	timer_modality_switch++;
  	printf("timer %d\n",timer_modality_switch);
  	heart_warning=0;
  	}
  if(timer_modality_switch>=4 && timer_modality_switch<6){ //12-24 seconds bad heart simulation
  	timer_modality_switch++;
  	printf("timer %d\n",timer_modality_switch);
  	heart_warning=1;
  	}
generate_values_body(&heart_warning, &fbs, &restecg, &trestbps, &thalach); //based on alarm value generate good heart or desease heart values
}
static void heart_monitor(){

  	
  generate_body();
  printf("%p\n", eml_net_activation_function_strs); // This is needed to avoid compiler error (warnings == errors);
  features[0]= trestbps ;
  features[1]= fbs ;
  features[2]=  restecg;
  features[3]=  thalach;
  eml_net_predict_proba(&cuore, features, 4, outputs, 2);
  printf("CONDIZIONI:\nTrestbps=%d\nFbs=%d\nRestecg=%d\nThalach=%d\nOutput1=%f\nOutput2=%f\n",trestbps,fbs,restecg,thalach,outputs[0],outputs[1]);
  if (outputs[0]<outputs[1]){
      patient_heart = 0; // Normale
      pat_status[1]=0;
      printf("Patient heart: normal\n");
    	//leds_on(2); //turn on green light
    	//leds_off(LEDS_RED);
    	//leds_on(LEDS_GREEN);
    	//leds_on(LEDS_RED);
    	pat_status[1]=0;
    
    	}
  if(outputs[0]>outputs[1]){
    printf("[WARNING] Patient heart: anomalous \n");
    patient_heart = 1; // Malato
    pat_status[1]=1;

  }
  
  
  
}

static void res_event_handler()
{

  heart_monitor();
  coap_notify_observers(&res_body);

    

}


