package com.feedzai.jervicectl;

import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

import com.feedzai.testServices.AbstractTestService;

public class ServiceManagerTest extends TestCase {

    String prefix = "com.feedzai.testServices.Service";
    private ServiceManager serviceMgr;
    private String[] serviceNames;

    public ServiceManagerTest( String testName )
    {
        super( testName );
    }

    public static Test suite()
    {
        return new TestSuite( ServiceManagerTest.class );
    }

    public void setUp(){
        this.serviceMgr = new ServiceManager();
        
        String[] suffixes = {"A","B","C","D","E","F",
                             "G","H","I","J","K","L",
                             "M","N","O","P","Q","R",
                             "S","T","U","V","W","X",
                             "Y","Z"};
        serviceNames = new String[26];
        for( int i=0; i<26; i++ )
            serviceNames[i] = prefix + suffixes[i];

        this.serviceMgr.loadServicesFromConf("testConfigs/big.cfg");
    }

    public AbstractTestService getAbstractTestServiceFromSuffix(String suffix){
        return (AbstractTestService)serviceMgr.getService(prefix + suffix).getService();
    }


    /*
    * Le tests ...
    * **Ze is in the living room testing his new TV set**
    * **There is plastic all over the place, it's a mess.**
    * **Maria arrives**
    *
    * Maria __ What the hell?! U kiddin' me?
    * Ze    __ I told you we need to hire a mexican maid . . .
    */

    public void testServicesLoadingFromConfFile(){
        for(String service : serviceNames)
            assert( this.serviceMgr.getService( service ) != null );
    }

    public void testDependenciesLoadingFromConfFile(){
        ServiceWrapper serviceD = this.serviceMgr.getService(prefix+"D"),
                       serviceG = this.serviceMgr.getService(prefix+"G"),
                       serviceK = this.serviceMgr.getService(prefix+"K"),
                       serviceL = this.serviceMgr.getService(prefix+"L"),
                       serviceO = this.serviceMgr.getService(prefix+"O"),
                       serviceP = this.serviceMgr.getService(prefix+"P");

        assert( serviceG.dependsOn(serviceD) );
        assert( serviceD.isDependencyOf(serviceG) );

        assert( serviceL.dependsOn(serviceK) );
        assert( serviceK.isDependencyOf(serviceL) );

        assert( serviceP.dependsOn(serviceO) );
        assert( serviceO.isDependencyOf(serviceP) );
    }

    public void testServiceStartingWithoutDependencies(){
        serviceMgr.startService(prefix + "A");
        serviceMgr.waitForJobsAndStopAllThreads(3000);

        assert( serviceMgr.getService(prefix+"A").getServiceState() == ServiceState.RUNNING );
    }

    public void testServiceStartingWithOneDependency(){
        serviceMgr.startService(prefix + "D");
        serviceMgr.waitForJobsAndStopAllThreads(3000);

        assert( serviceMgr.getService(prefix+"A").getServiceState() == ServiceState.RUNNING );
        assert( serviceMgr.getService(prefix+"D").getServiceState() == ServiceState.RUNNING );
    }

    public void testOneDependencyStartsBeforeService(){
        AbstractTestService srvD = getAbstractTestServiceFromSuffix("D");
        AbstractTestService srvA = getAbstractTestServiceFromSuffix("A");

        serviceMgr.startService(prefix + "D");
        serviceMgr.waitForJobsAndStopAllThreads(3000);

        assert( srvD.startedAfter(srvA) );
    }

    public void testServiceStartingWithManyDependencies(){
        serviceMgr.startService(prefix + "H");
        serviceMgr.waitForJobsAndStopAllThreads(3000);

        assert( serviceMgr.getService(prefix+"H").getServiceState() == ServiceState.RUNNING );
        assert( serviceMgr.getService(prefix+"G").getServiceState() == ServiceState.RUNNING );
        assert( serviceMgr.getService(prefix+"F").getServiceState() == ServiceState.RUNNING );
        assert( serviceMgr.getService(prefix+"D").getServiceState() == ServiceState.RUNNING );
        assert( serviceMgr.getService(prefix+"E").getServiceState() == ServiceState.RUNNING );
        assert( serviceMgr.getService(prefix+"A").getServiceState() == ServiceState.RUNNING );
    }

    public void testManyDependenciesStartInOrder(){
        AbstractTestService srvC = getAbstractTestServiceFromSuffix("C");
        AbstractTestService srvM = getAbstractTestServiceFromSuffix("M");
        AbstractTestService srvN = getAbstractTestServiceFromSuffix("N");
        AbstractTestService srvO = getAbstractTestServiceFromSuffix("O");
        AbstractTestService srvP = getAbstractTestServiceFromSuffix("P");
        AbstractTestService srvQ = getAbstractTestServiceFromSuffix("Q");

        serviceMgr.startService(prefix + "Q");
        serviceMgr.waitForJobsAndStopAllThreads(3000);

        assert( srvQ.startedAfter(srvP) );

        assert( srvP.startedAfter(srvM) );
        assert( srvP.startedAfter(srvN) );
        assert( srvP.startedAfter(srvO) );

        assert( srvM.startedAfter(srvC) );
        assert( srvN.startedAfter(srvC) );
        assert( srvO.startedAfter(srvC) );
    }

    public void testStartAll(){
        serviceMgr.startAll();
        serviceMgr.waitForJobsAndStopAllThreads(3000);

        for(String service : serviceNames){
            assert( serviceMgr.getService(service).getServiceState() == ServiceState.RUNNING );
        }
    }

    public void testStopAll(){
        serviceMgr.startAll();
        serviceMgr.stopAll();

        serviceMgr.waitForJobsAndStopAllThreads(3000);

        for(String service : serviceNames){
            assert( serviceMgr.getService(service).getServiceState() == ServiceState.STOPPED );
        }
    }

    public void testContradictoryCommandsOnServiceWithoutDependencies(){
        serviceMgr.startService(prefix + "W");
        serviceMgr.stopService(prefix + "W");
        serviceMgr.startService(prefix + "W");
        serviceMgr.stopService(prefix + "W");

        serviceMgr.waitForJobsAndStopAllThreads(3000);

        assert( serviceMgr.getService(prefix+"W").getServiceState() == ServiceState.STOPPED );
    }

    public void testDependencyConsistencyAfterBigSequenceOfOperations(){
        serviceMgr.startAll();
        serviceMgr.stopService(prefix + "D");
        serviceMgr.stopService(prefix + "N");
        serviceMgr.startAll();
        serviceMgr.stopAll();
        serviceMgr.startService(prefix + "N");
        serviceMgr.stopService(prefix + "N");
        serviceMgr.startService(prefix + "P");
        serviceMgr.startService(prefix + "A");
        serviceMgr.startService(prefix + "K");
        serviceMgr.startService(prefix + "L");
        serviceMgr.startService(prefix + "M");
        serviceMgr.stopService(prefix + "A");

        serviceMgr.waitForJobsAndStopAllThreads(3000);

        for(String sname : serviceNames){
            ServiceWrapper service = serviceMgr.getService(sname);
            
            assert( service.getServiceState() == ServiceState.RUNNING || 
                    service.getServiceState() == ServiceState.STOPPED );

            if( service.getServiceState() == ServiceState.RUNNING )
                for( ServiceWrapper dependency : service.getDependencies() )
                    assert( dependency.getServiceState() == ServiceState.RUNNING );

            if( service.getServiceState() == ServiceState.STOPPED )
                for( ServiceWrapper dependent : service.getDependents() )
                    assert( dependent.getServiceState() == ServiceState.STOPPED );
        }
    }



}
