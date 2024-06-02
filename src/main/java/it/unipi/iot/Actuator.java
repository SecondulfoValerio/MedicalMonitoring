package it.unipi.iot;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.coap.CoAP;
import org.eclipse.californium.core.coap.MediaTypeRegistry;
import org.eclipse.californium.core.coap.Request;
import org.json.simple.JSONObject;

public class Actuator {

    private final int patientID;

    private Database databaseHandler;
    FallDetection falldetector= new FallDetection();

    public Actuator(int patientID, Database databaseHandler){

        this.databaseHandler = databaseHandler;
        this.patientID = patientID;
    }
/*
    public void handler(int patientID, int xla, int yla, int zla, int xaa, int yaa, String mac, int zaa, String topic){
        System.out.println("Actuator received topic:"+topic);
        if(!topic.equals("patient_movement")){
            System.out.printf("Unknown topic: %s\n", topic);
            return;}
        falldetector.ang_acc(xaa,yaa,zaa);
        //calculates linear and angular acceleration with the new values
        double linear_acc=falldetector.lin_acc(xla,yla,zla);
        falldetector.add_lin_acc(falldetector.lin_acc(xla,yla,zla));
        falldetector.add_ang_acc(falldetector.ang_acc(xaa,yaa,zaa));
        //sets the max and linear and angular accelerations with the new values
        falldetector.set_min_lin_acc();
        falldetector.set_max_lin_acc();
        falldetector.set_min_ang_acc();
        falldetector.set_max_ang_acc();
        //verify if a fall happened
        int fall=falldetector.fallCheck(linear_acc);
        System.out.println("Databaase handler adds values for topic:"+topic);

        databaseHandler.addPatientDataMovement(topic,patientID, falldetector.get_last_linear_value(),falldetector.get_last_angular_value(),mac);

        if(fall==0){
            sendMessage("fall?=0", "normal", databaseHandler.findSensIP(patientID, "fall"));
            System.out.println("The patient seems normal");

        } else if (fall==1){
            sendMessage("fall?=1", "fallen", databaseHandler.findSensIP(patientID, "fall"));
            System.out.println("ATTENTION! The patient has fallen! ");
        }
    }

    public void valueHandler(int patientID,int trestbps,int fbs,int restecg,int thalach,String mac,String topic){
        System.out.println("Actuator received topic:"+topic);

        if(!topic.equals("patient_values")){
            System.out.printf("Unknown topic: %s\n", topic);
            return;}
        System.out.println("Databaase handler adds values for topic:"+topic);

        databaseHandler.addPatientDataBody(topic,patientID,trestbps,fbs,restecg,thalach,mac);

        sendBodyMessage("body", "0",trestbps,fbs,restecg,thalach, databaseHandler.findSensIP(patientID, "body"));
    } */
public void handler(int patientID, int xla, int yla, int zla, int xaa, int yaa,int zaa, int trestbps, int fbs, int restecg, int thalach, String mac, String topic){
    System.out.println("Actuator received topic:"+topic);
    if(!topic.equals("patient_values")){
        System.out.printf("Unknown topic: %s\n", topic);
        return;}
    falldetector.ang_acc(xaa,yaa,zaa);
    //calculates linear and angular acceleration with the new values
    double linear_acc=falldetector.lin_acc(xla,yla,zla);
    falldetector.add_lin_acc(falldetector.lin_acc(xla,yla,zla));
    falldetector.add_ang_acc(falldetector.ang_acc(xaa,yaa,zaa));
    //sets the max and linear and angular accelerations with the new values
    falldetector.set_min_lin_acc();
    falldetector.set_max_lin_acc();
    falldetector.set_min_ang_acc();
    falldetector.set_max_ang_acc();

    System.out.println("\n-----------------------------------------\n");
    System.out.println("VALUES:\n"+
                    "linear_acc="+linear_acc+"\n"+
                    "min lin="+falldetector.getMin_lin_acc()+"\n+" +
                    "max lin="+falldetector.getMax_lin_acc()+"\n+" +
                    "min ang="+falldetector.getMin_ang_acc()+"\n+" +
                    "max ang="+falldetector.getMax_ang_acc()+"\n");

    System.out.println("\n-----------------------------------------\n");

    //verify if a fall happened
    int fall=falldetector.fallCheck(xla);
    System.out.println("Fall value: "+ fall);
    System.out.println("Database handler adds values for body");

    databaseHandler.addPatientDataMovement(topic,patientID, falldetector.get_last_linear_value(),falldetector.get_last_angular_value(),mac);

    if(fall==0){
        //sendMessage("fall?=0", "normal", databaseHandler.findSensIP(patientID, "fall"));
        System.out.println("The patient seems normal");

    } else if (fall==1){
        //sendMessage("fall?=1", "fallen", databaseHandler.findSensIP(patientID, "fall"));
        System.out.println("ATTENTION! The patient has fallen! ");
    }
    valueHandler(patientID,trestbps,fbs,restecg,thalach,mac,topic);
    sendBodyMessage("body", String.valueOf(fall),trestbps,fbs,restecg,thalach, databaseHandler.findSensIP(patientID, "body"));

}
    //Adding body values to the database
    public void valueHandler(int patientID,int trestbps,int fbs,int restecg,int thalach,String mac,String topic){
        System.out.println("Actuator received topic:"+topic);

        if(!topic.equals("patient_values")){
            System.out.printf("Unknown topic: %s\n", topic);
            return;}
        System.out.println("Database handler adds values for topic:"+topic);

        databaseHandler.addPatientDataBody(topic,patientID,trestbps,fbs,restecg,thalach,mac);
    }

    /**
     * Sends a CoAP message to the actuator.
     * resource:the resurce to which the CoAP will be sent
     * command: the command to be executed by CoAP request
     * actuatorsIP: actuator IP address
     */
    public void sendMessage(String resource, String command, String actuatorID){
            String url = "coap://["+actuatorID+"]/"+resource;
            CoapClient client = new CoapClient(url);
            Request req = new Request(CoAP.Code.PUT);
            req.setPayload(command);
            req.getOptions().setAccept(MediaTypeRegistry.TEXT_PLAIN);
            CoapResponse response = client.advanced(req); //send request and wait for response
            if(response!=null) {
                CoAP.ResponseCode code = response.getCode();
                switch (code) {
                    case CHANGED:
                        // case 204 - Success
                        System.out.println("Response for patient movements code: 204 - Ok!");
                        break;
                    case BAD_REQUEST:
                        System.out.println("Response error: Bad Request error");
                        break;
                    case BAD_OPTION:
                        System.out.println("Response error: Bad Option error");
                        break;
                    default:
                        System.out.println("Response error: Actuator error");
                        break;
                }
            }

    }
    /*
    public void sendBodyMessage(String resource, String command,int trestbps,int fbs,int restecg,int thalach, String actuatorID) {
            String url = "coap://["+actuatorID+"]/"+resource;
            CoapClient client = new CoapClient(url);
            Request req = new Request(CoAP.Code.PUT);

            // Sends a JSON payload
            JSONObject payload = new JSONObject();
            payload.put("command", command);
            payload.put("trestbps", trestbps);
            payload.put("fbs", fbs);
            payload.put("restecg", restecg);
            payload.put("thalach", thalach);
            req.setPayload(payload.toString());
            req.getOptions().setAccept(MediaTypeRegistry.APPLICATION_JSON);
            req.getOptions().setContentFormat(MediaTypeRegistry.APPLICATION_JSON); //aggiunto dopo

        CoapResponse response = client.advanced(req); //send request and wait for response
            if(response!=null) {
                CoAP.ResponseCode code = response.getCode();
                switch (code) {
                    case CHANGED:
                        // case 204 - Success
                        System.out.println("Response for body message code: 204 - Ok!");
                        break;
                    case BAD_REQUEST:
                        System.out.println("Response error: Bad Request error");
                        break;
                    case BAD_OPTION:
                        System.out.println("Response error: Bad Option error");
                        break;
                    default:
                        System.out.println("Response error: Actuator error");
                        break;
                }
            }

    }*/
    public void sendBodyMessage(String resource, String command,int trestbps,int fbs,int restecg,int thalach, String actuatorID) {
        String url = "coap://["+actuatorID+"]/"+resource;
        CoapClient client = new CoapClient(url);
        Request req = new Request(CoAP.Code.PUT);

        // Sends a JSON payload
        JSONObject payload = new JSONObject();
        payload.put("command", command);
        payload.put("trestbps", trestbps);
        payload.put("fbs", fbs);
        payload.put("restecg", restecg);
        payload.put("thalach", thalach);
        req.setPayload(payload.toString());
        req.getOptions().setAccept(MediaTypeRegistry.APPLICATION_JSON);
        req.getOptions().setContentFormat(MediaTypeRegistry.APPLICATION_JSON); //aggiunto dopo

        CoapResponse response = client.advanced(req); //send request and wait for response
        if(response!=null) {
            System.out.println(response.getResponseText());
            CoAP.ResponseCode code = response.getCode();
            switch (code) {
                case CHANGED:
                    // case 204 - Success
                    System.out.println("Response for body message code: 204 - Ok!");
                    break;
                case BAD_REQUEST:
                    System.out.println("Response error: Bad Request error");
                    break;
                case BAD_OPTION:
                    System.out.println("Response error: Bad Option error");
                    break;
                default:
                    System.out.println("Response error: Actuator error");
                    break;
            }
        }

    }
}

