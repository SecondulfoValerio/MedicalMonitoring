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
    private static int fall_command =0;
    private static int heart_command=0;
    private static String status;
    private static int trestbps=0;
    private static int fbs=0;
    private static int restecg=0;
    private static int thalach=0;



    public Actuator(int patientID, Database databaseHandler){

        this.databaseHandler = databaseHandler;
        this.patientID = patientID;
    }
    public void setFall_command(int value){
        this.fall_command=value;
    }
    public void setHeart_command(int value){
        this.heart_command=value;
    }
    public void setStatus(String stat){this.status=stat;}
    public String getStatus(){return this.status;}
    public void setHeartConditions(int trestbps,int fbs,int restecg,int thalach){
        this.trestbps=trestbps;
        this.fbs=fbs;
        this.restecg=restecg;
        this.thalach=thalach;
    }
    public String getHeartConditions(){
        String heartcon= ("Patient body conditions:\n" +
                "trestbps= "+this.trestbps+ "\n" +
                "fbs= "     +this.fbs + "\n" +
                "restecg= "+ this.restecg + "\n" +
                "thalach= "+ this.thalach + "\n") ;
                 return heartcon;
    }





    //Calculates the accelerations and report the fall status saving data to DB and
    //sends a CoAP message with heart and fall values
public void handler(int patientID, int xla, int yla, int zla, int xaa, int yaa,int zaa, String mac, String topic){
    //System.out.println("Actuator received topic:"+topic);
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

    //verify if a fall happened
    int fall=falldetector.fallCheck(xla);
    //System.out.println("Fall value: "+ fall);
    //System.out.println("Database handler adds values for body");

    databaseHandler.addPatientDataMovement(topic,patientID, falldetector.get_last_linear_value(),falldetector.get_last_angular_value(),mac);

    if(fall==0){
        this.fall_command=0;
        //sendMessage("fall?=0", "normal", databaseHandler.findSensIP(patientID, "fall"));
        //System.out.println("The patient seems normal");

    } else if (fall==1){
        this.fall_command=1;
        //sendMessage("fall?=1", "fallen", databaseHandler.findSensIP(patientID, "fall"));
        //System.out.println("ATTENTION! The patient has fallen! ");
    }
    //alarm set to 3 does not change the heart alarm status in the Coap node
    sendBodyMessage("alarm", String.valueOf(this.fall_command),String.valueOf(this.heart_command), databaseHandler.findSensIP(patientID));

}
    /**
     * Sends a CoAP message to the actuator.
     * resource:the resurce to which the CoAP will be sent
     * command: is the fall value to report to CoAP alarm resource
     * alarm: the is the heart value to report to CoAP alarm resource
     * actuatorsIP: actuator IP address
     */

    public void sendBodyMessage(String resource, String command,String alarm, String actuatorID) {
        String url = "coap://[" + actuatorID + "]/" + resource;
        CoapClient client = new CoapClient(url);
        Request req = new Request(CoAP.Code.PUT);
        //System.out.println("\nSENDING BODY MESSAGE " + "FALL + "+command + " HEART + "+ alarm+"\n");
        // Sends a JSON payload
        JSONObject payload = new JSONObject();
        payload.put("command", command);
        payload.put("alarm", alarm);

        req.setPayload(payload.toString());
        req.getOptions().setAccept(MediaTypeRegistry.APPLICATION_JSON);
        req.getOptions().setContentFormat(MediaTypeRegistry.APPLICATION_JSON); //aggiunto dopo

        CoapResponse response = client.advanced(req); //send request and wait for response
        if (response != null) {
            setStatus(response.getResponseText());
            //System.out.println(response.getResponseText());
            CoAP.ResponseCode code = response.getCode();
            switch (code) {
                case CHANGED:
                    // case 204 - Success
                    //System.out.println("Response for body message code: 204 - Ok!");
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

