#ifndef CGTFS_TESTS_RECORDS_CALENDAR_DATES_C
#define CGTFS_TESTS_RECORDS_CALENDAR_DATES_C

#include "greatest/greatest.h"
#include "records/calendar_dates.h"

TEST calendar_date_compare(void) {
    calendar_date_t a = {
        .service_id = "SOME_ID1",
        .date = "01012019",
        .exception_type = ET_ADDED
    };

    calendar_date_t b = {
        .service_id = "SOME_ID1",
        .date = "01012019",
        .exception_type = ET_ADDED
    };

    calendar_date_t c = {
        .service_id = "SOME_ID2",
        .date = "01012019",
        .exception_type = ET_REMOVED
    };

    ASSERT_EQ(0, equal_calendar_date(&a, &b));
    ASSERT_EQ(1, equal_calendar_date(&a, &c));
    ASSERT_EQ(1, equal_calendar_date(&b, &c));

    PASS();
}

SUITE(CGTFS_RecordCalendarDate) {
    RUN_TEST(calendar_date_compare);
}

#endif