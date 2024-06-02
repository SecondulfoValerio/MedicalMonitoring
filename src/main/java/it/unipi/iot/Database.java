package it.unipi.iot;

import java.io.FileInputStream;
import java.io.IOException;
import java.sql.*;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;


/**
 * The DatabaseHandler class handles database operations for actuator and sensor data.
 * <p>
 * private String db_IP;
 * private String port;
 * private String db_name;
 * private String user;
 * private String password;
 * <p>
 * /**
 * Constructs a new DatabaseHandler object by loading the database configuration from a file.
 *
 * @param config_path the path to the configuration file
 */

/**Manages the Database for storing data from mqtt node about patient's status
 */
public class Database {
    String db_IP; //for db ip
    String port; //for db port
    String db_name; //for db name
    String user; //user of db: set to root
    String psw; //user password: set to root

    public Database(String file_conf) {
        Properties conf = new Properties();
        try (FileInputStream input = new FileInputStream(file_conf)) {
            conf.load(input);
        } catch (IOException error) {
            System.err.println("An error has occured: Couldn't load configuration file");
            error.printStackTrace();
        }

        this.db_IP = conf.getProperty("db.IP");
        this.db_name = conf.getProperty("db.name");
        this.port = conf.getProperty("db.port");
        this.user = conf.getProperty("db.username");
        this.psw = conf.getProperty("db.password");
        System.out.println(this.db_IP+" "+this.db_name+" "+this.port+" "+this.user+" "+this.psw);
    }

    /**
     * Creates an actuator to handle DB operations
     * it has an ip address
     * it's associated to a patient ID
     * it has a specific role (possibile da rimuovere)
     */
    public boolean addActuator(String ip, int patientID) {
        System.out.println("INSERTING ACTUATOR" );

        String url = "jdbc:mysql://" + db_IP + ":" + port + "/" + db_name;
        System.out.println("print db url:"+ url);
        String sql = "INSERT INTO Actuators(Actuator_ID, Patient_ID) values (?,?)";
        System.out.println("print db string:" +sql);

        try (Connection co = DriverManager.getConnection(url, user, psw);
            PreparedStatement pr = co.prepareStatement(sql)) {
            pr.setString(1, ip);
            pr.setInt(2, patientID);
            int rowsInserted = pr.executeUpdate();
            if (rowsInserted <= 0) {
                System.out.println("Cannot insert Actuator: Actuator already exists!");
                return false;
            }
        } catch (SQLIntegrityConstraintViolationException error) {
            System.out.println("Cannot insert Actuator: Actuator already exists!");
            return true;
        } catch (SQLException e) {
            e.printStackTrace();
            return false;
        }
        System.out.println("Actuator registered:\nPatientID: " + patientID);
        return true;
    }

    /**
     * Adds new data into the DB
     * Creates and insert new data related to a patient movement coming from the mqtt sensor
     */
    public boolean addPatientDataMovement(String topic,int patientID, double lastlin_acc, double last_ang_acc,String mac) {
        System.out.println("INSERTING DATA BODY" );

        String url = "jdbc:mysql://" + db_IP + ":" + port + "/" + db_name;
        System.out.println("print db url:"+ url);
        String sql = "INSERT INTO MovementData(Topic, Patient_ID, lastlin_acc, last_ang_acc, MAC) values (?,?,?,?,?)";
        System.out.println("print db string:" +sql);
        try (Connection co = DriverManager.getConnection(url, user, psw);
             PreparedStatement pr = co.prepareStatement(sql)) {
                pr.setString(1, topic);
                pr.setInt(2, patientID);
                pr.setDouble(3, lastlin_acc);
                pr.setDouble(4, last_ang_acc);
                pr.setString(5, mac);

                int rowsInserted = pr.executeUpdate();
                if (rowsInserted <= 0) {
                    System.out.println("Error: No data inserted");
                    return false;
                }
        } catch (SQLException error) {
            System.out.println("Error: No data inserted");
            error.printStackTrace();

            return false;
        }
        System.out.println("Data inserted:\n Topic: " + topic +"\n"+"Patiend ID: " + patientID + "\n" +"Last linear acceleration: " + lastlin_acc + "\n" +"Last angular acc: " + last_ang_acc + "\n"+"MAC address: " + mac + " \n");
        return true;
    }

    public boolean addPatientDataBody(String topic,int patientID, int trestbps, int fbs, int restecg, int thalach,String mac) {
        System.out.println("INSERTING PATIENT BODY" );

        String url = "jdbc:mysql://" + db_IP + ":" + port + "/" + db_name;
        System.out.println("print db url:"+ url);
        String sql = "INSERT INTO BodyData(Topic,Patient_ID, trestbps, fbs, restecg, thalach, MAC) values (?,?,?,?,?,?,?)";
        System.out.println("print db string:" +sql);

        try (Connection co = DriverManager.getConnection(url, user, psw);
             PreparedStatement pr = co.prepareStatement(sql)) {
            pr.setString(1, topic);
            pr.setInt(2, patientID);
            pr.setInt(3, trestbps);
            pr.setInt(4, fbs);
            pr.setInt(5, restecg);
            pr.setInt(6, thalach);
            pr.setString(7, mac);
            int rowsInserted = pr.executeUpdate();
            if (rowsInserted <= 0) {
                System.out.println("Error: No data insered");
                return false;
            }
        } catch (SQLException error) {
            System.out.println("Error: No data insered");
            error.printStackTrace();

            return false;
        }
        System.out.println("Data inserted:\n Topic: " + topic +"\n"+"Patiend ID: " + patientID + "\n" +"trestbps: " + trestbps + "\n" +"fbs: " + fbs +
                "restecg: " + restecg + "\n" +"thalach: " + thalach + "\n" +"MAC address: " + mac + " \n");
        return true;
    }

    /**
     *Return the IP address of the sensor associated to a specific patient in the DB
     */
    public String findSensIP(int patientID, String resource) {
        System.out.println("SEARCH FOR FINDSENS IP" );
                String ip_value = new String();
        String url = "jdbc:mysql://" + db_IP + ":" + port + "/" + db_name;
        String sql = "SELECT Actuator_ID FROM Actuators WHERE Patient_ID=?";
        try (Connection co = DriverManager.getConnection(url, user, psw);
             PreparedStatement pr = co.prepareStatement(sql)) {
                pr.setInt(1, patientID);
                //pr.setString(2, resource);
                ResultSet r = pr.executeQuery();
                while (r.next()) {
                    ip_value=(r.getString(1));
                }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return ip_value;
    }
}