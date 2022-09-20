#define _UTILS_IMPL
#include <utils/time.h>
#include <utils/str.h>
#include <utils/uri.h>

#include <schemehandler.h>
#include <OAuth.h>

#include <stdio.h>
#include <string.h>

void* handle_url_request(OAuth* oauth, char* value) {
    printf("%s\n", value);
    struct sc_uri* uri = sc_uri_create(value);
    if (strcmp(uri->host, "oauth")) return;
    const char* token = NULL;
    char* save = NULL;
    char* to_split = str_create(uri->query);
    while (token = str_token_begin(to_split, &save, "=")) {
        token = str_token_begin(to_split, &save, "&");
        char* code = str_create(token);
        oauth_auth(oauth, code);
        oauth_save(oauth);
        str_destroy(&code);
    } str_destroy(&to_split);
}

int main(int argc, char* argv[]) {
    OAuth* oauth = oauth_create();
    scheme_handler* handler = app_open(argc, argv, NULL, "TEST", handle_url_request, oauth);
    if (handler) {
        srand(time_ms());

        oauth_config_dir(oauth, "", "TEST");
        oauth_load(oauth);
        // oauth_start_refresh(oauth, 0);
        // oauth_save(oauth, "", "TEST");

        // char* url = oauth_auth_url(oauth);
        // scheme_open(url);

        oauth_start_request_thread(oauth);

        oauth_append_data(oauth, "fields", "id,title,main_picture,alternative_titles,start_date,end_date,synopsis,mean,rank,popularity,num_list_users,num_scoring_users,nsfw,created_at,updated_at,media_type,status,genres,my_list_status,num_episodes,start_season,broadcast,source,average_episode_duration,rating,pictures,background,related_anime,related_manga,recommendations,studios,statistics");

        response_data* response = oauth_request(oauth, GET, "https://api.myanimelist.net/v2/anime/30230");

        oauth_append_data(oauth, "fields", "id,title,main_picture,alternative_titles,start_date,end_date,synopsis,mean,rank,popularity,num_list_users,num_scoring_users,nsfw,created_at,updated_at,media_type,status,genres,my_list_status,num_episodes,start_season,broadcast,source,average_episode_duration,rating,pictures,background,related_anime,related_manga,recommendations,studios,statistics");

        response = oauth_request(oauth, GET, "https://api.myanimelist.net/v2/anime/30230");

        getchar();
        oauth_stop_request_thread(oauth);
        // oauth_stop_refresh(oauth);
        oauth_delete(oauth);
    } return 0;
}