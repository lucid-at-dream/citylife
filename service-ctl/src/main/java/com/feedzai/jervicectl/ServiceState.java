package com.feedzai.jervicectl;

public enum ServiceState{
    WAITING_DEPENDENCIES_START,
    STARTING,
    RUNNING,
    WAITING_DEPENDENCIES_STOP,
    STOPPING,
    STOPPED
}