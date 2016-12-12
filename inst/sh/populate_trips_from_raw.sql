ALTER TABLE trips_raw 
ALTER COLUMN start_time 
TYPE TIMESTAMP using to_timestamp(start_time, 'MM/DD/YYYY HH24:MI:SS');
ALTER TABLE trips_raw 
ALTER COLUMN stop_time 
TYPE TIMESTAMP using to_timestamp(stop_time, 'MM/DD/YYYY HH24:MI:SS');

INSERT INTO stations (id, name, latitude, longitude)
SELECT DISTINCT start_station_id, start_station_name, start_station_latitude, start_station_longitude
FROM trips_raw
WHERE start_station_id NOT IN (SELECT id FROM stations);

INSERT INTO stations (id, name, latitude, longitude)
SELECT DISTINCT end_station_id, end_station_name, end_station_latitude, end_station_longitude
FROM trips_raw
WHERE end_station_id NOT IN (SELECT id FROM stations);

INSERT INTO trips
(trip_duration, start_time, stop_time, start_station_id, end_station_id, bike_id, user_type, birth_year, gender)
SELECT 
  trip_duration, start_time, stop_time, start_station_id, end_station_id, bike_id, user_type,
  NULLIF(birth_year, '')::int, NULLIF(gender, '')::int
FROM trips_raw;

---TRUNCATE TABLE trips_raw;
DROP TABLE trips_raw;
