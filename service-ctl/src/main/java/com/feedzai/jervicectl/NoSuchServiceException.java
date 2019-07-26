package com.feedzai.jervicectl;

public class NoSuchServiceException extends Exception{
    //Parameterless Constructor
    public NoSuchServiceException() {}

    //Constructor that accepts a message
    public NoSuchServiceException(String message)
    {
     super(message);
    }
}