package com.feedzai.jervicectl;

import java.util.HashMap;
import java.util.ArrayList;
import java.util.Scanner;

public class Jervicectl{

    private ServiceManager serviceMgr;

    public static void main( String[] args )
    {
        Jervicectl jervicectl = new Jervicectl();

        Scanner stdin = new Scanner(System.in);
        while(true){
            try{
                jervicectl.parseCommand( stdin.nextLine() );
            }catch(Exception e){
                stdin = new Scanner(System.in);
            }
        }
    }

    public Jervicectl(){
        this.serviceMgr = new ServiceManager();
    }

    private String getCommand(String line){
        return line.split(" ")[0].trim();
    }

    private String getArg(String command, String line){
        return line.substring(command.length()).trim();
    }

    void parseCommand(String line){
        line = line.trim();

        String cmd = getCommand(line);
        String arg = getArg(cmd, line);

        switch(cmd){
            case "loadconfig":
                try{
                    serviceMgr.loadServicesFromConf( arg );
                }catch(NoSuchServiceException e){
                    System.err.println("Configuration file contains errors: " + e.getMessage());
                }catch(NoSuchConfigFileException e){
                    System.err.println("Configuration was nowhere to be seen: " + e.getMessage());
                }
                break;

            case "startall":
                serviceMgr.startAll();
                break;

            case "stopall":
                serviceMgr.stopAll();
                break;

            case "statusall":
                serviceMgr.logAllStatus();
                break;

            case "start":
                try{
                    serviceMgr.startService( "com.feedzai.testServices.Service" + arg );
                }catch(NoSuchServiceException e){
                    System.out.println( e.getMessage() );
                }
                break;

            case "stop":
                try{
                    serviceMgr.stopService( "com.feedzai.testServices.Service" + arg );
                }catch(NoSuchServiceException e){
                    System.out.println( e.getMessage() );
                }
                break;

            case "status":
                try{
                    serviceMgr.logServiceStatus( "com.feedzai.testServices.Service" + arg );
                }catch(NoSuchServiceException e){
                    System.out.println( e.getMessage() );
                }
                break;

            case "system":
                break;

            default:
                System.out.println(
                    "loadconfig <fname>\n" +
                    "startall\n" +
                    "stopall\n" +
                    "statusall\n" +
                    "start <service>\n"+
                    "stop <service>\n"+
                    "status <service>\n" +
                    "system"
                );
        }
    }
}
