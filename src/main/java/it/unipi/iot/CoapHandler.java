package it.unipi.iot;


import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.coap.CoAP;
import org.eclipse.californium.core.coap.MediaTypeRegistry;
import org.eclipse.californium.core.coap.Response;
import org.eclipse.californium.core.server.resources.CoapExchange;
import org.json.simple.JSONObject;
import org.json.simple.JSONValue;
import org.json.simple.parser.ParseException;

/**
 * The CoapRegistrationResource class represents a CoAP resource for actuator registration.
 * It handles GET and POST requests related to actuator registration.
 */
public class CoapHandler extends CoapResource {

    Database databaseHandler;

    /**
     * Constructs a new CoapRegistrationResource object.
     * name: the name of the CoAP resource
     * databaseHandler: Database obj
     */
    public CoapHandler(String name, Database databaseHandler) {
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
                System.out.println("Actuator Registration:\tPatient_ID: " + patient_ID);
                if(appName.equals("MedicalMonitoring")) {
                    databaseHandler.addActuator(exchange.getSourceAddress().toString().substring(1), patient_ID);
                    response.setPayload("200"); // 200 is the code for success
                }
            } catch (ParseException e) {
                throw new RuntimeException(e);
            }
        }
        else{
            response.setPayload("ERROR: Request expected in JSON format.");
        }
        exchange.respond(response);
    }
}
