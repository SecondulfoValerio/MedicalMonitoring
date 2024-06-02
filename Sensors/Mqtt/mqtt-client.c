
#include "contiki.h"
#include "net/routing/routing.h"
#include "mqtt.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-icmp6.h"
#include "net/ipv6/sicslowpan.h"
#include "sys/etimer.h"
#include "sys/ctimer.h"
#include "lib/sensors.h"
#include "dev/button-hal.h"
#include "dev/leds.h"
#include "os/sys/log.h"
#include "mqtt-client.h"
#include <string.h>
#include <strings.h>

#include "os/dev/leds.h"
#include <time.h>





/*---------------------------------------------------------------------------*/
#define LOG_MODULE "mqtt-client"
#ifdef MQTT_CLIENT_CONF_LOG_LEVEL
#define LOG_LEVEL MQTT_CLIENT_CONF_LOG_LEVEL
#else
#define LOG_LEVEL LOG_LEVEL_DBG
#endif

/*---------------------------------------------------------------------------*/
/* MQTT broker address. */
#define MQTT_CLIENT_BROKER_IP_ADDR "fd00::1"

static const char *broker_ip = MQTT_CLIENT_BROKER_IP_ADDR;

// Defaukt config values
#define DEFAULT_BROKER_PORT         1883
#define DEFAULT_PUBLISH_INTERVAL    (5 * CLOCK_SECOND) //Set to 1 second


// We assume that the broker does not require authentication


/*---------------------------------------------------------------------------*/
/* Various states */
static uint8_t state;

#define STATE_INIT    		  0
#define STATE_NET_OK    	  1
#define STATE_CONNECTING      2
#define STATE_CONNECTED       3
#define STATE_SUBSCRIBED      4
#define STATE_DISCONNECTED    5

/*---------------------------------------------------------------------------*/
PROCESS_NAME(mqtt_client_process);
AUTOSTART_PROCESSES(&mqtt_client_process);

/*---------------------------------------------------------------------------*/
/* Maximum TCP segment size for outgoing segments of our socket */
#define MAX_TCP_SEGMENT_SIZE    32
#define CONFIG_IP_ADDR_STR_LEN   64
/*---------------------------------------------------------------------------*/
/*
 * Buffers for Client ID and Topics.
 * Make sure they are large enough to hold the entire respective string
 */
#define BUFFER_SIZE 64
//DEFINING PROJECT VARIABLES

//global variables to store body values

static int trestbps =0;
static int fbs =0;
static int restecg =0;
static int thalac =0;
// Global variables to store sensor values
static int xla = 0;
static int yla = 0;
static int zla = 0;
static int xaa = 0;
static int yaa = 0;
static int zaa = 0;


//static int movements=0; //Initially set to 0 to generate movements values
//static int heart=0; //Initially set to 0 to generate movements values

static char pub_mov_topic[BUFFER_SIZE]; //topic movement
static char pub_bod_topic[BUFFER_SIZE]; //topic values
static char sub_mov_topic[BUFFER_SIZE];
static char sub_bod_topic[BUFFER_SIZE];
//
static char client_id[BUFFER_SIZE];
static int modality=0; //0= patient standing - heart good 1= patient standing - heart problem - 2 patient fallen - heart good - 3 patient fallen - heart problem

// Periodic timer to check the state of the MQTT client
//#define STATE_MACHINE_PERIODIC     (CLOCK_SECOND >> 1)
static struct etimer periodic_timer;

/*---------------------------------------------------------------------------*/
/*
 * The main MQTT buffers.
 * We will need to increase if we start publishing more data.
 */
#define APP_BUFFER_SIZE 512

static char app_buffer_mov[APP_BUFFER_SIZE];
static char app_buffer_bod[APP_BUFFER_SIZE];

/*---------------------------------------------------------------------------*/


static struct mqtt_connection conn;
static button_hal_button_t *btn;   //Pointer to the button


mqtt_status_t status_mov;
mqtt_status_t status_bod;
char broker_address[CONFIG_IP_ADDR_STR_LEN];


/*---------------------------------------------------------------------------*/
PROCESS(mqtt_client_process, "MQTT Client");

/*---------------------------------------------------------------------------*/
static void mqtt_event(struct mqtt_connection *m, mqtt_event_t event, void *data)
{
  switch(event) {
  case MQTT_EVENT_CONNECTED: {
    printf("Application has a MQTT connection\n");
    leds_on(1);
    state = STATE_CONNECTED;
    break;
  }
  case MQTT_EVENT_DISCONNECTED: {
    printf("MQTT Disconnect. Reason %u\n", *((mqtt_event_t *)data));
    leds_off(15);
    state = STATE_DISCONNECTED;
    process_poll(&mqtt_client_process);
    break;
  }
  case MQTT_EVENT_PUBLISH: {
     printf("Publishing..");
     break;
  }
  case MQTT_EVENT_SUBACK: {
#if MQTT_311
    mqtt_suback_event_t *suback_event = (mqtt_suback_event_t *)data;

    if(suback_event->success) {
      printf("Application is subscribed to topic successfully\n");
    } else {
      printf("Application failed to subscribe to topic (ret code %x)\n", suback_event->return_code);
    }
#else
    printf("Application is subscribed to topic successfully\n");
#endif
    break;
  }
  case MQTT_EVENT_UNSUBACK: {
    printf("Application is unsubscribed to topic successfully\n");
    break;
  }
  case MQTT_EVENT_PUBACK: {
    printf("Publishing complete.\n");
    break;
  }
  default:
    printf("Application got a unhandled MQTT event: %i\n", event);
    break;
  }
}

static bool
have_connectivity(void)
{
  if(uip_ds6_get_global(ADDR_PREFERRED) == NULL ||
     uip_ds6_defrt_choose() == NULL) {
    return false;
  }
  return true;
}
/*--------------------SUPPORT METHODS--------------------------------*/
static void changeModality(){
	modality++;
	if(modality>=4){
		modality=0;}
		printf("modality:%d\n",modality);
		if(modality==0){
    		leds_on((LEDS_GREEN));
		printf("//0= patient standing - heart good\n");
		}
		if(modality==1){
		leds_on((LEDS_BLUE));
		printf("patient standing - heart problem\n");
		}
		if(modality==2){
		leds_on((LEDS_BLUE));
		printf("patient fallen - heart good\n");
		}
		if(modality==3){
		leds_on((LEDS_RED));
		printf("patient fallen - heart problem\n");
		}
		
	
}
/*
static void setXzero(){
	if(movements==1){
		movements=0;
		printf("movements set to: %d\n", movements);
		leds_off(14);
		leds_on(1 << movements);
	}
}
static void setHeart(){
	if(heart==0){
		heart=1;
		printf("heart set to: %d\n", heart);
		leds_off(14);
		leds_on(1 << heart);
		return;
	}
		if(heart==1){
		heart=0;
		printf("heart set to: %d\n", heart);
		leds_off(14);
		leds_on(1 << heart);
		return;
	}
}
*/

double random_double(double min, double max) {
    return min + ((double)rand() / ((double)RAND_MAX + 1)) * (max - min);
}

// Funzione per generare le componenti dell'accelerazione
void generate_acceleration_components(int mod, int *xla, int *yla, int *zla, int *xaa, int *yaa, int *zaa) {
    if (mod == 2 || mod==3) {
        *xla = (int)random_double(1.0, 2);
        *yla = (int)random_double(1.0, 2);
        *zla = (int)random_double(1.0, 2);
        *xaa = (int)random_double(50.0, 100.0);
        *yaa = (int)random_double(50.0, 100.0);
        *zaa = (int)random_double(50.0, 100.0);
    } if(mod == 0 || mod==1) {
        *xla = (int)random_double(6.0, 9.0);
        *yla = (int)random_double(6.0, 9.0);
        *zla = (int)random_double(6.0, 9.0);
        *xaa = (int)random_double(600.0, 900.0);
        *yaa = (int)random_double(600.0, 900.0);
        *zaa = (int)random_double(600.0, 900.0);
    }
}
void generate() {
    srand(time(NULL));  // Inizializzazione del generatore di numeri casuali

    // Genera le componenti dell'accelerazione in base al valore di movements
    generate_acceleration_components(modality, &xla, &yla, &zla, &xaa, &yaa, &zaa);

}

int generate_random_body(int min, int max) {
    return min + rand() % (max - min + 1);
    }
    //genera valori randomici per i dati presi dal cuore ecc
    void generate_values_body(int mod, int *fbs, int *restecg, int *trestbps, int *thalac) {
    if (mod == 1 || mod==3) {
        *fbs = 1;
        *restecg = 1;
        *trestbps = generate_random_body(190, 300);
        *thalac = generate_random_body(190, 300);
    } else if (mod == 0 || mod==2) {
        *fbs = 0;
        *restecg = 0;
        *trestbps = generate_random_body(70, 120);
        *thalac = generate_random_body(70, 120);
    } else {
        printf("X deve essere 0 o 1\n");
        exit(1);
    }
}
void generate_body(){
generate_values_body(modality, &fbs, &restecg, &trestbps, &thalac);
}
/*---------------------------------------------------------------------------*/

//running method

static void sensor_sample(){
			  			  
		  if(state==STATE_INIT){
			 if(have_connectivity()==true)  
				 state = STATE_NET_OK;
		  } 
		  
		  if(state == STATE_NET_OK){
			  // Connect to MQTT server
			  printf("Connecting!\n");
			  
			  memcpy(broker_address, broker_ip, strlen(broker_ip));
			  
			  mqtt_connect(&conn, broker_address, DEFAULT_BROKER_PORT,
						   (DEFAULT_PUBLISH_INTERVAL * 1) / CLOCK_SECOND,
						   MQTT_CLEAN_SESSION_ON);
			  state = STATE_CONNECTING;
		  }
		  
		  if(state==STATE_CONNECTED){
		  
			  // Subscribe to a movement topic
			  strcpy(sub_mov_topic,"patient_movement");
			  // Subscribe to body values topic
			   strcpy(sub_bod_topic,"patient_values");

			  status_mov = mqtt_subscribe(&conn, NULL, sub_mov_topic, MQTT_QOS_LEVEL_0);
			  status_bod = mqtt_subscribe(&conn, NULL, sub_bod_topic, MQTT_QOS_LEVEL_0);
			  printf("Subscribing!\n");
			  if(status_mov == MQTT_STATUS_OUT_QUEUE_FULL || status_bod== MQTT_STATUS_OUT_QUEUE_FULL) {
				LOG_ERR("Tried to subscribe but command queue was full!\n");
				
			  }
			  
			  state = STATE_SUBSCRIBED;
		  }

			  
		if(state == STATE_SUBSCRIBED){
		// Publish Movement
		generate(); //Creation of the movement values
		generate_body();
		sprintf(pub_mov_topic, "%s", "patient_values");
		//Movement app buffer
		sprintf(app_buffer_mov, "{\"app\":\"MedicalMonitoring\"\n\"Patient_ID\":1,\n\"xla\": %d,\n\"yla\": %d,\n\"zla\": %d,\n\"xaa\": %d,\n\"yaa\": %d,\n\"zaa\": %d,\n\"trestbps\": %d,\n\"fbs\": %d,\n\"restecg\": %d,\n\"thalach\": %d,\n\"MAC\": \"%s\"}", (int)xla,(int)yla,(int)zla,(int)xaa,(int)yaa,(int)zaa,trestbps,fbs,restecg,thalac, client_id);
		printf("Publishing: %s\nTopic: %s\n", app_buffer_mov,pub_mov_topic);
		//Publish Body
		//generate_body();
		//sprintf(pub_bod_topic, "%s", "patient_values");
		//Body app buffer
		//sprintf(app_buffer_bod, "{\"app\":\"MedicalMonitoring\"\n\"Patient_ID\":1,\n\"trestbps\": %d,\n\"fbs\": %d,\n\"restecg\": %d,\n\"thalach\": %d,\n\"MAC\": \"%s\"}", trestbps,fbs,restecg,thalac, client_id);
		printf("Publishing: %s\nTopic: %s\n", app_buffer_bod, pub_bod_topic);



		//MQTT publishing	
		mqtt_publish(&conn, NULL, pub_mov_topic, (uint8_t *)app_buffer_mov, strlen(app_buffer_mov), MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);
		//mqtt_publish(&conn, NULL, pub_bod_topic, (uint8_t *)app_buffer_bod, strlen(app_buffer_bod), MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);
		} else if ( state == STATE_DISCONNECTED ){
		   LOG_ERR("Disconnected form MQTT broker\n");	
		   // Recover from error
         		state = STATE_INIT;

		}
		
		//etimer_set(&periodic_timer, STATE_MACHINE_PERIODIC);
      
    

  }
/*--------------------BEGIN--------------------------------------*/

PROCESS_THREAD(mqtt_client_process, ev, data)
{

  PROCESS_BEGIN();
  
  printf("MQTT Client Process\n");

  // Initialize the ClientID as MAC address
  snprintf(client_id, BUFFER_SIZE, "%02x%02x%02x%02x%02x%02x",
                     linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
                     linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
                     linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);

  // Broker registration					 
  mqtt_register(&conn, &mqtt_client_process, client_id, mqtt_event,
                  MAX_TCP_SEGMENT_SIZE);
				  
  state=STATE_INIT;
				    
  // Initialize periodic timer to check the status 
  //etimer_set(&periodic_timer, STATE_MACHINE_PERIODIC);
  etimer_set(&periodic_timer, DEFAULT_PUBLISH_INTERVAL);
  btn = button_hal_get_by_index(0);  //Return the button of index0 since it's the only one button
  /* Main loop */
  while(1) {

    PROCESS_YIELD();

		if(ev == button_hal_press_event) {
			changeModality();
		}

		if(ev == button_hal_periodic_event) {
			btn = (button_hal_button_t *)data;
			if(btn->press_duration_seconds >3) {
				modality=0;
				printf("Reset modality. Modality=%d\n",modality);}

		}
		if((ev == PROCESS_EVENT_TIMER && data == &periodic_timer) || ev == PROCESS_EVENT_POLL){
			sensor_sample();

			etimer_set(&periodic_timer, DEFAULT_PUBLISH_INTERVAL);
		}
	}

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
