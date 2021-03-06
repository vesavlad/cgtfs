#include "database/database.h"

feed_db_status_t init_feed_db(feed_db_t *db, const char *db_path, int writable) {
    db->path = strdup(db_path);
    db->rc = -1;
    db->error_msg = NULL;
    db->open = 0;
    db->in_transaction = 0;

    db->rc = sqlite3_open_v2(db_path, &(db->conn),
        writable ? (SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE) : (SQLITE_OPEN_READONLY),
        NULL);

    if (db->rc) {
        db->error_msg = strdup(sqlite3_errmsg(db->conn));
        return FEED_DB_ERROR;
    }

    db->open = 1;
    return FEED_DB_SUCCESS;
}

feed_db_status_t free_feed_db(feed_db_t *db) {
    free(db->path);
    free(db->error_msg);

    db->rc = sqlite3_close(db->conn);
    if (db->rc != SQLITE_OK) {
        db->error_msg = strdup(sqlite3_errmsg(db->conn));
        return FEED_DB_ERROR;
    }

    db->open = 0;
    return FEED_DB_SUCCESS;
}

feed_db_status_t import_csv_file_db(const char *path, const char *table, feed_db_t *db) {
    FILE *fp = fopen(path, "r");

    if (!fp) {
        db->error_msg = strdup("Failed to open .csv file");
        return FEED_DB_ERROR;
    }

    char **record_values = NULL;
    int lines_count = count_lines(fp) - 1;
    int record_count = 0;

    if (lines_count < 0) {
        free(record_values);
        fclose(fp);

        db->error_msg = strdup("Failed to count lines in CSV file");
        return FEED_DB_ERROR;
    }

    char **field_names = NULL;
    int field_count = read_header(fp, &field_names);

    if (field_count < 0) {
        free_cstr_arr(field_names, field_count);
        free(record_values);
        fclose(fp);

        db->error_msg = strdup("Failed to read CSV file header");
        return FEED_DB_ERROR;
    }

    // begin_transaction_db(db);

    char *create_query;
    bake_create_uni_query_db(table, field_count, field_names, &create_query);

    char *error_msg;
    db->rc = sqlite3_exec(db->conn, create_query, NULL, NULL, &error_msg);

    free(create_query);
    free_cstr_arr(field_names, field_count);

    if (db->rc) {
        if (error_msg != NULL) {
            db->error_msg = strdup(error_msg);
            sqlite3_free(error_msg);
        } else {
            db->error_msg = strdup("Failed to create a table");
        }
        fclose(fp);
        free(record_values);
        return FEED_DB_ERROR;
    }

    char *insert_query;
    bake_insert_uni_query_db(table, field_count, &insert_query);

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db->conn, insert_query, -1, &stmt, NULL);

    for (int i = 0; i < lines_count; i++) {
        if (read_record(fp, field_count, &record_values) > 0) {
            for (int j = 0; j < field_count; j++)
                sqlite3_bind_text(stmt, j + 1, record_values[j], -1, SQLITE_STATIC);

            db->rc = sqlite3_step(stmt);
            if (db->rc != SQLITE_DONE) {
                fclose(fp);
                free(insert_query);
                free_cstr_arr(record_values, field_count);
                db->error_msg = strdup(sqlite3_errmsg(db->conn));
                sqlite3_finalize(stmt);
                return FEED_DB_ERROR;
            }

            sqlite3_clear_bindings(stmt);
            sqlite3_reset(stmt);
            record_count++;
        }
        free_cstr_arr(record_values, field_count);
    }

    sqlite3_finalize(stmt);

    // end_transaction_db(db);

    free(insert_query);
    fclose(fp);

    return FEED_DB_SUCCESS;
}

feed_db_status_t import_feed_db(const char *dir, feed_db_t *db) {
    char **filenames;
    int filecount = 0;

    char *full_file_name;
    char *table_file_name;

    filecount = list_txt_files(dir, &filenames);

    begin_transaction_db(db);

    for (int i = 0; i < filecount; i++) {
        make_filepath(&full_file_name, dir, filenames[i]);
        table_file_name = get_filename_no_ext(filenames[i], *FILENAME_SEPARATOR);

        if (FEED_DB_SUCCESS != import_csv_file_db(full_file_name, table_file_name, db)) {
            free(table_file_name);
            free(full_file_name);
            free_cstr_arr(filenames, filecount);
            return FEED_DB_ERROR;
        }
        free(table_file_name);
        free(full_file_name);
    }

    end_transaction_db(db);

    free_cstr_arr(filenames, filecount);
    return FEED_DB_SUCCESS;
}

feed_db_status_t store_feed_db(const char *dir, feed_db_t *db, feed_t *feed_counter) {

    feed_db_status_t result = FEED_DB_SUCCESS;

    char *agencies_fname;
    char *calendar_dates_fname;
    char *calendar_records_fname;
    char *fare_attributes_fname;
    char *fare_rules_fname;
    char *feed_info_fname;
    char *frequencies_fname;
    char *levels_fname;
    char *pathways_fname;
    char *routes_fname;
    char *shapes_fname;
    char *stop_times_fname;
    char *stops_fname;
    char *transfers_fname;
    char *trips_fname;

    make_filepath(&agencies_fname, dir, "agency.txt");
    make_filepath(&calendar_dates_fname, dir, "calendar_dates.txt");
    make_filepath(&calendar_records_fname, dir, "calendar.txt");
    make_filepath(&fare_attributes_fname, dir, "fare_attributes.txt");
    make_filepath(&fare_rules_fname, dir, "fare_rules.txt");
    make_filepath(&feed_info_fname, dir, "feed_info.txt");
    make_filepath(&frequencies_fname, dir, "frequencies.txt");
    make_filepath(&levels_fname, dir, "levels.txt");
    make_filepath(&pathways_fname, dir, "pathways.txt");
    make_filepath(&routes_fname, dir, "routes.txt");
    make_filepath(&shapes_fname, dir, "shapes.txt");
    make_filepath(&stop_times_fname, dir, "stop_times.txt");
    make_filepath(&stops_fname, dir, "stops.txt");
    make_filepath(&transfers_fname, dir, "transfers.txt");
    make_filepath(&trips_fname, dir, "trips.txt");

    feed_t instance;
    init_feed(&instance);

    #ifdef CGTFS_STORING_BATCH_TRANSACTIONS_OFF
    if ((res = begin_transaction_db(db)) == FEED_DB_ERROR) {
        free(agencies_fname);
        free(calendar_dates_fname);
        free(calendar_records_fname);
        free(fare_attributes_fname);
        free(fare_rules_fname);
        free(feed_info_fname);
        free(frequencies_fname);
        free(levels_fname);
        free(pathways_fname);
        free(routes_fname);
        free(shapes_fname);
        free(stop_times_fname);
        free(stops_fname);
        free(transfers_fname);
        free(trips_fname);

        return FEED_DB_ERROR;
    }
    #endif

    FILE *fp_agencies = fopen(agencies_fname, "r");
    if (fp_agencies) {
        instance.agency_count = store_all_agencies_db(fp_agencies, db);
        fclose(fp_agencies);
    } else {
        instance.agency_count = -1;
        result = FEED_DB_PARTIAL;
    }

    FILE *fp_calendar_dates = fopen(calendar_dates_fname, "r");
    if (fp_calendar_dates) {
        instance.calendar_dates_count = store_all_calendar_dates_db(fp_calendar_dates, db);
        fclose(fp_calendar_dates);
    } else {
        instance.calendar_dates_count = -1;
        result = FEED_DB_PARTIAL;
    }

    FILE *fp_calendar_records = fopen(calendar_records_fname, "r");
    if (fp_calendar_records) {
        instance.calendar_records_count = store_all_calendar_records_db(fp_calendar_records, db);
        fclose(fp_calendar_records);
    } else {
        instance.calendar_records_count = -1;
        result = FEED_DB_PARTIAL;
    }

    FILE *fp_fare_attributes = fopen(fare_attributes_fname, "r");
    if (fp_fare_attributes) {
        instance.fare_attributes_count = store_all_fare_attributes_db(fp_fare_attributes, db);
        fclose(fp_fare_attributes);
    } else {
        instance.fare_attributes_count = -1;
        result = FEED_DB_PARTIAL;
    }

    FILE *fp_fare_rules = fopen(fare_rules_fname, "r");
    if (fp_fare_rules) {
        instance.fare_rules_count = store_all_fare_rules_db(fp_fare_rules, db);
        fclose(fp_fare_rules);
    } else {
        instance.fare_rules_count = -1;
        result = FEED_DB_PARTIAL;
    }

    FILE *fp_feed_info = fopen(feed_info_fname, "r");
    if (fp_feed_info) {
        instance.feed_info_count = store_all_feed_info_db(fp_feed_info, db);
        fclose(fp_feed_info);
    } else {
        instance.feed_info_count = -1;
        result = FEED_DB_PARTIAL;
    }

    FILE *fp_frequencies = fopen(frequencies_fname, "r");
    if (fp_frequencies) {
        instance.frequencies_count = store_all_frequencies_db(fp_frequencies, db);
        fclose(fp_frequencies);
    } else {
        instance.frequencies_count = -1;
        result = FEED_DB_PARTIAL;
    }

    FILE *fp_levels = fopen(levels_fname, "r");
    if (fp_levels) {
        instance.levels_count = store_all_levels_db(fp_levels, db);
        fclose(fp_levels);
    } else {
        instance.levels_count = -1;
        result = FEED_DB_PARTIAL;
    }

    FILE *fp_pathways = fopen(pathways_fname, "r");
    if (fp_pathways) {
        instance.pathways_count = store_all_pathways_db(fp_pathways, db);
        fclose(fp_pathways);
    } else {
        instance.pathways_count = -1;
        result = FEED_DB_PARTIAL;
    }

    FILE *fp_routes = fopen(routes_fname, "r");
    if (fp_routes) {
        instance.routes_count = store_all_routes_db(fp_routes, db);
        fclose(fp_routes);
    } else {
        instance.routes_count = -1;
        result = FEED_DB_PARTIAL;
    }

    FILE *fp_shapes = fopen(shapes_fname, "r");
    if (fp_shapes) {
        instance.shapes_count = store_all_shapes_db(fp_shapes, db);
        fclose(fp_shapes);
    } else {
        instance.shapes_count = -1;
        result = FEED_DB_PARTIAL;
    }

    FILE *fp_stop_times = fopen(stop_times_fname, "r");
    if (fp_stop_times) {
        instance.stop_times_count = store_all_stop_times_db(fp_stop_times, db);
        fclose(fp_stop_times);
    } else {
        instance.stop_times_count = -1;
        result = FEED_DB_PARTIAL;
    }

    FILE *fp_stops = fopen(stops_fname, "r");
    if (fp_stops) {
        instance.stops_count = store_all_stops_db(fp_stops, db);
        fclose(fp_stops);
    } else {
        instance.stops_count = -1;
        result = FEED_DB_PARTIAL;
    }

    FILE *fp_transfers = fopen(transfers_fname, "r");
    if (fp_transfers) {
        instance.transfers_count = store_all_transfers_db(fp_transfers, db);
        fclose(fp_transfers);
    } else {
        instance.transfers_count = -1;
        result = FEED_DB_PARTIAL;
    }

    FILE *fp_trips = fopen(trips_fname, "r");
    if (fp_trips) {
        instance.trips_count = store_all_trips_db(fp_trips, db);
        fclose(fp_trips);
    } else {
        instance.trips_count = -1;
        result = FEED_DB_PARTIAL;
    }

    #ifdef CGTFS_STORING_BATCH_TRANSACTIONS_OFF
    end_transaction_db(db);
    #endif


    if (feed_counter != NULL)
        *feed_counter = instance;


    free(agencies_fname);
    free(calendar_dates_fname);
    free(calendar_records_fname);
    free(fare_attributes_fname);
    free(fare_rules_fname);
    free(feed_info_fname);
    free(frequencies_fname);
    free(levels_fname);
    free(pathways_fname);
    free(routes_fname);
    free(shapes_fname);
    free(stop_times_fname);
    free(stops_fname);
    free(transfers_fname);
    free(trips_fname);

    return result;
}

void fetch_feed_db(feed_db_t *db, feed_t *feed) {
    feed->agency_count = fetch_all_agencies_db(db, &(feed->agencies));
    feed->calendar_dates_count = fetch_all_calendar_dates_db(db, &(feed->calendar_dates));
    feed->calendar_records_count = fetch_all_calendar_records_db(db, &(feed->calendar_records));
    feed->fare_attributes_count = fetch_all_fare_attributes_db(db, &(feed->fare_attributes));
    feed->fare_rules_count = fetch_all_fare_rules_db(db, &(feed->fare_rules));
    feed->feed_info_count = fetch_all_feed_info_db(db, &(feed->feed_info));
    feed->frequencies_count = fetch_all_frequencies_db(db, &(feed->frequencies));
    feed->levels_count = fetch_all_levels_db(db, &(feed->levels));
    feed->pathways_count = fetch_all_pathways_db(db, &(feed->pathways));
    feed->routes_count = fetch_all_routes_db(db, &(feed->routes));
    feed->shapes_count = fetch_all_shapes_db(db, &(feed->shapes));
    feed->stop_times_count = fetch_all_stop_times_db(db, &(feed->stop_times));
    feed->stops_count = fetch_all_stops_db(db, &(feed->stops));
    feed->transfers_count = fetch_all_transfers_db(db, &(feed->transfers));
    feed->trips_count = fetch_all_trips_db(db, &(feed->trips));
}

feed_db_status_t setup_feed_db(feed_db_t *db) {
    char sql[] = ""
        "CREATE TABLE agency ( "
        "	agency_id TEXT, "
        "	agency_name TEXT PRIMARY KEY NOT NULL, "
        "	agency_url TEXT NOT NULL, "
        "	agency_timezone TEXT NOT NULL, "
        "	agency_lang TEXT, "
        "	agency_phone TEXT, "
        "	agency_fare_url TEXT, "
        "	agency_email TEXT "
        ");\n"
        "CREATE TABLE stops ( "
        "	stop_id TEXT PRIMARY KEY NOT NULL, "
        "	stop_code TEXT, "
        "	stop_name TEXT NOT NULL, "
        "	stop_desc TEXT, "
        "	stop_lat DOUBLE NOT NULL, "
        "	stop_lon DOUBLE NOT NULL, "
        "	zone_id TEXT, "
        "	stop_url TEXT, "
        "	location_type INT, "
        "	parent_station TEXT, "
        "	stop_timezone TEXT, "
        "	wheelchair_boarding INT, "
        "   level_id TEXT, "
        "   platform_code TEXT "
        ");\n"
        "CREATE TABLE routes ( "
        "	route_id TEXT PRIMARY KEY NOT NULL, "
        "	agency_id TEXT, "
        "	route_short_name TEXT, "
        "	route_long_name TEXT, "
        "	route_desc TEXT, "
        "	route_type INT NOT NULL, "
        "	route_url TEXT, "
        "	route_color TEXT, "
        "	route_text_color TEXT, "
        "	route_sort_order INT "
        ");\n"
        "CREATE TABLE trips ( "
        "	route_id TEXT NOT NULL, "
        "	service_id TEXT NOT NULL, "
        "	trip_id TEXT PRIMARY KEY NOT NULL, "
        "	trip_headsign TEXT, "
        "	trip_short_name TEXT, "
        "	direction_id INT, "
        "	block_id TEXT, "
        "	shape_id TEXT, "
        "	wheelchair_accessible INT, "
        "	bikes_allowed INT "
        ");\n"
        "CREATE TABLE stop_times ( "
        "	trip_id TEXT NOT NULL, "
        "	arrival_time TEXT NOT NULL, "
        "	departure_time TEXT NOT NULL, "
        "	stop_id TEXT NOT NULL, "
        "	stop_sequence INT NOT NULL, "
        "	stop_headsign TEXT, "
        "	pickup_type INT, "
        "	drop_off_type INT, "
        "	shape_dist_traveled REAL, "
        "	timepoint INT "
        ");\n"
        "CREATE TABLE calendar ( "
        "	service_id TEXT PRIMARY KEY NOT NULL, "
        "	monday INT NOT NULL, "
        "	tuesday INT NOT NULL, "
        "	wednesday INT NOT NULL, "
        "	thursday INT NOT NULL, "
        "	friday INT NOT NULL, "
        "	saturday INT NOT NULL, "
        "	sunday INT NOT NULL, "
        "	start_date TEXT NOT NULL, "
        "	end_date TEXT NOT NULL "
        ");\n"
        "CREATE TABLE calendar_dates ( "
        "	service_id TEXT NOT NULL, "
        "	date TEXT NOT NULL, "
        "	exception_type INT NOT NULL "
        ");\n"
        "CREATE TABLE fare_attributes ( "
        "	fare_id TEXT NOT NULL, "
        "	price REAL NOT NULL, "
        "	currency_type TEXT NOT NULL, "
        "	payment_method INT NOT NULL, "
        "	transfers INT NOT NULL, "
        "	agency_id TEXT, "
        "	transfer_duration REAL "
        ");\n"
        "CREATE TABLE fare_rules ( "
        "	fare_id TEXT NOT NULL, "
        "	route_id TEXT, "
        "	origin_id TEXT, "
        "	destination_id TEXT, "
        "	contains_id TEXT "
        ");\n"
        "CREATE TABLE shapes ( "
        "	shape_id TEXT NOT NULL, "
        "	shape_pt_lat DOUBLE NOT NULL, "
        "	shape_pt_lon DOUBLE NOT NULL, "
        "	shape_pt_sequence INT NOT NULL, "
        "	shape_dist_traveled REAL "
        ");\n"
        "CREATE TABLE frequencies ( "
        "	trip_id TEXT NOT NULL, "
        "	start_time TEXT NOT NULL, "
        "	end_time TEXT NOT NULL, "
        "	headway_secs INT NOT NULL, "
        "	exact_times INT "
        ");\n"
        "CREATE TABLE transfers ( "
        "	from_stop_id TEXT NOT NULL, "
        "	to_stop_id TEXT NOT NULL, "
        "	transfer_type INT NOT NULL, "
        "	min_transfer_time INT "
        ");\n"
        "CREATE TABLE feed_info ( "
        "	feed_publisher_name TEXT NOT NULL, "
        "	feed_publisher_url TEXT NOT NULL, "
        "	feed_lang TEXT NOT NULL, "
        "	feed_start_date TEXT, "
        "	feed_end_date TEXT, "
        "	feed_version TEXT, "
        "	feed_contact_email TEXT, "
        "	feed_contact_url TEXT "
        ");\n"
        "CREATE TABLE levels ( "
        "	level_id TEXT NOT NULL, "
        "	level_index DOUBLE NOT NULL, "
        "	level_name TEXT "
        ");\n"
        "CREATE TABLE pathways ( "
        "	pathway_id TEXT NOT NULL, "
        "	from_stop_id TEXT NOT NULL, "
        "	to_stop_id TEXT NOT NULL, "
        "	pathway_mode INT NOT NULL, "
        "	is_bidirectional INT NOT NULL, "
        "	length DOUBLE, "
        "	traversal_time INT, "
        "	stair_count INT, "
        "	max_slope DOUBLE, "
        "	min_width DOUBLE, "
        "	signposted_as TEXT, "
        "	reversed_signposted_as TEXT "
        "); ";

    char *error_msg;
    db->rc = sqlite3_exec(db->conn, sql, NULL, NULL, &error_msg);

    if (db->rc) {
        if (error_msg != NULL) {
            db->error_msg = strdup(error_msg);
            sqlite3_free(error_msg);
        }
        return FEED_DB_ERROR;
    }

    return FEED_DB_SUCCESS;
}