package it.unipi.iot;

import org.eclipse.californium.core.CoapServer;

public class MedicalMonitoring {

        public static void main( String[] args ){

            // Setting MySQL Connection
            Database databaseHandler = new Database("db_config.properties");
            // Setting Actuators Handlers
            Actuator   actuator= new Actuator(1,databaseHandler);
            // COAP Server for actuators information stored in a MySQL database
            CoapServer server = new CoapServer();
            Coap_Handler handler=new Coap_Handler("registration", databaseHandler);
            server.add(handler);
            server.start();
            //Launch MQTT subscriber
            MqttSubscriber mqttSubscriber = new MqttSubscriber(actuator);
            //Launch user interface
            UserControl user = new UserControl(databaseHandler,handler);
            user.run();


        }
    }



