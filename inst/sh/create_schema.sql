CREATE EXTENSION postgis;

CREATE TABLE trips (
  id serial primary key,
  trip_duration numeric,
  start_time timestamp without time zone,
  stop_time timestamp without time zone,
  start_station_id integer,
  end_station_id integer,
  bike_id integer,
  user_type varchar,
  birth_year integer,
  gender integer
);

CREATE TABLE stations (
  id integer primary key,
  name varchar,
  latitude numeric,
  longitude numeric
);

CREATE VIEW trips_and_stations AS (
  SELECT
    t.*,
    ss.name AS start_station_name,
    ss.latitude AS start_station_latitude,
    ss.longitude AS start_station_longitude,
    es.name AS end_station_name,
    es.latitude AS end_station_latitude,
    es.longitude AS end_station_longitude
  FROM trips t
    INNER JOIN stations ss ON t.start_station_id = ss.id
    INNER JOIN stations es ON t.end_station_id = es.id
);

