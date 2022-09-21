#define _UTILS_IMPL
#include <utils/time.h>
#include <utils/str.h>
#include <utils/uri.h>

#include <schemehandler.h>
#include <OAuth.h>

#include <stdio.h>
#include <string.h>

void* handle_url_request(OAuth* oauth, const char* endpoint, const char* query) {

    if (!oauth) return;

    const char* save = NULL;
    const char* token;
    query = str_create(query);
    endpoint = str_create(endpoint);
    while (token = str_token_begin(query, &save, "=")) {
        token = str_token_begin(query, &save, "&");
        char* code = str_create(token);
        oauth_auth(oauth, code);
        str_destroy(&code);
    } str_destroy(&query);
}

int main(int argc, char* argv[]) {
    OAuth* oauth = oauth_create("./TEST.ini");
    scheme_handler* handler = app_open(argc, argv, NULL, "TEST", handle_url_request, oauth);
    if (handler) {
        srand(time_ms());

        oauth_append_data(oauth, "fields", "id,title,main_picture,alternative_titles,start_date,end_date,synopsis,mean,rank,popularity,num_list_users,num_scoring_users,nsfw,created_at,updated_at,media_type,status,genres,my_list_status,num_episodes,start_season,broadcast,source,average_episode_duration,rating,pictures,background,related_anime,related_manga,recommendations,studios,statistics");

        oauth_set_options(oauth, REQUEST_ASYNC | REQUEST_CACHE | REQUEST_AUTH);

        response_data* response = oauth_request(oauth, GET, "https://api.myanimelist.net/v2/anime/30230");

        getchar();
        oauth_delete(oauth);
    } return 0;
}