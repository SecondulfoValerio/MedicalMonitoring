package it.unipi.iot;

import org.eclipse.californium.core.*;
import org.eclipse.californium.core.coap.CoAP;
import org.eclipse.californium.core.coap.MediaTypeRegistry;
import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.coap.Response;
import org.eclipse.californium.core.server.resources.CoapExchange;
import org.json.simple.JSONObject;
import org.json.simple.JSONValue;
import org.json.simple.parser.ParseException;


/**
 * The CoapRegistrationResource class represents a CoAP resource for actuator registration.
 * It handles GET and POST requests related to actuator registration.
 */
public class Coap_Handler extends CoapResource {

    Database databaseHandler;
    private static CoapClient client = null;
    Actuator actuator= new Actuator(1,this.databaseHandler);

    /**
     * Constructs a new CoapRegistrationResource object.
     * name: the name of the CoAP resource
     * databaseHandler: Database obj
     */
    public Coap_Handler(String name, Database databaseHandler) {
        super(name);
        this.databaseHandler = databaseHandler;

    }




    /**
     * Get method for requests received from COAP resource.
     * Handles a GET request received by the CoAP resource.
     * exchange value for handling the request
     */
    public void handleGET(CoapExchange exchange) {
        exchange.respond("MedicalMonitoring");
    }


    /**
     * Post request received by the CoAP resource.
     * exchange value for handling the request
     */
    public void handlePOST(CoapExchange exchange) {
        Response response = new Response(CoAP.ResponseCode.CONTENT);
        if(exchange.getRequestOptions().getContentFormat() == MediaTypeRegistry.APPLICATION_JSON) {
            String payload = exchange.getRequestText();
            try {
                // unpack the request
                JSONObject requestJson = (JSONObject) JSONValue.parseWithException(payload);
                String appName = (String) requestJson.get("app");
                int patient_ID= Integer.parseInt(requestJson.get("Patient_ID").toString());
                //System.out.println("Actuator Registration:\tPatient_ID: " + patient_ID);
                if(appName.equals("MedicalMonitoring")) {
                    databaseHandler.addActuator(exchange.getSourceAddress().toString().substring(1), patient_ID);
                    response.setPayload("200"); // 200 is the code for success

                }
                //Starting observation
                //System.out.println("Starting observation...");
                startHeartObservation(databaseHandler.findSensIP(1));

            } catch (ParseException e) {
                System.out.println("ERROR IN: retrieve POST");

                throw new RuntimeException(e);
            }
        }
        else{
            response.setPayload("ERROR: Request expected in JSON format.");
        }
        exchange.respond(response);
    }



    public void startHeartObservation(final String ipAddress) {

        client = new CoapClient("coap://[" + ipAddress + "]/body");
        client.observe(
                new CoapHandler() {
                    @Override public void onLoad(CoapResponse response) {
                        handleBodyResponse(response);
                    }
                    @Override public void onError() {
                        System.out.println("Observing heart failed");
                    }
                }
        );
    }

    public void handleBodyResponse(CoapResponse response){
        //System.out.println("[!] Receiving heart values");
        String responseString = new String(response.getPayload());
        //System.out.println(" <  " + responseString+ ">");
        JSONObject request;
        try {
            request = (JSONObject) JSONValue.parseWithException(responseString);
            String status_paziente = request.get("stat").toString();
            int trestbps = Integer.parseInt(request.get("trestbps").toString());
            int fbs = Integer.parseInt(request.get("fbs").toString());
            int restecg = Integer.parseInt(request.get("restecg").toString());
            int thalach = Integer.parseInt(request.get("thalach").toString());
            //client.shutdown();
           // System.out.println("Risultato:");
            //System.out.println("status: "+status_paziente+"\ntrestbps: "+trestbps+" \nfbs: "+fbs+"\nrestecg: "+restecg+"\nthalach: "+thalach);
            //System.out.println("Saving values to DB...");
            databaseHandler.addPatientDataBody(1,trestbps,fbs,restecg,thalach);
            actuator.setHeartConditions(trestbps,fbs,restecg,thalach);
            if(status_paziente.equals("0")){
                //System.out.println("Patient heart is normal");
                //command 0 fall NO - command 1 fall YES - command 3 do nothing
                //alarm 0 Heart OK - alarm 1 bad heart start alarm - alarm 3 do nothing
                actuator.setHeart_command(0);
            }
            if(status_paziente.equals("1")){
                //System.out.println("Patient heart may have a desease");
                //command 0 fall NO - command 1 fall YES - command 3 do nothing
                //alarm 0 Heart OK - alarm 1 bad heart start alarm - alarm 3 do nothing
                actuator.setHeart_command(1);
            }

        } catch (ParseException e){
            e.printStackTrace();
            throw new RuntimeException(e);
        }

    }




    }
