package com.feedzai.testServices;

import com.feedzai.jervicectl.Service;
import java.util.Random;

public class AbstractTestService {

    public double startStartTime;
    public double startEndTime;
    public double stopStartTime;
    public double stopEndTime;

    public static Random randGen = new Random(0);    

    public boolean startedBefore(AbstractTestService s){
        return this.startEndTime <= s.startStartTime;
    }

    public boolean startedAfter(AbstractTestService s){
        return this.startStartTime >= s.startEndTime;
    }

    public boolean stopedBefore(AbstractTestService s){
        return this.stopEndTime <= s.stopStartTime;
    }

    public boolean stopedAfter(AbstractTestService s){
        return this.stopStartTime >= s.stopEndTime;
    }

}
