#ifndef CGTFS_TESTS_DATABASE_RECORDS_AGENCY_C
#define CGTFS_TESTS_DATABASE_RECORDS_AGENCY_C

#include "greatest/greatest.h"
#include "database/records/agency.h"

TEST db_agency_store(void) {
    feed_db_t db;
    feed_db_status_t res;

    agency_t record = {
        .id = "SOME_ID1",
        .name = "amazing name",
        .url = "",
        .timezone = "dfghjkl244124 qwe",
        .lang = "fi",
        .phone = "000111222",
        .fare_url = "https://example.com",
        .email = "no-reply@example.com"
    };

    ASSERT_EQ_FMTm(db.error_msg, FEED_DB_SUCCESS, init_feed_db(&db, "tests_tdw0.db", 1), "%i");
    ASSERT_EQ_FMTm(db.error_msg, FEED_DB_SUCCESS, setup_feed_db(&db), "%i");

    res = store_agency_db(&record, &db);
    ASSERT_EQ_FMTm(db.error_msg, FEED_DB_SUCCESS, res, "%i");

    ASSERT_EQ_FMTm(db.error_msg, FEED_DB_SUCCESS, free_feed_db(&db), "%i");

    PASS();
}

SUITE(CGTFS_DatabaseAgency) {
    RUN_TEST(db_agency_store);
}


#endif