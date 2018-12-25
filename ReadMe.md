
[![Build Status](https://travis-ci.org/futuo/citylife.svg?branch=master)](https://travis-ci.org/futuo/citylife)

[![code coverage](https://sonarcloud.io/api/project_badges/measure?project=futuo_citylife&metric=coverage)](https://sonarcloud.io/dashboard?id=futuo_citylife)

# What is city life?

City life is meant to be a group of horizontally scalable microservices, each targetted at providing a specific, yet generic and extensible, functionality.

# What is the roadmap?

Implementing the following microservices:
* File sharing
* Event scheduling

Implementing a gateway layer. This is another service that accepts endpoint registrations from other microservices and provides access from the outsite world via a REST api.

Implementing a packaging strategy for each module of the project that allows horizontal scalability and high availability.

# What is the end goal?

The purpose is to provide a solid base to be used as the backend for any application. The idea is that a developer needs only to specify which services he needs and then implement the frontend against the gateway service.

The services can then be deployed in kubernetes or any other container cloud solution.
