#ifndef CONFIG_SECRETS_H
#define CONFIG_SECRETS_H

struct ConfigSecrets
{
    const char *wifi_ssid = "<wifi_ssid>";
    const char *wifi_password = "<wifi_password>";

    // update at https://api.thingspeak.com/update?api_key=<api_key>&field1=<value>
    const char *config_url = "https://api.thingspeak.com/channels/<channel_id>/feeds.json?api_key=<api_key>&results=1";
};

extern ConfigSecrets config_secrets;
#endif
