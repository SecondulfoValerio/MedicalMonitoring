package it.unipi.iot;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;


public class UserControl implements Runnable {
    Database dbhandler;
    Coap_Handler handler;

    public UserControl(Database databaseHandler, Coap_Handler coap_handler){
        this.dbhandler=databaseHandler;
        this.handler=coap_handler;
    };
        public void showCommandList(){
            System.out.println("****************************************************************");
            System.out.println("                          Commands                              ");
            System.out.println("****************************************************************");
            System.out.println("[1] Patient Infos");
            System.out.println("[2] Patient Heart monitoring");
            System.out.println("[3] Patient Status");
            System.out.println("[4] Set off alarm");
            System.out.println("[5] Exit");
            System.out.println("****************************************************************");
        }

        public void selectCommand(){
            while(true){
                Actuator actuator= new Actuator(1,this.dbhandler);
                showCommandList();
                BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
                try {
                    int command = Integer.parseInt(reader.readLine());
                    switch (command) {
                        case 1:  //  Patient Infos
                            dbhandler.getPatientInfo(1);
                            break;

                        case 2: //  Patient Heart monitoring
                            System.out.println(actuator.getHeartConditions());
                            break;

                        case 3 : // Patient Status
                            System.out.println(actuator.getStatus());

                            break;

                        case 4: // Set off alarm
                            actuator.sendBodyMessage("alarm", "3","0", this.dbhandler.findSensIP(1));

                            break;
                        case 5:  // Exit
                            System.exit(0);
                            break;
                        default:
                            System.out.println("You must insert an integer between 1 and 5");
                            showCommandList();
                            break;
                    }

                } catch(IOException e) {

                     e.printStackTrace();
                     throw new RuntimeException(e);
                }
            }
        }

        @Override
        public void run() {
            selectCommand();
        }
    }


