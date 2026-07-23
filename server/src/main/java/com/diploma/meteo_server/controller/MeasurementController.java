package com.diploma.meteo_server.controller;

import com.diploma.meteo_server.model.Measurement;
import com.diploma.meteo_server.model.SettingsDto;
import com.diploma.meteo_server.repository.MeasurementRepository;
import org.springframework.format.annotation.DateTimeFormat;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.time.LocalDateTime;
import java.time.LocalTime;
import java.util.List;

@RestController
@RequestMapping("/api/sensor")
public class MeasurementController {
    private final MeasurementRepository repository;

    public static float targetTemperature;
    public static float targetHumidity;
    public static float deviation;

    public MeasurementController(MeasurementRepository repository) {
        this.repository = repository;
    }

    @GetMapping
    public ResponseEntity<List<Measurement>> getData(
            @RequestParam(required = false) @DateTimeFormat(iso = DateTimeFormat.ISO.DATE_TIME) LocalDateTime start,
            @RequestParam(required = false) @DateTimeFormat(iso = DateTimeFormat.ISO.DATE_TIME) LocalDateTime end) {

        if (start == null) {
            start = LocalDateTime.now().with(LocalTime.MIN);
        }
        if (end == null) {
            end = LocalDateTime.now();
        }

        List<Measurement> history = repository.findByTimestampBetweenOrderByTimestampAsc(start, end);

        return ResponseEntity.ok(history);
    }

    @GetMapping(value = "/settings", produces = MediaType.APPLICATION_JSON_VALUE)
    public ResponseEntity<String> getSettings() {
        return ResponseEntity.ok("{\"temp\":" + targetTemperature + ",\"hum\":" + targetHumidity + ",\"dev\":" + deviation + "}");
    }

    @PostMapping
    public ResponseEntity<String> receiveData(@RequestBody Measurement measurement) {
        repository.save(measurement);

        return ResponseEntity.ok(targetTemperature + "," + targetHumidity + "," + deviation);
    }

    @PostMapping(value = "/settings", consumes = MediaType.APPLICATION_JSON_VALUE)
    public ResponseEntity<String> updateSettings(@RequestBody SettingsDto settings) {
        targetTemperature = settings.getTemp();
        targetHumidity = settings.getHum();
        deviation = settings.getDev();

        return ResponseEntity.ok("Settings saved");
    }
}
