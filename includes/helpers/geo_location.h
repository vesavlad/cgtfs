/**
 * @file
 * @brief    Geographical location utilities.
 */

#ifndef CGTFS_GEO_LOCATION_H
#define CGTFS_GEO_LOCATION_H

/**
 * A semantic presentation of a geographical location.
 *
 * @ingroup    Helpers
 */
typedef struct {
    long double latitude;
    long double longitude;
} geo_location_t;

#endif
