/**
 * @file
 * @brief    Feed info entity handling functionality.
 */

#ifndef CGTFS_FEED_INFO_H
#define CGTFS_FEED_INFO_H

#include <stdio.h>
#include <string.h>

#include "xstrlengths.h"


/**
 * feed_info.txt record
 *
 * @see        init_feed_info()
 * @see        read_feed_info()
 * @see        https://developers.google.com/transit/gtfs/reference/#feed_infotxt
 *
 * @ingroup    Core__EntityTypes Core__EntityList__FeedInfo
 */
typedef struct {
    char feed_publisher_name[CGTFS_SL_NAM];   ///< [Required] Full name of the organization publishing the feed.
    char feed_publisher_url[CGTFS_SL_URL];    ///< [Required] URL of the publishing organization's website.
    char feed_lang[CGTFS_SL_LNG];             ///< [Required] Language of the feed (IETF BCP 47).
    char feed_start_date[CGTFS_SL_DAT];       ///< [Optional] The first date (YYYYMMDD) when the feed's info becomes valid/active.
    char feed_end_date[CGTFS_SL_DAT];         ///< [Optional] The last date (YYYYMMDD) when the feed's info is valid/active.
    char feed_version[CGTFS_SL_NAM];          ///< [Optional] String identifying the feed version.
    char feed_contact_email[CGTFS_SL_EML];    ///< [Optional] Email address to contact regarding the GTFS dataset.
    char feed_contact_url[CGTFS_SL_URL];      ///< [Optional] URL address to visit regatding the GTFS dataset.
} feed_info_t;


/**
 * Initializes the given feed info record with empty/default values.
 *
 * @param[out]    record    Feed info record pointer to initialize.
 *
 * @ingroup       Core__EntityFunctions Core__EntityList__FeedInfo
 */
void init_feed_info(feed_info_t *record);

/**
 * Reads given datafields and field names into the given feed info struct.
 *
 * @param[out]    record          The pointer to write into.
 * @param[in]     field_count     Number of rows (columns) the record has.
 * @param[in]     field_names     Names of the fields.
 * @param[in]     field_values    Contents of the record.
 *
 * @ingroup       Core__EntityFunctions Core__EntityList__FeedInfo
 */
void read_feed_info(feed_info_t *record, int field_count, const char **field_names, const char **field_values);

/**
 * Compares two structures.
 *
 * @param[in]    a    First structure
 * @param[in]    b    Second structure
 *
 * @returns      0 if the structures differ and 1 otherwise
 *
 * @ingroup      Core__EntityFunctions Core__EntityList__FeedInfo
 */
int equal_feed_info(const feed_info_t *a, const feed_info_t *b);

#endif