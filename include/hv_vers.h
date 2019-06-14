#define HYP_VERSION_MAJOR "1"
#define HYP_VERSION_MINOR "0"
#define HYP_VERSION_MICRO "2"
#define HYP_VERSION HYP_VERSION_MAJOR "." HYP_VERSION_MINOR "." HYP_VERSION_MICRO

#define HYP_RELEASE_YEAR "2019"
#define HYP_RELEASE_DATE "14-Jun-" HYP_RELEASE_YEAR

#define HYPVIEW_VERSION_MAJOR "1"
#define HYPVIEW_VERSION_MINOR "0"
#define HYPVIEW_VERSION_MICRO "2"
#define HYPVIEW_VERSION HYPVIEW_VERSION_MAJOR "." HYPVIEW_VERSION_MINOR "." HYPVIEW_VERSION_MICRO

/* UTF-8 of \u00a9 */
#define S_COPYRIGHT_SIGN "\xC2\xA9"

#define HYP_AUTHOR "Thorsten Otto"
#define HYP_COPYRIGHT "Copyright " S_COPYRIGHT_SIGN " 1991-" HYP_RELEASE_YEAR " by " HYP_AUTHOR
#define HYP_URL       "https://github.com/th-otto/hypview/"
#define HYP_HOMEPAGE  "https://www.tho-otto.de/"
#define HYP_EMAIL     "admin@tho-otto.de"
#undef PACKAGE_URL
#define PACKAGE_URL HYP_URL
