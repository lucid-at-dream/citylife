package gateway.service;

import gateway.service.Main;
import org.junit.jupiter.api.Test;

public class MainTest {

    @Test
    public void testTheSonarCoverageTest(){
        new Main().testSonarCoverage();
    }
}
