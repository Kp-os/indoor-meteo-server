package com.diploma.meteo_server.model;

public class SettingsDto {
    private float temp;
    private float hum;
    private float dev;

    public SettingsDto() {
    }

    public float getTemp() {
        return temp;
    }

    public void setTemp(float temp) {
        this.temp = temp;
    }

    public float getHum() {
        return hum;
    }

    public void setHum(float hum) {
        this.hum = hum;
    }

    public float getDev() {
        return dev;
    }

    public void setDev(float dev) {
        this.dev = dev;
    }
}