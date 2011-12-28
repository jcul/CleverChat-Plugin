#define PURPLE_PLUGINS
#define PLUGIN_ID "Clever Chat"

#include <glib.h>

#include "libpurple/notify.h"
#include "libpurple/plugin.h"
#include "libpurple/version.h"
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include <openssl/md5.h>
#include <unistd.h>
#include <debug.h>

GList *list;

typedef struct _post_data {
    char *name;
    char *stimulus;
    char *sessionid;
    char *vText8;
    char *vText7;
    char *vText6;
    char *vText5;
    char *vText4;
    char *vText3;
    char *vText2;
    char *icognocheck;
    char *prevref;
    CURL *curl;
    gboolean sending;
} post_data;

void my_debug(char *s);
void start_cleverbot_cb(PurplePluginAction *action, PurpleBuddy *buddy);
void stop_cleverbot_cb(PurplePluginAction *action, PurpleBuddy *buddy);
char* format_str( post_data* post );
void init_post_data( post_data* my_post );
size_t process_result( void *buffer, size_t size, size_t nmemb, post_data *my_post );
void cr2lf( char *string );
void rep_all_char( char *string, char c, char d );
gboolean check_if_enabled( char *name );
post_data* find_post_data( char *name );
char *send_clever_message( post_data *post_ptr );
void destroy_post_data( post_data* post );
char *gen_md5( char* input );
