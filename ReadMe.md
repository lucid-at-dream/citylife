[![Continuous Integration](https://github.com/lucid-at-dream/citylife/actions/workflows/ci.yml/badge.svg)](https://github.com/lucid-at-dream/citylife/actions/workflows/ci.yml)

[![Quality Gates](https://sonarcloud.io/api/project_badges/measure?project=futuo_citylife&metric=alert_status)](https://sonarcloud.io/dashboard?id=futuo_citylife)

[![Code Coverage](https://sonarcloud.io/api/project_badges/measure?project=futuo_citylife&metric=coverage)](https://sonarcloud.io/dashboard?id=futuo_citylife)

# What is city life?

City life is meant to be a group of horizontally scalable microservices, each targetted at providing a specific, yet generic and extensible, functionality.

# What is the roadmap?

Implementing the following microservices:
* Basic Accounting
* File sharing
* Event scheduling

Implementing a HTTP gateway service. A service that accepts endpoint registrations from other services and exposes access to them from outside. Communication from the outside to the service should be done with a REST api. Communication between the Gateway service and the other services should be done with Json via a TCP socket.

Implementing a packaging strategy for each module of the project that allows horizontal scalability and high availability.

# What is the end goal?

The purpose is to provide a solid base to be used as the backend for any application. The idea is that a developer needs only to specify which services he needs and then implement the frontend against the gateway service.

The services can then be deployed in kubernetes or any other container cloud solution.

# Commit rules

1. Create a feature (ft-*) or quality (qa-*) branch

2. Frequently commit & push your changes upstream

3. Make sure that the code is covered with tests. Strive for covering all added functionality with tests

4. Before merging the code into master, check the build status in Travis: https://travis-ci.org/futuo/citylife/

5. When the build is Green and the you made sure that the tests actually ran, merge with master and push upstream

6. Check the code coverage and make sure that you didn't introduce new debt that needs fixing in Sonar: https://sonarcloud.io/code?id=futuo_citylife

