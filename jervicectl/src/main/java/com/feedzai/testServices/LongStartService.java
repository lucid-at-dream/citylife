package com.feedzai.testServices;

import com.feedzai.jervicectl.Service;

public class LongStartService implements Service{

    public boolean start(){
        while(true){
            System.out.println("LongStartService::start()");
            try{
                Thread.sleep(500);
            }catch(InterruptedException e){
                e.printStackTrace();
            }
        }
    }

    public void stop(){
        System.out.println("LongStartService::stop()");
        try{
            Thread.sleep(500);
        }catch(InterruptedException e){
            e.printStackTrace();
        }
        System.out.println("LongStartService::stop()");
    }

}