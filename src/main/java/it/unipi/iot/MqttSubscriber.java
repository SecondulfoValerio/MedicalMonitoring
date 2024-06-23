package it.unipi.iot;

import org.eclipse.paho.client.mqttv3.*;
import org.json.simple.JSONObject;
import org.json.simple.JSONValue;
import org.json.simple.parser.ParseException;



/**
 Implementation of a MQTT subscriber for listening to the topic of the mqtt sensor
 Information taken from the sensor will be catched and stored in a JSON object
 From the sensor will be saved the x y z variables for the linear and angular acceleration of out
 accelerometer and gyroscope and the values of the heart sensor such as max heart beat blood pressure and others.
 */

/**
 * 1.	trestbps: The person’s resting blood pressure (mm Hg on admission to the hospital)
 * 3.	fbs: The person’s fasting blood sugar (> 120 mg/dl, 1 = true; 0 = false)
 * 4.	restecg: Resting electrocardiographic measurement (0 = normal, 1 = having ST-T wave abnormality,
 *       2 = showing probable or definite left ventricular hypertrophy by Estes’ criteria)
 * 5.	thalach: The person’s maximum heart rate achieved
 * */
public class MqttSubscriber implements MqttCallback {

    private final String[] topics = {"patient_movement" , "patient_values"};
    private final String broker = "tcp://127.0.0.1";

    private final String clientId = "MedicalMonitoring";

    private Actuator actuator;



    /**
     * Creates a Mttq Subscriber with the relative Database Handler
     * and uses actuatorHandler to send commands to the specified actuator
     */

    public MqttSubscriber(Actuator actuator){

        this.actuator = actuator;

        try {
            //Defining MqttClient
            //System.out.println("Connecting to topics:"+topics[0]+"  "+topics[1] );

            MqttClient mqttClient = new MqttClient(broker, clientId);
            mqttClient.setCallback(this);
            mqttClient.connect();
            mqttClient.subscribe(topics);

        } catch (MqttException exception){
            exception.printStackTrace();
        }

    }

    @Override
    public void connectionLost(Throwable throwable) {
        System.out.println("Connection with the broker: LOST " + throwable.getMessage());
        throwable.printStackTrace(); //EOFException thrown here within a few seconds

    }


    public void messageArrived(String topic, MqttMessage message) throws Exception {

        try {
            //System.out.println("Topic received: "+  topic);
            JSONObject request = (JSONObject) JSONValue.parseWithException(new String(message.getPayload()));
            String app = request.get("app").toString();

            //System.out.println(String.format("[%s] %s", topic, new String(message.getPayload())));
            if(message==null)
            {
                System.out.println("message NULL : " +message);
                return;
            }

            if(app.equals("MedicalMonitoring")) {
                if (topic.equals("patient_values")) {
                    //System.out.println("Saving for topic: "+  topic);
                    int patientID = Integer.parseInt(request.get("Patient_ID").toString());
                    // X Y Z values for linear acceleration
                    int xla = Integer.parseInt(request.get("xla").toString());
                    int yla = Integer.parseInt(request.get("yla").toString());
                    int zla = Integer.parseInt(request.get("zla").toString());
                    // X Y X values for angular acceleration
                    int xaa = Integer.parseInt(request.get("xaa").toString());
                    int yaa = Integer.parseInt(request.get("yaa").toString());
                    int zaa = Integer.parseInt(request.get("zaa").toString());
                    //MAC sensor address
                    String mac = request.get("MAC").toString();
                    //System.out.println("Actuator mov handling: "+  patientID + xla + yla +zla + xaa + yaa +zaa + mac);

                    // handle movement and body received
                    actuator.handler(patientID,xla, yla, zla, xaa, yaa,zaa, mac, "patient_values");
                    //actuator.valueHandler(patientID,trestbps, fbs, restecg, thalach, mac,"patient_values");
                }

                else{
                    System.out.println("Invalid Topic\n");
                }
            }

        } catch (ParseException error) {

            System.out.println("Error during data transmission: Data not received correctly");
            throw new RuntimeException(error);
        }

    }

    @Override
    public void deliveryComplete(IMqttDeliveryToken iMqttDeliveryToken) {

        //System.out.println("Delivery status: completed!");

    }

}