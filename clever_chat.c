//Copyright (C) 2011 Jack Culhane - jackculhane@gmail.com
//
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "clever_chat.h"

//This function replaces any 0Ds with 0As in a string
void cr2lf( char *string ){
    
    int i;
    int j = strlen(string);
    char c = 0x0D, d;

    for( i = 0; i < j; i++ ){
        
        d = *(string + i);

        if (c == d) {
            *(string + i) = 0x0A;
        }
    }
}


//This function formats the post data to send to cleverbot
char* format_str( post_data* post ){
    
    char* str1;
    char* md5;
    char* res; 

    str1 =  g_strconcat(    "stimulus=", (*post).stimulus,              //Message from contact to be send to cleverbot
                            "&start=y&sessionid=", (*post).sessionid,   //Session ID we received from Cleverbot
                            "&vText2=", (*post).vText2,                 //vText2-8 are the previous 7 responses from / to cleverbot
                            "&vText3=", (*post).vText3,
                            "&vText4=", (*post).vText4,
                            "&vText5=", (*post).vText5,
                            "&vText6=", (*post).vText6,
                            "&vText7=", (*post).vText7,
                            "&vText8=", (*post).vText8,
                            "&icognoid=wsf&icognocheck=",               //icongnocheck is the md5 of chars 9 to 20 of the string
    NULL);

    //Get that md5sum we were talking about...
    md5 = gen_md5( strndup( str1 + 9, 26 ) );

    res = g_strconcat( str1, md5, 
                        "&fno=0&prevref=", (*post).prevref,             //prevref is from the last response from Cleverbot
                        "&lineref=&sub=Say&islearning=1&cleanslate=false",  //This is always the same
    NULL);
   
    //Free the first chunk and the md5 as not needed anymore
    g_free(str1);
    g_free(md5);

    return res;

}


//Takes a string and returns the md5sum
char *gen_md5(char* input) {

    int n, len;
    char *temp_ptr, *res_ptr;
    
    temp_ptr = malloc( (sizeof(char) * 32) + 1 );
    res_ptr = temp_ptr; 

    MD5_CTX c;
    unsigned char out[MD5_DIGEST_LENGTH];

    MD5_Init(&c);

    MD5_Update(&c, input, strlen(input));
    MD5_Final(out, &c);

    g_free(input);
   
    for(n=0; n< MD5_DIGEST_LENGTH; n++){
        
        len = snprintf( temp_ptr, 3, "%02x", out[n] );
        
        if ((len == -1) || (len > 2)) {
            return NULL;
        }

        temp_ptr += 2;
    }

    return res_ptr;

}


//Iterate through global list and check if name has a post_data structure (enabled or not)
gboolean check_if_enabled( char *name ) {

    post_data *curr = NULL;
    GList *l;

    for( l = list; l; l = l->next ) {

        curr = ( post_data * ) l->data;

        if ( !g_strcmp0( curr->name, name ) ) {
            return TRUE;
        }
    }
    return FALSE;
}


//Same as check_if_enabled, but returns the actual post_data structure pointer (if found)
post_data* find_post_data( char *name ) {

    post_data *curr = NULL;
    GList *l;

    for( l = list; l; l = l->next ) {

        curr = ( post_data * ) l->data;

        if ( !g_strcmp0( curr->name, name ) ) {
            return curr;
        }
    }
    return NULL;
}


//Set our strings to "" and set some stuff for our curl handle
void init_post_data ( post_data* post) {

    post->curl = curl_easy_init();
    
    curl_easy_setopt(post->curl, CURLOPT_WRITEFUNCTION, process_result); 
    curl_easy_setopt(post->curl, CURLOPT_WRITEDATA, post); 
    curl_easy_setopt(post->curl, CURLOPT_POST, 1);
    curl_easy_setopt(post->curl, CURLOPT_URL, "http://cleverbot.com/webservicemin");
    curl_easy_setopt(post->curl, CURLOPT_COOKIEFILE, "/File_Does_Not_Exist");

    post->stimulus = g_strdup("");
    post->sessionid = g_strdup("");
    post->vText8 = g_strdup("");
    post->vText7 = g_strdup("");
    post->vText6 = g_strdup("");
    post->vText5 = g_strdup("");
    post->vText4 = g_strdup("");
    post->vText3 = g_strdup("");
    post->vText2 = g_strdup("");
    post->icognocheck = g_strdup("");
    post->prevref = g_strdup("");

    post->sending = FALSE;

}


//This function frees the strings in our post_data structure
void free_post_data ( post_data *post ) {

    if ( post->curl ) curl_free( post->curl );
    if ( post->name) g_free( post->name );

    if ( post->sessionid ) g_free( post->sessionid );
    if ( post->vText8 ) g_free( post->vText8 );
    if ( post->vText7 ) g_free( post->vText7 );
    if ( post->vText6 ) g_free( post->vText6 );
    if ( post->vText5 ) g_free( post->vText5 );
    if ( post->vText4 ) g_free( post->vText4 );
    if ( post->vText3 ) g_free( post->vText3 );
    if ( post->vText2 ) g_free( post->vText2 );
    if ( post->icognocheck ) g_free( post->icognocheck );
    if ( post->prevref ) g_free( post->prevref );
    
    g_free(post);
}


//Same as free_post_data, except this also removes the link from our GList
void destroy_post_data( post_data* post ) {
    
    GList *tempLink;

    if ( post->curl ) curl_free( post->curl );
    if ( post->name) g_free( post->name );

    if ( post->sessionid ) g_free( post->sessionid );
    if ( post->vText8 ) g_free( post->vText8 );
    if ( post->vText7 ) g_free( post->vText7 );
    if ( post->vText6 ) g_free( post->vText6 );
    if ( post->vText5 ) g_free( post->vText5 );
    if ( post->vText4 ) g_free( post->vText4 );
    if ( post->vText3 ) g_free( post->vText3 );
    if ( post->vText2 ) g_free( post->vText2 );
    if ( post->icognocheck ) g_free( post->icognocheck );
    if ( post->prevref ) g_free( post->prevref );

    tempLink = g_list_find (list, post);
    g_free( post );
    
   // list = g_list_remove( list, &post );
    list = g_list_delete_link( list , tempLink ); 

}


//Sends message to cleverbot
char *send_clever_message( post_data *post_ptr ) {

    char *send_string, *response;
    int error;
    
    //format the post data correctly (I hope) for curl
    send_string = format_str( post_ptr );

    curl_easy_setopt(post_ptr->curl, CURLOPT_POSTFIELDS, send_string);
    
    purple_debug_info(PLUGIN_ID, "Postdata:\n%s\n", send_string);

    error = curl_easy_perform(post_ptr->curl);

    if (send_string)
        free(send_string);
    
    //Make sure we were able to process some data
    if (error < 1) {
        response = curl_easy_unescape( post_ptr->curl , post_ptr->vText2, 0, NULL);
        return response;
    } else {
        purple_debug_info(PLUGIN_ID, "Error receiving message from cleverbot!\n");
        return NULL;
    }
}


//This is our callback for CURL, it processes the data returned by cleverbot.
//The data is a 0x0D delimited set of data 
size_t process_result( void *buffer, size_t size, size_t nmemb, post_data *my_post ){

    char *response, *rec_session, *rec_ref; 
    gchar** rec_array;
    
    //Duplicate the curl buffer so strtok can modify it
    //temp = g_strdup(buffer);

    if ( strstr( buffer, "DENIED" ) != NULL ) {
        return -1;
    }
    
    //cr2lf(temp);

    //We're not really interested in anything after the 15th field, so just hard coding this to 15.
    rec_array = g_strsplit_set ( buffer, "\x0D", 15 );
    
    //Actual response from Clever
    response = g_strdup( rec_array[0] );
    //Session ID (needed for next request)
    rec_session = g_strdup( rec_array[1] );
    //Prevref to use in next request
    rec_ref = g_strdup( rec_array[10] );
   
    //free the array again
    g_strfreev(rec_array);

    //printf("response: %s\n", response);     
    //printf("rec_session: %s\n", rec_session);     
    //printf("rec_ref: %s\n", rec_ref);     

    //push the previous messages down along the vTexts
    if (my_post->vText8)   free(my_post->vText8);
    if (my_post->vText7)   free(my_post->vText7);
    
    //The even ones are just what we've received from cleverbot
    if (my_post->vText6)   my_post->vText8 = my_post->vText6;
    if (my_post->vText4)   my_post->vText6 = my_post->vText4;
    if (my_post->vText2)   my_post->vText4 = my_post->vText2;
    
    my_post->vText2 = curl_easy_escape( my_post->curl, response, 0 );
    
    //The odd ones are what we've actually send
    if (my_post->vText5)   my_post->vText7 = my_post->vText5;
    if (my_post->vText3)   my_post->vText5 = my_post->vText3;
    if (my_post->stimulus) my_post->vText3 = g_strdup(my_post->stimulus);
    
    //Free the old session id and prev ref
    g_free(my_post->sessionid);
    g_free(my_post->prevref);

    my_post->sessionid = rec_session ;
    my_post->prevref = rec_ref;
    
    //freeing buffer causes a segmentation fault, assume curl will deal with it?
    g_free(my_post->stimulus);
    //g_free(temp);
    //Return the len of string for curl
    return size * nmemb;
}


//Get a new post_data structure for this buddy and initialise some stuff
void start_cleverbot_cb(PurplePluginAction *action, PurpleBuddy *buddy){

    post_data *curr_data;
    
    curr_data = (post_data *) malloc( sizeof (post_data) );

    if ( !curr_data ) {
        purple_debug_info(PLUGIN_ID, "Unable to allocate memory for %s\n", buddy->name);
        //ERROR - unable to allocate memory 
        return;
    }
   
    //Block sending until everything is initialised
    curr_data->sending = TRUE;

    init_post_data( curr_data );
    curr_data->name = g_strdup( buddy->name );
    
    list = g_list_append( list, curr_data ) ;
}


//Free the data pointer to in this post_data struct and remove the link
void stop_cleverbot_cb(PurplePluginAction *action, PurpleBuddy *buddy)
{

    post_data *curr_data;
    curr_data = find_post_data( buddy->name );

    curr_data->sending = TRUE;
    
    if (curr_data)
        destroy_post_data( curr_data );

}



//static void wrote_im(PurpleAccount *account, char *sender, char *message,
static void wrote_im(PurpleAccount *account, char *sender, char *message,
        PurpleConversation *conv, PurpleMessageFlags flags){
    
    post_data *post_ptr;
    char *response;
    PurpleConvIm *im;

    im = purple_conversation_get_im_data(conv);

    PurpleBuddy *b = purple_find_buddy( account, sender );

    //Check that the wrote message is one that was received
    if (!(flags & PURPLE_MESSAGE_RECV))
        return;
    
    //Make sure there is a message
    if (!message || !*message)
        return;

    //Make sure message isn't an autoresponse
    if (flags & PURPLE_MESSAGE_AUTO_RESP)
        return;
    
    //Make sure this is a IM conversation, not a chat
    if ( purple_conversation_get_type(conv) != PURPLE_CONV_TYPE_IM )
        return;
    
    //Search in our global GList, list for this buddy
    post_ptr = find_post_data( b->name );
    
    //If they are not in the list then return
    if ( !post_ptr) {
        return;
    }
    
    //Check and set this so we don't try to send two messages simultaneously
    if ( post_ptr->sending ) {
        return;
    }

    post_ptr->sending = TRUE;
    
    purple_debug_info(PLUGIN_ID, "Sending:\n%s: %s\n", sender, message);

    post_ptr->stimulus = curl_easy_escape( post_ptr->curl, message, 0 );
    
    //g_thread_create( (GThreadFunc)send_clever_message, post_ptr, FALSE, NULL );
    response = send_clever_message( post_ptr );

    if ( response ) {

        purple_debug_info(PLUGIN_ID, "Received: %s\n", response);
        
        //Send cleverbot's response to the buddie
        serv_send_im (account->gc, sender, response, PURPLE_MESSAGE_SEND);
        //And display it in the conversation
        purple_conv_im_write ( im, "Cleverbot", response, PURPLE_MESSAGE_AUTO_RESP & PURPLE_MESSAGE_SEND, time(NULL) );   

        curl_free( response );
    }
    
    post_ptr->sending = FALSE;
    
}


static void supply_extended_menu(PurpleBlistNode *node, GList **menu) {
 
    PurpleMenuAction *act; 
    PurpleBuddy *buddy; 
    PurpleAccount *acct; 

    //Check if the node we (right) clicked is a buddy, if not don't add the menu item
    if (!PURPLE_BLIST_NODE_IS_BUDDY(node)) return; 
    
    //Get the buddy!
    buddy = (PurpleBuddy *)node; 
    acct = buddy->account; 
    if (acct == NULL) return; 

    //Check if already enabled for cleverbot by searching for list in the list and give the appropriate option
    if ( check_if_enabled( buddy->name ) ) {
        act = purple_menu_action_new("Stop Cleverbot", (PurpleCallback)stop_cleverbot_cb, buddy, NULL); 
    } else {
        act = purple_menu_action_new("Start Cleverbot", (PurpleCallback)start_cleverbot_cb, buddy, NULL); 
    }

    *menu = g_list_append(*menu, act); 
}



static gboolean
plugin_load(PurplePlugin *plugin) {
    
    void *blist_handle = purple_blist_get_handle();
    void *conv = purple_conversations_get_handle(); 
    
    //Initialise our main GList to NULL (empty list)
    list = NULL;

    //Connect extended signal menu signal with callback to add option for enabling / disabling cleverchat
    purple_signal_connect(blist_handle, "blist-node-extended-menu", plugin, PURPLE_CALLBACK(supply_extended_menu), NULL);
    purple_signal_connect(conv, "wrote-im-msg", plugin, PURPLE_CALLBACK(wrote_im), NULL ); 

    return TRUE;
}



static gboolean
plugin_unload(PurplePlugin *plugin) {
    
    void *blist_handle = purple_blist_get_handle();
    void *conv = purple_conversations_get_handle(); 

    purple_signal_disconnect(blist_handle, "blist-node-extended-menu", plugin, PURPLE_CALLBACK(supply_extended_menu) );
    purple_signal_disconnect(conv, "wrote-im-msg", plugin, PURPLE_CALLBACK(wrote_im) ); 

    //Give the OS back it's precious memory
    g_list_free_full( list, ( GDestroyNotify ) free_post_data );

    return TRUE;

}


static PurplePluginInfo info = {
    PURPLE_PLUGIN_MAGIC,                                 //Purple Magic
    PURPLE_MAJOR_VERSION,                                //Major Version
    PURPLE_MINOR_VERSION,                                //Minor Version
    PURPLE_PLUGIN_STANDARD,                              //Type
    NULL,                                                //UI Requirement
    0,                                                   //FLAGS
    NULL,                                                //Dependencies
    PURPLE_PRIORITY_DEFAULT,                             //Priority

    "core-clever_chat",                                  //ID
    "CleverChat",                                       //Name
    "0.1",                                               //Version

    "CleverChat Plugin",                                //Summary

    "Allows you to send a contact's messages"
    "to Cleverbot and have Cleverbot respond.",          //Description

    "Jack Culhane <jackculhane@gmail.com>",              //Author
    "website.com",                                       //Website

    plugin_load,                                         //Load Function
    plugin_unload,                                       //Unload
    NULL,                                                //Destroy

    NULL,                                                //UI Struct
    NULL,
    NULL,
    NULL,                                                //Actions
    
    NULL,                                                //Reserved
    NULL,
    NULL,
    NULL,
};

static void
init_plugin(PurplePlugin *plugin)
{
}

PURPLE_INIT_PLUGIN(clever_chat, init_plugin, info)
