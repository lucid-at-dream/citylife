package com.feedzai.testServices;

import com.feedzai.jervicectl.Service;

public class AnotherService implements Service{

    public boolean start(){
        while(true){
            System.out.println("AnotherService::start()");
            try{
                Thread.sleep(500);
            }catch(InterruptedException e){
                e.printStackTrace();
            }
        }
    }

    public void stop(){
        System.out.println("AnotherService::stop()");
        try{
            Thread.sleep(500);
        }catch(InterruptedException e){
            e.printStackTrace();
        }
        System.out.println("AnotherService::stop()");
    }

}