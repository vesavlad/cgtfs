#include "trip.h"

wheelchair_accessible_t parse_wheelchair_accessibility(const char *value) {
    if (strcmp(value, "0") == 0 || strcmp(value, "") == 0)
        return WA_UNKNOWN;
    else if (strcmp(value, "1") == 0)
        return WA_POSSIBLE;
    else if (strcmp(value, "2") == 0)
        return WA_NOT_POSSIBLE;
    else
        return WA_NOT_SET;
}

bikes_allowed_t parse_bike_allowance(const char *value) {
    if (strcmp(value, "0") == 0 || strcmp(value, "") == 0)
        return BA_UNKNOWN;
    else if (strcmp(value, "1") == 0)
        return BA_POSSIBLE;
    else if (strcmp(value, "2") == 0)
        return BA_NOT_POSSIBLE;
    else
        return BA_NOT_SET;
}

void init_trip(trip_t *record) {
    strcpy(record->route_id, "");
    strcpy(record->service_id, "");
    strcpy(record->trip_id, "");
    strcpy(record->headsign, "");
    strcpy(record->short_name, "");
    record->direction_id = 0;
    strcpy(record->block_id, "");
    strcpy(record->shape_id, "");
    record->wheelchair_accessible = WA_UNKNOWN;
    record->bikes_allowed = BA_UNKNOWN;
    record->is_null = 1;
}

void read_trip(trip_t *record, int field_count, const char **field_names, const char **field_values) {
    init_trip(record);
    int assignment_counter = 0;

    for (int i = 0; i < field_count; i++) {
        if (strcmp(field_names[i], "route_id") == 0) {
            strcpy(record->route_id, field_values[i]);
            assignment_counter++;
            continue;
        }
        if (strcmp(field_names[i], "service_id") == 0) {
            strcpy(record->service_id, field_values[i]);
            assignment_counter++;
            continue;
        }
        if (strcmp(field_names[i], "trip_id") == 0) {
            strcpy(record->trip_id, field_values[i]);
            assignment_counter++;
            continue;
        }
        if (strcmp(field_names[i], "trip_headsign") == 0) {
            strcpy(record->headsign, field_values[i]);
            // assignment_counter++;
            continue;
        }
        if (strcmp(field_names[i], "trip_short_name") == 0) {
            strcpy(record->short_name, field_values[i]);
            // assignment_counter++;
            continue;
        }
        if (strcmp(field_names[i], "direction_id") == 0) {
            record->direction_id = (unsigned int)strtoul(field_values[i], NULL, 0);
            // assignment_counter++;
            continue;
        }
        if (strcmp(field_names[i], "block_id") == 0) {
            strcpy(record->block_id, field_values[i]);
            // assignment_counter++;
            continue;
        }
        if (strcmp(field_names[i], "shape_id") == 0) {
            strcpy(record->shape_id, field_values[i]);
            // assignment_counter++;
            continue;
        }
        if (strcmp(field_names[i], "wheelchair_accessible") == 0) {
            record->wheelchair_accessible = parse_wheelchair_accessibility(field_values[i]);
            // assignment_counter++;
            continue;
        }
        if (strcmp(field_names[i], "bikes_allowed") == 0) {
            record->bikes_allowed = parse_bike_allowance(field_values[i]);
            // assignment_counter++;
            continue;
        }
    }

    if (assignment_counter == 0)
        record->is_null = 1;
    else
        record->is_null = 0;
}