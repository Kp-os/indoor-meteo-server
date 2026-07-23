package com.diploma.meteo_server.repository;

import com.diploma.meteo_server.model.Measurement;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.time.LocalDateTime;
import java.util.List;

@Repository
public interface MeasurementRepository extends JpaRepository<Measurement, Long> {
    List<Measurement> findByTimestampBetweenOrderByTimestampAsc(LocalDateTime start, LocalDateTime end);
}
