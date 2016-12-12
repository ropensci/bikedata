CREATE INDEX idx_trips_on_start_station ON trips (start_station_id);
CREATE INDEX idx_trips_on_end_station ON trips (end_station_id);
CREATE INDEX idx_trips_on_dow ON trips (EXTRACT(DOW FROM start_time));
CREATE INDEX idx_trips_on_hour ON trips (EXTRACT(HOUR FROM start_time));
CREATE INDEX idx_trips_on_date ON trips (date(start_time));

CREATE TABLE station_to_station_counts AS
SELECT start_station_id, end_station_id, COUNT(*) AS count
FROM trips
GROUP BY start_station_id, end_station_id;
CREATE INDEX idx_station_to_station ON station_to_station_counts (start_station_id, end_station_id);
