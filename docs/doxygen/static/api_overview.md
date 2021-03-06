# API (brief) overview {#api_overview}

If you are looking for a very brief introduction, @ref data_transition "data transition guide" might be a better starting point. However, having gone through this file would prove highly benefitial when using the library.

@section ApiOverview__Introduction Basics

The library tries to provide a semantic, consistent and readable interface. The following diagrams show the ways GTFS data can be parsed or transferred between different storage types using this library.

_Note: **bold arrows** indicate that such a function exists in the API which makes it possible to transfer the entire contents of a feed between two types of storage with a single function call (not counting the setup and teardown functions)._

<div style="text-align: center;">
<div style="display: inline-block; width: 100%; max-width: 500px; padding-right: 5%;">
@image html df_straight.svg "Ways to parse/tranport data with CGTFS (simple diagram)" width=100%
</div>
<div style="display: inline-block; width: 100%; max-width: 400px;">
@image html df_scheme.svg "Ways to parse/tranport data with CGTFS (scheme)" width=100%
</div>
</div>

@image latex df_straight.eps "Ways to parse/tranport data with CGTFS (simple diagram)" width=12cm

@image latex df_scheme.eps "Ways to parse/tranport data with CGTFS (scheme)" width=12cm


@subsection ApiOverview__Introduction__Terms Terms

The terms used throughout the library code and documentation differ from those defined by the [GTFS reference](https://developers.google.com/transit/gtfs/reference/#term-definitions). The following table illustrates the relation between differing CGTFS terms and reference terms, and their definitions, as well as terms used in the library and abscent from the reference.

| CGTFS term | Reference term | Meaning and notes |
| ---------- | -------------- | ----------------- |
| Feed (entity/instance/object) | *not defined* | A data structure holding the entirety of a feed's data. *In the code and documentation, may be referred to as a feed entity, feed instance or feed object.* |
| Directory | Dataset | A set of files which constitutes a GTFS feed. In some contexts, in the code documentation, terms *feed* and *directory* are interchangable. *Note: GTFS datasets are distributed in form of `*.zip` feed archives. This library, however, only works with unpacked feeds.* |
| Entity (instance) | Record | A complete data structure containing information about a concrete GTFS entity (e.g. information about one route). The library uses the term *entity* to avoid ambiguity with database operations. _Note: however, *entity* is a more abstract term, thus a struct holding one entity's data is, in essense, an entity instance. This documentation may refer to structs simply as **entities** for shortness_. |
| File | *not defined* | A `*.txt` file, a part of the feed, holding information about all the feed's *entities* of a single type. |
| Database | n/a | A single *SQLite* database file, created using the supplied SQL schema (preferably, the creation of the database is left to the library, see the database section). |

@section ApiOverview__Principles Principles

There are several core principles which could help in understanding the vast interface of the library. Some of them have been enforced from the beginning, others may be gradually integrated into the API.

  - All structs and enumerations have names ending with `_t`.
  - All enumerations have members with names reflecting the enumeration's name.
    - Entity field enumerations start with the first letters of the enumeration's name, e.g. `payment_method_t` has elements `PM_ON_BOARD`, `PM_BEFOREHAND` and `PM_NOT_SET`.
      - There are exceptions at naming conflicts, e.g. `pathway_mode_t`, which would have to start its members' names with `PM` but uses `PTMD` instead.
      - Additionally, all entity field enumerations have `..._NOT_SET` members.
  - All structs have `init_...()` functions for initializing them. These functions MUST be called before the first use of the structure.
    - Feed entity and record entity structs also have `read_...()` and `equal_...()` functions.
    - Structs which need deallocation after use have `free_...()` functions.
  - All functions which have to do something with database operations have `_db` postfix.

@section ApiOverview__Strings String storage

String values in CGTFS are stored in memory using statically allocated c-strings. Hence, parsing an unsually long string value may lead to a fatal crash. Default string field lengths are rather sensible but might by too big (bloating the RAM used significantly) or too small (causing crash).

To mitigate that, all string field length definitions are located in the `xstrlengths.h` header. Actual field length definitions are heavily commented in the lower part of the file. By default, they are using `CGTFS_SL_BASE_` definitions found in the upper part of the file. There are three possible usage cases:

  1. All left as it is in hopes for lucky circumstances.
  2. Definition `CGTFS_SL_MODE_PREPARATION` is uncommented, reserving an obstinate amount of memory for all fields.
  3. Maximum length of each field type is deduced from the supposed data sources (useful if you're working with the data form a specific agency). This is left to the developer.

@section ApiOverview__Structure Structure

The library's API is divided into two so called layers, additional auxiliary functionality and loosely related helpers:

  - @ref Core "Core layer" provides basic definitions and functions for handling GTFS feeds and entities, and includes:
    - @ref Core__FeedEntity "feed object definition" to store data of an entire feed and functions for working with it:
      - a function to initialize a feed object;
      - a function to parse a feed object from a given directory path;
      - a function to determine whether two feed objects are equal;
    - field enumerations to represent types and values of the fields which can only take values from a limited set defined by the specification, e.g. [`routes.txt/route type`](https://developers.google.com/transit/gtfs/reference/#routestxt);
      - functions to parse field enumeration values from a char array;
    - @ref Core__EntityTypes "entity definitions" to represent e.g. an agency, a stop, a shape, etc. and @ref Core__EntityList "functions" for handling them;
      - functions to initialize entity instances;
      - functions to parse entity instances from a char array of field names and a char array of field values;
      - functions to determine whether two entity instances are equal;
    - @ref Core__EntityFileReading "batch entity parsing functions" which parse an array of entities from a given `*.txt` file path;
  - @ref Database "Database layer" provides definitions and functions for working with entities defined in the *core layer* with/through/in a SQLite database instance, and includes:
    - @ref feed_db_t "definition of a connection to a sqlite database" and @ref Database__FeedEntity "functions" for working with it:
      - a function to initialize a database connection;
      - a function to free/close a database connection;
      - a function to setup a database at an opened connection for a GTFS feed;
    - storage transition functions:
      - a @ref import_feed_db "function" to <strong>semantically</strong> store the contents of a feed from a specified directory into a specified database connection (see note below);
      - a @ref store_feed_db "function" to <strong>non-semantically</strong> store the contents of a feed from a specified directory into a specified database connection (see note below);
      - a @ref fetch_feed_db "function" to fetch the contents of a feed from a specified database connection into a specicfied feed object;
    - an @ref feed_db_status_t "enumeration" of general database operation results (success / failure / so-so);
    - @ref Database__EntityStoring "functions" to store entities using a specified database connection;
    - the so-called table operations:
      - @ref Database__TableStoring "batch entity storing functions" which parse an array of entities from a given `*.txt` file path into a database table (doing so directly, without keeping an intermediate array in the memory);
      - @ref Database__TableFetching "batch entity fetching functions" which retrieve an array of entities of a single type from a specified database connection;
  - @ref Utilities "Utilities" include:
    - @ref Utilities__File "functions" for reading CSV files;
    - an assisting @ref Utilities__Memory "function" for clearing a c-string array;
    - utilitary @ref Utils__Database "functions" for working a with sqlite database;
  - @ref Helpers "Helpers" include:
    - several preprocessor definitions used across the library;
    - a @ref get_filename_no_ext "function" for extracting filename without extension from a given path;
    - a @ref make_filepath "function" for making a filepath from a directory and a file in it;
    - a @ref dg_to_rad "function" for converting degrees into radians;
    - a @ref geo_location_t "geolocation definition" which holds a latitude value and a longitude value;
      - a @ref haversine_distance "function" for calculating a distance (in meters) between the two geolocation points.

_Note: CGTFS provides two ways of parsing a directory into a database, semantic and non-semantic. Semantic stores all values according to the specification, creating a reference-defined database layout and filling it with data of according types. Non-semantic directly translates GTFS *.txt files as CSVs into the database, creating a layout based on file headers and storing all data as text. See more in the related documentation page._

_Another note: up to release `1.0.0`, the library's API is a subject to a change without backwards-compatibility concerns._

A more detailed documentation for each layer, definition and function can be found in the module documentation, linked throughout the list.


@section ApiOverview__Examples Examples

Some example source code is located in the `examples/` folder of the library's source code. Digging into the `tests/` folder might as well be useful.

@subsection ApiOverview__Examples__0 Reading a feed into memory

Usage shown in this example is not recommended for parsing a real feed, as it **will** take **a lot of** memory for any substantially big amounts of data.

@include example_0.c

@subsection ApiOverview__Examples__1 An entity file reading

Read all bus stops and print out their information

@include example_1.c

@subsection ApiOverview__Examples__2 Database-backed querying

Store a gtfs folder as a database and query it for first 10 stop time records with arrival time within the next 10 minutes.

@include example_2.c
