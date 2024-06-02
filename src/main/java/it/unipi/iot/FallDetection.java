package it.unipi.iot;
import java.lang.Math;

public class FallDetection {
    private double min_lin_acc; //minimal linear acceleration
    private double min_ang_acc; //minimal angular acceleration
    private double max_lin_acc; //maximal linear acceleration
    private double max_ang_acc; //maximal angular acceleration
    private double[] linear_acc;
    private double[] angular_acc;
    private int lin_index;
    private int ang_index;

/**
public FallDetection(double xla,double yla,double zla,double xaa,double yaa,double zaa){
    this.linear_acc=new double[5];
    this.angular_acc=new double[5];
    this.lin_index=0;
    this.ang_index=0;
    this.min_lin_acc=0;
    this.max_lin_acc=0;
    this.min_ang_acc=0;
    this.max_ang_acc=0;

}*/
    public FallDetection(){
        this.linear_acc=new double[5];
        this.angular_acc=new double[5];
        this.lin_index=0;
        this.ang_index=0;
        this.min_lin_acc=0;
        this.max_lin_acc=0;
        this.min_ang_acc=0;
        this.max_ang_acc=0;

    }
    //calculates linear acceleration
    public double lin_acc(double xla,double yla,double zla){
        double lin_acc=Math.sqrt(Math.pow(xla,2)+Math.pow(yla,2)+Math.pow(zla,2));
        return lin_acc;
    }
    //calculates angular acceleration
    public double ang_acc(double xaa,double yaa,double zaa){
        double ang_acc=Math.sqrt(Math.pow(xaa,2)+Math.pow(yaa,2)+Math.pow(zaa,2));
        return ang_acc;
        }

    //adds a new value in the linear acceleration array
    public void add_lin_acc(double lin_acc){
        if(lin_index<5){
            this.linear_acc[lin_index]=lin_acc;
            this.lin_index++;
        }
        else{
            for(int i=0;i<4;i++){
                this.linear_acc[i]=this.linear_acc[i+1];
            }
            this.linear_acc[4]=lin_acc;
        }
    }
    //adds a new value in the angular acceleration array
    public void add_ang_acc(double ang_acc){
        if(ang_index<5){
            this.angular_acc[ang_index]=ang_acc;
            this.ang_index++;
        }
        else{
            for(int i=0;i<4;i++){
                this.angular_acc[i]=this.angular_acc[i+1];
            }
            this.angular_acc[4]=ang_acc;
        }
    }
    //set min linear acceleration
    public void set_min_lin_acc(){
        if(this.lin_index<5)
            return;
        else
            this.min_lin_acc=get_min(this.lin_index,this.linear_acc);
    }

    //set max linear acceleration
    public void set_max_lin_acc(){
        if(this.lin_index<5)
            return;
        else
            this.max_lin_acc=get_max(this.lin_index,this.linear_acc);
    }

    //set min angular acceleration
    public void set_min_ang_acc(){
        if(this.ang_index<5)
            return;
        else
            this.min_ang_acc=get_min(this.ang_index,this.angular_acc);
    }

    //set max angular acceleration
    public void set_max_ang_acc(){
        if(this.ang_index<5)
            return;
        else
            this.max_ang_acc=get_max(this.ang_index,this.angular_acc);
    }
    //get max value for an array
    public double get_max(int index, double[] arr){
        double max=arr[0];
        for(int i=1;i<index;i++){
            if(max < arr[i])
                max=arr[i];

    }
        return max;

    }

    //get min value for an array
    public double get_min(int index, double[] arr){
        double min=arr[0];
        for(int i=1;i<index;i++){
            if(min > arr[i])
                min=arr[i];

        }
        return min;

    }
    public double getMin_lin_acc(){return this.min_lin_acc;}
    public double getMax_lin_acc(){return this.max_lin_acc;}
    public double getMin_ang_acc() {return min_ang_acc;}
    public double getMax_ang_acc() {return max_ang_acc;}

    //returns the last linear acceleration value
    public double get_last_linear_value(){
        return this.linear_acc[this.lin_index-1];
    }
    //returns the last angular acceleration value
    public double get_last_angular_value(){
        return this.angular_acc[this.ang_index-1];
    }
    //fall detection method
    //returns 0 if the subject is in a normal status or falls intentionally
    // returns 1 if the fall is unintentional
    public int fallCheck(double xla){
        //check if a fall happened
        if(Math.abs(this.max_lin_acc-this.min_lin_acc)<0.4 && Math.abs(this.max_ang_acc-this.min_ang_acc)<60){
            //check if the subject us lying on a surface
            System.out.println("\n----------------------------------------\n");
            System.out.println("Calcolo accelerazioni:\nLin acc="+xla+"\nmax lin acc= "+this.max_lin_acc+"\nmax ang acc= "+ this.max_ang_acc+"\n"+(Math.acos(xla/9.8)>35));
            System.out.println("\n----------------------------------------\n");
            double arccos=Math.acos(xla/9.8);
            double arccosdeg=Math.toDegrees(arccos);
            if(arccosdeg>35)
                //check if the fall was intentional
                if(this.max_lin_acc>2.5 && this.max_ang_acc>340)
                    return 0; //intentional
            else
                    return 1; //unintentional
        }
        return 0; //by default return 0 as a normal state for the subject
    }




}
